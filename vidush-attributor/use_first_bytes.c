#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Foo{
  int a;
  int b;
  char c; 
}Foo;

Foo *foo(int *val) {
  Foo f;
  f.a = 2 * (*val);
  *val = *val * 10;
  printf("field a is %d\n", f.a);
  return &f;
}

int main(int argc, char *argv[]) {

  int val;
  val = atoi(argv[1]);
  Foo *f = foo(&val);

  f->a += val;

  printf("val is now %d\n", val);
  printf("field a is now %d in main\n", f->a);

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
