#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

int (*pthread_mutex_lock_cp)(pthread_mutex_t *mutex);
int (*pthread_mutex_unlock_cp)(pthread_mutex_t *mutex);

typedef struct{
	long long t_id;
	char *ptr;
	int sign;
}Package;

int pthread_mutex_lock(pthread_mutex_t *mutex){

	pthread_mutex_lock_cp = dlsym(RTLD_NEXT, "pthread_mutex_lock") ;

	int fd = open(".ddtrace", O_WRONLY | O_SYNC) ;
	Package p,k;
	p.t_id = pthread_self();
	p.ptr = mutex;
	p.sign = 1;
	write(fd, &p, sizeof(p)) ;
	pthread_mutex_lock_cp(mutex);
	k.t_id = pthread_self();
	k.ptr = mutex;
	k.sign = 2;
	write(fd, &k, sizeof(k));
	close(fd) ;
	return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex){

	pthread_mutex_unlock_cp = dlsym(RTLD_NEXT, "pthread_mutex_unlock") ;
	
	int fd = open(".ddtrace", O_WRONLY | O_SYNC) ;

	Package p;
	p.t_id = pthread_self();
	p.ptr = mutex;
	p.sign = 0;
	write(fd, &p, sizeof(p)) ;
	
	close(fd) ;
	pthread_mutex_unlock_cp(mutex);
	return 0;
}
