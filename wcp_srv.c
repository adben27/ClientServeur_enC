/* Adel BENNOUAR 12003494
 * Je déclare qu'il s'agit de mon propre travail */

/* fichiers de la bibliothèque standard */
#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
/* bibliothèque standard unix */
#include <unistd.h> /* close, read, write */
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <dirent.h>
#include <errno.h>
/* spécifique à internet */
#include <arpa/inet.h> /* inet_pton */
/* spécifique aux comptines */
#include "comptine_utils.h"
#include <pthread.h>
#include <bits/pthreadtypes.h>
/* Pour getdate*/
#include <time.h>

#define PORT_WCP 4321

void usage(char *nom_prog)
{
	fprintf(stderr, "Usage: %s repertoire_comptines\n"
			"serveur pour WCP (Wikicomptine Protocol)\n"
			"Exemple: %s comptines\n", nom_prog, nom_prog);
}
/** Retourne en cas de succès le descripteur de fichier d'une socket d'écoute
 *  attachée au port port et à toutes les adresses locales. */
int creer_configurer_sock_ecoute(uint16_t port);

/** Écrit dans le fichier de desripteur fd la liste des comptines présents dans
 *  le catalogue c comme spécifié par le protocole WCP, c'est-à-dire sous la
 *  forme de plusieurs lignes terminées par '\n' :
 *  chaque ligne commence par le numéro de la comptine (son indice dans le
 *  catalogue) commençant à 0, écrit en décimal, sur 6 caractères
 *  suivi d'un espace
 *  puis du titre de la comptine
 *  une ligne vide termine le message */
void envoyer_liste(int fd, struct catalogue *c);

/** Lit dans fd un entier sur 2 octets écrit en network byte order
 *  retourne : cet entier en boutisme machine. */
uint16_t recevoir_num_comptine(int fd);

/** Écrit dans fd la comptine numéro ic du catalogue c comme spécifié par le
 *  protocole WCP, c'est-à-dire :
 *  chaque ligne du fichier de comptine est écrite avec son '\n' final, y
 *  compris son titre
 *  deux lignes vides terminent le message */
void envoyer_comptine(int fd, const char *dirname, struct catalogue *c, uint16_t ic);

struct work{
	int sd;
	struct catalogue *c;
	char* dirname;
	char* addr_ipv4;
};

void* worker (void* arg);

/* Donne la date actuelle*/
char* getdate();

/*Ecrit dans un fichier de log la date de la connexion, l'addresse IPv4, le répertoire, le numéro de comptine, et son titre*/
void printtolog(char* date, char* addr_ipv4, char* dirname, int ic, struct catalogue *c);

pthread_mutex_t acces_log;

int main(int argc, char *argv[])
{
	if (argc != 2) {
		usage(argv[0]);
		return 1;
	}
	int sd=creer_configurer_sock_ecoute(PORT_WCP);
	struct catalogue *c;
	if((c=creer_catalogue(argv[1]))==NULL){
		perror("creer_catalogue"); exit(1);
	};
	/*struct sockaddr_in sa_clt;
	socklen_t sl=sizeof(sa_clt);
	if((sd=accept(sd, (struct sockaddr *) &sa_clt, &sl))<0){
		perror("accept"); exit(2);
	};
	envoyer_liste(sd, c);
	int ic=recevoir_num_comptine(sd);
	envoyer_comptine(sd, argv[1], c, ic);
	close(sd);
	liberer_catalogue(c);*/

	for (;;) {
		struct work *w;
		if(pthread_mutex_init(&acces_log, NULL)!=0){
			perror("pthread_mutex_init"); exit(1);
		}
		if((w=malloc(sizeof(struct work)))==NULL){
			perror("malloc"); exit(1);
		}
		struct sockaddr_in sa_clt;
		socklen_t sl=sizeof(sa_clt);

		w->sd =accept(sd, (struct sockaddr *) &sa_clt, &sl);
		if(w->sd==-1){
			perror("accept"); exit(-1);
		}
		w->c=c; w->dirname=argv[1];
		char buf[64];
		inet_ntop(AF_INET, &(sa_clt.sin_addr), buf, sl);
		w->addr_ipv4=buf;
		pthread_t th;
		if(pthread_create(&th, NULL, worker, w)<0){
			perror("pthread_create"); exit(1);
		}
		pthread_detach(th);
		pthread_mutex_destroy(&acces_log);
	}
	close(sd);
	liberer_catalogue(c);
	return 0;
}

