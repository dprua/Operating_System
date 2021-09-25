#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


void jerry_usage(){
	printf("\tjerry usage\n");
	printf("\n\t./jerry \"1 username\" \"fname\"  <- this user can't open file that contains fname\n");
	printf("\t./jerry \"2 username\" <- this user's process is protect of killing\n");
	printf("\t./jerry \"R-1\"  <- function 1 restore(back to nomal)\n");
	printf("\t./jerry \"R-2\"  <- function 2 restore(bact to nomal)\n");	
}


int
main(int argc, char *argv[]){

	char buf_1[500] ;
	
	size_t readSize = 0;
	
	int fd1 = open("/proc/mousehole",O_RDWR);
	int fd2 = open("/proc/mousehole",O_RDWR);
	if(argv[1] == NULL){
		jerry_usage();
		return 0;
	}
	int a = 0;
	char f[3];
	f[0] = argv[1][0];
	f[1] = ' ';
	f[2] = '\0';
		
	//If user want to show jerry usage
	if(strcmp(argv[1],"-h") == 0) {
		jerry_usage();
		return 0;
	}
	//If user want to restore function1 enter input "R-1"
	else if(strcmp(argv[1],"R-1") == 0){
		int a;
		char res1[128] = "1 -1";
		char res2[128] = "NO_FILE_NAME";

		write(fd1,res1,strlen(res1));
		write(fd2,res2,strlen(res2));	
	}
	//if user want to restore function2 enter input "R-2" 
	else if(strcmp(argv[1],"R-2") == 0){
		char res1[128] = "2 -1";
		write(fd1,res1,strlen(res1));
	}
	//if user want to show current stat
	else if(strcmp(argv[1],"S") == 0){
		char stat[300];
		read(fd1,stat,300);
                puts(stat);
	}
	//If user want to protect the specific user then enter "2 username" this format 
	else if(argv[1][0] == '2'  && argv[2] == NULL){
		printf("Function 2\n");
		char temp[500];
		int i,len;
		strcpy(temp,argv[1]);
		len = strlen(temp);
		for(i=0;i<len-2;i++){
			temp[i] = temp[i+2];
		}
		temp[len-2] = 0;
	        char uid[500] = "id -u ";
                strcat(uid,temp);
                FILE *id = popen(uid,"r");
                if(id!=NULL){
                        readSize = fread((void*)buf_1,sizeof(char),500-1,id);
                }
                pclose(id);
		buf_1[readSize] = 0;
		
		strcat(f,buf_1);
				
		write(fd1,f,strlen(f));
	}
	//if user want to block the specific file from specific user then enter "1 username" "filename" this format	
	else if(argv[1][0] == '1' && argv[2] != NULL){ 
		printf("Function 1\n");
		char temp[500];
        int i,len;
		
        strcpy(temp,argv[1]);
            	
        len = strlen(temp);
        for(i=0;i<len-2;i++){
			temp[i] = temp[i+2];
        }
        temp[len-2] = 0;
               
        char uid[500] = "id -u ";
        strcat(uid,temp);
                
        FILE *id = popen(uid,"r");
        if(id!=NULL){
            readSize = fread((void*)buf_1,sizeof(char),500-1,id);
        }
        pclose(id);
        buf_1[readSize] = 0;
               	 
        strcat(f,buf_1);
                		 
        write(fd1,f,strlen(f));		
		write(fd2,argv[2],strlen(argv[2]));
	}
	else{
		jerry_usage();
	}
	close(fd1);
	close(fd2);

	return 0;
}
