/** Operációs rendszerek - 2. beadandó 
 ** Balogh Dávid (GLVH9Z)
 **
 ** A "Szakács Tamás" utazási iroda csődbe ment.
 **  /.../
 **
 ** A csődbiztos (szülő) amint gyűlnek a mentésre várók, készíti fel a mentőexpedíciókat. Minden úti célhoz adott egy minimum létszám, ami alatt nem indítható expedíció.
 **
 ** Amint összejön a létszám adott helyen, a mentőexpedíció elindul (gyerekfolyamat). Amint odaér, visszajelez csődbiztosnak, hogy kéri az utaslistát.
 ** 
 ** Ekkor csődbiztos csövön továbbítja az adatokat az expedíciónak, hogy kiket kell felvenni.
 ** 
 ** Amint az expedíció hazaért, jelzést küld a csődbiztosnak, majd üzenetsoron összegzi, hogy honnan, hány utast hoztak haza. 
 **/
 
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <sys/types.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

#define HELYSZINEK_SZAMA 5
#define JARMUVEK_SZAMA 3

enum helyszinek {BALI, MALI, COOK, BAHAMAK, IZLAND};
enum jarmuvek {AUTO, HAJO, REPULO};


typedef struct adat_tipus{
  char		  	   nev[100];
  enum helyszinek  hely; 
  char		       tel[100];
  enum jarmuvek    jarmu;
}adat_t;

void mentoexpedicio(enum helyszinek h, adat_t lista[], int *meret, int nevindex_lista[], int talalatok);

void udvozles(adat_t *a);

void adatok_felvetele(adat_t *a);
void szerkesztett_adat_mutatasa(adat_t *a);
int adatok_listahoz_fuzese(adat_t *a, adat_t lista[], int meret);

int lista_olvasasa_fajlbol(char *file, adat_t lista[]);
void lista_irasa_fajlba(char *file, adat_t lista[], int meret);
void miben_mit_keres(int *v, char keres[]);
int listaban_keres(adat_t lista[], int meret, int v, char keres[]);
void egesz_lista_megjelenitese(adat_t lista[], int meret);

int lista_helyszin_alapjan(enum helyszinek h_ind, adat_t lista[], int meret);
int szamlalas_helyszin_alapjan(enum helyszinek h_ind, adat_t lista[], int meret);
void nevkigyujtes_helyszin_alapjan(enum helyszinek h_ind, adat_t belista[], int bemeret, int nevindex_lista[]);
int helyszin_bekerese();

void adat_megjelenitese_listabol(adat_t lista[], int index);
void adat_szerkesztese_listaban(adat_t *a, adat_t lista[], int index);
int adat_torlese_listabol(adat_t *a, adat_t lista[], int meret);
int index_torlese_listabol(int index, adat_t lista[], int meret);

