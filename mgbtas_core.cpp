#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>

#include "mgbtas_time.h"
#include "mgbtas_core.h"
#include "mgbtas_ui.h"

// bullet methods
inline bool mgbtas_bullet_is_valid( mgbtas_bullet_t * bullet )
{
   return bullet->recieved_time.tv_sec != 0 ? true : false;
}

inline uint64_t mgbtas_bullet_calculate( mgbtas_bullet_t * bullet, int which_one )
{
   switch ( which_one )
   {
      case EXECUTION:
         return mgbtas_time_sub( &bullet->finished_time, &bullet->start_time );
      case LATENCY:
         return mgbtas_time_sub( &bullet->start_time, &bullet->recieved_time );
      case PAYLOAD:
         return bullet->payload_length;
      case FULLTIME:
         return mgbtas_time_sub( &bullet->finished_time, &bullet->recieved_time );
   }

   return -1;
}

// trajectory methods
void mgbtas_trajectory_append( mgbtas_trajectory_t * trajectory, mgbtas_bullet_t * bullet, int which_one )
{
   uint64_t maximum_val = mgbtas_bullet_calculate( &trajectory->maximum, which_one );
   uint64_t minimum_val = mgbtas_bullet_calculate( &trajectory->minimum, which_one );
   uint64_t bullet_val = mgbtas_bullet_calculate( bullet, which_one );

   if ( !mgbtas_bullet_is_valid( &trajectory->maximum ) || maximum_val < bullet_val )
   {
      trajectory->maximum = *bullet;
   }

   if ( !mgbtas_bullet_is_valid( &trajectory->minimum ) || bullet_val < minimum_val )
   {
      trajectory->minimum = *bullet;
   }

   trajectory->total += bullet_val; 
}

mgbtas_round_t * mgbtas_round_alloc_and_init( mgbtas_round_t * prev_rnd )
{
   mgbtas_round_t * rnd = (mgbtas_round_t *)malloc( sizeof( mgbtas_round_t ) );

   if ( !rnd )
      return NULL;

   // zero all memory
   memset( rnd, 0, sizeof( *rnd ) );

   // initilaize rnd
   rnd->prev_rnd = prev_rnd;

   rnd->begin_rnd_time = mgbtas_time_now();
   rnd->total_rnd_duration = MGBTAS_DEFAULT_ROUND * 1000 * 1000; // convert to ms

   return rnd;
}

bool mgbtas_round_append( mgbtas_round_t * rnd, mgbtas_bullet_t * bullet )
{
   if ( rnd == NULL || bullet == NULL )
      return false;

   // check time, if a request receieved time fits in this duration, then it counts.
   // no matter the full time exceeds the rnd duration or not.
   if ( mgbtas_time_sub( &bullet->recieved_time, &rnd->begin_rnd_time ) < rnd->total_rnd_duration )
   {
      for ( int traj = 0; traj < MAX_TRAJECTORY; traj++ )
      {
         mgbtas_trajectory_append( &rnd->trajectorys[traj], bullet, traj );
      }

      rnd->total_input_times += 1;
      return true;
   }

   return false;
}

mgbtas_time_t mgbtas_round_begin_time( mgbtas_round_t * rnd )
{
   return rnd->begin_rnd_time;
}

mgbtas_time_t mgbtas_round_end_time( mgbtas_round_t * rnd )
{
   return mgbtas_time_add( &rnd->begin_rnd_time, rnd->total_rnd_duration );
}

mgbtas_fraction_t mgbtas_round_average( mgbtas_round_t * rnd, int traj )
{
   return { rnd->trajectorys[traj].total, rnd->total_input_times };
}

mgbtas_fraction_t mgbtas_round_throughput( mgbtas_round_t * rnd )
{
   return { rnd->total_input_times, rnd->total_rnd_duration };
}

mgbtas_fraction_t mgbtas_round_bandwidth( mgbtas_round_t * rnd )
{
   return { rnd->trajectorys[PAYLOAD].total, rnd->total_rnd_duration };
}

// track
mgbtas_track_t * mgbtas_track_alloc_and_init()
{
   mgbtas_track_t * track = (mgbtas_track_t *)malloc( sizeof( mgbtas_track_t ) );

   if ( !track )
      return NULL;

   memset( track, 0, sizeof( *track ) );
   return track;
}

mgbtas_time_t mgbtas_track_begin_time( mgbtas_track_t * track )
{
   mgbtas_time_t ret = { 0, 0 };

   if ( !track || !track->first_rnd )
      return ret;

   return mgbtas_round_begin_time( track->first_rnd );
}

mgbtas_time_t mgbtas_track_end_time( mgbtas_track_t * track )
{
   mgbtas_time_t ret = { 0, 0 };

   if ( !track || !track->first_rnd )
      return ret;

   return mgbtas_round_end_time( track->last_rnd->prev_rnd );
}

void mgbtas_track_free( mgbtas_track_t * track )
{
   if ( !track || !track->first_rnd )
      return;

   mgbtas_round_t * rnd = track->first_rnd;

   while ( rnd != NULL )
   {
      mgbtas_round_t * tmp = rnd->next_rnd;
      free( rnd );
      rnd = tmp;
   }

   free( track );
}

