#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <semaphore.h>
//#define N 17
#define MAX 50
#define X 10
#define P_MAX 8

typedef struct {
	sem_t filled ;
	sem_t empty ;

	pthread_mutex_t lock ;
	int ** elem ;
	int capacity ;
	int num ; 
	int front ;
	int rear ;
} bounded_buffer ;


bounded_buffer * buf = 0x0 ;
pthread_mutex_t result_lock;
pthread_mutex_t rear_lock;
pthread_mutex_t count_lock;
pthread_mutex_t read_lock;

pthread_t prod ;
pthread_t iteractive ;
pthread_t cons[P_MAX] ;


int m[MAX][MAX] ;
void setting(int *path,int *used, int *msg);
void find_prefix(int* prefix,int index);
void travel(int idx,int pi,int *used, int *path);
int *ptr_pre[38];
int flag = 1;
//int count = 0;
int read = 0;
int make = 0;
int N;
int P_N;
int fg = 0;
int ok;
int count = 0;
int process = 0;
//result variable///////
int result = 999999999;
long long result_count = 0;
char result_route[300];
////////////////////////

//consumer variable/////
int ix = 0;
int t_result[P_MAX];
char t_route[P_MAX][300];
int t_prefix[P_MAX][MAX];
int min[P_MAX];
int length[P_MAX];
int pre_length[P_MAX];
char rs[P_MAX][300];
int max_num_prefix = 1 ;

pthread_t tid[P_MAX]; //ID
int check[P_MAX];
int check_current[P_MAX];
////////////////////////

void
handler(int sig)
{	
	if(sig == SIGINT){
		
		printf("LENGTH : %d -> ROUTE : %s\n",result,result_route);
		printf("TOTAL COUNT : %lld\n",result_count);
		exit(0);
	}
}

void 
bounded_buffer_init(bounded_buffer * buf, int capacity) {
	int i,j;
	sem_init(&(buf->filled), 0, 0) ;
	sem_init(&(buf->empty), 0, capacity) ;
	pthread_mutex_init(&(buf->lock), 0x0) ;
	buf->capacity = capacity ;
	buf->elem = (int **) malloc(sizeof(int *)* P_MAX) ;
	for(i=0;i<P_MAX;i++) buf->elem[i] = (int *)malloc(sizeof(int)*N);

	buf->num = 0 ;
	buf->front = 0 ;
	buf->rear = 0 ;
}

void update_process(int x)
{
	pthread_mutex_lock(&(rear_lock)) ;
		if(x == 1)
			process++;
		else
			process--;
	pthread_mutex_unlock(&(rear_lock)) ;
}

void update_read(int x)
{
	
	pthread_mutex_lock(&(read_lock)) ;
		if(x==1){
			read++;
		}
		else{
			read--;
		}
	pthread_mutex_unlock(&(read_lock)) ;
}

int 
bounded_buffer_queue(bounded_buffer * buf, int *msg, int x) 
{
	int i;

	//printf("PRODUCER WAIT!!!!\n");
	usleep(10);
	sem_wait(&(buf->empty)) ;
	pthread_mutex_lock(&(buf->lock)) ;
		usleep(10);
		//printf("PRODUCER GO!!!!\n");
		for(i = 0 ; i < N; i++)
			buf->elem[buf->rear][i] = msg[i];

		buf->rear = (buf->rear + 1) % buf->capacity ;
		buf->num += 1 ;

		update_read(0);

	pthread_mutex_unlock(&(buf->lock)) ;
	sem_post(&(buf->filled)) ;
	return  1;
}

int * 
bounded_buffer_dequeue(bounded_buffer * buf,int room) 
{
	int * r;
	
	int i;
	
	if(read == max_num_prefix){
		return r;
	}
	//printf("CONSUMER WAIT!!!!\n");
	usleep(10);
	sem_wait(&(buf->filled)) ;
	pthread_mutex_lock(&(buf->lock)) ;
		usleep(10);
		//printf("CONSUMER GO!!!!\n");
		update_read(1);
		r = (int *)malloc(sizeof(int)*N);
		for(i = 0; i < N; i++)
			r[i] = buf->elem[buf->front][i];
		
		buf->front = (buf->front + 1) % buf->capacity ;
		buf->num -= 1 ;

	pthread_mutex_unlock(&(buf->lock)) ;
	sem_post(&(buf->empty)) ;

	return r ;
}

