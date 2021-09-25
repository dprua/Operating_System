#include <stdio.h>
#include "smalloc.h"

int 
main()
{
   void *p1, *p2, *p3, *p4, *p5, *p6;

   print_sm_containers() ;

   p1 = smalloc(2000) ; 
   printf("smalloc(2000):%p\n", p1) ; 
   print_sm_containers() ;



   p2 = smalloc(2500) ; 
   printf("smalloc(2500):%p\n", p2) ; 
   print_sm_containers() ;
/*
   sfree(p1) ; 
   printf("sfree(%p)\n", p1) ; 
   print_sm_containers() ;
*/
   p3 = smalloc(3000) ; 
   printf("smalloc(3000):%p\n", p3) ; 
   print_sm_containers() ;

   p4 = smalloc(1000) ; 
   printf("smalloc(1000):%p\n", p4) ; 
   print_sm_containers() ;



   p5 = smalloc(500) ; 
   printf("smalloc(500):%p\n", p5) ; 
   print_sm_containers() ;

   sfree(p4) ; 
   printf("sfree(%p)\n", p4) ; 
   print_sm_containers() ;

   sfree(p5) ; 
   printf("sfree(%p)\n", p5) ; 
   print_sm_containers() ;

   p5 = smalloc(3500) ; 
   printf("smalloc(3500):%p\n", p5) ; 
   print_sm_containers() ;

   print_mem_uses();

   p1 = smalloc(2500) ; 
   printf("smalloc(2500):%p\n", p1) ; 
   print_sm_containers() ;

   p2 = smalloc(1000) ; 
   printf("smalloc(1000):%p\n", p2) ; 
   print_sm_containers() ;

   p2 = srealloc(p2,500) ; 
   printf("srealloc(500):%p\n", p2) ; 
   print_sm_containers() ;

   p6 = smalloc(3600) ; 
   printf("smalloc(3600):%p\n", p6) ; 
   print_sm_containers() ;

   p6 = srealloc(p6,3700) ; 
   printf("srealloc(3700):%p\n", p6) ; 
   print_sm_containers() ;

   sfree(p5) ; 
   printf("sfree(%p)\n", p5) ; 
   print_sm_containers() ;

   sfree(p3) ; 
   printf("sfree(%p)\n", p3) ; 
   print_sm_containers() ;


   p2 = srealloc(p2,4500) ; 
   printf("srealloc(4500):%p\n", p2) ; 
   print_sm_containers() ;
   

   print_mem_uses();
   printf("sshrink!\n") ; 
   sshrink();
   print_sm_containers() ;
   print_mem_uses();

   sfree(p2) ; 
   printf("sfree(%p)\n", p2) ; 
   print_sm_containers() ;


   printf("sshrink!\n") ; 
   sshrink();
   print_sm_containers() ;
   print_mem_uses();
}

