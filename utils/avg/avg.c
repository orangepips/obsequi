#include <stdlib.h>
#include <stdio.h>

int
main()
{
  char *line = NULL;
  int   space = 0, len;
  int   cur_num, count = 0;
  long long sum = 0;
  char  dummy;
  
  while((len = getline(&line, &space, stdin)) >= 0){
    if(sscanf(line, "%c %d", &dummy, &cur_num) != 2) continue;
    
    sum += cur_num;
    count++;
        
    printf("%c %d -> %d\n", dummy, cur_num, sum);
  }
  
  sum = sum / count;
  
  printf("%d\n", sum);
  return 0;
}

