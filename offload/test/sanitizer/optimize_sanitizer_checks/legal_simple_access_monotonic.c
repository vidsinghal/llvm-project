#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef int IntTy;

// void printArray(int *a, int Size);
// void printArray(IntTy a[], int Size);

IntTy *foo(IntTy Size) {

  IntTy *a;
  a = (IntTy *)malloc(sizeof(IntTy) * Size);

#pragma omp target teams map(from : a[0:Size])
  {
    for (IntTy I = 0; I < Size; I++) {
      a[I] = I;
    }
  }

  // printArray(a, Size);

  return a;
}

void printArray(IntTy *a, IntTy Size) {

  for (IntTy I = 0; I < Size; I++) {
    printf("a: %d ", a[I]);
  }
}

/*void printArray(IntTy a[], int Size) {

  for (IntTy I = 0; I < Size; I++) {
    printf("a: %d ", a[I]);
  }
}*/

int main() {

  int N = 1000;
  int *a = foo(N);
  printArray(a, N);
}
