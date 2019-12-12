#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   //fork
#include <sys/wait.h> //waitpid
#include <errno.h>
#include <time.h>
#include <string.h>

void handler(int signumber)
{
    printf("\nPlayer is ready, pid number: %i .\n", signumber);
}

int main(int argc, char *argv[])
{
    
    srand(time(NULL));
    pid_t player1, player2;

    struct sigaction sigact;
    sigact.sa_handler = handler;
    sigemptyset(&sigact.sa_mask);
    sigaction(SIGUSR1, &sigact, NULL);
    sigaction(SIGUSR2, &sigact, NULL);

    sigset_t sigset;
    sigfillset(&sigset);
    sigdelset(&sigset, SIGUSR1);
    sigdelset(&sigset, SIGUSR2);

    const char *names[100] = {"Kevin", "Oksana", "Bubu", "Kombajn", "Ibit"};

    int pipefd[2];
    int pipefd2[2];

    if (pipe(pipefd) == -1 || pipe(pipefd2) == -1)
    {
        perror("Error while opening pipe!");
        exit(EXIT_FAILURE);
    }
    int random_szamok[2];
    for (int i = 0; i < 2; i++){
        random_szamok[i] = rand() % 5;
    }

    player2 = fork();
    if (player2 > 0)
    {
        if (player2 < 0)
        {
            perror("\nThe fork calling was not succesful\n");
            exit(1);
        }
        player1 = fork();

        if (player1 > 0)
        {
            //in parent process
            close(pipefd[1]);
            close(pipefd2[1]);

            printf("\nWaiting for player one.\n");
            sigsuspend(&sigset);
            printf("\nPlayer one arrived\n", SIGUSR1);

            printf("\nWaiting for player two.\n");
            sigsuspend(&sigset);
            printf("\nPlayer two arrived\n", SIGUSR2);

            char name1[100];
            char name2[100];

            sleep(2);

            read(pipefd[0], name1, sizeof(name1));
            printf("\nName of player1: %s\n", name1);

            close(pipefd[0]);
            sleep(2);


            read(pipefd2[0], name2, sizeof(name2));
            printf("\nName of player2: %s\n", name2);

            sleep(5);
            close(pipefd2[0]);

            printf("\nEnd of parent\n");
        }
        else
        {
            //in player1 process
            sleep(2);
            kill(getppid(), SIGUSR1);
            sleep(2);

            close(pipefd[0]);
            //int r = rand() % 5;
            int r = random_szamok[0];
            
            write(pipefd[1], names[r], sizeof(names));

            sleep(5);
            close(pipefd[1]);

            printf("\nEnd of child1\n");
            exit(1);
        }
    }
    else
    {
        //in player2 process
        sleep(4);
        kill(getppid(), SIGUSR2);

        sleep(5);

        close(pipefd2[0]);
        //int rttt = rand() % 1229;
        //int r2 = rttt % 5;
        int r2 = random_szamok[1];
        

        write(pipefd2[1], names[r2], sizeof(names));

        sleep(2);
        close(pipefd2[1]);

        printf("\nEnd of child2\n");
        exit(1);
    }

    return 0;
}
