#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <wait.h>

#define MAX_STRING_LENGTH 50

struct customer
{
    char name[MAX_STRING_LENGTH];
    char position[MAX_STRING_LENGTH];
    char phonenumber[15];
    char transmod[MAX_STRING_LENGTH];
};

struct message_core
{
    int count;
    char from[80];
};

struct message
{
    long mtype;
    struct message_core msg;
};

void draw_menu();
int menu();
void readName(struct customer *current);
void readPlace(struct customer *current);
void readPhone(struct customer *current);
void readTransport(struct customer *current);
void saveToFile(struct customer *current);
void checkValues(struct customer *current);
void getNewData(struct customer *current);
void draw_list_menu();
void getList();
int getFileSize();
void modData(struct customer *current);
void deleteData(struct customer *current);
int count_peolpe(char count_at_pos[MAX_STRING_LENGTH]);

//minimum required people for expeditions
int min_pos[] = {4, 3, 2, 3, 2};
//handler for signals
void handler(int signumber){}

int main(int argc, char **argv)
{

    int uzenetsor, status;
    key_t kulcs;

    kulcs = ftok(argv[0], 1);
    uzenetsor = msgget(kulcs, 0600 | IPC_CREAT);

    int pipefd[2];

    struct sigaction sigact;
    sigact.sa_handler = handler;
    sigemptyset(&sigact.sa_mask);

    sigact.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sigact, NULL);

    sigset_t sigset;
    sigfillset(&sigset);
    sigdelset(&sigset, SIGUSR1);

    int choice;

    do
    {
        choice = menu();

        printf("\n Ez utan indulnak a mentoexpediciok, amennyiben van elegendo ember \n \n");

        for (int i = 0; i < 5; ++i)
        {
            char p[MAX_STRING_LENGTH]; //position
            switch (i)
            {
            case 0:
                strcpy(p, "Bali");
                break;
            case 1:
                strcpy(p, "Mali");
                break;
            case 2:
                strcpy(p, "Cook_szigetek");
                break;
            case 3:
                strcpy(p, "Bahamak");
                break;
            case 4:
                strcpy(p, "Izland");
                break;
            }
            int count_p = count_peolpe(p);
            if (min_pos[i] <= count_p)
            {
                if (pipe(pipefd) == -1)
                {
                    perror("Hiba a pipe nyitaskor!");
                    exit(EXIT_FAILURE);
                }
                pid_t child = fork();
                if (child < 0)
                {
                    perror("Fork hivasa kozben hiba keletkezett.\n");
                    exit(1);
                }
                if (child == 0)
                {
                    close(pipefd[1]);

                    //expedition gets there
                    sleep(1);
                    kill(getppid(), SIGUSR1);

                    int db; //number of people at place
                    struct customer cust;

                    read(pipefd[0], &db, sizeof(int));
                    for (int j = 0; j < db; j++)
                    {
                        read(pipefd[0], &cust, sizeof(cust));
                    }
                    close(pipefd[0]);

                    //expedition gets back
                    sleep(1);

                    kill(getppid(), SIGUSR1);
                    sleep(1);

                    struct message m;
                    m.mtype = 1;
                    m.msg.count = db;
                    strcpy(m.msg.from, cust.position);
                    int idx = 0;

                    status = msgsnd(uzenetsor, &m, sizeof(m.msg), 0);
                    if (status < 0)
                    {
                        perror("msgsnd");
                    }
                    sleep(1);

                    exit(1);
                }
                else
                {
                    printf("A mentes elindult a kovetkezo helyre: %s", p);
                    sigsuspend(&sigset);
                    close(pipefd[0]);
                    write(pipefd[1], &count_p, sizeof(int));

                    struct customer cust;

                    int size = getFileSize();
                    struct customer cust_new[size];
                    int idx = 0;

                    FILE *infile = fopen("file.txt", "r");
                    if (infile == NULL)
                    {
                        fprintf(stderr, "\nHiba a file megnyitasa kozben\n");
                        exit(1);
                    }

                    while (fread(&cust, sizeof(struct customer), 1, infile))
                    {
                        if (strcmp(cust.position, p) == 0)
                        {
                            write(pipefd[1], &cust, sizeof(cust));
                        }
                        else
                        {
                            strcpy(cust_new[idx].name, cust.name);
                            strcpy(cust_new[idx].position, cust.position);
                            strcpy(cust_new[idx].phonenumber, cust.phonenumber);
                            strcpy(cust_new[idx].transmod, cust.transmod);

                            idx++;
                        }
                    }
                    size = idx;
                    fclose(infile);

                    FILE *newfile = fopen("file.txt", "w+");
                    fseek(newfile, 0, SEEK_SET);
                    idx = 0;
                    if (newfile != NULL)
                    {
                        for (idx = 0; idx < size; ++idx)
                        {
                            fwrite(&cust_new[idx], sizeof(struct customer), 1, newfile);
                        }
                    }
                    fclose(newfile);

                    close(pipefd[1]); // Closing write descriptor
                    sigsuspend(&sigset);

                    struct message m;
                    status = msgrcv(uzenetsor, &m, sizeof(m.msg), 1, 0);
                    if (status < 0)
                    {
                        perror("msgsnd");
                    }
                    else
                    {
                        printf("\n%s helyrol %i db utas hazatert\n", m.msg.from, m.msg.count);
                    }
                    sleep(1);
                }
            }
        }
    } while (choice != 5);

    wait(NULL);

    status = msgctl(uzenetsor, IPC_RMID, NULL);
    if (status < 0)
    {
        perror("msgctl");
    }
    return 0;
}
//write out menu for user
void draw_menu()
{
    printf("\nMenu\n");
    printf("1. Adatok feltoltese\n");
    printf("2. Adatok modositasa\n");
    printf("3. Adatok listazasa\n");
    printf("4. Adatok torlese\n");
    printf("5. Kilepes\n");
}

