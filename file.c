#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "serial.h"

const char *sd_storage = "/tmp/mnt/SD";
const char *usb_storage = "/tmp/mnt/USB";
int digital = 0;
int check_time = 0;
int sd_size = 0;
int usb_size = 0;
extern int sd_enable;
extern int usb_enable;
extern int record;

int check_size(const char * file_dir)
{
    struct statvfs buf;
    int total,used;

#if DEBUG
    printf("check_size [ %s ]\n",file_dir);
#endif       

    if ( statvfs(file_dir,&buf) < 0 )
    {
#if DEBUG
    printf("statvfs fail!!\n");
#endif       
        return 101;
    }
    used = (buf.f_blocks - buf.f_bfree);
    total = buf.f_blocks ;
#if DEBUG
    printf("check_size use : %d total : %d [ %d ]\n",used,total,( ( used * 100 ) / total ));
#endif       
    return ( ( used * 100 ) / total );

}

int detect_storage(const char * file_dir)
{
    DIR *dir = NULL;

    if( file_dir == NULL ){ 
#if DEBUG
    printf("file_dir is NULL!!\n");
#endif       
            return 0;
    }

#if DEBUG
    printf("detect_storage [ %s ]\n",file_dir);
#endif       

    if ( ( dir = opendir(file_dir) ) == NULL){
#if DEBUG
    printf("Open dir fail!!\n");
#endif       
        return 0;
    }

    closedir(dir);

    
#if DEBUG
    printf("Storage success!!\n");
#endif       
    return 1;
}

int scan_file(const char *file_dir,int del)
{
    struct dirent **namelist = NULL;
    int n,i = 1,fin_num = 0;
    char file_name[NAME_LEN] = {0};
    char num[NAME_LEN] = {0};
    char path[NAME_LEN] = {0};

#if DEBUG
    printf("Enter scan_file!!\n");
#endif       

#if DEBUG
    printf("scan_file path [ %s ]!!\n",file_dir);
#endif  

    n = scandir( file_dir, &namelist, 0, alphasort);
    if (n < 0 ){
#if DEBUG
        printf("Dir is empty!!\n");
#endif       
        free(namelist); 
        return 0;
    }else{
        while(i < n)
        {
            strcpy(file_name,namelist[i]->d_name); 
            if (strlen(file_name) == 14)
            {
                sscanf(file_name,"%[0-9]",num);
                if (strlen(num) == 10)  
                { 
#if DEBUG
                    printf("Find File [ %s.txt ] !!\n",num);
#endif       
                    if(del)
                    {
                        sprintf(path,"%s/%s",file_dir,namelist[i]->d_name);
                        remove(path);
                    }
		    fin_num = MAXNO(fin_num,atoi(num));
                }
            }
            free(namelist[i]); 
            i++;
        }
        free(namelist); 
#if DEBUG
        printf("Last file [ %010d.txt ] !!\n",fin_num);
#endif       
        return fin_num;
    }
}

int find_digital(char *dir)
{
    int sd_num = 0 , usb_num = 0;
    char path[1024] = {0};

    if (detect_storage(sd_storage) > 0 && check_dir(sd_storage,dir) > 0)
    {
       sprintf(path,"%s/%s",sd_storage,dir); 
       sd_num = scan_file(path,0);
    }

    memset(path,0,sizeof(path));

    if (detect_storage(usb_storage) > 0 && check_dir(usb_storage,dir) > 0)
    {
       sprintf(path,"%s/%s",usb_storage,dir); 
       usb_num = scan_file(path,0);
    }
   
    return MAXNO(sd_num,usb_num);
}

int check_digital(char *file_dir)
{
    int now_time = time_number();

    digital = MAXNO(find_digital(file_dir),digital);

    if ( check_time == 0 || check_time != now_time )
    {
        check_time = now_time;
        return digital++;
    }else{
        return digital;
    }
}

int create_dir(char *path)
{
    if(mkdir(path,0777) == -1)
    {
#if DEBUG
    printf("Mkdir fail!!\n");
#endif
        return 0;
    }
    return 1;
}

int check_dir(char *storage_path,char *file_dir)
{
    char file_path[1024];
    int result = 0;

    sprintf(file_path,"%s/%s",storage_path,file_dir);

#ifdef DEBUG
    printf("check dir [ %s ]\n",file_path);
#endif

    if (detect_storage(file_path)){
        return 1;
    }else{
        result = create_dir(file_path);
        return result;
    }
    
}

void save_file(char *file, int len, char *dir)
{
    char *storage = malloc(1024);
    char file_path[1024]={0};
    FILE *fp = NULL;
    int garbage,file_num;
    char path[NAME_LEN] = {0};

#if DEBUG
    printf("Enter file mode!!\n");
#endif       

    if (len <= 0 )
    {
#if DEBUG
        printf("Data Error !!\n");
#endif       
        free(storage);
        return;
    }
    memset(storage, 0, sizeof(storage));
    if ( usb_enable && detect_storage(usb_storage)){
#if DEBUG
    printf("Detect USB!!\n");
#endif       
        if(record == 1 && check_size(usb_storage) >= 94)
        {
            sprintf(path,"%s/%s",usb_storage,dir); 
            scan_file(path,1);
            strcpy(storage , usb_storage );
        }

        if (check_size(usb_storage) >= 95){
	    if(usb_size == 0)
	    {
                system("/bin/sh /etc/init.d/send_mail USB Size_over");
	        usb_size = 1;
	    }
        }else{
	    if(usb_size == 1)
	    {
                system("nvram replace attr alert_rule 0 usb 0");
	        usb_size = 0;
	    }
        }

        if (record == 1 || check_size(usb_storage) < 95)
           strcpy(storage , usb_storage );
    }
    if ( sd_enable && detect_storage(sd_storage) ){
#if DEBUG
    printf("Detect SD!!\n");
#endif       
        if(record == 1 && check_size(sd_storage) >= 94)
        {
            sprintf(path,"%s/%s",sd_storage,dir); 
            scan_file(path,1);
            strcpy(storage , sd_storage );
        }

        if (check_size(sd_storage) >= 95){
	    if(sd_size == 0)
	    {
                system("/bin/sh /etc/init.d/send_mail SD Size_over");
	        sd_size = 1;
	    }
        }else{
	    if(sd_size == 1)
	    {
            	system("nvram replace attr alert_rule 0 sd 0");
	    	sd_size = 0;
	    }
        }

        if (record == 1 || check_size(sd_storage) < 95)
            strcpy(storage , sd_storage );
    }
    if (!storage)
    {
#if DEBUG
    printf("NO detect any storage skip file mode!!\n");
#endif       
        free(storage);
        return;
    }
    
    sprintf(file_path,"%s/%s/%010d.txt",storage,dir,check_digital(dir));

#if DEBUG
    printf("File path : %s !!\n",file_path);
#endif

    fp = fopen(file_path,"a"); 

    if(!fp){
#if DEBUG
        printf("Write file fail!!\n");
#endif       
        free(storage);
        return;
    }
    fwrite(file,1,len,fp);

    fclose(fp);
    free(storage);
}

