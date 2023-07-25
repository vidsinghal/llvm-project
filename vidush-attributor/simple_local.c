#include <stdio.h>
#include <stdlib.h>

typedef struct Foo {
    int field1; 
    int field2; 
    char field3; 
} Foo;


void foo(int val){

    Foo f; 
    f.field1 = 10;
    //f.field1 += 1; 
    printf("Field 1 is %d\n", f.field1 + val);
}


int main(){

    foo(10);
}

/*
 * ~/workdisk/git/llvm-project/build/bin/clang -Xclang -disable-O0-optnone -disable-llvm-passes -O0 -emit-llvm -S simple_local.c -o simple_local.ll
 * 
 * ~/workdisk/git/llvm-project/build/bin/opt -p attributor -debug-only=attributor simple_local.ll -o simple_local.bc &> attributor.out
 * 
 * ~/workdisk/git/llvm-project/build/bin/llvm-dis simple_local.bc -o simple_local_new.ll
 * 
 */
