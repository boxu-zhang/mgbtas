#ifndef MGBTAS_DEMO_H
#define MGBTAS_DEMO_H

#include "mgbtas.h"

// Tag definition, must start from 0, we'll use it as array index
#if !defined( MGBTAS_DEFAULT_TRACK )
#define MGBTAS_DEFAULT_TRACK           0
#endif // MGBTAS_DEFAULT_TRACK

#define APPEND                         MGBTAS_DEFAULT_TRACK
#define REMOVE                         1
#define ALTER                          2
#define QUERY                          3
#define LIST                           4

#define MGBTAS_MAXIMUM_TRACKS          5

typedef struct _mgbtas_game_t
{
   mgbtas_track_t * tracks[ MGBTAS_MAXIMUM_TRACKS ];
} mgbtas_game_t;

#if defined( __cplusplus )
extern "C" {
#endif

   bool mgbtas_game_init( mgbtas_game_t * game );
   void mgbtas_game_clean( mgbtas_game_t * game );

   void mgbtas_game_append( mgbtas_game_t * game, int tag, mgbtas_bullet_t * bullet );
   void mgbtas_game_dump_track( FILE * fp, mgbtas_game_t * game, int tag );
   void mgbtas_game_detail_track( FILE * fp, mgbtas_game_t * game, int tag );

#if defined( __cplusplus )
}
#endif

#endif // MGBTAS_DEMO_H
