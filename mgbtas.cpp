#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>

#include "mgbtas.h"

const char * str_traj( int traj )
{
   switch ( traj )
   {
      case EXECUTION:
         return "execution";
      case LATENCY:
         return "latency";
      case PAYLOAD:
         return "payload";
      case FULLTIME:
         return "fulltime";
   }

   return "unknown";
}

// value to string
static char buffer[64] = { 0 };

const char * universal_time_str( universal_time_t time_val )
{
   struct tm * tm_info = localtime( &time_val.tv_sec );

   strftime( buffer, sizeof( buffer ), "%Y-%m-%d %H:%M:%S", tm_info );

   return buffer;
}

const char * universal_time_str_detail( universal_time_t time_val )
{
   char buf[64] = { 0 };

   struct tm * tm_info = localtime( &time_val.tv_sec );

   strftime( buf, sizeof( buf ), "%Y-%m-%d %H:%M:%S", tm_info );

   snprintf( buffer, sizeof( buffer ), "%s:%ld:%ld", buf, time_val.tv_usec / 1000, time_val.tv_usec % 1000 );

   return buffer;
}

const char * payload_length_str( uint64_t payload_length )
{
   uint64_t payload_length_kb = payload_length / 1024;
   uint64_t payload_length_kb_reminder = payload_length % 1024;
   uint64_t payload_length_mb = payload_length_kb / 1024;
   uint64_t payload_length_mb_reminder = payload_length_kb % 1024;

   if ( payload_length_mb ) {
      snprintf( buffer, sizeof( buffer ), "%ld.%ld mb", payload_length_mb, payload_length_mb_reminder * 10 / 1024  );
   }
   else if ( payload_length_kb ) {
      snprintf( buffer, sizeof( buffer ), "%ld.%ld kb", payload_length_kb, payload_length_kb_reminder * 10 / 1024 );
   }
   else {
      snprintf( buffer, sizeof( buffer ), "%ld bytes", payload_length );
   }

   return buffer;
}

// default payload length is 'bytes per times(bytes/t)'
const char * average_payload_length_str( mgbtas_fraction_t payload_length )
{
   uint64_t payload_length_in_bytes = mgbtas_fraction_calculate( payload_length );
   uint64_t payload_length_in_kb = payload_length_in_bytes / 1024;
   uint64_t payload_length_in_kb_reminder = payload_length_in_bytes % 1024;
   uint64_t payload_length_in_mb = payload_length_in_kb / 1024;
   uint64_t payload_length_in_mb_reminder = payload_length_in_kb % 1024;

   if ( DEBUG ) {
      snprintf( buffer, sizeof( buffer ), "%ld / %ld", payload_length.numerator, payload_length.denominator );
   } else if ( payload_length_in_mb ) {
      snprintf( buffer, sizeof( buffer ), "%ld.%ld mb", payload_length_in_mb, payload_length_in_mb_reminder * 10 / 1024 );
   } else if ( payload_length_in_kb ) {
      snprintf( buffer, sizeof( buffer ), "%ld.%ld kb", payload_length_in_kb, payload_length_in_kb_reminder * 10 / 1024 );
   } else {
      snprintf( buffer, sizeof( buffer ), "%ld bytes", payload_length_in_bytes );
   }

   return buffer;
}

// default duration is in 'microsecond(us)'
const char * duration_str( uint64_t dur )
{
   uint64_t dur_in_ms = dur / 1000;
   uint64_t dur_in_ms_reminder = dur % 1000;
   uint64_t dur_in_s = dur_in_ms / 1000;
   uint64_t dur_in_s_reminder = dur_in_ms % 1000;

   if ( dur_in_s ) {
      snprintf( buffer, sizeof( buffer ), "%ld.%ld s", dur_in_s, dur_in_s_reminder * 10 / 1000 );
   } else if ( dur_in_ms ) {
      snprintf( buffer, sizeof( buffer ), "%ld.%ld ms", dur_in_ms, dur_in_ms_reminder * 10 / 1000 );
   } else {
      snprintf( buffer, sizeof( buffer ), "%ld us", dur );
   }

   return buffer;
}

// default duration value is 'microsecond per times(us/t)'
const char * average_duration_str( mgbtas_fraction_t dur )
{
   uint64_t dur_in_uspt = mgbtas_fraction_calculate( dur );
   uint64_t dur_in_mspt = dur_in_uspt / 1000;
   uint64_t dur_in_mspt_reminder = dur_in_uspt % 1000;
   uint64_t dur_in_spt = dur_in_mspt / 1000;
   uint64_t dur_in_spt_reminder = dur_in_mspt % 1000;

   if ( DEBUG ) {
      snprintf( buffer, sizeof( buffer ), "%ld / %ld", dur.numerator, dur.denominator );
   } else if ( dur_in_spt ) {
      snprintf( buffer, sizeof( buffer ), "%ld.%ld s/t", dur_in_spt, dur_in_spt_reminder * 10 / 1000 );
   } else if ( dur_in_mspt ) {
      snprintf( buffer, sizeof( buffer ), "%ld.%ld ms/t", dur_in_mspt, dur_in_mspt_reminder * 10 / 1000 );
   } else {
      snprintf( buffer, sizeof( buffer ), "%ld us/t", dur_in_uspt );
   }

   return buffer;
}