//menu options
int menu()
{
    draw_menu();
    struct customer current_customer;
    int choice = 0;

    scanf("%i", &choice);
    switch (choice)
    {
    case 1:
        getNewData(&current_customer);
        break;
    case 2:
        modData(&current_customer);
        break;
    case 3:
        getList(&current_customer);
        break;
    case 4:
        deleteData(&current_customer);
        break;
    case 5:
        printf("Kilep a programbol.\n");
        break;
    default:
        printf("Hibas opcio. Kerem 1-5-ig irjon be egy szamot!\n");
        break;
    }
    return choice;
}

//read customer's name
void readName(struct customer *current)
{
    printf("\nUgyfel neve: \n");
    scanf("%s", &current->name);
}
//read current position
void readPosition(struct customer *current)
{
    printf("\nA helyszin: \n");
    printf("1. Bali\n");
    printf("2. Mali\n");
    printf("3. Cook_szigetek\n");
    printf("4. Bahamak\n");
    printf("5. Izland\n");

    int choice = 0;

    scanf("%i", &choice);
    switch (choice)
    {
    case 1:
        strcpy(current->position, "Bali");
        break;
    case 2:
        strcpy(current->position, "Mali");
        break;
    case 3:
        strcpy(current->position, "Cook_szigetek");
        break;
    case 4:
        strcpy(current->position, "Bahamak");
        break;
    case 5:
        strcpy(current->position, "Izland");
        break;
    }
};

//read customer's phone number
void readPhone(struct customer *current)
{
    printf("Az ugyfel telefonszama: ");
    scanf("%s[^\n]", &current->phonenumber);
}

//read transport mode
void readTransport(struct customer *current)
{
    printf("\nAz utazas modja: \n1. repulo\n2. hajo\n3. autobusz\n");

    int choice = 0;

    scanf("%i", &choice);

    switch (choice)
    {
    case 1:
        strcpy(current->transmod, "repulo");
        break;
    case 2:
        strcpy(current->transmod, "hajo");
        break;
    case 3:
        strcpy(current->transmod, "autobusz");
        break;
    }
};

