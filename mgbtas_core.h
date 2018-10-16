#ifndef MGBTAS_CORE_H
#define MGBTAS_CORE_H

#ifndef DEBUG 
#define DEBUG 0
#endif

#include "mgbtas_time.h"

// for mgbtas
#ifndef _countof
#define _countof( ary ) (sizeof((ary)) / sizeof((ary)[0]))
#endif 

// Default round duration is 60 seconds, you can use short value for test, like 1.
#if !defined( MGBTAS_DEFAULT_ROUND )
#define MGBTAS_DEFAULT_ROUND           (60)
#endif

// Default round count is 60 which means that each track can keeps data in 1 hour (60 * 60 = 3600s) only.
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

#define UNIT_KILOBYTES                 1024
#define UNIT_TIMES                     1
#define UNIT_MICROSECOND               (1000 * 1000)

// value of fraction is numerator / denominator
typedef struct _mgbtas_fraction_t
{
   uint64_t                            numerator;
   uint64_t                            denominator;
} mgbtas_fraction_t;

typedef struct _mgbtas_bullet_t
{
   int                                 payload_length;
   mgbtas_time_t                       recieved_time;
   mgbtas_time_t                       start_time;
   mgbtas_time_t                       finished_time;
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
   mgbtas_time_t                       begin_rnd_time;

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

// mgbtas_counter_t is a simple tickcounter for calculating throughput and bandwidth
typedef struct _mgbtas_counter_t
{
   uint64_t tickcount;
   uint64_t totalvalue;
   mgbtas_time_t begin_time;
} mgbtas_counter_t;

#if defined( __cplusplus )
extern "C" {
#endif

   // bullet
   inline void mgbtas_bullet_recieved( mgbtas_bullet_t * bullet )
   {
      bullet->recieved_time = mgbtas_time_now();
   }

   inline void mgbtas_bullet_start( mgbtas_bullet_t * bullet )
   {
      bullet->start_time = mgbtas_time_now();
   }

   inline void mgbtas_bullet_finish( mgbtas_bullet_t * bullet )
   {
      bullet->finished_time = mgbtas_time_now();
   }

   inline void mgbtas_bullet_payload( mgbtas_bullet_t * bullet, int payload_length )
   {
      bullet->payload_length += payload_length;
   }

   // round
   mgbtas_round_t * mgbtas_round_alloc_and_init( mgbtas_round_t * prev_rnd );
   mgbtas_time_t mgbtas_round_begin_time( mgbtas_round_t * rnd );
   mgbtas_time_t mgbtas_round_end_time( mgbtas_round_t * rnd );
   void mgbtas_round_free( mgbtas_round_t * rnd );

   bool mgbtas_round_append( mgbtas_round_t * rnd, mgbtas_bullet_t * bullet );
   mgbtas_fraction_t mgbtas_round_average( mgbtas_round_t * rnd, int traj );
   mgbtas_fraction_t mgbtas_round_throughput( mgbtas_round_t * rnd );
   mgbtas_fraction_t mgbtas_round_bandwidth( mgbtas_round_t * rnd );

   // track
   mgbtas_track_t * mgbtas_track_alloc_and_init();
   mgbtas_time_t mgbtas_track_begin_time( mgbtas_track_t * track );
   mgbtas_time_t mgbtas_track_end_time( mgbtas_track_t * track );
   void mgbtas_track_free( mgbtas_track_t * track );

   void mgbtas_track_append( mgbtas_track_t * chain, mgbtas_bullet_t * bullet );
   mgbtas_fraction_t mgbtas_track_average_throughput( mgbtas_track_t * chain );
   mgbtas_fraction_t mgbtas_track_average_bandwidth( mgbtas_track_t * chain );
   uint64_t mgbtas_track_information(
         mgbtas_track_t * track, int traj,
         mgbtas_round_t ** maximum, mgbtas_round_t ** minimum );

   // counter
   void mgbtas_counter_init( mgbtas_counter_t * counter );
   void mgbtas_counter_clean( mgbtas_counter_t * counter );
   void mgbtas_counter_tick( mgbtas_counter_t * counter, int payload_length = 0 );
   mgbtas_fraction_t mgbtas_counter_throughput( mgbtas_counter_t * counter );
   mgbtas_fraction_t mgbtas_counter_bandwidth( mgbtas_counter_t * counter );

   // mgbtas_fraction_t
   uint64_t mgbtas_fraction_calculate( mgbtas_fraction_t v );
   mgbtas_fraction_t mgbtas_fraction_multiply( mgbtas_fraction_t v, mgbtas_fraction_t mulitplier );
   mgbtas_fraction_t mgbtas_fraction_divide( mgbtas_fraction_t v, mgbtas_fraction_t divisor );

#if defined( __cplusplus )
}
#endif

#endif // MGBTAS_CORE_H
