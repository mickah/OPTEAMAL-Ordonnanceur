#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include "pingpong.h"
#include "hw.h"
  

#define RETURN_SUCCESS 0
#define RETURN_FAILURE 1
 

struct sem_s *semaphore;
int main ( int argc, char *argv[]){
	
	printf("START\n") ;
	printf("START\n") ;
	
	semaphore = (struct sem_s *)malloc(sizeof(struct sem_s));
	printf("Before init") ;
	sem_init(semaphore, 1);
	printf("After init") ;
	
	//create_ctx(16384, f_pong, NULL); 
	create_ctx(16384, f_ping, NULL); 
	//create_ctx(16384, f_poong, NULL); 
	start_sched(); 
	exit(EXIT_SUCCESS); 
} 

void f_ping(void *args) 
{ 
    while(1)  
    { 
      printf("A") ; 
      printf("B") ;
      //sem_up(semaphore);
      printf("C") ;
    } 
} 

void f_pong(void *args) 
{ 
    while(1)  
    { 
      printf("1") ;  
      printf("2") ;  
      printf("3") ;
    } 
}

void f_poong(void *args) 
{ 
    while(1)  
    { 
      printf("$") ;  
      printf("#") ; 
      printf("@") ;
    } 
}

/* Initialisation du contexte d'execution associee a f*/
int init_ctx(struct ctx_s *ctx, int stack_size, func_t f, void *args) 
{ 	
    ctx->stack = malloc(stack_size); 
    if (!ctx->stack) return 0; 
    ctx->esp = (void *)((unsigned char*)ctx->stack + stack_size - 4    ); 
    ctx->ebp = (void *)((unsigned char*)ctx->stack + stack_size - 4    ); 
    ctx->f = f; 
    ctx->args = args; 
    ctx->etat=INITIAL; 
    ctx->ctx_magic = CTX_MAGIC;
} 


void switch_to_ctx(struct ctx_s *ctx) 
{ 
    assert(ctx->ctx_magic==CTX_MAGIC); 
    assert(ctx->etat!=FINI); 
    if (current_ctx) /* Si il y a un contexte courant */
        asm("movl %%esp, %0" "\n" "movl %%ebp, %1" 
            :"=r"(current_ctx->esp), 
            "=r"(current_ctx->ebp)  
        );
    current_ctx=ctx; 
    asm("movl %0, %%esp" "\n" "movl %1, %%ebp" 
        : 
        :"r"(current_ctx->esp), 
         "r"(current_ctx->ebp) 
    ); 
    if (current_ctx->etat == INITIAL) 
    { 
        start_current_ctx(); 
    } 
}  

void start_current_ctx(void) 
{ 
    current_ctx->etat=ACTIF;
    //irq_enable(); 
    current_ctx->f(current_ctx->args);   
    current_ctx->etat=FINI; 
    free(current_ctx->stack); 
    exit(EXIT_SUCCESS); 
} 

int create_ctx(int stack_size, func_t f, void *args) 
{
    struct ctx_s *new_ctx_s = (struct ctx_s *)malloc(sizeof(struct ctx_s));
    if (! new_ctx_s) return 0;

    init_ctx(new_ctx_s, stack_size, f, args); 

    if ( (!current_ctx) && (!first_ctx))/*Si aucun contexte deja cree */ 
    { 
    		/* On execute tjrs le meme contexte */
 	      /*printf(" first:%d",first_ctx);*/
 	      /*return 32;*/
 	      /*printf(" new:%d",new_ctx_s);*/
 	      new_ctx_s->next = new_ctx_s;
 	      first_ctx = new_ctx_s;
        last_ctx = first_ctx;
    } 
    else 
    {
        new_ctx_s->next = first_ctx; 
        last_ctx->next = new_ctx_s;
        last_ctx = new_ctx_s;
    } 
} 