void update_result(int idx)
{
	int i,j;
	pthread_mutex_lock(&(result_lock)) ;
		if(result > t_result[idx]){
			result = t_result[idx];
			strcpy(result_route,t_route[idx]);
		}
	pthread_mutex_unlock(&(result_lock)) ;
}

void update(int x)
{
	
	pthread_mutex_lock(&(count_lock)) ;
		if(x == 1)
			result_count++;
		else
			result_count -= 39916800;
	pthread_mutex_unlock(&(count_lock)) ;
}

void * 
producer(void * ptr) 
{
	char msg[128] ;
	char temp[128];
	char pre[128];
	int path[MAX];
	int prefix[MAX];
	
	pthread_t tid ;
	int i , j, k;
	
	for(i=0;i<N;i++){ 
		prefix[i] = -1;
	}
	for(i=0;i<N-11;i++){ 
		prefix[i] = i;
	}
	for(i=0;i<N-11;i++){ 
		ptr_pre[i] = &prefix[i];
	}
	
	tid = pthread_self() ;
	
	while(1){
		while(fg);
		k=bounded_buffer_queue(buf, prefix, 1) ;
		if(k == 1){
			find_prefix(prefix,N-11-1);
		}
	}
	return 0x0 ;
}

void * 
consumer(void * ptr) 
{
	
	
	int * msg ; 
	int i ;
	int j = 0;
	int used[MAX];
	int path[MAX];
	
	int room = ix;
	check[room] = 0;
	check_current[room] = 0;
	min[room] = -1;
	length[room] = 0;
	t_result[room] = 0;
	
	tid[room] = pthread_self() ;

	while(1){
		msg = bounded_buffer_dequeue(buf,room) ;
		if (msg != 0x0) {
			for(i=0;i<N;i++) t_prefix[room][i] = msg[i];

			setting(path,used,msg);
			
			pre_length[room] = 0;
			for(i=0;i<N-11-1;i++){
				pre_length[room] += m[path[i]][path[i+1]];
			}

			travel(N-11,room,used,path);
			//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
			update_process(1);
			//t_result[room] = min[room] + pre_length[room];
			check[room]++;

			t_result[room] = 0;
			min[room] = -1;
			length[room] = 0;
			check_current[room] = 0;
			
			free(msg) ;

		}
		if(process == max_num_prefix) raise(SIGINT);
	}
	
}

int 
main(int argc, char *argv[]) 
{
	
	int i ,j, t, n,c;
	N = atoi(argv[1]+2);
	P_N = atoi(argv[2]);

	c = N-11;
	//printf("\n");
	for(i=0;i<c;i++)
		max_num_prefix = max_num_prefix * (N-i);
	

	buf = malloc(sizeof(bounded_buffer)) ;
	bounded_buffer_init(buf, P_N) ;
	pthread_mutex_init(&result_lock, 0x0) ;
	pthread_mutex_init(&rear_lock, 0x0) ;
	pthread_mutex_init(&count_lock, 0x0) ;
	pthread_mutex_init(&read_lock, 0x0) ;
	
	signal(SIGINT, handler);
	
	FILE * fp = fopen(argv[1], "r") ;

	for (i = 0 ; i < N ; i++) {
		for (j = 0 ; j < N ; j++) {
			fscanf(fp, "%d", &t) ;
			m[i][j] = t ;
		}
	}
	fclose(fp) ;
	//pthread_create(&iteractive, 0x0 , iteract, 0x0) ;
	pthread_create(&prod, 0x0 , producer, 0x0) ; 
	for (i = 0 ; i < P_N ; i++) {	
		pthread_create(&(cons[i]), 0x0, consumer, 0x0) ;	
		
		sleep(1);
		ix++;
	}

	
	char str[100];
	int xx,tt,ch,o=0,confirm;
	printf("=======================\n");
	printf("THE PROGRAM IS WORKING!\n");
	printf("=======================\n");
	while(1){
		if(process == max_num_prefix) raise(SIGINT);
		printf("======================\n");
		printf("PLEASE INPUT ANYTHING!\n");
		printf("======================\n");
		fgets(str,10,stdin);
		printf("====================\n");
		printf("===PLEASE WAITING===\n");
		printf("====================\n");
			if(strncmp(str, "stat",4) == 0){
				printf("CURRENT RESULT : %d -> %s\n",result,result_route);
				printf("CURRENT COUNT  : %lld\n",result_count);
			}
			else if(strncmp(str, "threads",7) == 0){
				for(i=0;i<P_N;i++){
					printf("THREAD ID : %ld\n",tid[i]);
					printf("THREAD [%ld] PROCESSED SUBTASK'S NUMS ARE %d\n",tid[i],check[i]);
					printf("THREAD [%ld] CURRENT SUBTASK'S PROCESSING NUMS ARE %d\n",tid[i],check_current[i]); 
				}
			}
			else if(strncmp(str,"num",3) == 0){						
				tt = str[4] - 48;
				if(tt>P_MAX || tt <= 0) printf("ERROR!\n");
				else if(tt == P_N) printf("NOTHING CHANGE!\n");
				else if(tt > P_N){ 
					xx = P_N; 
					ch = tt-P_N; 
					P_N = tt; 

					for(i=xx;i<P_N;i++){			
						pthread_create(&(cons[i]), 0x0, consumer, 0x0) ;
						sleep(1);
						ix++;
					}
					
					printf("THE %d'S THREAD CREATE!\n",ch);
				}
				else if(tt < P_N){
					fg = 1;
					xx = P_N; 
					ch = P_N - tt; 
					P_N = tt; 
					for(i=0;i<ch;i++){
						confirm = pthread_cancel(cons[P_N + i]);
						
						//printf("CONFIRM %d\n",confirm);						
					}
					for(i=0;i<ch;i++){
						check[P_N + i] = 0;		
						t_result[P_N + i] = 0;
						
						o = 0;	
						o = bounded_buffer_queue(buf, t_prefix[P_N+i], 2) ;					
						ix--;
						
						update(0);
						update_process(0);
						//printf("\n");
						printf("SUCCEED!\n");
					}
					fg = 0;
					
				}
				
			}
			else{
				printf("ERROR!\n");
			}				
	}
	
	while(process < max_num_prefix);
	pthread_join(prod, 0x0) ;
	for (i = 0 ; i < P_N ; i++) {
		pthread_join(cons[i], 0x0) ;
		
	}
}