static char *jarmu2string(enum jarmuvek j);
static enum jarmuvek string2jarmu(char jnev[]);
static char *helyszin2string(enum helyszinek h);
static enum helyszinek string2helyszin(char hnev[]);
//---------------------------------------------------------------------------
int main() {
   FILE *fp;
   int i,j;
   int c;
   static adat_t adat;
   static adat_t lista[100];
   static int nevindex_lista[100];
   int meret;
   
   int v=0, talalatok=0;
   char keres[20];
   int szerk_lista_index;
   int min_letszam[HELYSZINEK_SZAMA] = {1,2,10,10,10};
   
   
   printf("* LISTA BEOLVASASA\n");
   meret=lista_olvasasa_fajlbol("lista",lista);
   printf("* %d adat beolvasva\n", meret);
   
   strcpy(adat.nev,"-");
   strcpy(adat.tel,"-");
  
   szerk_lista_index=-1;
   c=100;
   for(i=100;i>0;i--)
   {
		int van_mentheto_utas=0;
		
		switch(c)
		{
		  case 1: //felvetel
				  printf("* ADATOK FELVETELE\n");
				  adatok_felvetele(&adat);
				  sleep(1);
				  c=4;
				  break;
		  case 2: //modositas
				  printf("* ADATOK MODOSITASA\n");
				  printf("* Regi adatok:\n");
				  szerkesztett_adat_mutatasa(&adat);
				  printf("* Kerem az uj adatokat:\n");
				  adatok_felvetele(&adat);
				  sleep(1);
				  c=0;
				  break;
		  case 3: //megjelenites
				  printf("* SZERKESZTETT ADATOK MEGJELENITESE\n");
				  szerkesztett_adat_mutatasa(&adat);
				  sleep(1);
				  c=0;
				  break;
		  case 4: //adatok hozzafuzese a listahoz
				  printf("* ADATOK LISTAHOZ FUZESE\n");
				  meret=adatok_listahoz_fuzese(&adat,lista,meret);
				  sleep(1);
				  c=100;
				  break;
		  case 5: //adatok torlese a listabol
				  printf("* ADATOK TORLESE A LISTABOL\n");
				  meret=adat_torlese_listabol(&adat,lista,meret);
				  sleep(1);
				  c=0;
				  break;
		  
		  case 6: // kiiras
				  printf("* LISTA KIIRASA FAJLBA\n");
				  lista_irasa_fajlba("lista", lista, meret);
				  sleep(1);
				  c=10;
				  break;
		  case 7: // kereses
				  printf("* KERESES A LISTABAN\n");
				  
				  miben_mit_keres(&v,keres);
				  szerk_lista_index=listaban_keres(lista, meret,v,keres);
				  if(szerk_lista_index>-1)
				  {
					adat_szerkesztese_listaban(&adat, lista, szerk_lista_index);
				  }
				  
				  sleep(1);
				  c=0;
				  break;
		  case 8: // lista listazasa
				  printf("* UTASLISTA MEGJELENITESE\n");
				  egesz_lista_megjelenitese(lista, meret);
				  sleep(1);
				  c=0;
				  break;
		  case 9: // lista listazasa helyszin alapjan
				  printf("* UTASOK LISTAZASA HELYSZIN ALAPJAN\n");
				  talalatok = lista_helyszin_alapjan(helyszin_bekerese(), lista, meret);
				  printf("* Talalatok szama: %d\n", talalatok);
				  sleep(1);
				  c=0;
				  break;
		  case 10: //osszegzes
					printf("* UTASLISTA OSSZEGZESE\n");
					printf("* --------------------\n");
					printf("* Helyszin | Talalatok | Minimum\n");
					for(j=0; j<HELYSZINEK_SZAMA; j++)
					{
						printf("* %s |", helyszin2string(j));
						talalatok = szamlalas_helyszin_alapjan(j, lista, meret);
						printf(" %d | %d\n", talalatok, min_letszam[j]);
					}
					printf("\n");
					sleep(1);
					c=0;
					break;
		  case 100: //szamlalas
					for(j=0; j<HELYSZINEK_SZAMA; j++)
					{
						talalatok = szamlalas_helyszin_alapjan(j, lista, meret);
						if(talalatok>=min_letszam[j])
						{
							van_mentheto_utas=1;
						}
					}
					if(van_mentheto_utas==0){ c=0; break;}
		  case 101: //mentoexpedicio
					//printf("* Helyszin szamlalas\n");
					//printf("* --------------------\n");
					//printf("* Helyszin | Talalatok | Minimum\n");
					for(j=0; j<HELYSZINEK_SZAMA; j++)
					{
						//printf("* %s |", helyszin2string(j));
						talalatok = szamlalas_helyszin_alapjan(j, lista, meret);
						//printf(" %d | %d\n", talalatok, min_letszam[j]);
						if(talalatok>=min_letszam[j]) //helyszinek megszamolasa es hozzajuk expedicio inditasa
						{
							//csak az indexeket atadni a gyereknek, hazahozzak, majd szulo torli a listabol
							nevkigyujtes_helyszin_alapjan(j, lista, meret, nevindex_lista);
							mentoexpedicio(j, lista, &meret, nevindex_lista, talalatok);
						}
					}
					printf("* --------------------\n");
					sleep(1);
				  c=6; //kiiras fajlba
				  break;
		  case 11: //viszlat
				  printf("* Viszlat\n");
				  i=0;
				  break;
		  default: //alap, bill bekeres
				  udvozles(&adat);

				  if(scanf("%d", &c)<1){while( getchar() != '\n' );};
				  if(c>99){ c=0; break; }
				  i=100;
		}
   }

    return 0;
}