void mgbtas_track_append( mgbtas_track_t * chain, mgbtas_bullet_t * bullet )
{
   if ( !bullet )
      return;

   // try append
   if ( !mgbtas_round_append( chain->last_rnd, bullet ) )
   {
      mgbtas_round_t * rnd = mgbtas_round_alloc_and_init( chain->last_rnd );

      if ( !rnd )
         return; // no enough memory, ignore this request info

      // chain it
      if ( chain->last_rnd )
         chain->last_rnd->next_rnd = rnd;

      chain->last_rnd = rnd;
      chain->rnd_count ++;

      // check rnd count
      if ( chain->rnd_count > MGBTAS_MAXIMUM_ROUNDS )
      {
         chain->first_rnd = chain->first_rnd->next_rnd;
         free( chain->first_rnd->prev_rnd );
         chain->first_rnd->prev_rnd = NULL;
         chain->rnd_count --;
      }

      mgbtas_round_append( chain->last_rnd, bullet );
   }

   if ( chain->first_rnd == NULL )
      chain->first_rnd = chain->last_rnd;
}

mgbtas_fraction_t mgbtas_track_average_throughput( mgbtas_track_t * track )
{
   if ( !track || !track->first_rnd )
      return { 0, 0 };

   mgbtas_round_t * rnd = track->first_rnd->next_rnd;

   uint64_t sum = 0;
   uint64_t i = 0;

   while ( rnd != NULL )
   {
      sum += mgbtas_fraction_calculate( mgbtas_round_throughput( rnd ) );
      rnd = rnd->next_rnd;
      i++;
   }

   return { sum, i };
}

mgbtas_fraction_t mgbtas_track_average_bandwidth( mgbtas_track_t * track )
{
   if ( !track || !track->first_rnd )
      return { 0, 0 };

   mgbtas_round_t * rnd = track->first_rnd->next_rnd;

   uint64_t sum = 0;
   uint64_t i = 0;

   while ( rnd != NULL )
   {
      sum += mgbtas_fraction_calculate( mgbtas_round_bandwidth( rnd ) );
      rnd = rnd->next_rnd;
      i++;
   }

   return { sum, i };
}

uint64_t mgbtas_track_information(
      mgbtas_track_t * track, int traj,
      mgbtas_round_t ** maximum, mgbtas_round_t ** minimum )
{
   if ( !track || !track->first_rnd )
      return 0;

   *maximum = track->first_rnd;
   *minimum = track->first_rnd;

   mgbtas_round_t * rnd = track->first_rnd->next_rnd;

   uint64_t maximum_average = mgbtas_fraction_calculate( mgbtas_round_average( *maximum, traj ) );
   uint64_t minimum_average = maximum_average;

   uint64_t i = 0;
   uint64_t sum = maximum_average;

   while ( rnd != NULL )
   {
      uint64_t rnd_average = mgbtas_fraction_calculate( mgbtas_round_average( rnd, traj ) );

      sum += rnd_average;

      // shorter execution time is better
      if ( rnd_average < maximum_average )
      {
         *maximum = rnd;
         maximum_average = rnd_average;
      }
      
      if ( rnd_average > minimum_average )
      {
         *minimum = rnd;
         minimum_average = rnd_average;
      }

      rnd = rnd->next_rnd;
      i ++;
   }

   return i == 0 ? 0 : sum / i;
}

// counter
void mgbtas_counter_init( mgbtas_counter_t * counter )
{
   if ( counter )
   {
      counter->begin_time = mgbtas_time_now();
      counter->tickcount = 0;
      counter->totalvalue = 0;
   }
}

void mgbtas_counter_clean( mgbtas_counter_t * counter )
{
   mgbtas_counter_init( counter );
}

void mgbtas_counter_tick( mgbtas_counter_t * counter, int payload_length /*= 0*/ )
{
   if ( counter )
   {
      counter->tickcount ++;
      counter->totalvalue += payload_length;
   }
}

mgbtas_fraction_t mgbtas_counter_throughput( mgbtas_counter_t * counter )
{
   if ( counter )
   {
      mgbtas_time_t now = mgbtas_time_now();
      uint64_t dur = mgbtas_time_sub( &now, &counter->begin_time );

      return { counter->tickcount, dur };
   }

   return { 0, 0 };
}

mgbtas_fraction_t mgbtas_counter_bandwidth( mgbtas_counter_t * counter )
{
   if ( counter )
   {
      mgbtas_time_t now = mgbtas_time_now();
      uint64_t dur = mgbtas_time_sub( &now, &counter->begin_time );

      return { counter->totalvalue, dur };
   }

   return { 0, 0 };
}

uint64_t mgbtas_fraction_calculate( mgbtas_fraction_t v )
{
   return v.denominator == 0 ? 0 : v.numerator / v.denominator;
}

mgbtas_fraction_t mgbtas_fraction_multiply( mgbtas_fraction_t v, mgbtas_fraction_t multiplier )
{
   v.denominator *= multiplier.denominator;
   v.numerator *= multiplier.numerator;

   return v;
}

mgbtas_fraction_t mgbtas_fraction_divide( mgbtas_fraction_t v, mgbtas_fraction_t divisor )
{
   // inverse divisor and mulitply
   return mgbtas_fraction_multiply( v, { divisor.denominator, divisor.numerator } );
}
