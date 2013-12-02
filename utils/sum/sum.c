#include <stdlib.h>
#include <stdio.h>

int
main()
{
  char token[100];
  int  sum = 0;
    
  while(scanf("%s", token) == 1){
    sum += atoi(token);
  }
  
  return sum;
}

