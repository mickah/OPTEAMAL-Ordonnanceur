#include <setjmp.h>
#include <stdio.h>

static jmp_buf buf;
static int i = 0;

static int cpt()
{
	int j = 0;
	if (setjmp(buf)) {
		for (j=0; j<5; j++)
			printf("coucou3");
			i++;
	} else {
		printf("coucou4");
		for (j=0; j<5; j++)
			i--;
	}
	printf("coucou2");
}

int main()
{
	int np = 0 ;
	cpt();
	if (! np++)
		longjmp(buf,~0);
	printf("i = %d\n", i );
	printf("fin\n");
}

/*
Cause de l'erreur :
"The stack context will be invalidated  if  the  function  which  called
setjmp() returns."*/
