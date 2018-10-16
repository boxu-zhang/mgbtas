#include "mgbtas.h"

void mgbtas_version( int * major, int * minor )
{
   if ( major )
      *major = MGBTAS_VERSION_MAJOR;

   if ( minor )
      *minor = MGBTAS_VERSION_MINOR;
}

void mgbtas_description( const char ** description )
{
   *description = "MGBTAS - Machine Gun Ballistic Trajectory Analysis System";
}
