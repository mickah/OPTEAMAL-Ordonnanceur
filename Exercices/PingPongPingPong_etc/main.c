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



int main ( int argc, char *argv[]){ 
        create_ctx(16384, f_pong, NULL); 
        create_ctx(16384, f_ping, NULL); 
        create_ctx(16384, f_poong, NULL);
        yield(); 
        exit(EXIT_SUCCESS); 
} 

void f_ping(void *args) 
{ 
        while(1)  
    { 
            printf("A") ; 
            yield(); 
            printf("B") ; 
            yield(); 
            printf("C") ; 
            yield(); 
        } 
} 

void f_pong(void *args) 
{ 
        while(1)  
    { 
               printf("1") ; 
            yield(); 
            printf("2") ; 
            yield(); 
            printf("3") ; 
            yield(); 
        } 
}

void f_poong(void *args) 
{ 
        while(1)  
    { 
               printf("$") ; 
            yield(); 
            printf("#") ; 
            yield(); 
            printf("@") ; 
            yield(); 
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

    if ( (!current_ctx) && (!first_ctx))/*Si aucun contextes deja crees */ 
    { 
    		/* On execute tjrs le meme contexte */
        new_ctx_s->next = new_ctx_s;
        first_ctx = new_ctx_s;
        last_ctx = first_ctx;
    } 
    else 
    { 
    		/**/
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

start_sched (void)
{
	start_hw();

/*
    sigemptyset (&set);
    sigaddset (&set, SIGALRM);
 
    sigemptyset (&(handler.sa_mask));
    handler.sa_handler = commut;
    handler.sa_flags |= SA_RESTART;
    sigaction (SIGALRM, &handler, NULL);
 
    ualarm (250000, 1000000);
 
    commut (0);
   */
}