void* worker(void* arg)
{
	struct work *w= arg;
	envoyer_liste(w->sd, w->c);
	int ic=recevoir_num_comptine(w->sd);
	pthread_mutex_lock(&acces_log);
	printtolog(getdate(), w->addr_ipv4, w->dirname, ic, w->c);
	pthread_mutex_unlock(&acces_log);
	envoyer_comptine(w->sd, w->dirname, w->c, ic);
	close(w->sd);
	free(w);
	return NULL;
}

int creer_configurer_sock_ecoute(uint16_t port)
{
	int sd=socket(AF_INET, SOCK_STREAM,0);
	if(sd<0){
		perror("socket"); exit(2); 
	}
	struct sockaddr_in sa= { .sin_family=AF_INET, 
		.sin_port=htons(PORT_WCP),
		.sin_addr.s_addr = htonl(INADDR_ANY) };
	socklen_t sl=sizeof(sa);
	int opt=1;
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
	if(bind(sd, (struct sockaddr *) &sa, sl) < 0){
		perror("bind"); exit(2);
	}
	if(listen(sd, 128) < 0){
		perror("listen"); exit(2);
	}
	return sd;
}

char* getdate()
{

    time_t currentTime;
    struct tm *localTime;
    char * datetime;
    if((datetime=malloc(80*sizeof(char)))==NULL){
	perror("malloc"); exit(1);
    }

    currentTime = time(NULL);

    localTime = localtime(&currentTime);

    int year = localTime->tm_year + 1900;
    int month = localTime->tm_mon + 1; 
    int day = localTime->tm_mday; 

    int hour = localTime->tm_hour; 
    int minute = localTime->tm_min; 
    int second = localTime->tm_sec; 

    sprintf(datetime, "%02d-%02d-%d %02d:%02d:%02d", day, month, year, hour, minute, second);

    return datetime;
}

void printtolog(char* date, char* addr_ipv4, char* dirname, int ic, struct catalogue *c)
{
	int logfd;
	if((logfd=open("wcp.log", O_WRONLY | O_APPEND))<0){
		perror("printtolog"); exit(3);
	}

	dprintf(logfd, "%s - %s - Répertoire %s - Comptine numéro %d de titre : %s\n\n", date, addr_ipv4, dirname, ic, c->tab[ic]->titre);
	
	free(date);
}



void envoyer_liste(int fd, struct catalogue *c)
{
	for(int i=0; i<c->nb; i++)
		dprintf(fd, "%6d %s", i, c->tab[i]->titre);
	dprintf(fd, "\n");
}

uint16_t recevoir_num_comptine(int fd)
{
	int nc;
	if(read(fd, &nc, sizeof(nc))<0){
		perror("read"); exit(3);
	}
	nc=ntohs(nc);
	return nc;
}

void envoyer_comptine(int fd, const char *dirname, struct catalogue *c, uint16_t ic)
{
	FILE* cptfp; char buf[256];
	char* filename;
	if((filename=malloc(strlen(dirname) + strlen(c->tab[ic]->nom_fichier) + 2))==NULL){
		perror("malloc"); exit(4);
	};
	strcpy(filename, dirname); strcat(filename, "/"); strcat(filename, c->tab[ic]->nom_fichier);
	if((cptfp=fopen(filename, "r"))==NULL){
		perror("fopen"); exit(3);
	}
	free(filename);
	while(fgets(buf, sizeof(buf), cptfp)!=NULL){
		dprintf(fd, "%s", buf);
	}
	dprintf(fd, "\n");
	fclose(cptfp);
}
