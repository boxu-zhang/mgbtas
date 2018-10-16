#ifndef MGBTAS_H
#define MGBTAS_H

#include "mgbtas_config.h"
#include "mgbtas_time.h"
#include "mgbtas_core.h"
#include "mgbtas_ui.h"

#if defined( __cplusplus )
extern "C" {
#endif

   void mgbtas_version( int * major, int * minor );
   void mgbtas_description( const char ** description );

#if defined( __cplusplus )
}
#endif 

#endif // MGBTAS_H
