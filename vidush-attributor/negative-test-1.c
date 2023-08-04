#include <stdio.h>
#include <stdlib.h>

typedef struct Foo {
    int  field1;
    char field2;  
    int* field3; 
} Foo;

Foo *foo(int val){
     
    Foo *f = (Foo*) malloc(sizeof(Foo)); 
    f->field1 = 2;
    f->field1 += 10 + val;
    return f;
}

int main(){
    
    int a = 20;
    Foo *ff = foo(a);
    return 0;
}


/*
 * ~/workdisk/git/llvm-project/build/bin/clang -Xclang -disable-O0-optnone -disable-llvm-passes -O0 -emit-llvm -S negative-test-1.c -o negative-test-1.ll
 * 
 * ~/workdisk/git/llvm-project/build/bin/opt -p attributor -debug-only=attributor negative-test-1.ll -o negative-test-1.bc &> attributor.out
 * 
 * ~/workdisk/git/llvm-project/build/bin/llvm-dis negative-test-1.bc -o negative-test-1_new.ll
 * 
 */
