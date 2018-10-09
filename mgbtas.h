#ifndef XFLOW_METER_H
#define XFLOW_METER_H

// for universial time
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

typedef long int uint64_t;
typedef struct timeval universal_time_t;

#if defined( __cplusplus )
extern "C" {
#endif

   const char * universal_time_str( universal_time_t time_val );
   const char * universal_time_str_detail( universal_time_t time_val );
   universal_time_t universal_time_now();
   uint64_t universal_time_normalize( uint64_t time_in_microseconds );
   uint64_t universal_time_normalize_to_us( uint64_t time_in_normalized );
   uint64_t universal_time_sub( universal_time_t * end, universal_time_t * begin );
   universal_time_t universal_time_add( universal_time_t * begin, uint64_t dur_in_ms );

#if defined( __cplusplus )
}
#endif

// for mgbtas
#ifndef _countof
#define _countof( ary ) (sizeof((ary)) / sizeof((ary)[0]))
#endif 

// Default rnd duration is 1 hour, ie. 3600s, you can use short value for test, like 30
// Default rnd count is 24 which means that the analyzer only keeps data in one day.
#if !defined( MGBTAS_DEFAULT_ROUND )
#define MGBTAS_DEFAULT_ROUND           (60)
#endif

#if !defined( MGBTAS_MAXIMUM_ROUNDS )
#define MGBTAS_MAXIMUM_ROUNDS          (60)
#endif 

// Define trajectory types
#define DEFAULT_TRAJECTORY             0

#define EXECUTION                      DEFAULT_TRAJECTORY
#define LATENCY                        1
#define PAYLOAD                        2
#define FULLTIME                       3

#define MAX_TRAJECTORY                 (FULLTIME + 1)

// all time unit is in millisecond
typedef struct _mgbtas_bullet_t
{
   int                                 payload_length;
   universal_time_t                    recieved_time;
   universal_time_t                    start_time;
   universal_time_t                    finished_time;
} mgbtas_bullet_t;

typedef struct _mgbtas_trajectory_t
{
   mgbtas_bullet_t                     maximum;
   mgbtas_bullet_t                     minimum;
   uint64_t                            total;
} mgbtas_trajectory_t;

typedef struct _mgbtas_round_t
{
   // double direction linked rnds list
   struct _mgbtas_round_t *            prev_rnd;
   struct _mgbtas_round_t *            next_rnd;

   // when does this rnd beign
   universal_time_t                    begin_rnd_time;

   // how much time this rnd calcualte
   uint64_t                            total_rnd_duration;

   // There are five types of trajectorys:
   //    EXECUTION: how much time that the service used on processing requests recorded by this rnd. 
   //    LATENCY: how much time between the time request recieved and its start.
   //    PAYLOAD: how much bytes a request carreis.
   //    THROUGHPUT: how many requests that the service handles per second
   //    BANDWIDTH: how many bytes that the service transfered per second
   mgbtas_trajectory_t                 trajectorys[MAX_TRAJECTORY];

   // total in-coming requests
   uint64_t                            total_input_times;
} mgbtas_round_t;

typedef struct _mgbtas_track_t
{
   int                                 rnd_count;
   mgbtas_round_t *                    first_rnd;
   mgbtas_round_t *                    last_rnd;
} mgbtas_track_t;

#if defined( __cplusplus )
extern "C" {
#endif

   // bullet
   inline void bullet_recieved( mgbtas_bullet_t * bullet )
   {
      bullet->recieved_time = universal_time_now();
   }

   inline void bullet_start( mgbtas_bullet_t * bullet )
   {
      bullet->start_time = universal_time_now();
   }

   inline void bullet_finish( mgbtas_bullet_t * bullet )
   {
      bullet->finished_time = universal_time_now();
   }

   inline void bullet_payload( mgbtas_bullet_t * bullet, int payload_length )
   {
      bullet->payload_length = payload_length;
   }

   // round
   mgbtas_round_t * mgbtas_round_alloc_and_init( mgbtas_round_t * prev_rnd );
   universal_time_t mgbtas_round_start_time( mgbtas_round_t * rnd );
   universal_time_t mgbtas_round_end_time( mgbtas_round_t * rnd );
   void mgbtas_round_free( mgbtas_round_t * rnd );

   bool mgbtas_round_append( mgbtas_round_t * rnd, mgbtas_bullet_t * bullet );
   uint64_t mgbtas_round_average( mgbtas_round_t * rnd, int traj );
   uint64_t mgbtas_round_throughput( mgbtas_round_t * rnd );
   uint64_t mgbtas_round_bandwidth( mgbtas_round_t * rnd );

   // track
   mgbtas_track_t * mgbtas_track_alloc_and_init( int round_time = 0, int max_rounds = 0 );
   universal_time_t mgbtas_track_begin_time( mgbtas_track_t * track );
   universal_time_t mgbtas_track_end_time( mgbtas_track_t * track );
   void mgbtas_track_free( mgbtas_track_t * track );

   void mgbtas_track_append( mgbtas_track_t * chain, mgbtas_bullet_t * bullet );
   uint64_t mgbtas_track_average_throughput( mgbtas_track_t * chain );
   uint64_t mgbtas_track_average_bandwidth( mgbtas_track_t * chain );
   uint64_t mgbtas_track_information(
         mgbtas_track_t * track, int traj,
         mgbtas_round_t ** maximum, mgbtas_round_t ** minimum );

   // dumper
   void mgbtas_round_dump( FILE * fp, mgbtas_round_t * rnd );
   void mgbtas_track_dump_detail( FILE * fp, mgbtas_track_t * track );
   void mgbtas_track_dump( FILE * fp, mgbtas_track_t * track );

#if defined( __cplusplus )
}
#endif

#endif // XFLOW_METER_H