// default throughput value is 'times per microsecond(t/us)'
const char * throughput_str( mgbtas_fraction_t throughput )
{
   uint64_t throughput_tpus = mgbtas_fraction_calculate( throughput );
   uint64_t throughput_tpms = mgbtas_fraction_calculate( mgbtas_fraction_multiply( throughput, { 1000, 1 } ) );
   uint64_t throughput_tps = mgbtas_fraction_calculate( mgbtas_fraction_multiply( throughput, { 1000 * 1000, 1 } ) );

   if ( DEBUG ) {
      snprintf( buffer, sizeof( buffer ), "%ld / %ld", throughput.numerator, throughput.denominator );
   } else if ( throughput_tps ) {
      snprintf( buffer, sizeof( buffer ), "%ld t/s", throughput_tps );
   } else if ( throughput_tpms ) {
      snprintf( buffer, sizeof( buffer ), "%ld t/ms", throughput_tpms );
   } else {
      snprintf( buffer, sizeof( buffer ), "%ld t/us", throughput_tpus );
   }

   return buffer;
}

// default bandwidth value is 'bytes per microseconds(kb/us)'
const char * bandwidth_str( mgbtas_fraction_t bandwidth )
{
   uint64_t bandwidth_bpus = mgbtas_fraction_calculate( bandwidth );
   uint64_t bandwidth_kbpus = mgbtas_fraction_calculate( mgbtas_fraction_multiply( bandwidth, { 1, 1024 } ) );
   uint64_t bandwidth_kbpms = mgbtas_fraction_calculate( mgbtas_fraction_multiply( bandwidth, { 1000, 1024 } ) );
   uint64_t bandwidth_kbps = mgbtas_fraction_calculate( mgbtas_fraction_multiply( bandwidth, { 1000 * 1000, 1024 } ) );
   uint64_t bandwidth_mbps = mgbtas_fraction_calculate( mgbtas_fraction_multiply( bandwidth, { 1000 * 1000, 1024 * 1024 } ) );

   if ( DEBUG ) {
      snprintf( buffer, sizeof( buffer ), "%ld / %ld", bandwidth.numerator, bandwidth.denominator );
   } else if ( bandwidth_mbps ) {
      snprintf( buffer, sizeof( buffer ), "%ld mb/s", bandwidth_mbps );
   } else if ( bandwidth_kbps ) {
      snprintf( buffer, sizeof( buffer ), "%ld kb/s", bandwidth_kbps );
   } else if ( bandwidth_kbpms ) {
      snprintf( buffer, sizeof( buffer ), "%ld kb/ms", bandwidth_kbpms );
   } else if ( bandwidth_kbpus ) {
      snprintf( buffer, sizeof( buffer ), "%ld kb/us", bandwidth_kbpus );
   } else {
      snprintf( buffer, sizeof( buffer ), "%ld bytes/us", bandwidth_bpus );
   }

   return buffer;
}

// universal_time_t
universal_time_t universal_time_now()
{
   struct timeval tv;

   ::gettimeofday( &tv, NULL );

   return tv;
}

uint64_t universal_time_normalize( uint64_t time_in_microseconds )
{
   return time_in_microseconds / 1000;
}

uint64_t universal_time_normalize_to_us( uint64_t time_in_normalized )
{
   return time_in_normalized * 1000;
}

uint64_t universal_time_sub( universal_time_t * end, universal_time_t * begin )
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

universal_time_t universal_time_add( universal_time_t * begin, uint64_t dur_in_us )
{
   universal_time_t t;

   t.tv_sec = begin->tv_sec + dur_in_us / 1000000;
   t.tv_usec = begin->tv_usec + dur_in_us % 1000000;

   return t;
}

// bullet methods
inline bool bullet_is_valid( mgbtas_bullet_t * bullet )
{
   return bullet->recieved_time.tv_sec != 0 ? true : false;
}

inline uint64_t bullet_calculate( mgbtas_bullet_t * bullet, int which_one )
{
   switch ( which_one )
   {
      case EXECUTION:
         return universal_time_sub( &bullet->finished_time, &bullet->start_time );
      case LATENCY:
         return universal_time_sub( &bullet->start_time, &bullet->recieved_time );
      case PAYLOAD:
         return bullet->payload_length;
      case FULLTIME:
         return universal_time_sub( &bullet->finished_time, &bullet->recieved_time );
   }

   return -1;
}