//---------------------------------------------------------------------------
void mentoexp_handler(int signumber){
  if(signumber == SIGUSR1){
	printf("Mentoexpediciotol erkezett jelzes!\n");
  }else{
	printf("%i szamu jelzes erkezett\n",signumber);
  }
}
//---------------------------------------------------------------------------
struct uzenet { 
     long mtype;//ez egy szabadon hasznalhato ertek, pl uzenetek osztalyozasara
     //char mtext [ 1024 ];
	 int num;
};
//---------------------------------------------------------------------------
int kuld(int uzenetsor, int utasokszama) 
{ 
	const struct uzenet uz = {5, utasokszama}; 
	int status; 
     
    status = msgsnd(uzenetsor, &uz, sizeof(uz.num), IPC_NOWAIT); 
    if(status<0){ perror("msgsnd"); }
    return 0; 
} 
//---------------------------------------------------------------------------
int fogad(int uzenetsor, enum helyszinek h)
{ 
    struct uzenet uz; 
    int status; 

	status = msgrcv(uzenetsor, &uz, sizeof(uz.num), 5, IPC_NOWAIT); 
    
    if(status<0){ perror("msgsnd"); }
    else{ printf("* %s-rol hozott emberek szama: %d\n", helyszin2string(h), uz.num); }
	
    return 0; 
} 
//---------------------------------------------------------------------------
void mentoexpedicio(enum helyszinek h, adat_t lista[], int *meret, int nevindex_lista[], int talalatok)
{
    key_t kulcs;
	int uzenetsor, status;
	
	struct sigaction sigact;
	sigact.sa_handler=mentoexp_handler;
	sigemptyset(&sigact.sa_mask);   
	sigact.sa_flags=0;
	sigaction(SIGTERM,&sigact,NULL);
	sigaction(SIGUSR1,&sigact,NULL); //mentoexp jelzese
	
	int pipeutaslistafd[2]; //0: olv, 1:ir 
	
	kulcs = ftok("bead_2",55); 
    uzenetsor = msgget(kulcs, 0600 | IPC_CREAT); 
    
	if (uzenetsor<0) {  perror("msgget"); exit(1); } 
	if (pipe(pipeutaslistafd) == -1){ perror("Hiba a pipe nyitaskor!"); exit(EXIT_FAILURE); }
  
	printf("* %s mentoexpedicio inditasa\n", helyszin2string(h));
	
	pid_t child=fork(); //innentol a kod lemasolodik, es a kovetkezo utasitastol szulo-gyerek
	
	if(child>1) //szulo 
	{
		adat_t utaslista[talalatok];
		
		close(pipeutaslistafd[0]); //olv bezarasa
		
		sigset_t sigset;
		sigfillset(&sigset);
		sigdelset(&sigset,SIGUSR1);
		
		for(int i=0;i<talalatok;i++)
		{
			strcpy(utaslista[i].nev,lista[nevindex_lista[i]].nev);
			strcpy(utaslista[i].tel,lista[nevindex_lista[i]].tel);
		}
		
		//szulo var az utaslista keresre
		sigsuspend(&sigset); //pause
		//szulo kuld utaslistat
		write(pipeutaslistafd[1], utaslista, sizeof(adat_t)*talalatok);
		close(pipeutaslistafd[1]);//ir bezarasa
		//ha gyerek jelez, h megkapta es hazaert
		//szulo torli a listabol a neveket(torolni kell?)
		sigsuspend(&sigset); //pause
		fogad(uzenetsor,h);
		
		printf("* Csodbiztos sikeresen megerkeztette az utasokat!\n\n");
		for(int i=talalatok-1; i>=0; i--){
			
			*meret=index_torlese_listabol(nevindex_lista[i], lista, *meret);
			//printf("* Utasok szama: %d\n", *meret);
		}
		//waitpid(child,&status,0);
	}
	else if(child==0) //gyerek
	{
		adat_t mentesinevek[100];
		
		close(pipeutaslistafd[1]); //ir bezarasa
		sleep(1);
		
		//gyerek jelez h keri a listat
		kill(getppid(),SIGUSR1);
		//szulo kuld
		sleep(1);
		//gyerek megkap es jelez
		read(pipeutaslistafd[0],mentesinevek,sizeof(adat_t)*talalatok); //csalas
		close(pipeutaslistafd[0]);
		
		
		printf("Hazahozott utasok:\n");
		for(int i=0;i<talalatok;i++)
		{
			printf("%s %s\n",mentesinevek[i].nev, mentesinevek[i].tel);
		}
		
		printf("* Mentoexpedicio hazaert!\n");
		
		//jelzes szulonek, hazaert es kuldi az uzenetsort
		kuld(uzenetsor,talalatok);
		kill(getppid(),SIGUSR1);
		sleep(1);
		
		status = msgctl(uzenetsor, IPC_RMID, NULL); 
        if(status<0){ perror("msgctl"); }
		exit(0);
	}
	else
	{
		printf("err\n");
		exit(1);
	}
}

