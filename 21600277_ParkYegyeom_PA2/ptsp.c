#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>

#define childnum 12
#define MAX 50
void find_prefix(int index);
void setting();
void settingp();
void travel(int start);
void _travel(int idx);

int m[MAX][MAX] ;
int path[MAX] ;
int used[MAX] ;
int length = 0 ;
int min = -1 ;
int prefix[MAX];
int *ptr[38];
int pre_length = 0;
int count = 0;
char rs[300];
int num = 0;
int a = 0;
int processnum=0;
pid_t parent;
int pipes[childnum][2];
int flag = 1;
int arr[childnum] = {0,};
int temp;
int final = -1;
char final_route[300];
long long final_count = 0;
int process_num;
int N;
int COUNT;
int max_prefix = 1;

void
handler(int sig)
{
	pid_t child ;
	int exitcode,i,k,ix,j;
	int count;
	char rou[300];
	
	if(sig == SIGCHLD){
		
		while((child = wait(&exitcode))>0) {
			for( i = 0 ; i < process_num ; i++ ){
				if(arr[i] == child){
					ix = i;
					
					close(pipes[ix][1]);
					read(pipes[ix][0],&k,sizeof(k));
					read(pipes[ix][0],rou,300);
					read(pipes[ix][0],&count,sizeof(count));
					close(pipes[ix][0]);
					processnum--;
					arr[ix] = 0;
					
					final_count += count;
					if(final == -1 || k < final){
						final = k;
						strcpy(final_route,rou);			
					}
					
					break;
				}
			}
		}
			
	}
	else if(sig == SIGINT){
		
		pid_t child;
		int exitcode;

		if(parent == getpid()){
			
			raise(SIGCHLD);
			printf("min : %d\n",final);
			printf("route : %s\n",final_route);
			printf("count : %lld\n",final_count);				
			
			exit(0);
		}
		else{
			int a;
			child = getpid();
			a = min+pre_length;
 
			close(pipes[temp][0]);
			write(pipes[temp][1],&a,sizeof(a));
			write(pipes[temp][1],rs,sizeof(rs));
			write(pipes[temp][1],&num,sizeof(num));
			close(pipes[temp][1]);
			
			exit(0);
		}
	}
}

int
main(int argc, char *argv[])
{
	pid_t pids[childnum],pid;

	parent = getpid();
	int n,i, j, t,terminal_count=0;

	n = atoi(argv[1]+2);
	process_num = atoi(argv[2]);

	N = n;
	if(N < 13 || N > 50){
		printf("This program can't solve this problem\n\n");
		return 0;
	}
	if(process_num < 1 || process_num > 12){
		printf("Process is too much!!\n\n");
		return 0;
	}
	count = n-12;
	COUNT = count;

	signal(SIGINT, handler) ; 
	signal(SIGCHLD, handler) ;


	FILE * fp = fopen(argv[1], "r") ;

	for (i = 0 ; i < N ; i++) {
		for (j = 0 ; j < N ; j++) {
			fscanf(fp, "%d", &t) ;
			m[i][j] = t ;
		}
	}
	fclose(fp) ;
	

	for(i=0;i<N;i++){ 
		prefix[i] = -1;
	}
	for(i=0;i<count;i++){ 
		path[i] = i;
		used[i] = 1;
		prefix[i] = i;
	}
	for(i=0;i<count;i++){ 
		ptr[i] = &prefix[i];
	}
	for(i=0;i<count-1;i++){
		pre_length += m[prefix[i]][prefix[i+1]];
	}
	
	for(i=0;i<count;i++)
		max_prefix *= (N-i);
	
	while(flag && terminal_count<max_prefix){
		
		while(processnum<process_num && flag == 1 && terminal_count<max_prefix)
		{		
			for(i=0;i<process_num;i++){
				if(arr[i] == 0){
					temp = i;
					break;
				}
			}
			pipe(pipes[temp]);
			pids[temp] = fork();
			arr[temp] = (int)pids[temp];

			if(pids[temp]<0){
				printf("error");
				return -1;
			}
			else if(pids[temp]==0)
			{
				pid_t pid = getpid();
				setting();

				travel(count);
				int a = min+pre_length;
				close(pipes[temp][0]);
				write(pipes[temp][1],&a,sizeof(a));
				write(pipes[temp][1],rs,sizeof(rs));
				write(pipes[temp][1],&num,sizeof(num));
				close(pipes[temp][1]);	
				sleep(3);
				exit(0);
			}		
			else{
				
				terminal_count++;
				processnum++;
				num = 0;			
				find_prefix(count-1);
								
			}
			
			sleep(1);
		
		}
	
	}
	kill(parent,SIGINT);
} 

void _travel(int idx) {
	int i ;
	char route[200];
	char r[300] = {'(',};

	if (idx == N) {
		num++;
		length += m[path[N-1]][path[0]] ;
		if (min == -1 || min > length) {
			min = length ;

			
				
			for (i = 0 ; i < N ; i++){
				sprintf(route,"%d ", path[i]) ;
				strcat(r,route);
			}
			sprintf(route,"%d)\n", path[0]) ;
			strcat(r,route);
			strcpy(rs,r);
			
			r[0] = '\0';
				
		}
		length -= m[path[N-1]][path[0]] ;
	}
	else {
		for (i = 0 ; i < N ; i++) {
			if (used[i] == 0) {
				path[idx] = i ;
				used[i] = 1 ;
				length += m[path[idx-1]][i] ;
				_travel(idx+1) ;
				length -= m[path[idx-1]][i] ;
				used[i] = 0 ;
			}
		}
	}
}



void travel(int start) {

	_travel(start) ;

}

void find_prefix(int index){
	
	int i,j,current;
	
	
	for ( i = 0 ; i < N ; i++ ) {
		
		if ( prefix[i] == index ) {
		
			current = i;
			break;

		}

	}

	for ( i = current ; i < N ; i++ ) {

		if ( prefix[i] == -1 || prefix[i] > index ) {

			prefix[current] = -1;
			ptr[index] = &prefix[i];
			prefix[i] = index;
			return ;
		}
	}

	find_prefix(index-1);
	
	for ( i = 0 ; i < current ; i++) {

		if ( prefix[i] == -1 ) {
			
			if ( *ptr[index] == index )
				*ptr[index] = -1;
			ptr[index] = &prefix[i];
			prefix[i] = index;
			break;
		}
	}
	return;
}

void setting() {

	int i,t;
	int result[MAX];	       

	for( i = 0 ; i < N ; i++ ) {
		used[i] = 0;
	}

	for ( i = 0 ; i < COUNT ; i++ ) {
		
		for ( t = 0 ; t < N; t++ ) {
			
				if ( prefix[t] == i ) { 	

					result[i] = t;
					break;
				
				}
		}
	}
	for ( i = 0 ; i < COUNT ; i++ ) {
		
		path[i] = result[i];
		used[result[i]] = 1;
	}
	pre_length = 0;
	for(i=0;i<COUNT-1;i++){
		pre_length += m[path[i]][path[i+1]];
	}
	
} 