//save data into file
void saveToFile(struct customer *current)
{
    FILE *file = fopen("file.txt", "a");
    if (file == NULL)
    {
        fprintf(stderr, "\nHiba a file megnyitasa kozben!\n");
        exit(1);
    }
    fwrite(current, sizeof(struct customer), 1, file);

    if (fwrite != 0)
    {
        printf("Az adatok sikeresen mentve!\n");
    }
    else
    {
        printf("\nHiba iras kozben!\n");
    }

    fclose(file);
};

//check before save
void checkValues(struct customer *current)
{
    printf("\n *** \n");
    printf("Nev: %s \n", current->name);
    printf("Hely: %s \n", current->position);
    printf("tel: %s \n", current->phonenumber);
    printf("utazasi mod: %s \n", current->transmod);
    printf(" *** \n \n");
};

//add new data
void getNewData(struct customer *current)
{
    readName(current);
    readPosition(current);
    readPhone(current);
    readTransport(current);

    checkValues(current);
    printf("Jok az adatok? (i/n)");

    char answer;
    char yes = 'i';
    scanf(" %c", &answer);

    if (answer == yes)
    {
        printf("Mentes folyamatban...\n");
        saveToFile(current); //save to file
        printf("Mentve\n \n");
    }
    else
    {
        printf("Add meg ujra az adatokat! \n");
        getNewData(current);
    }
};

//get the size of the database
int getFileSize()
{
    int size = 0;
    struct customer *obj = malloc(sizeof(struct customer));
    FILE *file = fopen("file.txt", "rb");
    fseek(file, 0, SEEK_SET);
    while (fread(obj, sizeof(struct customer), 1, file))
    {
        ++size;
    }
    fclose(file);
    return size;
};

//modify data in file
void modData(struct customer *current)
{
    printf("Modositando ugyfel telefonszama: ");
    char phone[15];
    scanf("%s", phone);

    int size = getFileSize();
    struct customer cust[size];
    struct customer *obj = malloc(sizeof(struct customer));
    FILE *file = fopen("file.txt", "rb");
    fseek(file, 0, SEEK_SET);
    int idx = 0;
    while (fread(obj, sizeof(struct customer), 1, file))
    {
        if (strcmp(obj->phonenumber, phone) == 0)
        {
        }
        else
        {
            strcpy(cust[idx].name, obj->name);
            strcpy(cust[idx].position, obj->position);
            strcpy(cust[idx].phonenumber, obj->phonenumber);
            strcpy(cust[idx].transmod, obj->transmod);

            idx++;
        }
    }
    size = idx;
    fclose(file);
    fseek(file, 0, SEEK_SET);
    FILE *newfile = fopen("file.txt", "w+");
    idx = 0;
    if (newfile != NULL)
    {
        for (idx = 0; idx < size; ++idx)
        {
            fwrite(&cust[idx], sizeof(struct customer), 1, newfile);
        }
    }
    fclose(newfile);
    getNewData(current);
    printf("A modositasok mentesre kerultek\n");
};

//menu for listing choice
void draw_list_menu()
{
    printf("A listazando helyszin: \n");
    printf("0 TEST MINDEN\n");
    printf("1. Bali\n");
    printf("2. Mali\n");
    printf("3. Cook_szigetek\n");
    printf("4. Bahamak\n");
    printf("5. Izland\n");
    printf("6. Listazasbol kilepes\n");
}

