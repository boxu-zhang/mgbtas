#include <stdio.h>
#include <stdlib.h>
#include "mgbtas_time.h"

// mgbtas_time_t
static char buffer[64] = { 0 };

const char * mgbtas_time_str( mgbtas_time_t time_val )
{
   struct tm * tm_info = localtime( &time_val.tv_sec );

   strftime( buffer, sizeof( buffer ), "%Y-%m-%d %H:%M:%S", tm_info );

   return buffer;
}

const char * mgbtas_time_str_detail( mgbtas_time_t time_val )
{
   char buf[64] = { 0 };

   struct tm * tm_info = localtime( &time_val.tv_sec );

   strftime( buf, sizeof( buf ), "%Y-%m-%d %H:%M:%S", tm_info );

   snprintf( buffer, sizeof( buffer ), "%s:%ld:%ld", buf, time_val.tv_usec / 1000, time_val.tv_usec % 1000 );

   return buffer;
}

mgbtas_time_t mgbtas_time_now()
{
   struct timeval tv;

   ::gettimeofday( &tv, NULL );

   return tv;
}

uint64_t mgbtas_time_normalize( uint64_t time_in_microseconds )
{
   return time_in_microseconds / 1000;
}

uint64_t mgbtas_time_normalize_to_us( uint64_t time_in_normalized )
{
   return time_in_normalized * 1000;
}

uint64_t mgbtas_time_sub( mgbtas_time_t * end, mgbtas_time_t * begin )
{
   int64_t usec = end->tv_usec - begin->tv_usec;
   int64_t sec = end->tv_sec - begin->tv_sec;

   // take care with the borrow algorithm. 'usec' is less than 0, so we should use operator '+' rather than '-'.
   if ( usec < 0 ) {
      usec = 1000 * 1000 + usec;
      sec -= 1;
   }

   return (uint64_t)sec * 1000 * 1000 + usec;
}

mgbtas_time_t mgbtas_time_add( mgbtas_time_t * begin, uint64_t dur_in_us )
{
   mgbtas_time_t t;

   t.tv_sec = begin->tv_sec + dur_in_us / 1000000;
   t.tv_usec = begin->tv_usec + dur_in_us % 1000000;

   return t;
}

