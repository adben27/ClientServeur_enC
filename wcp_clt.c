/* fichiers de la bibliothèque standard */
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
/* bibliothèque standard unix */
#include <unistd.h> /* close, read, write */
#include <sys/types.h>
#include <sys/socket.h>
/* spécifique à internet */
#include <arpa/inet.h> /* inet_pton */
/* spécifique aux comptines */
#include "comptine_utils.h"

#define PORT_WCP 4321

void usage(char *nom_prog)
{
	fprintf(stderr, "Usage: %s addr_ipv4\n"
			"client pour WCP (Wikicomptine Protocol)\n"
			"Exemple: %s 208.97.177.124\n", nom_prog, nom_prog);
}

/** Retourne (en cas de succès) le descripteur de fichier d'une socket
 *  TCP/IPv4 connectée au processus écoutant sur port sur la machine d'adresse
 *  addr_ipv4 */
int creer_connecter_sock(char *addr_ipv4, uint16_t port);

/** Lit la liste numérotée des comptines dans le descripteur fd et les affiche
 *  sur le terminal.
 *  retourne : le nombre de comptines disponibles */
uint16_t recevoir_liste_comptines(int fd);

/** Demande à l'utilisateur un nombre entre 0 (compris) et nc (non compris)
 *  et retourne la valeur saisie. */
uint16_t saisir_num_comptine(uint16_t nb_comptines);

/** Écrit l'entier ic dans le fichier de descripteur fd en network byte order */
void envoyer_num_comptine(int fd, uint16_t nc);

/** Affiche la comptine arrivant dans fd sur le terminal */
void afficher_comptine(int fd);

int main(int argc, char *argv[])
{
	if (argc != 2) {
		usage(argv[0]);
		return 1;
	}
	int sd;
	sd=creer_connecter_sock(argv[1], PORT_WCP);
	int nbc=recevoir_liste_comptines(sd);
	int nc=saisir_num_comptine(nbc);
	envoyer_num_comptine(sd,nc);
	afficher_comptine(sd);
	close(sd);
	return 0;
}

int creer_connecter_sock(char *addr_ipv4, uint16_t port)
{
	int sd=socket(AF_INET, SOCK_STREAM, 0);
	if(sd<0){
		perror("socket"); exit(2);
	}
	struct sockaddr_in sa={ .sin_family=AF_INET, .sin_port=htons(port) };
	if((inet_pton(AF_INET, addr_ipv4, &sa.sin_addr)) != -1){
		socklen_t sl=sizeof(sa);
		if(connect(sd, (struct sockaddr *) &sa, sl) < 0){
			perror("connect"); exit(2);
		}
		return sd;
	}
	return -1;
}

uint16_t recevoir_liste_comptines(int fd)
{
	int count=0; char buf[256];
	while(read_until_nl(fd, buf)!=0){
		count++;
		printf("%s", buf);
	}
	return count;
}

uint16_t saisir_num_comptine(uint16_t nb_comptines)
{
	int select; int valid=0;
	do{
		printf("Quelle comptine voulez-vous ? (Entrer un entier entre 0 et %hd non compris) : ", nb_comptines); 
		scanf("%d", &select);
		if(select < nb_comptines)
			valid=1;
	} while(valid==0);
	return select;
}

void envoyer_num_comptine(int fd, uint16_t nc)
{
	nc=htons(nc);
	if(write(fd, &nc, sizeof(nc))<0){
		perror("write"); exit(3);
	};
}

void afficher_comptine(int fd)
{
	char buf[256]; FILE* fp;
	if((fp=fdopen(fd, "r"))==NULL){
		perror("fdopen"); exit(3);
	}
	while(fgets(buf, sizeof(buf), fp)!=NULL){
		printf("%s", buf);
	}
	fclose(fp);
}
