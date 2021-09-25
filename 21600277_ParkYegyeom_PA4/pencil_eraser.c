#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

pthread_mutex_t pencil[3];
pthread_mutex_t eraser[3];
pthread_mutex_t temp;

int a[3];

void 
wating() {
    usleep(rand() % 1000000) ;
}

void
swating()
{
	long long tid;
	
	tid = pthread_self();

	printf("[%lld] is using pencil and eraser!!\n",tid);	
	usleep(rand() % 1000) ;
}



void
get(int index)
{

	long long tid;
	
	tid = pthread_self();
	pthread_mutex_lock(&temp);	
	pthread_mutex_lock(&pencil[index]);
	printf("[%lld] get %d th's pencil!\n",tid,index);
	pthread_mutex_lock(&eraser[index]);
	printf("[%lld] get %d th's eraser!\n",tid,index);
	pthread_mutex_unlock(&temp);
}

void
stop(int index)
{
	long long tid;
	tid = pthread_self();
	usleep(1000);	

	pthread_mutex_lock(&temp);
	
	pthread_mutex_unlock(&eraser[index]);
	printf("[%lld]'work finish!!\n",tid);

	pthread_mutex_unlock(&pencil[index]);
	pthread_mutex_unlock(&temp);
	
}

void *
want () {
	
	int r = rand()%3 ;	

	int i,index;

	for(i = 0 ; i < 2 ; i++) {
		index = rand()%3;
		wating();
		get(index) ;
		swating() ;
		stop(index) ;
	}
}

int
main(int argc, char *argv[])
{
	pthread_t people[5] ;
	int i ;

	srand(time(0x0)) ;

	for(i=0;i<3;i++){
		a[i] = 0;
	}
	pthread_mutex_init(&temp, 0x0) ;
	for(i=0;i<3;i++){
		pthread_mutex_init(&(pencil[i]), 0x0) ;
		pthread_mutex_init(&(eraser[i]), 0x0) ;
	}

	for (i = 0 ; i < 5; i++) {
	   pthread_create(&(people[i]), 0x0, want, 0x0) ;
	}

	for (i = 0 ; i < 5 ; i++) {
	   pthread_join(people[i], 0x0) ;
	}
	exit(0) ;
}
