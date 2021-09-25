#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "smalloc.h" 

sm_container_t sm_head = {
	0,
	&sm_head, 
	&sm_head,
	0 
} ;

static 
void * 
_data (sm_container_ptr e)
{
	return ((void *) e) + sizeof(sm_container_t) ;
}

static 
void 
sm_container_split (sm_container_ptr hole, size_t size)
{
	sm_container_ptr remainder = (sm_container_ptr) (_data(hole) + size) ;

	remainder->dsize = hole->dsize - size - sizeof(sm_container_t) ;	
	remainder->status = Unused ;
	remainder->next = hole->next ;
	remainder->prev = hole ;
	hole->dsize = size ;
	hole->next->prev = remainder ;
	hole->next = remainder ;
}

static 
void * 
retain_more_memory (int size)
{
	sm_container_ptr hole ;
	int pagesize = getpagesize() ;
	int n_pages = 0 ;

	n_pages = (sizeof(sm_container_t) + size + sizeof(sm_container_t)) / pagesize  + 1 ;
	hole = (sm_container_ptr) sbrk(n_pages * pagesize) ;
	if (hole == 0x0)
		return 0x0 ;
	
	hole->status = Unused ;
	hole->dsize = n_pages * getpagesize() - sizeof(sm_container_t) ;
	return hole ;
}

void * 
smalloc (size_t size) 
{
	sm_container_ptr hole = 0x0, itr = 0x0 ;
	int check = -1;
	int bestfit = 0;

	for (itr = sm_head.next ; itr != &sm_head ; itr = itr->next) {
		if (itr->status == Busy)
			continue ;
		if (itr->dsize == size /*|| (size + sizeof(sm_container_t) < itr->dsize)*/) {
			hole = itr ;
			break ; 
		}
		if (itr->dsize > size + sizeof(sm_container_t)){
			if (check == -1){
				check = 0;
				bestfit = itr->dsize - (size + sizeof(sm_container_t));
				hole = itr;
				continue;
			}
			if (itr->dsize - (size + sizeof(sm_container_t)) > bestfit)
				continue;
			if (itr->dsize - (size + sizeof(sm_container_t)) < bestfit){
				bestfit = itr->dsize - (size + sizeof(sm_container_t));
				hole = itr;
				continue;
			}
		}
	}
	if (hole == 0x0) {
		hole = retain_more_memory(size) ;
		if (hole == 0x0)
			return 0x0 ;
		hole->next = &sm_head ;
		hole->prev = sm_head.prev ;
		(sm_head.prev)->next = hole ;
		sm_head.prev = hole ;
	}
	if (size < hole->dsize) 
		sm_container_split(hole, size) ;
	hole->status = Busy ;
	return _data(hole) ;
}

void 
sfree (void * p)
{
	sm_container_ptr itr ;
	for (itr = sm_head.next ; itr != &sm_head ; itr = itr->next) {
		if (p == _data(itr)) {
			itr->status = Unused ;
			break ;
		}
	}

	if(itr != sm_head.next && itr->prev->status == Unused){

		itr = itr->prev;
		itr->dsize = itr->dsize + itr->next->dsize + sizeof(sm_container_t);
		itr->next = itr->next->next;
		itr->next->prev = itr;
		
	}

	if(itr -> next != &sm_head && itr->next->status == Unused){
		
		itr->dsize = itr->dsize + itr->next->dsize + sizeof(sm_container_t);
		itr->next = itr->next->next;
		itr->next->prev = itr;

	}
}

void 
print_sm_containers ()
{
	sm_container_ptr itr ;
	int i ;

	printf("==================== sm_containers ====================\n") ;
	for (itr = sm_head.next, i = 0 ; itr != &sm_head ; itr = itr->next, i++) {
		printf("%3d:%p:%s:", i, _data(itr), itr->status == Unused ? "Unused" : "  Busy") ;
		printf("%8d:", (int) itr->dsize) ;

		int j ;
		char * s = (char *) _data(itr) ;
		for (j = 0 ; j < (itr->dsize >= 8 ? 8 : itr->dsize) ; j++) 
			printf("%02x ", s[j]) ;
		printf("\n") ;
	}
	printf("\n") ;

}

void
print_mem_uses()
{
	
	sm_container_ptr itr = 0x0;
	int total = 0 ,used = 0 ,unused = 0, b_count = 0, u_count = 0;
	for(itr = sm_head.next; itr != &sm_head ; itr = itr->next){
		if(itr->status == Busy){
			used += itr->dsize + sizeof(sm_container_t);
			b_count++;
		}
		else if(itr->status == Unused){
			unused += itr->dsize + sizeof(sm_container_t);
			u_count++;
		}
	}
	total = used + unused;
	fprintf(stderr, "  ======= Current Memory Status =======\n") ;
	fprintf(stderr,"  =====================================\n");
	fprintf(stderr,"   TOTAL ALLOCATED MEMORY : %d bytes \n   TOTAL BUSY MEMORY : %d bytes\n   TOTAL UNUSED MEMOTY : %d bytes\n",total,used,unused);
	fprintf(stderr,"  =====================================\n\n");
	
}

void *
srealloc(void * p, size_t size)
{
	sm_container_ptr itr , temp, hole;
	int free = 0,diff,check = -1;
	printf("REAL!\n");
	
	
	for (itr = sm_head.next ; itr != &sm_head ; itr = itr->next) {
		if (p == _data(itr)) {
			//itr->status = Unused ;
			break ;
		}
	}
	if (itr->dsize == size){
		return _data(itr);
	}
	if (itr->dsize > size){
		sm_container_split(itr, size) ;
		itr->status = Busy;
		itr->next->status = Unused;
		sfree(_data(itr->next));
		return _data(itr);
	}
	else{
		diff = size - itr->dsize;//500
		for(temp = itr->next; temp != &sm_head; temp = temp->next){
			if(temp->status == Busy)
				break;
			if(free >= diff){
				check = 1;
				break;
			}
			free += temp->dsize + sizeof(sm_container_t);
		}
		if(free >=diff)
			check = 1;
		if(check == 1){
			itr->next = temp;
			temp->prev = itr;
			itr->dsize += free;
			sm_container_split(itr, size) ;
			sfree(_data(itr->next));

			return _data(itr);
		}
		else{
			hole = retain_more_memory(size);
			if (hole == 0x0)
				return 0x0;
			hole->next = &sm_head ;
			hole->prev = sm_head.prev ;
			(sm_head.prev)->next = hole ;
			sm_head.prev = hole ;
			if (size < hole->dsize) 
				sm_container_split(hole, size) ;
			memcpy(_data(hole),_data(itr),itr->dsize);
			itr->status = Unused;
			hole->status = Busy;
			sfree(_data(itr));
			
			return _data(hole);

		}
	}
}

void sshrink()
{
	int free = 0;
	sm_container_ptr itr;
	for(itr = sm_head.prev; itr != &sm_head; itr = itr->prev){
		if(itr->status == Busy)
			break;
		free += itr->dsize + sizeof(sm_container_t);
		sm_head.prev = itr->prev;
		itr->prev->next = &sm_head;
	}

	free *= -1;
	sbrk(free);
}














