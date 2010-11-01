#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
 
float Q_rsqrt( float number );
 
uint64_t r;
uint64_t rdtsc() {
  asm("cpuid\n\t"
      "rdtsc"
      : "=A" (r)
      : /* no inputs */ );
  return r;
}
 
typedef enum {
  arg_fBegin=0,
  arg_fStep,
  arg_fEnd,
  arg_outfile
} arg_names;

#define INNER_ITERATION 10000
 
int main ( int argc, char * argv[] ) {
 
  float fBegin, fStep, fEnd, f, r0, r1, rQ, rF;
  char * outfile;
  int c;
  int option_index = 0;
  uint64_t begin, end, q_avg, i_avg;
  size_t i;
 
  while (1) {
    static struct option long_options[] = {
      {"fBegin", required_argument, 0, 0},
      {"fStep",  required_argument, 0, 0},
      {"fEnd",   required_argument, 0, 0},
      {"outfile",required_argument, 0, 0},
      {0,        0,                 0, 0}
    };
    
    c = getopt_long (argc, argv, "", long_options, &option_index);
    
    if (c==-1)
      break;
      
    switch(c) {
      case 0:
        if(long_options[option_index].flag != 0)
          break;
        if(option_index == arg_fBegin)
          fBegin = (float)atof(optarg);
        else if(option_index == arg_fStep)
          fStep = (float)atof(optarg);
        else if(option_index == arg_fEnd)
          fEnd = (float)atof(optarg);
        else if(option_index == arg_outfile)
          outfile = optarg;
        break;
      case '?':
        break;
      default:
        break;
    }
  }
  
  if (fBegin > fEnd) {
    printf("Start cannot be greater than end!\n");
    return 0;
  }
  
  printf("Iterating from %f to %f on %f increments.  Will perform %d iterations.\n",
      fBegin,
      fEnd,
      fStep,
      ((int)((fEnd - fBegin) / fStep)));
  
  for ( float fIter = fBegin;
        fIter < fEnd;
        fIter += fStep ) {
    
    q_avg = 0;
    for ( i = 0;
          i < INNER_ITERATION;
          ++i ) {
      
      begin = rdtsc();
      r0 = Q_rsqrt(fIter);
      end = rdtsc();
      
      if (i > 0)
        assert(r0 == r1);
      r1 = r0;
      
      if (end - begin > 700) // pitch the timings
        --i;
      else if (i > 0)
        q_avg = ((uint64_t)((q_avg + (end - begin)) / 2.0f));
      else
        q_avg = end - begin;
        
    }
    rQ = r0;
    
    i_avg = 0;
    for ( i = 0;
          i < INNER_ITERATION;
          ++i ) {
          
      begin = rdtsc();
      r0 = 1.0f / sqrt(fIter);
      end = rdtsc();
      
      if (i > 0)
        assert(r0 == r1);
      r1 = r0;
      
      if (end - begin > 700) // toss the timing
        --i;
      else if (i > 0)
        i_avg = ((uint64_t)((i_avg + (end - begin)) / 2.0f));
      else
        i_avg = end - begin;
        
    }
    rF = r0;
    
    printf("Q_rsqrt result: %f\tAvg: %llu%s\n", rQ, q_avg, (q_avg < i_avg)?" (*)":"");
    printf("fsqrt   result: %f\tAvg: %llu%s\n", r0, i_avg, (q_avg > i_avg)?" (*)":"");
    printf("Error: %f\n\n", rF - rQ);
    
  }
 
  return 0;
}
 
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
