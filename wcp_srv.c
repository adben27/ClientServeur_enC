/* fichiers de la bibliothèque standard */
#include <asm-generic/socket.h>
#include <netinet/in.h>
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
void envoyer_comptine(int fd, struct catalogue *c, uint16_t ic);

int main(int argc, char *argv[])
{
	if (argc != 2) {
		usage(argv[0]);
		return 1;
	}
	struct sockaddr_in sa_clt= { .sin_family=AF_INET,
		.sin_port=htons(PORT_WCP), };
	if(inet_pton(AF_INET, argv[1], &sa_clt.sin_addr) != -1){
		socklen_t sl=sizeof(sa_clt);
		int sd=creer_configurer_sock_ecoute(PORT_WCP);
		//for(;;){
			sd=accept(sd, (struct sockaddr *) &sa_clt, &sl);
			struct catalogue *c=creer_catalogue("comptines");
			envoyer_liste(sd, c);
			//int ic=recevoir_num_comptine(sd);
			//printf("IC : %d\n", ic);
			//envoyer_comptine(sd, c, 3);
			close(sd);
		//}
	}
	return 0;
}
int creer_configurer_sock_ecoute(uint16_t port)
{
	int sd=socket(AF_INET, SOCK_STREAM,0);
	if(sd<0){
		perror("socket"); exit(-1); 
	}
	struct sockaddr_in sa= { .sin_family=AF_INET, 
		.sin_port=htons(PORT_WCP),
		.sin_addr.s_addr = htonl(INADDR_ANY) };
	socklen_t sl=sizeof(sa);
	int opt=1;
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
	if(bind(sd, (struct sockaddr *) &sa, sl) < 0){
		perror("bind"); exit(-1);
	}
	if(listen(sd, 128) < 0){
		perror("listen"); exit(-1);
	}
	return sd;
}

void envoyer_liste(int fd, struct catalogue *c)
{
	for(int i=0; i<c->nb; i++){
		if(dprintf(fd, "%6d %s", i, c->tab[i]->titre)<0){
			perror("dprintf"); exit(-1);
		};
	}
	dprintf(fd, "\n");
}

uint16_t recevoir_num_comptine(int fd)
{
	int nc;
	if(read(fd, &nc, sizeof(nc))<0){
		perror("read"); exit(-1);
	}
	return ntohs(nc);
}

void envoyer_comptine(int fd, struct catalogue *c, uint16_t ic)
{
	int textfd; char buf[256];
	//char* filename=malloc(sizeof("comptines") + sizeof(c->tab[ic]->nom_fichier) + 1);
	//strcpy(filename, "comptines"); 
	//strcat(filename, "/");
	//strcat(filename, c->tab[ic]->nom_fichier);
	if((textfd=open("comptines/petit_lapin.cpt", O_RDONLY))<0){
		perror("open"); exit(-1);
	}
	//free(filename);
	//dprintf(fd, "%s", c->tab[ic]->titre);	
	while(read_until_nl(textfd, buf)>=0){
		printf("%s", buf);
		//dprintf(fd, "%s", buf);
	}
	//dprintf(fd, "\n\n");
	close(textfd);
}
