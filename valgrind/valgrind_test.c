/* gcc -g -Wall -ansi -pedantic -o valgrind_test.exe valgrind_test.c */

#include <stdlib.h>
#include <stdio.h>

#include <windows.h>

int main()
{
  HANDLE hDefaultProcessHeap;
  void *data;

  printf ("process launched...\n");

  hDefaultProcessHeap = GetProcessHeap();
  data = HeapAlloc(hDefaultProcessHeap, 0, 10);
  if (!data)
    {
      printf ("no heap alloc...\n");
      return -1;
    }
  HeapFree(hDefaultProcessHeap, 0, data);

  data = malloc(10);
  if (!data)
    {
      printf ("no malloc...\n");
      return -1;
    }
  free(data);

  printf ("process finished...\n");

  return 0;
}