void yield(void) 
{ 
    if (current_ctx) /* Si on a un contexte courant */
        switch_to_ctx(current_ctx->next); 
    else 
        switch_to_ctx(first_ctx);
} 

void start_sched (void)
{
	start_hw();
	irq_disable();
	setup_irq(TIMER_IRQ, ordonnanceur);
	ordonnanceur();
	printf("start_sched\n");
}

/* Ordonnaceur basic (on prend betement les fonctions a la suite */
void ordonnanceur(void)
{
	irq_disable();
	struct ctx_s * nextCtx = NULL;
	
	if ( first_ctx )// Si contextes dans la liste active 
	{
		if (current_ctx) // Si on a un contexte courant
      {
      	nextCtx = current_ctx->next;
      	last_ctx = current_ctx;
      }
    else
    {
    	nextCtx = first_ctx;
    }

    switch_to_ctx(nextCtx);   
	}
	
/*	if(listeSem) //Si il y a des semaphores
	{
		struct listeSem_s *ite = listeSem;
		while(ite->current)
		{
			if(ite->current->jeton > 0)//on peut prendre
			{
				ite->current->jeton--;
				
				// mise a jour de la liste des ctx en attent sur sem
				if( ite->next == NULL)
					ite->current->lastWaitingCtx = NULL; 
				
				// on insert la tache a la fin des taches actives
				if (nextCtx){ //Si il y a un contexte actif en attente
				ite->current->waitingCtx->next = nextCtx;
				last_ctx->next = ite->current->waitingCtx;
				ite->current->waitingCtx = ite->current->waitingCtx->next;		
				}else{ //Si c'est la seule		
					first_ctx = ite->current->waitingCtx;
        	last_ctx = ite->current->waitingCtx;
					current_ctx = ite->current->waitingCtx;
				}
			} 
			ite = ite->next; 			
		}
		
	}
*/	
//	irq_enable();
	
}

void sem_init(struct sem_s *sem, unsigned int val){
	struct listeSem_s *new_listeSem_s;
	sem->jeton = val;
	sem->waitingCtx = NULL;
	sem->lastWaitingCtx = NULL;
	new_listeSem_s = (struct listeSem_s *)malloc(sizeof(struct listeSem_s));
	new_listeSem_s->current = sem;
	new_listeSem_s->next = NULL;
	
	if(listeSem){ /* Si ce n'est pas le premier */
		lastListeSem->next = new_listeSem_s;
	}
	else{
		listeSem = NULL; new_listeSem_s;
	}
	lastListeSem = new_listeSem_s;

}


void sem_up(struct sem_s *sem){

	struct ctx_s * next_of_current_ctx = current_ctx->next;
/*	
	// On insert ctx dans la liste d'attente du semaphore 
	if(!sem->lastWaitingCtx) // Si aucun contexte en attente
	{
		sem->waitingCtx = current_ctx;
		sem->waitingCtx->next = NULL;
		sem->lastWaitingCtx = sem->waitingCtx;
	}else{
		//On rajoute le ctx en fin de liste d'attente
		sem->lastWaitingCtx->next = current_ctx;
		sem->lastWaitingCtx = current_ctx;
		sem->lastWaitingCtx->next = NULL;
	}
*/
	// On retire ctx de la liste des ctx actifs
	if(current_ctx == next_of_current_ctx)//Si il n'y avait qu'une tache active
	{
			first_ctx = NULL;
			current_ctx = NULL; 
			last_ctx = NULL;
		
	}else{ //Si il y a d'autres taches actives
		if( current_ctx == first_ctx ) //Si c'est le premier à retirer
		{
			first_ctx = next_of_current_ctx;
			current_ctx = next_of_current_ctx; 
		}else{
			if(current_ctx == last_ctx){ //Si c'est le dernier à retirer
				current_ctx = first_ctx;
			}else{ //Si celui à retirer est au milieu
				current_ctx = next_of_current_ctx;
			}
		}	
	
	}
	

		
	ordonnanceur();

}









