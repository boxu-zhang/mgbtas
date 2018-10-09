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

const char * str_traj_unit( int traj )
{
   switch ( traj )
   {
      case EXECUTION:
         return "us";
      case LATENCY:
         return "us";
      case PAYLOAD:
         return "bytes";
      case FULLTIME:
         return "us";
   }

   return "unknown";
}

// for universal time
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

   snprintf( buffer, sizeof( buffer ), "%s(%ld)", buf, time_val.tv_usec / 1000 );

   return buffer;
}

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

   if ( usec < 0 ) {
      usec = 1000 * 1000 - usec;
      sec -= 1;
   }

   return universal_time_normalize( (uint64_t)sec * 1000 * 1000 + usec );
}

universal_time_t universal_time_add( universal_time_t * begin, uint64_t dur_in_ms )
{
   universal_time_t t;

   t.tv_sec = begin->tv_sec + dur_in_ms / 1000;
   t.tv_usec = begin->tv_usec + ( dur_in_ms % 1000 ) * 1000;

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
   rnd->total_rnd_duration = MGBTAS_DEFAULT_ROUND * 1000; // convert to ms

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

uint64_t mgbtas_round_average( mgbtas_round_t * rnd, int traj )
{
   return rnd->total_input_times ? (rnd->trajectorys[traj].total / rnd->total_input_times) : 0;
}

uint64_t mgbtas_round_throughput( mgbtas_round_t * rnd )
{
   return rnd->total_rnd_duration ? rnd->total_input_times * 1000 / rnd->total_rnd_duration : 0;
}

uint64_t mgbtas_round_bandwidth( mgbtas_round_t * rnd )
{
   return rnd->total_rnd_duration ? rnd->trajectorys[PAYLOAD].total * 1000 / ( 1024 * rnd->total_rnd_duration ) : 0;
}

// track
mgbtas_track_t * mgbtas_track_alloc_and_init( int round_time, int max_rounds )
{
   if ( round_time <= 0 )
      round_time = MGBTAS_DEFAULT_ROUND;

   if ( max_rounds <= 0 )
      round_time = MGBTAS_MAXIMUM_ROUNDS;

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
      }

      mgbtas_round_append( chain->last_rnd, bullet );
   }

   if ( chain->first_rnd == NULL )
      chain->first_rnd = chain->last_rnd;
}

uint64_t mgbtas_track_average_throughput( mgbtas_track_t * track )
{
   if ( !track || !track->first_rnd )
      return 0;

   mgbtas_round_t * rnd = track->first_rnd->next_rnd;

   uint64_t sum = 0;
   uint64_t i = 0;

   while ( rnd != NULL )
   {
      sum += mgbtas_round_throughput( rnd );
      rnd = rnd->next_rnd;
      i++;
   }

   return i == 0 ? 0 : sum / i;
}

uint64_t mgbtas_track_average_bandwidth( mgbtas_track_t * track )
{
   if ( !track || !track->first_rnd )
      return 0;

   mgbtas_round_t * rnd = track->first_rnd->next_rnd;

   uint64_t sum = 0;
   uint64_t i = 0;

   while ( rnd != NULL )
   {
      sum += mgbtas_round_bandwidth( rnd );
      rnd = rnd->next_rnd;
      i++;
   }

   return i == 0 ? 0 : sum / i;
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

   uint64_t maximum_average = mgbtas_round_average( *maximum, traj );
   uint64_t minimum_average = maximum_average;

   uint64_t i = 0;
   uint64_t sum = maximum_average;

   while ( rnd != NULL )
   {
      uint64_t rnd_average = mgbtas_round_average( rnd, traj );

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

   return sum / i;
}

void mgbtas_trajectorys_dump( FILE * fp, mgbtas_round_t * rnd )
{
   // total request count
   fprintf( fp, "\t\t\ttotal_input_times: %ld\n", rnd->total_input_times );
   fprintf( fp, "\t\t\tthroughput: %ld, // t/s\n", mgbtas_round_throughput( rnd ) );
   fprintf( fp, "\t\t\tbandwidth: %ld, // kb/s\n", mgbtas_round_bandwidth( rnd ) );

   for ( int traj = DEFAULT_TRAJECTORY; traj < (int)_countof( rnd->trajectorys ); traj++ )
   {
      fprintf( fp, "\t\t\t%s: { total: %ld, maximum: %ld, minimum: %ld, average: %ld },\n",
            str_traj( traj ),
            rnd->trajectorys[traj].total,
            bullet_calculate( &rnd->trajectorys[traj].maximum, traj ),
            bullet_calculate( &rnd->trajectorys[traj].minimum, traj ),
            mgbtas_round_average( rnd, traj ) );

      fprintf( fp, 
            "\t\t\t%s_max_detail: { recieved_time: %ld.%ld, start_time: %ld.%ld, finish_time: %ld.%ld, payload_lenth: %d },\n",
            str_traj( traj ),
            rnd->trajectorys[traj].maximum.recieved_time.tv_sec, rnd->trajectorys[traj].maximum.recieved_time.tv_usec,
            rnd->trajectorys[traj].maximum.start_time.tv_sec, rnd->trajectorys[traj].maximum.start_time.tv_usec,
            rnd->trajectorys[traj].maximum.finished_time.tv_sec, rnd->trajectorys[traj].maximum.finished_time.tv_usec,
            rnd->trajectorys[traj].maximum.payload_length );

      fprintf( fp, 
            "\t\t\t%s_min_detail: { recieved_time: %ld.%ld, start_time: %ld.%ld, finish_time: %ld.%ld, payload_lenth: %d },\n",
            str_traj( traj ),
            rnd->trajectorys[traj].minimum.recieved_time.tv_sec, rnd->trajectorys[traj].minimum.recieved_time.tv_usec,
            rnd->trajectorys[traj].minimum.start_time.tv_sec, rnd->trajectorys[traj].minimum.start_time.tv_usec,
            rnd->trajectorys[traj].minimum.finished_time.tv_sec, rnd->trajectorys[traj].minimum.finished_time.tv_usec,
            rnd->trajectorys[traj].minimum.payload_length );
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
   fprintf( fp, "Average Throughput: %ld(t/s)\n", mgbtas_track_average_throughput( track ) );
   fprintf( fp, "Average Bandwidth: %ld(kb/s)\n", mgbtas_track_average_bandwidth( track ) );
   fprintf( fp, "Round Count: %d\n", track->rnd_count );

   for ( int traj = 0; traj < MAX_TRAJECTORY; traj++ )
   {
      mgbtas_round_t * maximum = NULL;
      mgbtas_round_t * minimum = NULL;
      
      uint64_t average = mgbtas_track_information( track, traj, &maximum, &minimum );

      fprintf( fp, "\t%s: %ld(%s)\n", str_traj( traj ), average, str_traj_unit( traj ) );
      fprintf( fp, "\t%s_maximum: %ld(%s)\n", 
            str_traj( traj ), mgbtas_round_average( maximum, traj ), str_traj_unit( traj ) );
      fprintf( fp, "\t%s_minimum: %ld(%s)\n",
            str_traj( traj ), mgbtas_round_average( minimum, traj ), str_traj_unit( traj ) );
   }

   fprintf( fp, "}\n" );

   fprintf( fp, "=============================================================================*/\n" );
}

