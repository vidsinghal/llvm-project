#include <stdio.h>
#include <stdlib.h>


struct Product {
	int *a; 
	int *b; 
	int *c;
};

Product *foo(){

	int *a; 
	int *b; 
	int *c; 
	int N = 1000000; 

	a = (int*) malloc(sizeof(int)*N);
	b = (int*) malloc(sizeof(int)*N);
	c = (int*) malloc(sizeof(int)*N);

	for (int i = 0; i < N; i++){
		a[i] = i;
		b[i] = i;
		c[i] = a[i] + b[i];
	}

	Product *P = (Product*) malloc(sizeof(Product));
	P->a = a;
	P->b = b;
	P->c = c;
	return P;
}