// trajectory methods
void mgbtas_trajectory_append( mgbtas_trajectory_t * trajectory, mgbtas_bullet_t * bullet, int which_one )
{
   uint64_t maximum_val = bullet_calculate( &trajectory->maximum, which_one );
   uint64_t minimum_val = bullet_calculate( &trajectory->minimum, which_one );
   uint64_t bullet_val = bullet_calculate( bullet, which_one );

   if ( !bullet_is_valid( &trajectory->maximum ) || maximum_val < bullet_val )
   {
      trajectory->maximum = *bullet;
   }

   if ( !bullet_is_valid( &trajectory->minimum ) || bullet_val < minimum_val )
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

   rnd->begin_rnd_time = universal_time_now();
   rnd->total_rnd_duration = MGBTAS_DEFAULT_ROUND * 1000 * 1000; // convert to ms

   return rnd;
}

bool mgbtas_round_append( mgbtas_round_t * rnd, mgbtas_bullet_t * bullet )
{
   if ( rnd == NULL || bullet == NULL )
      return false;

   // check time, if a request receieved time fits in this duration, then it counts.
   // no matter the full time exceeds the rnd duration or not.
   if ( universal_time_sub( &bullet->recieved_time, &rnd->begin_rnd_time ) < rnd->total_rnd_duration )
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

universal_time_t mgbtas_round_begin_time( mgbtas_round_t * rnd )
{
   return rnd->begin_rnd_time;
}

universal_time_t mgbtas_round_end_time( mgbtas_round_t * rnd )
{
   return universal_time_add( &rnd->begin_rnd_time, rnd->total_rnd_duration );
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

universal_time_t mgbtas_track_begin_time( mgbtas_track_t * track )
{
   universal_time_t ret = { 0, 0 };

   if ( !track || !track->first_rnd )
      return ret;

   return mgbtas_round_begin_time( track->first_rnd );
}

universal_time_t mgbtas_track_end_time( mgbtas_track_t * track )
{
   universal_time_t ret = { 0, 0 };

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

void mgbtas_trajectorys_dump( FILE * fp, mgbtas_round_t * rnd )
{
   // total request count
   fprintf( fp, "\t\ttotal_input_times: %ld\n", rnd->total_input_times );
   fprintf( fp, "\t\tthroughput: '%s',\n", throughput_str( mgbtas_round_throughput( rnd ) ) );
   fprintf( fp, "\t\tbandwidth: '%s',\n", bandwidth_str( mgbtas_round_bandwidth( rnd ) ) );

   for ( int traj = DEFAULT_TRAJECTORY; traj < (int)_countof( rnd->trajectorys ); traj++ )
   {
      if ( traj == PAYLOAD )
      {
         fprintf( fp, "\t\t%s: {\n", str_traj( traj ) );

         fprintf( fp, "\t\t\ttotal: '%s',\n", payload_length_str( rnd->trajectorys[traj].total ) );
         fprintf( fp, "\t\t\taverage: '%s',\n", average_payload_length_str( mgbtas_round_average( rnd, traj ) ) );

         fprintf( fp, "\t\t},\n" );
      }
      else
      {
         fprintf( fp, "\t\t%s: {\n", str_traj( traj ) );

         fprintf( fp, "\t\t\ttotal: '%s',\n", duration_str( rnd->trajectorys[traj].total ) );
         fprintf( fp, "\t\t\taverage: '%s',\n", average_duration_str( mgbtas_round_average( rnd, traj ) ) );

         fprintf( fp, "\t\t},\n" );
      }

      fprintf( fp, "\t\t%s_max_detail: {\n", str_traj( traj ) );

      fprintf( fp, "\t\t\trecieved_time: '%s',\n", universal_time_str_detail( rnd->trajectorys[traj].maximum.recieved_time ) );
      fprintf( fp, "\t\t\tstart_time: '%s',\n", universal_time_str_detail( rnd->trajectorys[traj].maximum.start_time ) );
      fprintf( fp, "\t\t\tfinish_time: '%s',\n", universal_time_str_detail( rnd->trajectorys[traj].maximum.finished_time ) );
      fprintf( fp, "\t\t\tpayload_length: '%s', \n", payload_length_str( rnd->trajectorys[traj].maximum.payload_length ) );

      fprintf( fp, "\t\t},\n" );

      fprintf( fp, "\t\t%s_min_detail: {\n", str_traj( traj ) );

      fprintf( fp, "\t\t\trecieved_time: '%s',\n", universal_time_str_detail( rnd->trajectorys[traj].minimum.recieved_time ) );
      fprintf( fp, "\t\t\tstart_time: '%s',\n", universal_time_str_detail( rnd->trajectorys[traj].minimum.start_time ) );
      fprintf( fp, "\t\t\tfinish_time: '%s',\n", universal_time_str_detail( rnd->trajectorys[traj].minimum.finished_time ) );
      fprintf( fp, "\t\t\tpayload_length: '%s', \n", payload_length_str( rnd->trajectorys[traj].minimum.payload_length ) );

      fprintf( fp, "\t\t},\n" );
   }
}

void mgbtas_round_dump( FILE * fp, mgbtas_round_t * rnd )
{
   if ( !rnd || !fp )
      return;

   fprintf( fp, "{\n" );
   fprintf( fp, "\tbegin_time: '%s',\n", universal_time_str( mgbtas_round_begin_time( rnd ) ) );
   fprintf( fp, "\tend_time: '%s',\n", universal_time_str( mgbtas_round_end_time( rnd ) ) );
   fprintf( fp, "\ttrajectorys: {\n" );

   // dump barrel
   mgbtas_trajectorys_dump( fp, rnd );

   fprintf( fp, "\t},\n" );
   fprintf( fp, "},\n" );
}

void mgbtas_track_dump( FILE * fp, mgbtas_track_t * track )
{
   if ( !track || !fp )
      return;

   if ( track->last_rnd->prev_rnd == NULL )
   {
      printf( "only one rnd is working on saving data, please try dump after %d s\n", MGBTAS_DEFAULT_ROUND );
      return;
   }

   fprintf( fp, "/*=============================================================================\n" );
   fprintf( fp, "Name: mgbtas track dump\n" );
   fprintf( fp, "Start Time: %s\n", universal_time_str( mgbtas_round_begin_time( track->first_rnd ) ) );
   fprintf( fp, "End Time: %s\n", universal_time_str( mgbtas_round_end_time( track->last_rnd->prev_rnd ) ) );
   fprintf( fp, "Window Count: %d\n", track->rnd_count );
   fprintf( fp, "=============================================================================*/\n" ); 
   
   // loop through the chain
   for ( mgbtas_round_t * rnd = track->first_rnd;
         rnd != track->last_rnd;
         rnd = rnd->next_rnd )
   {
      mgbtas_round_dump( fp, rnd );
   }
}

void mgbtas_track_dump_detail( FILE * fp, mgbtas_track_t * track )
{
   if ( !track || !fp )
      return;

   if ( track->last_rnd->prev_rnd == NULL )
   {
      printf( "only one rnd is working on saving data, please try dump after %d s\n", MGBTAS_DEFAULT_ROUND );
      return;
   }

   fprintf( fp, "/*=============================================================================\n" );
   fprintf( fp, "Name: mgbtas track information\n" );
   fprintf( fp, "Start Time: %s\n", universal_time_str( mgbtas_round_begin_time( track->first_rnd ) ) );
   fprintf( fp, "End Time: %s\n", universal_time_str( mgbtas_round_end_time( track->last_rnd->prev_rnd ) ) );
   fprintf( fp, "Average Throughput: %s\n", throughput_str( mgbtas_track_average_throughput( track ) ) );
   fprintf( fp, "Average Bandwidth: %s\n", bandwidth_str( mgbtas_track_average_bandwidth( track ) ) );
   fprintf( fp, "Round Count: %d\n", track->rnd_count );

   for ( int traj = 0; traj < MAX_TRAJECTORY; traj++ )
   {
      mgbtas_round_t * maximum = NULL;
      mgbtas_round_t * minimum = NULL;
      
      uint64_t average = mgbtas_track_information( track, traj, &maximum, &minimum );

      if ( traj == PAYLOAD )
      {
         fprintf( fp, "\t%s: '%s'\n", str_traj( traj ), duration_str( average ) );

         fprintf( fp, "\t%s_maximum: '%s'\n", 
               str_traj( traj ), average_duration_str( mgbtas_round_average( maximum, traj ) ) );
         fprintf( fp, "\t%s_minimum: '%s'\n",
               str_traj( traj ), average_duration_str( mgbtas_round_average( minimum, traj ) ) );
      }
      else
      {
      }
   }

   fprintf( fp, "}\n" );

   fprintf( fp, "=============================================================================*/\n" );
}

// counter
void mgbtas_counter_init( mgbtas_counter_t * counter )
{
   if ( counter )
   {
      counter->begin_time = universal_time_now();
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
      universal_time_t now = universal_time_now();
      uint64_t dur = universal_time_sub( &now, &counter->begin_time );

      return { counter->tickcount, dur };
   }

   return { 0, 0 };
}

mgbtas_fraction_t mgbtas_counter_bandwidth( mgbtas_counter_t * counter )
{
   if ( counter )
   {
      universal_time_t now = universal_time_now();
      uint64_t dur = universal_time_sub( &now, &counter->begin_time );

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
