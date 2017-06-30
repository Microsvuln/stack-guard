#include <iostream>
#include <cassert>
#include <cstring>

using namespace std;

void runTest() {
  int a;
  int b;
  char buf[10];
  char buf2[10];
  char buf3[10];
  char *ptr = buf2 + 3;

  strcpy(ptr, "test buffer");

  assert((void *)&buf2 > (void *)&a);
  assert((void *)&buf2 > (void *)&b);
  assert((void *)&buf2 > (void *)&buf);
  assert((void *)&buf2 > (void *)&buf3);

  assert((void *)&buf < (void *)&a);
  assert((void *)&buf < (void *)&b);
  assert((void *)&buf3 < (void *)&a);
  assert((void *)&buf3 < (void *)&b);
}

int main() {
  runTest();
  return 0;
}