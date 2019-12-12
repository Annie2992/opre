#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>  //fork
#include <sys/wait.h> // waitpid
#include <signal.h> // signal
#include <sys/types.h> // signal
#include <sys/ipc.h> //uzenet
#include <sys/msg.h> //uzenet
#include <time.h> //time(randhoz)

struct uzenet {
        //az elsőnek longnak kell lennie, és ezt a longot használja kategóriának
        long mtype;//ez egy szabadon hasznalhato ertek, pl uzenetek osztalyozasara
        char mtext [ 1024 ];
    };

void handler(int signumber)
{
	printf("(Kevin) Betoro probalkozik!\n");
}

int main(int argc,char* argv[]){

    int uzenetsor;
    key_t kulcs;
    kulcs = ftok(argv[0],50);
    printf ("A kulcs: %d\n",kulcs);

    signal(SIGTERM,handler);

    uzenetsor = msgget( kulcs, 0600 | IPC_CREAT );
    if ( uzenetsor < 0 ) {
        perror("msgget");
        exit(1);
    }

    int pipefdc1c2[2]; // unnamed pipe file descriptor array
    if (pipe(pipefdc1c2) == -1) {
        perror("Hiba a pipe nyitaskor!");
            exit(EXIT_FAILURE);
    }

    int pipefdc2c1[2]; // unnamed pipe file descriptor array
    if (pipe(pipefdc2c1) == -1) {
        perror("Hiba a pipe nyitaskor!");
            exit(EXIT_FAILURE);
    }

    pid_t child1, child2;

    child1 = fork();
    if(child1 == 0){ //child1 Kevin
        srand(time(NULL));
        int rnd = rand() % 31 + 20; //0 és 30 között
        printf("(Kevin) %d apro jatekot szetszortam!\n", rnd);
        struct uzenet uzk;
        uzk.mtype = 1;
        sprintf(uzk.mtext, "%d", rnd);
        if (msgsnd( uzenetsor, &uzk, 1024, 0 ) < 0){
            perror("msgsnd");
        }

        struct uzenet uzf1;
        uzf1.mtype = 2;
        if(msgrcv(uzenetsor, &uzf1, 1024, 2, 0) < 0){
                perror("msgrcv");
            }
        printf("\n(Kevin) Fogadott uzenet: %s\n", uzf1.mtext);

        pause();

        close(pipefdc1c2[0]); // close unused read end
        write(pipefdc1c2[1], "festek", 7);
        close(pipefdc1c2[1]); // closing write descriptor

        char fogadott_valasz[50];
        close(pipefdc2c1[1]);  // close unused write end
		read(pipefdc2c1[0], fogadott_valasz, sizeof(fogadott_valasz));
		printf("(Kevin) Betoro: %s\n", fogadott_valasz);
		close(pipefdc2c1[0]);

    }else // child2, betoro
    {
        child2 = fork();
        if(child2 == 0){ //child2
        struct uzenet uzf;
            if(msgrcv(uzenetsor, &uzf, 1024, 1, 0) < 0){
                perror("msgrcv");
            }
        printf("\n(Betoro) Hanyatt esek %s jatekon!\n", uzf.mtext);

        struct uzenet uzk1;
        uzk1.mtype = 2;
        strcpy(uzk1.mtext, "csak varj, amig a kezunk koze nem kerulsz");
        if (msgsnd(uzenetsor, &uzk1, 1024, 0) < 0)
				perror("msgsnd");


        sleep(1);
        printf("(Betoro) Ujra probalkozom!\n");
        kill(child1, SIGTERM);

        char tmp[7];
        sleep(1);
        close(pipefdc1c2[1]);  // close unused write end
        read(pipefdc1c2[0], &tmp ,7);
        close(pipefdc1c2[0]); // close the used read end

        close(pipefdc2c1[0]); // close unused read end
        srand(time(NULL));
        char valasz[50];
        int r = rand() % 2;
        if(r == 0){
            strcpy(valasz, "na megallj csak");
        }else{
            strcpy(valasz, "nem uszod meg szarazon");
        }
        close(pipefdc2c1[0]); // close unused read end
        write(pipefdc2c1[1], &valasz, sizeof(valasz));
        close(pipefdc2c1[1]); // closing write descriptor

        }else{ //Parent

        wait(NULL);


        }
    }

    return 0;
}
