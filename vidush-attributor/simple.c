#include <stdio.h>
#include <stdlib.h>

typedef struct Foo {
    int  field1;
    char field2;  
    int* field3; 
} Foo;

Foo foo(int val){
     
    Foo f; 
    f.field1 = 10;
    f.field3 = &val;
    return f;
}

Foo bar(){

    Foo a; 
    return a;

} 

int main(){
    
    int a = 20;
    Foo ff = foo(a);
    return 0;
}
