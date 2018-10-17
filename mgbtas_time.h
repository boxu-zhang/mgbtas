#ifndef MGBTAS_TIME_H
#define MGBTAS_TIME_H

// for universial time
#include <sys/time.h>
#include <time.h>

typedef long unsigned int uint64_t;
typedef struct timeval mgbtas_time_t;

#if defined( __cplusplus )
extern "C" {
#endif

   // unit of mgbtas_time_t is microsecond(us) which is 1 million percent of 1 second.
   const char * mgbtas_time_str( mgbtas_time_t time_val );
   const char * mgbtas_time_str_detail( mgbtas_time_t time_val );
   mgbtas_time_t mgbtas_time_now();
   uint64_t mgbtas_time_normalize( uint64_t time_in_microseconds );
   uint64_t mgbtas_time_normalize_to_us( uint64_t time_in_normalized );
   uint64_t mgbtas_time_sub( mgbtas_time_t * end, mgbtas_time_t * begin );
   mgbtas_time_t mgbtas_time_add( mgbtas_time_t * begin, uint64_t dur_in_us );

#if defined( __cplusplus )
}
#endif

#endif // MGBTAS_TIME_H