//List out people
void getList()
{
    struct customer obj;
    FILE *infile = fopen("file.txt", "r");
    if (infile == NULL)
    {
        fprintf(stderr, "\nHiba a file megnyitasa kozben\n");
        exit(1);
    }

    int choice = 0;

    draw_list_menu();

    scanf("%i", &choice);

    switch (choice)
    {
    //list everything out for testing purposes
    /*case 0:
        while (fread(&obj, sizeof(struct customer), 1, infile))
        {
            printf("nev = %s helyszin = %s telefonszam = %s utazas: %s\n", obj.name, obj.position,
                   obj.phonenumber, obj.transmod);
        }
        break;*/
    case 1:
        while (fread(&obj, sizeof(struct customer), 1, infile))
        {
            if (strcmp(obj.position, "Bali") == 0)
            {
                printf("nev = %s helyszin = %s telefonszam = %s utazas: %s\n",
                       obj.name, obj.position, obj.phonenumber, obj.transmod);
            }
        }
        break;
    case 2:
        while (fread(&obj, sizeof(struct customer), 1, infile))
        {
            if (strcmp(obj.position, "Mali") == 0)
            {
                printf("nev = %s helyszin = %s telefonszam = %s utazas: %s\n",
                       obj.name, obj.position, obj.phonenumber, obj.transmod);
            }
        }
        break;
    case 3:
        while (fread(&obj, sizeof(struct customer), 1, infile))
        {
            if (strcmp(obj.position, "Cook_szigetek") == 0)
            {
                printf("nev = %s helyszin = %s telefonszam = %s utazas: %s\n",
                       obj.name, obj.position, obj.phonenumber, obj.transmod);
            }
        }
        break;
    case 4:
        while (fread(&obj, sizeof(struct customer), 1, infile))
        {
            if (strcmp(obj.position, "Bahamak") == 0)
            {
                printf("nev = %s helyszin = %s telefonszam = %s utazas: %s\n",
                       obj.name, obj.position, obj.phonenumber, obj.transmod);
            }
        }
        break;
    case 5:
        while (fread(&obj, sizeof(struct customer), 1, infile))
        {
            if (strcmp(obj.position, "Izland") == 0)
            {
                printf("nev = %s helyszin = %s telefonszam = %s utazas: %s\n",
                       obj.name, obj.position, obj.phonenumber, obj.transmod);
            }
        }
    case 6:
        printf("Kilepes a listazasbol.\n");
        break;
    }

    fclose(infile);
};

//delete a customer's data from file
void deleteData(struct customer *current)
{
    printf("Kerem adja meg a torolni kivant ugyfel telefonszamat: ");
    char phone[15];
    scanf("%s", phone);

    int size = getFileSize();
    struct customer cust[size];
    struct customer *obj = malloc(sizeof(struct customer));
    FILE *file = fopen("file.txt", "rb");
    fseek(file, 0, SEEK_SET);
    int idx = 0;
    while (fread(obj, sizeof(struct customer), 1, file))
    {
        if (strcmp(obj->phonenumber, phone) == 0)
        {
            printf("Adat torolve\n");
        }
        else
        {
            strcpy(cust[idx].name, obj->name);
            strcpy(cust[idx].position, obj->position);
            strcpy(cust[idx].phonenumber, obj->phonenumber);
            strcpy(cust[idx].transmod, obj->transmod);

            idx++;
        }
    }
    size = idx;
    fclose(file);

    FILE *newfile = fopen("file.txt", "w+");
    fseek(file, 0, SEEK_SET);
    idx = 0;
    if (newfile != NULL)
    {
        for (idx = 0; idx < size; ++idx)
        {
            fwrite(&cust[idx], sizeof(struct customer), 1, newfile);
        }
    }
    fclose(newfile);
    printf("A kert torles mentve\n");
};

int count_peolpe(char count_at_pos[MAX_STRING_LENGTH])
{
    struct customer obj;

    FILE *infile = fopen("file.txt", "r");
    if (infile == NULL)
    {
        fprintf(stderr, "\nHiba a file megnyitasa kozben\n");
        exit(1);
    }

    int sum = 0;

    while (fread(&obj, sizeof(struct customer), 1, infile))
    {
        if (strcmp(obj.position, count_at_pos) == 0)
        {
            ++sum;
        }
    }

    fclose(infile);

    return sum;
};