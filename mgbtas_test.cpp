#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 1

#include "mgbtas.h"

#ifndef UNUSED
#define UNUSED(x) (x)=(x)
#endif

typedef int (*pfn_test_entry_t)( int argc, char * argv[] );

typedef struct _mgbtas_test_unit_t
{
   const char *      name;
   pfn_test_entry_t  entry;
} mgbtas_test_unit_t;

int test_mgbtas_bandwidth_str( int argc, char * argv[] )
{
   UNUSED( argc );
   UNUSED( argv );

   ::mgbtas_fraction_t bandwidth_cases[] = {
      { 1024, 1 },
      { 457399992, 1543934 },
   };

   for ( int i = 0; i < (int)_countof( bandwidth_cases ); i++ )
   {
      // calculate the case
      uint64_t val = bandwidth_cases[i].numerator * 1000 * 1000 / bandwidth_cases[i].denominator;
      const char * result = ::bandwidth_str( bandwidth_cases[i] );
      bool flag = false;

      if ( val > 1024 * 1024 && ::strstr( result, "mb/s" ) != NULL )
         flag = true;
      else if ( val > 1024 && ::strstr( result, "kb/s" ) != NULL )
         flag = true;
      else if ( val > 0 && ::strstr( result, "bytes/s" ) != NULL )
         flag = true;
      else
         flag = false;

      if ( !flag )
      {
         printf( "not match, case {%ld, %ld} gives %s\n",
               bandwidth_cases[i].numerator, bandwidth_cases[i].denominator, result );
         return -1;
      }
      else
      {
         printf( "match, case {%ld, %ld} gives %s\n",
               bandwidth_cases[i].numerator, bandwidth_cases[i].denominator, result );
      }
   }

   return 0;
}

mgbtas_test_unit_t tests[] = 
{
   { "test mgbtas_bandwidth_str", test_mgbtas_bandwidth_str },
};

int main( int argc, char * argv[] )
{
   for ( int i = 0; i < (int)_countof( tests ); i++ )
   {
      printf( "test %s %s\n", tests[i].name, tests[i].entry( argc - 1, &argv[1] ) == 0 ? "succeeded" : "failed" );
   }

   return 0;
}
