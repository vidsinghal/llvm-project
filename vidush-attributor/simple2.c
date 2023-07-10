#include <stdio.h>
#include <stdlib.h>

typedef struct Foo {
    int  field1;
    char field2;  
    int* field3; 
} Foo;

int main (){

    int *C = malloc(sizeof(Foo));
    int * z  = (int*) C + 3;
    *z = 42;
    return *z;

}