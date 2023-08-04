#include <stdio.h>
#include <stdlib.h>

typedef struct Foo {
    int  field1;
    char field2;  
    int* field3; 
} Foo;

Foo foo(int val){
     
    Foo f; 
    f.field1 = 2;
    f.field3 = &val;
    return f;
}

int main(){
    
    int a = 20;
    Foo ff = foo(a);
    return 0;
}



/*
 * ~/workdisk/git/llvm-project/build/bin/clang -Xclang -disable-O0-optnone -disable-llvm-passes -O0 -emit-llvm -S negative-test-2.c -o negative-test-2.ll
 * 
 * ~/workdisk/git/llvm-project/build/bin/opt -p attributor -debug-only=attributor negative-test-2.ll -o negative-test-2.bc &> attributor.out
 * 
 * ~/workdisk/git/llvm-project/build/bin/llvm-dis negative-test-2.bc -o negative-test-2_new.ll
 * 
 */
