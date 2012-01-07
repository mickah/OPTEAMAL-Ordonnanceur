#include <setjmp.h>
#include <stdio.h>
static jmp_buf buf;

static int mul(int depth)
{
	int i=0;
	if (depth == 0 && setjmp(buf)) {
		printf("Return final depth: %d\n",depth);
		return 0;
	}
	else{
	
		switch (scanf("%d", &i)) {
		case -1 :
			printf("Ok\n");
			return 1; /* neutral element */
		case 0 :
			printf("Ok\n");
			return mul(depth+1); /* erroneous read */
		case 1 :
			printf("Ok %d\n",i);
			if (i){
				if (i == 1){
					printf("Return depth: %d\n",depth);
					return 1;
				}
				else
				{
					printf("Return depth: %d\n",depth);
					return i * mul(depth+1);
				}		
			}
			else
			{
				printf("Return depth: %d\n",depth);
				longjmp(buf,~0);
				return 0;
			}
		}
		
	}
}

int main()
{
	int product;
	printf("A list of int, please\n");
	product = mul(0);
	printf("product = %d\n", product);
}

