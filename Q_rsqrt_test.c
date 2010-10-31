#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
 
float Q_rsqrt( float number );
 
uint64_t r;
uint64_t rdtsc() {
  asm("cpuid\n\t"
      "rdtsc"
      : "=A" (r)
      : /* no inputs */ );
  return r;
}
 
int main ( int args, char * argv[] ) {
 
  float f, r0, r1, rQ, rF;
  uint64_t begin, end, q_avg, i_avg;
  size_t i;
 
  do {
 
    printf("Q_rsqrt: ");
    scanf("%f", &f);
 
    if(f<0.0f)
      return 0;
 
    q_avg = 0;
    for ( i = 0;
          i < 101;
          ++i ) {
 
      begin = rdtsc();
      r0 = Q_rsqrt(f);
      end = rdtsc();
 
      if (i > 0)
        assert(r0 == r1);
      r1 = r0;
 
      if (end - begin > 700 ) // (*)
        --i; // pitch the timings
      else if (i > 0)
        q_avg = ((uint64_t)((q_avg + (end - begin)) / 2.0f));
      else
        q_avg = end - begin;
 
    }
    rQ = r0;
 
    i_avg = 0;
    for ( i = 0;
          i < 101;
          ++i ) {
 
      begin = rdtsc();
      r0 = 1.0f / sqrt(f);
      end = rdtsc();
 
      if (i > 0)
        assert(r0 == r1);
      r1 = r0;
 
      if (end - begin > 700 ) // (*)
        --i; // pitch the timings
      else if (i > 0)
        i_avg = ((uint64_t)((i_avg + (end - begin)) / 2.0f));
      else
        i_avg = end - begin;
 
    }
    rF = r0;
 
    printf("Q_rsqrt result: %f\tAvg: %llu%s\n", rQ, q_avg, (q_avg < i_avg)?" (*)":"");
    printf("fsqrt   result: %f\tAvg: %llu%s\n", r0, i_avg, (q_avg > i_avg)?" (*)":"");
 
  } while ( 0 == 0 ) ;
 
  return 0;
}
 
// (*) rough estimates for average time observed on an Intel Core 2 Duo.
 
/*
** float q_rsqrt( float number )
*/
float Q_rsqrt( float number ) {
    long i;
    float x2, y;
    const float threehalfs = 1.5F;
 
    x2 = number * 0.5F;
    y  = number;
    i  = * ( long * ) &y;     // evil floating point bit level hacking
    i  = 0x5f3759df - ( i >> 1 );               // what the fuck?
    y  = * ( float * ) &i;
    y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
 
    return y;
}
