#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define T_NUM 10
#define MU_NUM 10
#define VER_NUM T_NUM + MU_NUM 

typedef struct{
	long long t_id;
	char *ptr;
	int num;
	struct Node *(need[VER_NUM]);
}Node;

typedef struct{
	long long t_id;
	char *ptr;
	int sign;
}Package;

Node graph[VER_NUM];
int t_count = 0;
int m_count = T_NUM;

int tnode_add(long long tid)
{
	graph[t_count].t_id = tid;
	graph[t_count].num = t_count;
	return t_count++;
}

int tnode_find(long long tid)
{
	int i,id;
	for(i=0;i<t_count;i++)
		if(graph[i].t_id == tid) return i;
	
	id = tnode_add(tid);

	return id;
}

int mnode_add(char *ptr)
{
	graph[m_count].ptr = ptr;
	graph[m_count].num = m_count;
	return m_count++;
}


int mnode_find(char *ptr)
{
	int i,id;
	for(i=T_NUM;i<m_count;i++)
		if(graph[i].ptr == ptr) return i;
			
	id = mnode_add(ptr);

	return id;
}

void edge_create(long long tid, char *ptr)
{
	int t_node,mu_node;

	t_node = tnode_find(tid);
	mu_node = mnode_find(ptr);

	graph[t_node].need[mu_node] = &graph[mu_node];
}

void edge_delete(long long tid, char *ptr)
{
	int t_node,mu_node;

	t_node = tnode_find(tid);
	mu_node = mnode_find(ptr);

	graph[mu_node].need[t_node] = 0x0;
}

void edge_change(long long tid, char *ptr)
{
	int t_node,mu_node;

	t_node = tnode_find(tid);
	mu_node = mnode_find(ptr);
	
	graph[t_node].need[mu_node] = 0x0;
	graph[mu_node].need[t_node] = &graph[t_node];
}

int checking(int index, int *checked)
{
	int i;

	for(i=0;i<VER_NUM;i++){
		if(graph[index].need[i] != 0x0){
			if(checked[i] == 1) return 1;
			checked[i] = 1;
			return checking(i, checked);
		}
	}
	return 0;
}

int confirm_cycle(long long tid)
{
	int t_node;
	int checked[VER_NUM];

	for(int i = 0 ; i < VER_NUM; i++) checked[i] = 0;
		
	t_node = tnode_find(tid);
	
	checked[t_node] = 1;

	return checking(t_node,checked);
}


int main()
{
	int fd = open(".ddtrace", O_RDONLY | O_SYNC) ;
	int i,j;
	for(i=0;i<VER_NUM;i++){
		graph[i].t_id = 0;
		graph[i].ptr = 0x0;
		for(j=0;j<VER_NUM;j++){
			graph[i].need[j] = 0x0;
		}
	}
	while(1){
		Package p;
		int k;
		char c;
		k = read(fd, &p, sizeof(p));
		if(k != 0){
			if(p.sign == 0){//unlock
				edge_delete(p.t_id,p.ptr);
			}
			else if(p.sign == 1){//lock request
				edge_create(p.t_id,p.ptr);
				if(confirm_cycle(p.t_id) == 1){
					printf("\n\n\t\tALERT!! DEADLOCK DETECT!!\n\n");
					printf("\t\tTHREAD ID --------> [%lld]\n\n",p.t_id);
					printf("\t\tMUTEX ADDRESS ----> [%p]\n\n",p.ptr);
					usleep(100);
					close(fd) ;
					for(i=0;i<VER_NUM;i++){
						graph[i].t_id = 0;
						graph[i].ptr = 0x0;
						for(j=0;j<VER_NUM;j++){
							graph[i].need[j] = 0x0;
						}
					}
					t_count = 0;
					m_count = T_NUM;
					printf("\n\n\t\tDO YOU WANT TO RESTART? PLEASE INPUT (Y / N) : ");
					scanf("%c",&c);
					if(c == 'N' || c == 'n'){
						printf("\n\n\t\t\t====================\n");
						printf("\t\t\t==\tBYE!!\t  ==\n");
						printf("\t\t\t====================\n\n\n");
						exit(0);
					}
					else{
						fd = open(".ddtrace", O_RDONLY | O_SYNC) ;
						c = getchar();
						
					}
					printf("\n\n\n");
						
				}
				
			}
			else{//lock success
				edge_change(p.t_id,p.ptr);
			}
		}

	}
	close(fd) ;
	return 0 ;
}
