#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   //fork
#include <sys/wait.h> //waitpid
#include <errno.h>
#include <time.h>
#include <string.h>

void handler(int signumber)
{
    printf("\nThe child is ready for work, pid: %i .\n", signumber);
};

void handler2(int signumber)
{
    printf("\nChecking child is sending the results to Sealing child.\n");
};

struct uzenet
{
    char szavaz[20];
    int random;
};

struct valid
{
    char val[20];
    int v_db;
    int nv_db
};

int main(int argc, char *argv[])
{
    int count;

    if (argc < 1)
    {
        printf("enter 2 arguments.\"filename arg1!\"");
        return 0;
    }

    count = atoi(argv[1]);

    pid_t check_child, seal_child;

    struct uzenet uz;
    strcpy(uz.szavaz, "Id of voter: ");

    struct valid valid_get;

    struct sigaction sigact;
    sigact.sa_handler = handler;
    sigemptyset(&sigact.sa_mask);
    sigaction(SIGUSR1, &sigact, NULL);
    sigaction(SIGUSR2, &sigact, NULL);

    sigset_t sigset;
    sigfillset(&sigset);
    sigdelset(&sigset, SIGUSR1);
    sigdelset(&sigset, SIGUSR2);

    struct sigaction sigact2;
    sigact2.sa_handler = handler2;
    sigemptyset(&sigact2.sa_mask);
    sigaction(SIGRTMIN, &sigact2, NULL);

    sigset_t sigset2;
    sigfillset(&sigset2);
    sigdelset(&sigset2, SIGRTMIN);

    int pipefd[2];
    int pipefd2[2];
    int pipefd3[3];

    char c[30];

    if (pipe(pipefd) == -1 || pipe(pipefd2) == -1 || pipe(pipefd3) == -1)
    {
        perror("Error while opening pipe!");
        exit(EXIT_FAILURE);
    }

    printf("\nELECTION\n");

    srand(time(NULL));

    seal_child = fork();

    if (seal_child > 0)
    {
        if (seal_child < 0)
        {
            perror("\nThe fork calling was not succesful\n");
            exit(1);
        }
        check_child = fork();

        if (check_child > 0)
        {
            //in parent process

            printf("\nWaiting for child 1.\n");
            //pause();
            sigsuspend(&sigset);
            printf("\nChecking child's signal arrived\n", SIGUSR1);

            printf("\nWaiting for child 2.\n");
            //pause();
            sigsuspend(&sigset);
            printf("\nSealing child's signal arrived\n", SIGUSR2);

            write(pipefd[1], &count, sizeof(int));

            for (int i = 0; i < count; i++)
            {
                int r = rand() % 150 + 1;
                struct uzenet u;
                strcpy(u.szavaz, "Id of voter: ");
                u.random = r;

                write(pipefd[1], &u, sizeof(u));
            }

            sleep(5);

            close(pipefd3[1]);
            sleep(2);

            read(pipefd3[0], &valid_get, sizeof(valid_get));

            FILE *file = fopen("file.txt", "a");
            if (file == NULL)
            {
                fprintf(stderr, "\nHiba a file megnyitasa kozben!\n");
                exit(1);
            }
            fwrite(&valid_get, sizeof(struct valid), 1, file);

            printf("\nThe validator's message: %s \n valid votes: %d\n not valid votes: %d\n", valid_get.val,
                   valid_get.v_db, valid_get.nv_db);

            sleep(2);
            close(pipefd[0]);
        }
        else
        {
            //in check_child process
            printf("\nIn checkin child process.\n");
            sleep(2);
            kill(getppid(), SIGUSR1);
            //close(pipefd[1]);

            sleep(8);

            printf("The checking child got the following messages :\n");

            read(pipefd[0], &count, sizeof(int));
            for (int j = 0; j < count; j++)
            {
                read(pipefd[0], &uz, sizeof(uz));
                printf("\nMessage: %s %i\n", uz.szavaz, uz.random);
            }
            sleep(2);
            write(pipefd2[1], &count, sizeof(int));

            close(pipefd[0]);

            for (int i = 0; i < count; i++)
            {
                int d = rand() % 100;
                if (d < 20)
                {
                    strcpy(&c, "no");
                    write(pipefd2[1], &c, 30);
                }
                else
                {
                    strcpy(&c, "yes");
                    write(pipefd2[1], &c, 30);
                }
            }
            sleep(2);
            close(pipefd2[1]);

            kill(seal_child, SIGRTMIN);
            printf("\nChecking child process ended\n");

            exit(1);
        }
    }
    else
    {
        //in seal_child process
        printf("\nIn sealing child process.\n");
        sleep(4);
        kill(getppid(), SIGUSR2);
        sleep(10);

        int valid = 0;
        int notvalid = 0;

        close(pipefd2[1]);

        sigsuspend(&sigset2);
        printf("\nSeal child got a signal from Checking child\n", SIGRTMIN);

        read(pipefd2[0], &count, sizeof(int));
        for (int i = 0; i < count; i++)
        {
            read(pipefd2[0], &c, 30);
            if (strcmp(c, "no") == 0)
            {
                ++notvalid;
            }
            else if (strcmp(c, "yes") == 0)
            {
                ++valid;
            }
        }
        sleep(2);
        close(pipefd2[0]);

        sleep(5);

        close(pipefd3[0]);

        struct valid v;
        strcpy(v.val, "Valid and not valid votes: ");
        v.v_db = valid;
        v.nv_db = notvalid;

        write(pipefd3[1], &v, sizeof(v));

        sleep(2);
        close(pipefd3[1]);

        printf("\nSealing child process ended\n");

        exit(1);
    }

    return 0;
}