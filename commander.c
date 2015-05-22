#include <stdio.h>


void command (char *command)
{
    FILE *fp = NULL;

    fp = popen(command,"r");
    if(fp < 0){
#ifdef DEBUG
        printf("Command [%s] fail!\n",command);
#endif                
    }else{
        pclose(fp);
#ifdef DEBUG
        printf("Command [%s] success!\n",command);
#endif                
    }
}