void find_prefix(int* prefix,int index){
	
	int i,j,current;
	
	
	for ( i = 0 ; i < N ; i++ ) {
		
		if ( prefix[i] == index ) {
		
			current = i; //find maknae
			break;

		}

	}

	for ( i = current ; i < N ; i++ ) {

		if ( prefix[i] == -1 || prefix[i] > index ) {

			prefix[current] = -1;
			ptr_pre[index] = &prefix[i];
			prefix[i] = index;
			return ;
		}
	}

	find_prefix(prefix,index-1);
	
	for ( i = 0 ; i < current ; i++) {

		if ( prefix[i] == -1 ) {
			
			if ( *ptr_pre[index] == index )
				*ptr_pre[index] = -1;
			ptr_pre[index] = &prefix[i];
			prefix[i] = index;
			break;
		}
	}
	return ;
}

void setting(int *path,int *used, int *msg) {

	int i,t;
	int result[MAX];
	      
	for( i = 0 ; i < N ; i++ ) {
		used[i] = 0;
	}
	for ( i = 0 ; i < N-11 ; i++ ) {		
		for ( t = 0 ; t < N; t++ ) {		
				if ( msg[t] == i ) { 	
					result[i] = t;
					break;				
				}
		}
	}
	for ( i = 0 ; i < N-11 ; i++ ) {
		
		path[i] = result[i];
		used[result[i]] = 1;
	}
} 





void travel(int idx,int pi,int *used, int *path) {
	int i ;
	char route[200];
	char r[300] = {'(',};

	if (idx == N) {

		check_current[pi]++;
		update(1);
		length[pi] += m[path[N-1]][path[0]] ;
		if (min[pi] == -1 || min[pi] > length[pi]) {

			min[pi] = length[pi] ;

			
			for (i = 0 ; i < N ; i++){
				sprintf(route,"%d ", path[i]) ;
				strcat(r,route);
			}
			sprintf(route,"%d)\n", path[0]) ;
			strcat(r,route);
			strcpy(rs[pi],r);
			t_result[pi] = min[pi] + pre_length[pi];
			strcpy(t_route[pi],rs[pi]);
			update_result(pi);

			r[0] = '\0';
				
		}
		length[pi] -= m[path[N-1]][path[0]] ;
	}
	else {
		for (i = 0 ; i < N ; i++) {
			if (used[i] == 0) {
				path[idx] = i ;
				used[i] = 1 ;
				length[pi] += m[path[idx-1]][i] ;
				travel(idx+1,pi,used,path) ;
				length[pi] -= m[path[idx-1]][i] ;
				used[i] = 0 ;
			}
		}
	}
}
