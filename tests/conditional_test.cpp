#include <iostream>
#include <cassert>
#include <cstring>

using namespace std;

void runTest() {
  int a;
  int b;
  char buf[20];
  char buf2[20];
  char buf3[20];

  if (buf[3] > buf2[4]) {
    a = 3;
  } else {
    a = 4;
  }

  strcpy(buf2, "test buffer");

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