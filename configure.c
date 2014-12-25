#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>

#include "serial.h"

static struct conf {
    char name[128];
    char data[128];
    struct conf *next;
};

struct conf *list = NULL;

char * get_conf(char *str)
{
    struct conf *now = NULL;

    if (list == NULL)
        return NULL;

    now = list;

    while(now != NULL)
    {
        if(!strcmp(now->name,str)){
            return &now->data;
        }
        now = now->next;
    }
    return NULL;
}

void configure_init(char * file)
{
    FILE *fp = NULL;
    struct conf *new = NULL;
    char buff[BUF_LEN] = {0},name[128] = {0},data[128] = {0};

#ifdef DEBUG
    printf("configure init()\n");
#endif

    fp = fopen( file ,"r");

    if( fp <= 0 )
    {
#ifdef DEBUG
        printf("Can't load configure[ %s ]\n",file);
#endif
    }

    
    while (fgets(buff, sizeof(buff),fp) != NULL){

        if(!strncmp(buff,"#",1) || strlen(buff) == strcspn(buff,"="))
            continue;

        new = malloc(sizeof(struct conf)); 
        memset(name, 0 ,sizeof(name));
        memset(data, 0 ,sizeof(data));
        
        sscanf(buff,"%[^=]=%s",name,data);

        strcpy(new->name,name);
        strcpy(new->data,data);

        if(list == NULL){
            new->next = NULL;            
        }else{
            new->next = list;
        }
        list = new;
        
    }
    fclose(fp);
}
