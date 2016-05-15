#include <time.h>
#include <stdio.h>

long timeGetTime(void);

void main(void)
{
   long time;
   int i = 0;
   while(1) {
      time = timeGetTime();
      while(timeGetTime() - time < 1000)
     {
       // printf("%d\n", timeGetTime());
     } 
      printf("%d\n", i++);
   };
}

long timeGetTime()
{
   struct timespec tspec;
   if (clock_gettime(CLOCK_REALTIME, &tspec) == -1)
       return -1;
   return tspec.tv_sec * 1000 + tspec.tv_nsec/1.0e6;
}
