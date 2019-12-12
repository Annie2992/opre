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
        //az elsõnek longnak kell lennie, és ezt a longot használja kategóriának
        long mtype;//ez egy szabadon hasznalhato ertek, pl uzenetek osztalyozasara
        char mtext [ 1024 ];
    };

int main(int argc,char* argv[]){

    if (argc < 2)
    {
        printf(" Tovabbi argumentumra van szukseg a folytatashoz.\n"); // valójában azt nézi, hogy pontosan egy parancssori argumetuma legyen
        exit(0);                                                                  // Lelovi a programot
    }

    int pipefdc1p[2];
    if (pipe(pipefdc1p) == -1) {
        perror("Hiba a pipe nyitaskor!");
        exit(EXIT_FAILURE);
    }

    int uzenetsor;
    key_t kulcs;
    kulcs = ftok(argv[0],50);
    printf ("A kulcs: %d\n",kulcs);

    uzenetsor = msgget( kulcs, 0600 | IPC_CREAT );
    if ( uzenetsor < 0 ) {
        perror("msgget");
        exit(1);
    }

    pid_t child1, child2;

    child1 = fork();
    if(child1 == 0){ //child1 utas

        close(pipefdc1p[0]);
		char cim[256] ="";
		strcpy(cim, argv[1]); //egy parameterrel ok lenne
		int i;
        for (i = 2; i < argc ; ++i)
        {
                strcat(cim, " ");
                strcat(cim, argv[i]);
        }
        //strcpy(cim, argv[1]); //egy parameterrel ok lenne

		printf("(Utas) Lakcim kuldese a kozpontnak.\n");
		write(pipefdc1p[1], &cim, sizeof(cim));




    }else
    {
        child2 = fork();
        if(child2 == 0){  // child2 kiszolgalo auto ~ taxis
            struct uzenet uzf;
            if(msgrcv(uzenetsor, &uzf, 1024, 1, 0) < 0){
                perror("msgrcv");
            }
            printf("\n(Taxis) Kapott cim es telszam: %s!\n", uzf.mtext);

            struct uzenet uzk1;
            uzk1.mtype = 2;
            strcpy(uzk1.mtext, "Elindultam az utasert!");
            if (msgsnd(uzenetsor, &uzk1, 1024, 0) < 0)
				perror("msgsnd");





        }else{ //parent kozpont
            close(pipefdc1p[1]);
            char cim_fog[256];
            read(pipefdc1p[0], &cim_fog, sizeof(cim_fog));
            printf("(Kozpont) Utas lakcime: %s\n", cim_fog);

            strcat(cim_fog, "-");
            char tmp[30];
            sprintf(tmp, "%d", child1);
            strcat(cim_fog, tmp);

            struct uzenet uzk;
            uzk.mtype = 1;
            strcpy(uzk.mtext, cim_fog);
            if (msgsnd( uzenetsor, &uzk, 1024, 0 ) < 0){
                perror("msgsnd");
            }

            struct uzenet uzf1;
            uzf1.mtype = 2;
            if(msgrcv(uzenetsor, &uzf1, 1024, 2, 0) < 0){
                perror("msgrcv");
            }
            printf("\n(Kozpont) Fogadott uzenet: %s\n", uzf1.mtext);

            wait(NULL);

        }
    }

    return 0;
}
