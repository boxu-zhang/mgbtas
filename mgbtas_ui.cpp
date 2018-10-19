#include "mgbtas_ui.h"

const char * mgbtas_traj_str( int traj )
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

const char * mgbtas_payload_length_str( uint64_t payload_length )
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
const char * mgbtas_average_payload_length_str( mgbtas_fraction_t payload_length )
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
const char * mgbtas_duration_str( uint64_t dur )
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
const char * mgbtas_average_duration_str( mgbtas_fraction_t dur )
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
const char * mgbtas_throughput_str( mgbtas_fraction_t throughput )
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
const char * mgbtas_bandwidth_str( mgbtas_fraction_t bandwidth )
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

void mgbtas_trajectorys_dump( FILE * fp, mgbtas_round_t * rnd )
{
   // total request count
   fprintf( fp, "\t\ttotal_input_times: %ld\n", rnd->total_input_times );
   fprintf( fp, "\t\tthroughput: '%s',\n", mgbtas_throughput_str( mgbtas_round_throughput( rnd ) ) );
   fprintf( fp, "\t\tbandwidth: '%s',\n", mgbtas_bandwidth_str( mgbtas_round_bandwidth( rnd ) ) );

   for ( int traj = DEFAULT_TRAJECTORY; traj < (int)_countof( rnd->trajectorys ); traj++ )
   {
      if ( traj == PAYLOAD )
      {
         fprintf( fp, "\t\t%s: {\n", mgbtas_traj_str( traj ) );

         fprintf( fp, "\t\t\ttotal: '%s',\n", mgbtas_payload_length_str( rnd->trajectorys[traj].total ) );
         fprintf( fp, "\t\t\taverage: '%s',\n", mgbtas_average_payload_length_str( mgbtas_round_average( rnd, traj ) ) );

         fprintf( fp, "\t\t},\n" );
      }
      else
      {
         fprintf( fp, "\t\t%s: {\n", mgbtas_traj_str( traj ) );

         fprintf( fp, "\t\t\ttotal: '%s',\n", mgbtas_duration_str( rnd->trajectorys[traj].total ) );
         fprintf( fp, "\t\t\taverage: '%s',\n", mgbtas_average_duration_str( mgbtas_round_average( rnd, traj ) ) );

         fprintf( fp, "\t\t},\n" );
      }

      fprintf( fp, "\t\t%s_max_detail: {\n", mgbtas_traj_str( traj ) );

      fprintf( fp, "\t\t\trecieved_time: '%s',\n", mgbtas_time_str_detail( rnd->trajectorys[traj].maximum.recieved_time ) );
      fprintf( fp, "\t\t\tstart_time: '%s',\n", mgbtas_time_str_detail( rnd->trajectorys[traj].maximum.start_time ) );
      fprintf( fp, "\t\t\tfinish_time: '%s',\n", mgbtas_time_str_detail( rnd->trajectorys[traj].maximum.finished_time ) );
      fprintf( fp, "\t\t\tpayload_length: '%s', \n", mgbtas_payload_length_str( rnd->trajectorys[traj].maximum.payload_length ) );

      fprintf( fp, "\t\t},\n" );

      fprintf( fp, "\t\t%s_min_detail: {\n", mgbtas_traj_str( traj ) );

      fprintf( fp, "\t\t\trecieved_time: '%s',\n", mgbtas_time_str_detail( rnd->trajectorys[traj].minimum.recieved_time ) );
      fprintf( fp, "\t\t\tstart_time: '%s',\n", mgbtas_time_str_detail( rnd->trajectorys[traj].minimum.start_time ) );
      fprintf( fp, "\t\t\tfinish_time: '%s',\n", mgbtas_time_str_detail( rnd->trajectorys[traj].minimum.finished_time ) );
      fprintf( fp, "\t\t\tpayload_length: '%s', \n", mgbtas_payload_length_str( rnd->trajectorys[traj].minimum.payload_length ) );

      fprintf( fp, "\t\t},\n" );
   }
}

