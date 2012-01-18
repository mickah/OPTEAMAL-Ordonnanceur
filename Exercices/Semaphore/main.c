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
	printf("Before init\n") ;
	sem_init(semaphore, 1);
	printf("After init\n") ;
	
	create_ctx(16384, f_pong, NULL);
	create_ctx(16384, f_ping, NULL);
	create_ctx(16384, f_poong, NULL);
	start_sched(); 
	exit(EXIT_SUCCESS); 
} 

void f_ping(void *args) 
{ 
	int i =0;
	int j =0;
    while(1)
    { 
      //printf("A\n") ;
      //printf("B\n") ;
      //printf("C\n") ;
      i++;
      if(i>994000){
    	  i=0;
    	  printf("sem_down\n") ;
    	  sem_down(semaphore);
          if(j>3){
        	  j=0;
          }
          j++;

      }
    }
    printf("FINI\n");
} 

void f_pong(void *args) 
{ 
  while(1)
  { 
    //printf("1\n") ;
    //printf("2\n") ;
    printf("sem_up 1\n") ;
    sem_up(semaphore);
    //printf("3\n") ;
    
  } 
}

void f_poong(void *args) 
{ 
    while(1)  
    { 
      printf("sem_up 2\n") ;
      sem_up(semaphore);
      //printf("$\n") ;
      //printf("#\n") ;
      //printf("@\n") ;
    } 
}

void f_idle(void *args)//TODO
{
    while(1)
    {
      printf("idle\n") ;
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

    return 0;
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
    irq_enable();
    current_ctx->f(current_ctx->args);   
    current_ctx->etat=FINI; 
    free(current_ctx->stack);
    remove_Current_ctx();
    ordonnanceur();
    exit(EXIT_SUCCESS); 
} 

int create_ctx(int stack_size, func_t f, void *args) 
{
    struct ctx_s *new_ctx_s = (struct ctx_s *)malloc(sizeof(struct ctx_s));
    if (! new_ctx_s) return 0;

    init_ctx(new_ctx_s, stack_size, f, args); 

    if ( (!current_ctx) && (!first_ctx))/*Si aucun contexte deja cree */ 
    { 
		new_ctx_s->next = new_ctx_s;
		first_ctx = new_ctx_s;
		last_ctx = first_ctx;
		prev_ctx=NULL;
    } 
    else 
    {
        new_ctx_s->next = first_ctx; 
        last_ctx->next = new_ctx_s;
        last_ctx = new_ctx_s;
    }

    return 0;
} 

void yield(void) 
{ 
	ordonnanceur();
	/*
    if (current_ctx) // Si on a un contexte courant
        switch_to_ctx(current_ctx->next); 
    else 
        switch_to_ctx(first_ctx);
    */
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


	struct ctx_s * nextCtx = NULL;
	irq_disable();
	printf("----------------------------------------------------------Ordonnancement\n");
	
	if ( first_ctx )// Si contextes dans la liste active 
	{
	if (current_ctx) // Si on a un contexte courant
	  {
			switch(current_ctx->etat){
				case INITIAL:

				case ACTIF:
					nextCtx = current_ctx->next;
					prev_ctx=current_ctx;
					break;
				case ATTENTE:
					nextCtx = prev_ctx->next;
					break;
				case FINI:
					nextCtx = prev_ctx->next;//TODO
					break;
				default:
					break;
			}
	  }
    else
    {
    	nextCtx = first_ctx;
    }
	if(nextCtx != current_ctx)
		switch_to_ctx(nextCtx);
	}
	irq_enable();
}

void sem_init(struct sem_s *sem, unsigned int val){
	sem->jeton = val;
	sem->waitingCtx = NULL;
	sem->lastWaitingCtx = NULL;
}

//On prend un jeton
void sem_up(struct sem_s *sem){

	irq_disable();
	
	if(sem->jeton > 0){//Si un jeton est disponible
		sem->jeton--;
	}else{ //Mise en attente de la tache
		remove_Current_ctx();
		current_ctx->etat = ATTENTE;

		// On insert ctx dans la liste d'attente sur le semaphore
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
		ordonnanceur();
	}
}

// Donner un jeton
void sem_down(struct sem_s *sem){
	irq_disable();
	sem->jeton++;
	
	while(sem->jeton > 0 && sem->waitingCtx) //une tache peut prendre un jeton
	{
		sem->jeton--;
		sem->waitingCtx->etat = ACTIF;
		
		// mise a jour de la liste des ctx en attente sur sem
		if( sem->waitingCtx->next == NULL)
			sem->lastWaitingCtx = NULL;
				
		// on insert la tache a la fin des taches actives
		last_ctx->next = sem->waitingCtx;
		sem->waitingCtx = sem->waitingCtx->next;
		last_ctx = last_ctx->next;
		last_ctx->next = first_ctx;			
	}
	irq_enable();
}

// On supprimme le contexte courant de la liste chainée des taches actives
void remove_Current_ctx(){
	struct ctx_s * next_of_current_ctx = current_ctx->next;
	// On retire ctx de la liste des ctx actifs
	if(current_ctx == next_of_current_ctx)//Si il n'y avait qu'une tache active (useless with idle)
	{
			//useless with idle
	}else{ //Si il y a d'autres taches actives
		if( current_ctx == first_ctx ) //Si c'est le premier à retirer de la liste des taches actives
		{
			if(current_ctx == last_ctx){ //Si c'est aussi le dernier aussi à retirer
				//useless with idle
			}else{
				first_ctx = next_of_current_ctx;
				last_ctx->next = first_ctx;
				prev_ctx = last_ctx;
			}
		}else{
			if(current_ctx == last_ctx){ //Si c'est le dernier à retirer et pas le premier
				prev_ctx->next = first_ctx;
				last_ctx = prev_ctx;

			}else{ //Si celui à retirer est au milieu
				prev_ctx->next = next_of_current_ctx;
			}
		}
	}
}










