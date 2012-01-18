#ifndef _PINGPONG_H_ 
#define _PINGPONG_H_ 

#define EXIT_SUCCESS 0 
#define CTX_MAGIC 0xDEADBEEF

enum ctx_state_e {FINI, ACTIF, ATTENTE, INITIAL}; 

typedef void (func_t)(void *); 

struct ctx_s { 
    void *esp; 
    void *ebp; 
    void *args; 
    void *stack; 
    enum ctx_state_e etat; 
    func_t *f; 
    unsigned int ctx_magic; 
    struct ctx_s *next;
};

struct ctx_s *first_ctx = NULL;
struct ctx_s *current_ctx = NULL;
struct ctx_s *prev_ctx = NULL;
struct ctx_s *last_ctx = NULL;

struct sem_s {
    int jeton;
    struct ctx_s *waitingCtx;
    struct ctx_s *lastWaitingCtx;
};

void f_ping(void *arg); 
void f_pong(void *arg);
void f_poong(void *arg);

void start_sched (void);
void switch_to_ctx(struct ctx_s *ctx); 
void start_current_ctx(void); 
void yield(void);
void ordonnanceur(void);

int init_ctx(struct ctx_s *ctx, int stack_size, func_t f, void *args); 
int create_ctx(int stack_size, func_t f, void* args); 

void sem_init(struct sem_s *sem, unsigned int val);
void sem_up(struct sem_s *sem);
void sem_down(struct sem_s *sem);

void remove_Current_ctx();

#endif 

