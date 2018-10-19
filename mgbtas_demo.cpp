#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "mgbtas_demo.h"

bool thread_stop_flag = false;

const char * str_tag( int tag )
{
   switch ( tag )
   {
      case APPEND:
         return "Append(Default)";
      case REMOVE:
         return "Remove";
      case ALTER:
         return "Alter";
      case QUERY:
         return "Query";
      case LIST:
         return "List";
   }

   return "Unknown";
}

bool mgbtas_game_init( mgbtas_game_t * game )
{
   if ( !game )
      return false;

   // set all to NULL
   for ( int i = 0; i < (int)_countof( game->tracks ); i++ )
      game->tracks[i] = NULL;

   // allocate tracks
   for ( int i = 0; i < (int)_countof( game->tracks ); i++ )
   {
      game->tracks[i] = mgbtas_track_alloc_and_init();

      if ( !game->tracks[i] )
      {
         mgbtas_game_clean( game );
         return false;
      }
   }

   return true;
}

void mgbtas_game_clean( mgbtas_game_t * game )
{
   if ( !game )
      return;

   for ( int i = 0; i < (int)_countof( game->tracks ); i++ )
   {
      if ( game->tracks[i] )
         mgbtas_track_free( game->tracks[i] );
   }
}

void mgbtas_game_append( mgbtas_game_t * game, int tag, mgbtas_bullet_t * bullet )
{
   if ( game && tag < (int)_countof(game->tracks) )
      mgbtas_track_append( game->tracks[tag], bullet );
}

void mgbtas_game_dump_track( FILE * fp, mgbtas_game_t * game, int tag )
{
   if ( game && tag < (int)_countof(game->tracks) )
      ::mgbtas_track_dump( fp, game->tracks[tag] );
}

void mgbtas_game_detail_track( FILE * fp, mgbtas_game_t * game, int tag )
{
   if ( game && tag < (int)_countof(game->tracks) )
      ::mgbtas_track_dump_detail( fp, game->tracks[tag] );
}

typedef struct _mgbtas_demo_t
{
   mgbtas_game_t game;
   mgbtas_counter_t counter;
} mgbtas_demo_t;

void * simulate_produce_data_thread( void * ctx )
{
   mgbtas_demo_t * demo = (mgbtas_demo_t *)ctx;
   mgbtas_game_t * game = &demo->game;
   mgbtas_counter_t * counter = &demo->counter;

   if ( !game )
      return NULL;

   struct 
   {
      int recived_sleep;
      int start_sleep;
   } tag_sleep_array[ MGBTAS_MAXIMUM_TRACKS ] = {
      { 1000, 1000 }, // APPEND
      { 2000, 2000 }, // REMOVE
      { 3000, 3000 }, // DELETE
      { 500, 500 },   // QUERY
      { 4000, 4000 }, // LIST
   };

   while ( !thread_stop_flag )
   {
      int rval = rand() % 0x400000;
      int tag = rval % MGBTAS_MAXIMUM_TRACKS;

      mgbtas_counter_tick( counter, abs( rval ) );

      mgbtas_bullet_t bullet;
      memset( &bullet, 0, sizeof( bullet ) );

      mgbtas_bullet_recieved( &bullet );
      mgbtas_bullet_payload( &bullet, abs( rval ) );

      ::usleep( tag_sleep_array[tag].recived_sleep );

      mgbtas_bullet_start( &bullet );

      ::usleep( tag_sleep_array[tag].start_sleep );

      mgbtas_bullet_finish( &bullet );

      mgbtas_game_append( game, tag, &bullet );
   }

   return NULL;
}

static int pipe_fds[2];

