#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void handler(int signal) { printf("%d\n", signal); }

int main() {
  time_t t;
  srand(time(&t));

  struct sigaction sigusr_handler;
  sigusr_handler.sa_handler = handler;
  sigemptyset(&sigusr_handler.sa_mask);
  sigusr_handler.sa_flags = 0;
  sigaction(SIGUSR1, &sigusr_handler, NULL);

  sigaction(SIGUSR2, &sigusr_handler, NULL);

  int pipefd1[2];
  int pipefd2[2];
  int pipefd3[2];

  if (pipe(pipefd1) == -1) {
    perror("Pipe1 open error!");
    return 1;
  }
  if (pipe(pipefd1) == -1) {
    perror("Pipe2 open error!");
    return 1;
  }
  if (pipe(pipefd3) == -1) {
    perror("Pipe3 open error!");
    return 1;
  }

  pid_t child1, child2;
  pid_t parent = getpid();
  child1 = fork();
  int status;
  int sweets = rand() % 2;
  int one = 1;
  int two = 2;
  int three = 3;
  int tmp = rand() % 3;
  int fat = 0;
  if (tmp == 2) {
    fat = 1;
  } else if (tmp == 1) {
    fat = -1;
  }
  char fats[100];

  if (child1 == 0) { // child1

    sleep(1);
    // printf("Juliska\n");
    kill(getppid(), SIGUSR1);
    read(pipefd1[0], &sweets, sizeof(sweets));
    if (sweets == 1) {
      printf("Kalacs!\n");
    } else {
      printf("Sutemeny!\n");
    }
    sleep(3);
    kill(getppid(), SIGUSR1);
    read(pipefd1[0], &one, sizeof(one));
    sleep(1);
    kill(getppid(), SIGUSR1);
    read(pipefd1[0], &two, sizeof(two));
    sleep(1);
    kill(getppid(), SIGUSR1);
    read(pipefd1[0], &three, sizeof(three));

  } else {
    child2 = fork();
    if (child2 == 0) { // child2
      sleep(2);
      // printf("Jancsi\n");
      kill(getppid(), SIGUSR2);
      read(pipefd2[0], &sweets, sizeof(sweets));
      if (sweets == 1) {
        printf("Kalacs2!\n");
      } else {
        printf("Sutemeny2!\n");
      }
      // read(pipefd2[0], &three, sizeof(three));
      sleep(1);
      kill(getppid(), SIGUSR2);
      write(pipefd3[1], &fat, sizeof(fat));

    } else { // parent
      pause();
      printf("Juliska:KOPPKOPP!\n", SIGUSR1);
      pause();
      printf("Jancsi:KOPPKOPP!\n", SIGUSR2);

      write(pipefd1[1], &sweets, sizeof(sweets));
      write(pipefd2[1], &sweets, sizeof(sweets));

      pause();
      printf("Juliska dolgozz (1)!\n", SIGUSR1);
      write(pipefd1[1], &one, sizeof(one));
      pause();
      printf("Juliska dolgozz (2)!\n", SIGUSR1);
      write(pipefd1[1], &two, sizeof(two));
      pause();
      printf("Juliska dolgozz (3)!\n", SIGUSR1);
      write(pipefd1[1], &three, sizeof(three));
      pause();
      printf("Jancsi dugd ki az ujjad!", SIGUSR2);
      // write(pipefd2[1], &three, sizeof(three));

      read(pipefd3[0], &fat, sizeof(fat));
      printf("\n");
      if (fat == 1) {
        printf("Kover!\n");
      } else if (fat == 0) {
        printf("Elmegy!\n");
      } else {
        printf("Sovany!\n");
      }
      waitpid(child1, NULL, 0);
      waitpid(child2, NULL, 0);
    }
  }
}