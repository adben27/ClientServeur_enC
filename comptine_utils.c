#include <asm-generic/errno-base.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "comptine_utils.h"

int main(){
	struct catalogue* ct=creer_catalogue("comptines");
	for(int i=0; i<ct->nb; i++)
		printf("%s", ct->tab[i]->nom_fichier);
	liberer_catalogue(ct);
}

int read_until_nl(int fd, char *buf)
{
	char c; int i;
	for(i=0;read(fd, &c, 1)==1; i++){
		if(c!='\n'){
			buf[i]=c;
		} else {
			buf[i]='\n';
			break;
		}
	}
	close(fd);
	return i+1;
}

int est_nom_fichier_comptine(char *nom_fich)
{
	char* extension=".cpt"; int longueur_ext=strlen(extension);
	int longueur_nom_fich=strlen(nom_fich);
	int cmp=strcmp(nom_fich + longueur_nom_fich - longueur_ext,extension);
	switch (cmp) {
		case 0:
			return 1; // Retourne 1 si est un fichier comptine
		default:
			return 0;
	}
}

struct comptine *init_cpt_depuis_fichier(const char *dir_name, const char *base_name)
{
	char buf[128]; int fd; char* filename=malloc(sizeof(dir_name)+sizeof(base_name)+1);
	struct comptine* c=malloc(sizeof(struct comptine));
	strcpy(filename, dir_name);
	strcat(filename, "/");
	strcat(filename, base_name);
	if((fd=open(filename, O_RDONLY))<0){
		perror("open"); return NULL;
	}
	int count=read_until_nl(fd, buf);
	close(fd);
	c->titre=malloc(count*sizeof(char));
	c->nom_fichier=malloc(sizeof(base_name));
	strncpy(c->titre, buf, count); strcpy(c->nom_fichier, base_name);
	return c;
}

void liberer_comptine(struct comptine *cpt)
{
	free(cpt->titre); free(cpt->nom_fichier);
	free(cpt);
	cpt=NULL;
}

struct catalogue *creer_catalogue(const char *dir_name)
{
	DIR* dir; struct dirent* d; int count=0;
	if((dir=opendir(dir_name))==NULL){
		perror("opendir"); return NULL;
	}
	if((d=readdir(dir))==NULL){
		if(errno==EBADF){
			perror("readdir"); return NULL;
		}
	};
	struct catalogue* ct=malloc(sizeof(struct catalogue));
	ct->tab=malloc(10*sizeof(struct comptine));
	
	while((d=readdir(dir))!=NULL){
		if(!est_nom_fichier_comptine(d->d_name))
			continue;
		ct->tab[count]=init_cpt_depuis_fichier(dir_name, d->d_name);
		count++;
	}
	closedir(dir);
	ct->nb=count+1;
	return ct;
}

void liberer_catalogue(struct catalogue *c)
{
	for(int i=0; i<c->nb; i++)
		liberer_comptine(c->tab[i]);
	free(c->tab);
	free(c);
	c=NULL;
}