//---------------------------------------------------------------------------
void udvozles(adat_t *a)
{
  printf("* **************************************** *\n");
  printf("* Utasnyilvantarto program                 *\n");
  printf("* Valasszon az alabbi lehetosegek kozul:   *\n");
  printf("*                -------                   *\n");
  printf("* 1: Adatok felvetele                      *\n");
  printf("* 2: Adatok modositasa                     *\n");
  printf("* 3: Adatok megjelenitese                  *\n");
  printf("* 4: Adatok listahoz fuzese                *\n");
  printf("* 5: Szerkesztett adat torlese a listabol  *\n");
  printf("*                  ***                     *\n");
  printf("* 6: Lista kiirasa fajlba                  *\n");
  printf("*                                          *\n");
  printf("* 7: Kereses a listaban                    *\n");
  printf("* 8: Utaslista megjelenitese               *\n");
  printf("* 9: Utasok listazasa helyszin alapjan     *\n");
  printf("* 10: Utaslista osszegzese                 *\n");
  printf("*                  ***                     *\n");
  printf("* 11: Kilepes                              *\n");
  printf("*                -------                   *\n");
  printf("* ---------------------------------------- *\n");
  printf("* Jelenleg szerkesztett adatok             *\n");
  szerkesztett_adat_mutatasa(a);
  printf("* ---------------------------------------- *\n");
  printf("* **************************************** *\n");
}
//---------------------------------------------------------------------------
void adatok_felvetele(adat_t *a)
{
	char hnev[20];
	char jnev[20];
	int valasz;
	
	int i;
	int v=0;
	
	//nev
    printf("* Adja meg a nevet:\n");
	scanf("%s", a->nev);
	printf("* %s\n", a->nev);
	
	//helyszin
	a->hely=helyszin_bekerese();
	printf("* %s\n", helyszin2string(a->hely));
	
	//tel szam
	printf("\n* Adja meg a telefonszamat:\n");
	scanf("%s", a->tel);
	printf("* %s\n", a->tel);
	
	//utazasi mod
	printf("\n* Adja meg az utazasi modot\n");
	printf("* Valasszon az alabbi lehetosegek kozul:\n");
	for(i=0; i<JARMUVEK_SZAMA; i++)
	{
		printf("* %d:", i+1);
		strcpy(jnev, jarmu2string(i));
		printf(" %s\n", jnev);
	}
	scanf("%d", &v);
	a->jarmu=v-1;
	printf("* %s\n", jarmu2string(a->jarmu));
	
	printf("\n* Megadott adatok:\n");
	szerkesztett_adat_mutatasa(a);
	
	//err check
	
	printf("Adatok OK\n");
	printf("\n");
}
//---------------------------------------------------------------------------
void szerkesztett_adat_mutatasa(adat_t *a)
{
	char hnev[20];
	char jnev[20];
	
	printf("* Nev: %s\n", a->nev);
	printf("* Helyszin: %s\n", helyszin2string(a->hely));
	printf("* Tel. szam: %s\n", a->tel);
	printf("* Utazasi mod: %s\n", jarmu2string(a->jarmu));
}
//---------------------------------------------------------------------------
int adatok_listahoz_fuzese(adat_t *a, adat_t lista[], int meret)
{
	strcpy(lista[meret].nev,a->nev);
	lista[meret].hely=a->hely;
	strcpy(lista[meret].tel,a->tel);
	lista[meret].jarmu=a->jarmu;
	
	meret++;
	
	return meret;
}
//---------------------------------------------------------------------------
int lista_olvasasa_fajlbol(char *file, adat_t lista[])
{
	FILE *fp;
	int i,j;
	char sor[200];

	char *token;
	//fgets max 100 sorig, karaktertombbe
	//vesszovel elvalasztva, az elso pontosvesszoig
	//beolvassa egy lista tombbe az adatokat, ellenorzes nelkul, az adatokat	helyesnek feltetelezve
	
	fp = fopen("lista", "r");
	if(fp==NULL){return -1;}
	
	for(i=0;i<100;i++)
	{
		if(fgets(sor,100,fp)==NULL){return i;}
		//printf("%s", sor);
		
		token = strtok(sor,";");
		if(token==NULL){break;}
		strcpy(lista[i].nev,token);
		printf("%s, ",token);
		
		token = strtok(NULL,";");
		if(token==NULL){break;}
		for(j=0;j<HELYSZINEK_SZAMA;j++)
		{
			if(strcmp(helyszin2string(j),token)==0){lista[i].hely=j;break;}else{lista[i].hely=0;}
		}
		printf("%s, ",token);
		
		token = strtok(NULL,";");
		if(token==NULL){break;}
		strcpy(lista[i].tel,token);
		printf("%s, ",token);
		
		token = strtok(NULL,";");
		if(token==NULL){break;}
		for(j=0;j<JARMUVEK_SZAMA;j++)
		{
			if(strcmp(jarmu2string(j),token)==0){lista[i].jarmu=j;break;}else{lista[i].jarmu=0;}
		}
		printf("%s\n",token);
		
		token = strtok(NULL,";");
	}
	fclose(fp);
}
//---------------------------------------------------------------------------
void lista_irasa_fajlba(char *file, adat_t lista[], int meret)
{
	FILE *fp;
	int i,n;
	
	fp = fopen("lista", "w");
	for(i=0;i<meret;i++)
	{
		n=fprintf(fp,"%s;%s;%s;%s;\n",lista[i].nev,helyszin2string(lista[i].hely),lista[i].tel,jarmu2string(lista[i].jarmu));
	}
	fclose(fp);
	if(n<0){printf("* Fajlirasi hiba\n");return;}
	
	printf("\n");
	printf("* Lista kiirva OK\n");
	printf("\n");
}
//---------------------------------------------------------------------------
void egesz_lista_megjelenitese(adat_t lista[], int meret)
{
	int i;
	
	for(i=0;i<meret;i++)
	{
		printf("%s, ",lista[i].nev);
		printf("%s, ",helyszin2string(lista[i].hely));
		printf("%s, ",lista[i].tel);
		printf("%s\n",jarmu2string(lista[i].jarmu));
	}
}
//---------------------------------------------------------------------------
void miben_mit_keres(int *v,char keres[])
{
	printf("* Milyen adatmezőben keressek?           *\n");
	printf("* ------                                 *\n");
	printf("* 1: Név                                 *\n");
	printf("* 2: Helyszín                            *\n");
	printf("* 3: Telefonszám                         *\n");
	printf("* 4: Jármű                               *\n");
	scanf("%d", v);
	
	printf("* Mit keressek?                          *\n");
	scanf("%s", keres);
}
//---------------------------------------------------------------------------
int listaban_keres(adat_t lista[], int meret, int v, char keres[])
{
	int i,index;
	
	index=meret;
	
	printf("* Kereses: %s\n",keres);
	for(i=0; i<meret; i++)
	{
		switch(v)
		{
			case 1:
				if(strcmp(lista[i].nev,keres)==0){index=i;i=meret;}
				break;
			case 2:
				if(strcmp(helyszin2string(lista[i].hely),keres)==0){index=i;i=meret;}
				break;
			case 3:
				if(strcmp(lista[i].tel,keres)==0){index=i;i=meret;}
				break;
			case 4:
				if(strcmp(jarmu2string(lista[i].jarmu),keres)==0){index=i;i=meret;}
				break;
			default:
				i=meret;
				index=meret;
				break;
		}
	}

	if(index<meret)
	{
		printf("* Talalt adatok:\n");
		adat_megjelenitese_listabol(lista, index);
		return index;
	}
	
	return -1;
}
//---------------------------------------------------------------------------
void adat_megjelenitese_listabol(adat_t lista[], int index)
{	
	printf("%s, ",lista[index].nev);
	printf("%s, ",helyszin2string(lista[index].hely));
	printf("%s, ",lista[index].tel);
	printf("%s\n",jarmu2string(lista[index].jarmu));
}
//---------------------------------------------------------------------------
void adat_szerkesztese_listaban(adat_t *a, adat_t lista[], int index)
{	
	strcpy(a->nev,lista[index].nev);
	a->hely=lista[index].hely;
	strcpy(a->tel,lista[index].tel);
	a->jarmu=lista[index].jarmu;
}
//---------------------------------------------------------------------------
int lista_helyszin_alapjan(enum helyszinek h_ind, adat_t lista[], int meret)
{
	int i, v=0;
	char hnev[50];
	
	strcpy(hnev,helyszin2string(h_ind));
	
	for(i=0;i<meret;i++)
	{
		if(strcmp(helyszin2string(lista[i].hely),hnev)==0)
		{
			adat_megjelenitese_listabol(lista,i);
			v++;
		}
	}
	
	return v;
}
//---------------------------------------------------------------------------
int szamlalas_helyszin_alapjan(enum helyszinek h_ind, adat_t lista[], int meret)
{
	int i, v=0;
	char hnev[50];
	
	strcpy(hnev,helyszin2string(h_ind));
	
	for(i=0;i<meret;i++)
	{
		if(strcmp(helyszin2string(lista[i].hely),hnev)==0)
		{
			v++;
		}
	}
	
	return v;
}
//---------------------------------------------------------------------------
void nevkigyujtes_helyszin_alapjan(enum helyszinek h_ind, adat_t belista[], int bemeret, int nevindex_lista[])
{
	//kilista meretet a szamlalas_ adja vissza
	//de ennek kene, majd csere
	
	int i, v=0;
	char hnev[50];
	
	strcpy(hnev,helyszin2string(h_ind));
	
	for(i=0;i<bemeret;i++)
	{
		if(strcmp(helyszin2string(belista[i].hely),hnev)==0)
		{
			nevindex_lista[v]=i;
			
			v++;
		}
	}
}
//---------------------------------------------------------------------------
int helyszin_bekerese()
{
	int i,v;
	
	printf("\n* Adja meg a helyszint\n");
	printf("* Valasszon az alabbi lehetosegek kozul:\n");
	for(i=0; i<HELYSZINEK_SZAMA; i++)
	{
		printf("* %d:", i+1);
		printf(" %s\n", helyszin2string(i));
	}
	scanf("%d", &v);
	
	return (v-1);
}
//---------------------------------------------------------------------------
int adat_torlese_listabol(adat_t *a, adat_t lista[], int meret)
{
	int i,index;
	
	index=-1;
	index=listaban_keres(lista, meret, 1, a->nev);
	if(index>-1)
	{
		printf("*** \n");
		adat_megjelenitese_listabol(lista, index);
		for(i=index;i<meret-1;i++)
		{
			lista[i]=lista[i+1];
		}
		printf("* Adatok torolve!\n");
		
		return --meret;
	}
	
	printf("* Nem talaltam ilyen adatot\n");
	return meret;
}
//---------------------------------------------------------------------------
int index_torlese_listabol(int index, adat_t lista[], int meret)
{
	int i;
	//printf("index_torlese_listabol: %d %d\n",index, meret);
	if(index>-1 && index<meret)
	{
		for(i=index;i<meret-1;i++)
		{
			lista[i]=lista[i+1];
		}
		meret--;
		
		//printf("uj meret: %d\n", meret);
	}
	
	return meret;
}
//---------------------------------------------------------------------------
static char *helyszin2string(enum helyszinek h)
{
	static char *strings[] = {"Bali","Mali","Cook-szigetek","Bahamak","Izland"};

    return strings[h];
}
static enum helyszinek string2helyszin(char hnev[])
{
	int i;
	
	for(i=0;i<HELYSZINEK_SZAMA;i++)
	{
		if(strcmp(helyszin2string(i),hnev)==0){return i;}
	}
}
//---------------------------------------------------------------------------
static char *jarmu2string(enum jarmuvek j)
{
	static char *strings[] = {"Auto","Hajo","Repulo"};

    return strings[j];
}
static enum jarmuvek string2jarmu(char jnev[])
{
	int i;
	
	for(i=0;i<JARMUVEK_SZAMA;i++)
	{
		if(strcmp(jarmu2string(i),jnev)==0){return i;}
	}
}