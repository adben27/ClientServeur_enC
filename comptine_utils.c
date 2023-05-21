#include <asm-generic/errno-base.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "comptine_utils.h"

int read_until_nl(int fd, char *buf)
{
	char c; int i;
	for(i=0;read(fd, &c, 1)==1; i++){
		if(c!='\n'){
			buf[i]=c;
		} else { 
			buf[i]='\n';
			buf[i+1]='\0';
			break;
		}
	}
	return i;
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
	int fd; char* filename;
	if((filename=malloc(strlen(dir_name)+strlen(base_name)+1))<0){
		perror("malloc"); return NULL;
	};
	struct comptine* c;
	if((c=malloc(sizeof(struct comptine)))<0){
		perror("malloc"); return NULL;
	};
	strcpy(filename, dir_name);
	strcat(filename, "/");
	strcat(filename, base_name);
	if((fd=open(filename, O_RDONLY))<0){
		perror("open"); return NULL;
	}
	if((c->titre=malloc(256*sizeof(char)))<0){
		perror("malloc"); return NULL;
	};
	int count=read_until_nl(fd, c->titre);
	close(fd);
	free(filename);
	if((c->titre=realloc(c->titre, (count+2)*sizeof(char)))==NULL){
		perror("realloc"); return NULL;
	};
	c->nom_fichier=strdup(base_name);
	return c;
}

void liberer_comptine(struct comptine *cpt)
{
	free(cpt->titre); free(cpt->nom_fichier);
	free(cpt);
}

struct catalogue *creer_catalogue(const char *dir_name)
{
	DIR* dir; struct dirent* d; int count=0; int max_size=60;
	if((dir=opendir(dir_name))==NULL){
		perror("opendir"); return NULL;
	}
	if((d=readdir(dir))==NULL){
		if(errno==EBADF){
			perror("readdir"); return NULL;
		}
	};
	struct catalogue* ct; 
	if((ct=malloc(sizeof(struct catalogue)))<0){
		perror("malloc"); return NULL;
	};
	if((ct->tab=malloc(max_size*sizeof(struct comptine)))<0){
		perror("malloc"); return NULL;
	};
	
	while((d=readdir(dir))!=NULL){
		if(!est_nom_fichier_comptine(d->d_name))
			continue;
		if((ct->tab[count]=init_cpt_depuis_fichier(dir_name, d->d_name))<0){
			perror("init_cpt"); return NULL;
		};
		count++;
		if(count>=max_size-5){
			if((ct->tab=realloc(ct->tab, (max_size+5)*sizeof(struct comptine)))<0){
				perror("realloc"); return NULL;
			};
			max_size+=5;
		}
	}
	closedir(dir);
	ct->nb=count;
	return ct;
}

void liberer_catalogue(struct catalogue *c)
{
	for(int i=0; i<c->nb; i++)
		liberer_comptine(c->tab[i]);
	free(c->tab);
	free(c);
}