void * mgbtas_demo_main( void * ctx )
{
   mgbtas_demo_t demo;
   pthread_t producer;
   int err;

   if ( !mgbtas_game_init( &demo.game ) )
   {
      printf( "can't init game\n" );
      return NULL;
   }

   mgbtas_counter_init( &demo.counter );

   err = ::pthread_create( &producer, NULL, &simulate_produce_data_thread, &demo );

   if ( err != 0 ) 
   {
      printf( "can't create thread: [%s]\n", strerror(err) );
      return NULL;
   }

   fd_set descriptor_set;
   FD_ZERO(&descriptor_set); 
   FD_SET(STDIN_FILENO, &descriptor_set); 
   FD_SET(pipe_fds[0], &descriptor_set);

   for ( ;; )
   {
      if ( select( FD_SETSIZE, &descriptor_set, NULL, NULL, NULL) < 0 )
      {
         // select() error
      }

      if ( FD_ISSET( pipe_fds[0], &descriptor_set ) )
      {
         // Special event. break
         printf( "quit signal recieved\n" );
         break;
      }

      if ( FD_ISSET( STDIN_FILENO, &descriptor_set ) )
      {
         char * line = NULL;
         size_t size = 0;

         ::getline( &line, &size, stdin );

         if ( feof( stdin ) || !strcmp( "quit\n", line ) ) {
            printf( "quit recieved\n" );
            thread_stop_flag = true;
            ::pthread_join( producer, NULL );
            break;
         }

         if ( !strcmp( "dump append\n", line ) ) {
            mgbtas_game_dump_track( stdout, &demo.game, APPEND );
         } else if ( !strcmp( "dump remove\n", line ) ) {
            mgbtas_game_dump_track( stdout, &demo.game, REMOVE );
         } else if ( !strcmp( "dump alter\n", line ) ) {
            mgbtas_game_dump_track( stdout, &demo.game, ALTER );
         } else if ( !strcmp( "dump query\n", line ) ) {
            mgbtas_game_dump_track( stdout, &demo.game, QUERY );
         } else if ( !strcmp( "dump list\n", line ) ) {
            mgbtas_game_dump_track( stdout, &demo.game, LIST );
         } else if ( !strcmp( "detail append\n", line ) ) {
            mgbtas_game_detail_track( stdout, &demo.game, APPEND );
         } else if ( !strcmp( "detail remove\n", line ) ) {
            mgbtas_game_detail_track( stdout, &demo.game, REMOVE );
         } else if ( !strcmp( "detail alter\n", line ) ) {
            mgbtas_game_detail_track( stdout, &demo.game, ALTER );
         } else if ( !strcmp( "detail query\n", line ) ) {
            mgbtas_game_detail_track( stdout, &demo.game, QUERY );
         } else if ( !strcmp( "detail list\n", line ) ) {
            mgbtas_game_detail_track( stdout, &demo.game, LIST );
         } else if ( !strcmp( "counter\n", line ) ) {
            printf( "{\n\tthroughput: '%s',\n", mgbtas_throughput_str( mgbtas_counter_throughput( &demo.counter ) ) );
            printf( "\tbandwidth: '%s'\n},\n", mgbtas_bandwidth_str( mgbtas_counter_bandwidth( &demo.counter ) ) );
         } else if ( !strcmp( "help\n", line ) ) {
            printf( "usage: [dump|detail|counter] [append|remove|alter|query|list]\n" );
            printf( "\texample:\tdump append \t# show statistic information about 'append' operation\n" );
            printf( "\t\t\tdetail alter \t# show statistic information about 'alter' operation\n" );
            printf( "\t\t\tcounter \t# show demo counter info\n" );
         } else {
            printf( "error! invalid command, type help for usage.\n" );
         }

         fflush( stdout );
      }
   }

   return NULL;
}

int main( int argc, char * argv[] )
{
   pipe(pipe_fds);
   pthread_t command_loop;
   ::pthread_create( &command_loop, NULL, &mgbtas_demo_main, NULL );

   for ( int i = 5; i >= 0; i-- )
   {
      printf( "quit after %d seconds\n", i );
      ::sleep( 1 );
   }
   
   printf( "quit\n" );
   close(pipe_fds[1]);

   ::pthread_join( command_loop, NULL );
   printf( "mgbtad_demo command loop exit!" );
   return 0;
}
