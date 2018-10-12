#ifndef MGBTAS_UI_STRING_H
#define MGBTAS_UI_STRING_H

#include <stdio.h>
#include "mgbtas_core.h"

#if defined( __cplusplus )
extern "C" {
#endif 

   // value to string
   const char * mgbtas_traj_str( int traj );
   const char * mgbtas_payload_length_str( uint64_t payload_length );
   const char * mgbtas_average_payload_length_str( uint64_t payload_length );
   const char * mgbtas_duration_str( uint64_t dur );
   const char * mgbtas_average_duration_str( mgbtas_fraction_t dur );
   const char * mgbtas_throughput_str( mgbtas_fraction_t throughput );
   const char * mgbtas_bandwidth_str( mgbtas_fraction_t bandwidth );

   // dumper
   void mgbtas_round_dump( FILE * fp, mgbtas_round_t * rnd );
   void mgbtas_track_dump_detail( FILE * fp, mgbtas_track_t * track );
   void mgbtas_track_dump( FILE * fp, mgbtas_track_t * track );

#if defined( __cplusplus )
}
#endif

#endif // MGBTAS_UI_STRING_H