void mgbtas_round_dump( FILE * fp, mgbtas_round_t * rnd )
{
   if ( !rnd || !fp )
      return;

   fprintf( fp, "{\n" );
   fprintf( fp, "\tbegin_time: '%s',\n", mgbtas_time_str( mgbtas_round_begin_time( rnd ) ) );
   fprintf( fp, "\tend_time: '%s',\n", mgbtas_time_str( mgbtas_round_end_time( rnd ) ) );
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

   if ( track->last_rnd == NULL )
   {
      fprintf( fp, "no round is available, it means 'mgbtas' hasn't recieved any payload\n" );
      return;
   }

   if ( track->last_rnd->prev_rnd == NULL )
   {
      fprintf( fp, "only one rnd is working on saving data, please try dump after %d s\n", MGBTAS_DEFAULT_ROUND );
      return;
   }

   fprintf( fp, "/*=============================================================================\n" );
   fprintf( fp, "Name: mgbtas track dump\n" );
   fprintf( fp, "Start Time: %s\n", mgbtas_time_str( mgbtas_round_begin_time( track->first_rnd ) ) );
   fprintf( fp, "End Time: %s\n", mgbtas_time_str( mgbtas_round_end_time( track->last_rnd->prev_rnd ) ) );
   fprintf( fp, "Window Count: %d\n", track->rnd_count );
   fprintf( fp, "=============================================================================*/\n" ); 
   
   // loop through the chain
   for ( mgbtas_round_t * rnd = track->first_rnd;
         rnd != track->last_rnd;
         rnd = rnd->next_rnd )
   {
      mgbtas_round_dump( fp, rnd );
   }

   fflush( fp );
}

void mgbtas_track_dump_detail( FILE * fp, mgbtas_track_t * track )
{
   if ( !track || !fp )
      return;

   if ( track->last_rnd == NULL )
   {
      fprintf( fp, "no round is available, it means 'mgbtas' hasn't recieved any payload\n" );
      return;
   }

   if ( track->last_rnd->prev_rnd == NULL )
   {
      printf( "only one rnd is working on saving data, please try dump after %d s\n", MGBTAS_DEFAULT_ROUND );
      return;
   }

   fprintf( fp, "/*=============================================================================\n" );
   fprintf( fp, "Name: mgbtas track information\n" );
   fprintf( fp, "Start Time: %s\n", mgbtas_time_str( mgbtas_round_begin_time( track->first_rnd ) ) );
   fprintf( fp, "End Time: %s\n", mgbtas_time_str( mgbtas_round_end_time( track->last_rnd->prev_rnd ) ) );
   fprintf( fp, "Average Throughput: %s\n", mgbtas_throughput_str( mgbtas_track_average_throughput( track ) ) );
   fprintf( fp, "Average Bandwidth: %s\n", mgbtas_bandwidth_str( mgbtas_track_average_bandwidth( track ) ) );
   fprintf( fp, "Round Count: %d\n", track->rnd_count );

   for ( int traj = 0; traj < MAX_TRAJECTORY; traj++ )
   {
      mgbtas_round_t * maximum = NULL;
      mgbtas_round_t * minimum = NULL;
      
      uint64_t average = mgbtas_track_information( track, traj, &maximum, &minimum );

      if ( traj == PAYLOAD )
      {
         fprintf( fp, "\t%s: '%s'\n", mgbtas_traj_str( traj ), mgbtas_duration_str( average ) );

         fprintf( fp, "\t%s_maximum: '%s'\n", 
               mgbtas_traj_str( traj ), mgbtas_average_duration_str( mgbtas_round_average( maximum, traj ) ) );
         fprintf( fp, "\t%s_minimum: '%s'\n",
               mgbtas_traj_str( traj ), mgbtas_average_duration_str( mgbtas_round_average( minimum, traj ) ) );
      }
      else
      {
      }
   }

   fprintf( fp, "}\n" );

   fprintf( fp, "=============================================================================*/\n" );

   fflush( fp );
}

