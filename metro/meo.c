#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <string.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sem.h>

#define STR_SIZE 256

int elromlott = 0;

typedef struct {
	long mtype;
	char mtext[STR_SIZE];
} msg_t;

void handler(int signo, siginfo_t *info, void *ptr)
{
	elromlott = !info->si_value.sival_int;
	printf("(P) A szerelveny jelzese: %d\n", info->si_value.sival_int);
}

int randreason(char *reason)
{
	int rnd = rand() % 10;
	if (rnd == 0)
		strcpy(reason, "elcsuszott egy bananhejon");
	else if (rnd == 1)
		strcpy(reason, "lefagyott a zord telben");
	else
		strcpy(reason, "sikeresen teljesitette az uzemet");

	return rnd;
}

void up(sem_id)
{
	struct sembuf op;
	op.sem_num = 0;
	op.sem_op = 1;
	op.sem_flg = 0;

	if (semop(sem_id, &op, 1) < 0)
		perror("semop");
}

void down(sem_id)
{
	struct sembuf op;
	op.sem_num = 0;
	op.sem_op = -1;
	op.sem_flg = 0;

	if (semop(sem_id, &op, 1) < 0)
		perror("semop");
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Helytelen parameterezes!\n");
		exit(EXIT_FAILURE);
	}

	srand(time(NULL));
	int jaratok = atoi(argv[1]);

	key_t key = ftok(argv[0], 1);
	int mq = msgget(key, 0600 | IPC_CREAT);
	if (mq < 0)
	{
		perror("msgget");
		exit(EXIT_FAILURE);
	}

	int shm_id = shmget(key, jaratok * sizeof(int), IPC_CREAT | S_IRUSR | S_IWUSR);
	if (shm_id < 0)
	{
		perror("shmget");
		exit(EXIT_FAILURE);
	}
	int *shm = (int *)shmat(shm_id, NULL, 0);
	int *shm_orig = shm;

	int sem_id = semget(key, 1, IPC_CREAT | S_IRUSR | S_IWUSR);
	if (sem_id < 0)
	{
		perror("semget");
		exit(EXIT_FAILURE);
	}

	semctl(sem_id, 0, SETVAL, 1);

	int i, count = 0;
	for (i = 0; i < jaratok; i++)
	{
		pid_t child = fork();
		if (child == -1)
		{
			perror("fork");
			exit(EXIT_FAILURE);
		}

		if (child > 0)	// parent
		{
			msg_t msg;
			msg.mtype = 1;
			sprintf(msg.mtext, "%d", i + 1);

			if (msgsnd(mq, &msg, STR_SIZE, 0) < 0)
				perror("msgsnd #1");

			sprintf(msg.mtext, "%d", jaratok);
			msg.mtype = 2;

			if (msgsnd(mq, &msg, STR_SIZE, 0) < 0)
				perror("msgsnd #2");

			struct sigaction sigact;
			sigemptyset(&sigact.sa_mask);
			sigact.sa_flags = SA_SIGINFO;
			sigact.sa_sigaction = handler;
			sigaction(SIGUSR1, &sigact, NULL);

			sigset_t sigset;
			sigfillset(&sigset);
			sigdelset(&sigset, SIGUSR1);
			sigsuspend(&sigset);

			if (msgrcv(mq, &msg, STR_SIZE, 3, 0) < 0)
				perror("msgrcv #3");

			printf("(P) A(z) %d. szerelveny %s\n", i + 1, msg.mtext);

			wait(NULL);

			int utazasi_ido = -1;

			if (elromlott)
			{
				count++;

				pid_t pot = fork();
				if (pot == -1)
				{
					perror("fork");
					exit(EXIT_FAILURE);
				}

				if (pot > 0) // parent
				{
					msg.mtype = 4;
					strcpy(msg.mtext, "ez egy potszerelveny");
					if (msgsnd(mq, &msg, STR_SIZE, 0) < 0)
						perror("msgsnd #4");

					msg.mtype = 5;
					sprintf(msg.mtext, "%d", i + 1);
					if (msgsnd(mq, &msg, STR_SIZE, 0) < 0)
						perror("msgsnd #5");

					wait(NULL);
				}
				else // child
				{
					char txt[STR_SIZE];
					int jarat;

					msg_t msgrec;
					if (msgrcv(mq, &msgrec, STR_SIZE, 4, 0) < 0)
						perror("msgrcv #4");

					strcpy(txt, msgrec.mtext);

					if (msgrcv(mq, &msgrec, STR_SIZE, 5, 0) < 0)
						perror("msgrcv #5");

					jarat = atoi(msgrec.mtext);

					printf("(C) Kozpont mondja: %s, potolt jarat: %d\n", txt, jarat);

					char reason[STR_SIZE];
					randreason(reason);

					printf("(C) A szerelveny %s\n", reason);

					exit(0);
				}

				elromlott = 0;
			}
			else
				utazasi_ido = rand() % 21 + 10;

			down(sem_id);
			*shm = utazasi_ido;
			shm++;
			up(sem_id);
		}
		else // child
		{
			struct timeval time;
			gettimeofday(&time, NULL);
			srand((time.tv_sec * 1000) + (time.tv_usec / 1000));

			msg_t msgrec;
			int szerelveny_szam, szerelveny_max;

			if (msgrcv(mq, &msgrec, STR_SIZE, 1, 0) < 0)
				perror("msgrcv #1");

			szerelveny_szam = atoi(msgrec.mtext);

			if (msgrcv(mq, &msgrec, STR_SIZE, 2, 0) < 0)
				perror("msgrcv #2");

			szerelveny_max = atoi(msgrec.mtext);

			char reason[STR_SIZE];
			int rnd = randreason(reason);

			printf("(C) %d. szerelveny, van meg %d db, a szerelveny %s.\n", szerelveny_szam, szerelveny_max - szerelveny_szam, reason);

			union sigval value;
			value.sival_int = rnd > 1;
			sigqueue(getppid(), SIGUSR1, value);

			msgrec.mtype = 3;
			strcpy(msgrec.mtext, reason);
			if (msgsnd(mq, &msgrec, STR_SIZE, 0) < 0)
				perror("msgsnd #3");

			exit(0);
		}
	}

	shm = shm_orig;
	int j;
	float sum = 0;
	for (j = 0; j < jaratok; j++)
	{
		sum += *shm;
		shm++;
	}

	printf("A nap folyaman %d mentesito jaratot kellett inditani, az atlag utido %.2f perc volt.\n", count, sum / (float)jaratok);

	if (msgctl(mq, IPC_RMID, NULL) < 0)
		perror("msgctl");

	if (shmctl(shm_id, IPC_RMID, NULL) < 0)
		perror("shmctl");

	if (semctl(sem_id, 0, IPC_RMID) < 0)
		perror("semctl");

	return 0;
}
