#include <stdio.h>
#include <stdlib.h>

typedef struct Foo{
    int a; 
    int b; 
    char c; 
}Foo;


void foo(){
    
    //Foo* f = (Foo*) malloc(sizeof(Foo));
    Foo f;
    f.a = 10; 
    printf("field a is %d\n", f.a);
    
} 


int main(){
    
    foo();
    return 0;
}


/*
 * ~/workdisk/git/llvm-project/build/bin/clang -Xclang -disable-O0-optnone -disable-llvm-passes -O0 -emit-llvm -S use_first_bytes.c -o use_first_bytes.ll
 * 
 * ~/workdisk/git/llvm-project/build/bin/opt -p attributor -debug-only=attributor use_first_bytes.ll -o use_first_bytes.bc &> attributor.out
 * 
 * ~/workdisk/git/llvm-project/build/bin/llvm-dis use_first_bytes.bc -o use_first_bytes_new.ll
 * 
 */
