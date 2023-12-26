// Copyright (C) 2023, Anubhav Bhatla, Navneet, Biswabandan Panda: Indian Institute of Technology Bombay.

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "mtrand.h"

/////////////////////////////////////////////////////
// COMMAND-LINE ARGUMENTS
/////////////////////////////////////////////////////
//argv[1] : EXTRA TAGS PER SET (PER SKEW)
int EXTRA_BUCKET_CAPACITY = 6;

//argv[2] : NUMBER OF BALLS THROWN
int NUM_BILLION_TRIES = 10;

//argv[3] : SEED
unsigned int myseed = 1;

/////////////////////////////////////////////////////
// DEFINES
/////////////////////////////////////////////////////
//Cache Configuration
//Default: 16 Way LLC

#ifndef CUSTOM_REUSE_WAYS_PER_SKEW
#define REUSE_WAYS_PER_SKEW        (3)
#else
#define REUSE_WAYS_PER_SKEW        (CUSTOM_REUSE_WAYS_PER_SKEW)
#endif

#ifndef CUSTOM_BASE_WAYS_PER_SKEW
#define BASE_WAYS_PER_SKEW        (6)
#else
#define BASE_WAYS_PER_SKEW        (CUSTOM_BASE_WAYS_PER_SKEW)
#endif

#define NUM_SKEWS                 (2)

//16MB LLC
#define CACHE_SZ_BYTES            (12*1024*1024)
#define LINE_SZ_BYTES             (64)
#define NUM_BUCKETS               ((CACHE_SZ_BYTES/LINE_SZ_BYTES)/(BASE_WAYS_PER_SKEW))
#define NUM_BUCKETS_PER_SKEW      (NUM_BUCKETS/NUM_SKEWS)

//Bucket Capacities
#define BALLS_PER_BUCKET      (BASE_WAYS_PER_SKEW)
#ifndef CUSTOM_MAX_FILL
#define MAX_FILL              (40)
#else
#define MAX_FILL              (CUSTOM_MAX_FILL)
#endif
int SPILL_THRESHOLD = BALLS_PER_BUCKET + (REUSE_WAYS_PER_SKEW) + EXTRA_BUCKET_CAPACITY;

// Tie-Breaking Policy between Skews on Ball-Throws
//0 - Randomly picks either skew on Ties. 
//1 - Always picks Skew-0 on Ties.
#define BREAK_TIES_PREFERENTIALLY      (0)

//Experiment Size
#define BILLION_TRIES             (1000*1000*1000)
#define HUNDRED_MILLION_TRIES     (100*1000*1000)

//Types
typedef unsigned int uns;
typedef unsigned long long uns64;
typedef double dbl;


/////////////////////////////////////////////////////
// EXPERIMENT VARIABLES
/////////////////////////////////////////////////////

//For each Bucket (Set), number of Balls in Bucket
//(Data-Structure Similar to Tag-Store)
uns64 bucket[NUM_BUCKETS];
uns64 bucket0[NUM_BUCKETS];

//For each Ball (Cache-Line), which Bucket (Set) it is in
//(Data-Structure Similar to Data-Store RPTR)
uns64 balls[NUM_BUCKETS*BALLS_PER_BUCKET];
uns64 balls0[NUM_BUCKETS*REUSE_WAYS_PER_SKEW];

//Number of Times Each Bucket Indexed 
uns64 bucket_fill_observed[MAX_FILL+1];
uns64 bucket_fill_observed0[MAX_FILL+1];
//Number of Times Bucket Containing N Balls has a Ball-Insertion
uns64 stat_counts[MAX_FILL+1];

//Number of Spills from Buckets
uns64 spill_count = 0;
//Number of Spills despite relocation attempts.
uns64 cuckoo_spill_count = 0;

//Tracks if Initialization of Buckets Done
bool init_buckets_done = false;

//Mersenne Twister Rand Generator
MTRand *mtrand=new MTRand();

/////////////////////////////////////////////////////
// FUNCTIONS - Ball Insertion, Removal, Spill, etc.
/////////////////////////////////////////////////////

/////////////////////////////////////////////////////
// Spill Ball: relocating filled bucket
// -- Based on which skew spill happened;
// -- cuckoo into other recursively.
/////////////////////////////////////////////////////

void spill_ball(uns64 index, uns64 ballID, bool priority){
  uns done=0;

  if (priority){
    bucket[index]--;
  }
  else{
    bucket0[index]--;
  }

  while(done!=1){
    //Pick skew & bucket-index where spilled ball should be placed.
    uns64 spill_index;
    //If current index is in Skew0, then pick Skew1. Else vice-versa.
    if(index < NUM_BUCKETS_PER_SKEW)
      spill_index = NUM_BUCKETS_PER_SKEW + mtrand->randInt(NUM_BUCKETS_PER_SKEW-1);
    else
      spill_index = mtrand->randInt(NUM_BUCKETS_PER_SKEW-1);

    //If new spill_index bucket where spilled-ball is to be installed has space, then done.
    if(bucket[spill_index]+bucket0[spill_index] < SPILL_THRESHOLD){
      done=1;
      if (priority){
        bucket[spill_index]++;
        balls[ballID] = spill_index;
      }
      else{
        bucket0[spill_index]++;
        balls0[ballID] = spill_index;
      }
     
    } else {
      assert(bucket[spill_index]+bucket0[spill_index] == SPILL_THRESHOLD);
      //if bucket of spill_index is also full, then recursive-spill, we call this a cuckoo-spill
      index = spill_index;
      cuckoo_spill_count++;
    }
  }

  spill_count++;
}

/////////////////////////////////////////////////////
// Insert Ball in Bucket
// -- ballID => ID of Data-Store Entry (ball) which got evicted via global eviction
/////////////////////////////////////////////////////

uns insert_ball(uns64 ballID, bool priority){

  //Index for Rand Bucket in Skew-0
  uns64 index1 = mtrand->randInt(NUM_BUCKETS_PER_SKEW - 1);
  //Index for Rand Bucket in Skew-1
  uns64 index2 = NUM_BUCKETS_PER_SKEW + mtrand->randInt(NUM_BUCKETS_PER_SKEW - 1);

  //Increments Tracking of Indexed Buckets
  if(init_buckets_done){
      bucket_fill_observed[bucket[index1]+bucket0[index1]]++;
      bucket_fill_observed[bucket[index2]+bucket0[index2]]++;
    if (!priority){
      bucket_fill_observed0[bucket0[index1]]++;
      bucket_fill_observed0[bucket0[index2]]++;
    }
  }
    
  uns64 index;
  uns retval;

  //------ LOAD AWARE SKEW SELECTION -----
  //Identify which Bucket (index) has Less Load
  if(bucket[index2]+bucket0[index2] < bucket[index1]+bucket0[index1]){
    index = index2;
  } else if (bucket[index1]+bucket0[index1] < bucket[index2]+bucket0[index2]){
    index = index1;    
  } else if (bucket[index2]+bucket0[index2] == bucket[index1]+bucket0[index1]) {

#if BREAK_TIES_PREFERENTIALLY == 0
    //Break ties randomly
    if(mtrand->randInt(1) == 0)
      index = index1;
    else
      index = index2;

#elif BREAK_TIES_PREFERENTIALLY == 1
    //Break ties favoring Skew-0.
    index = index1;
#endif
     
  } else {
    assert(0);
  }
  
  //if(init_buckets_done){
  //  bucket_fill_observed[bucket[index]]++;
  //}

  //Increments count for Bucket where Ball Inserted 
  retval = bucket[index]+bucket0[index];
  
  if (priority) {
    bucket[index]++;
    //Track which bucket the new Ball was inserted in
    assert(balls[ballID] == (uns64)-1);
    balls[ballID] = index;
  }
  else{
    bucket0[index]++;
    //Track which bucket the new Ball was inserted in
    assert(balls0[ballID] == (uns64)-1);
    balls0[ballID] = index;
  }
  
  //----------- SPILL --------
  if(SPILL_THRESHOLD && (retval >= SPILL_THRESHOLD)){
    //Overwrite balls[ballID] with spill_index.
    spill_ball(index,ballID,priority);   
  }

  // Return num-balls in bucket where new ball inserted.
  return retval;  
}

/////////////////////////////////////////////////////
// Remove Random Ball (Global Eviction)
/////////////////////////////////////////////////////

uns64 remove_ball(bool priority){
  // Select Random BallID from all Balls
  uns64 ballID, bucket_index;
  if (priority){
    ballID = mtrand->randInt(NUM_BUCKETS*BASE_WAYS_PER_SKEW -1);

    // Identify which bucket this ball is in 
    assert(balls[ballID] != (uns64)-1);
    bucket_index = balls[ballID];

    // Update Ball Tracking
    assert(bucket[bucket_index] != 0 );  
    bucket[bucket_index]--;
    balls[ballID] = -1;
    
    uns64 ball0ID = remove_ball(0);
    balls0[ball0ID] = bucket_index;
    bucket0[bucket_index]++;
  }
  else{
    ballID = mtrand->randInt(NUM_BUCKETS*REUSE_WAYS_PER_SKEW -1);

    // Identify which bucket this ball is in 
    assert(balls0[ballID] != (uns64)-1);
    bucket_index = balls0[ballID];

    // Update Ball Tracking
    assert(bucket0[bucket_index] != 0 );  
    bucket0[bucket_index]--;
    balls0[ballID] = -1;
  }
  // Return BallID removed (ID will be reused for new ball to be inserted)  
  return ballID;
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

void display_histogram(void){
  uns ii;
  uns s_count[MAX_FILL+1];

  for(ii=0; ii<= MAX_FILL; ii++){
    s_count[ii]=0;
  }

  for(ii=0; ii< NUM_BUCKETS; ii++){
    s_count[bucket[ii]+bucket0[ii]]++;
  }

  //  printf("\n");
  printf("\nOccupancy: \t\t Count");
  for(ii=0; ii<= MAX_FILL; ii++){
    double perc = 100.0 * (double)s_count[ii]/(double)(NUM_BUCKETS);
    printf("\nBucket[%2u Fill]: \t %u \t (%4.2f)", ii, s_count[ii], perc);
  }

  printf("\n");
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

void sanity_check(bool priority){
  uns ii, count1=0, count0=0;
  // uns s_count[MAX_FILL+1];

  // for(ii=0; ii<= MAX_FILL; ii++){
  //   s_count[ii]=0;
  // }
  
  for(ii=0; ii< NUM_BUCKETS; ii++){
    count1 += bucket[ii];
    count0 += bucket0[ii];
    // s_count[bucket[ii]]++;
  }

  if (!priority){
    if(count0 != (NUM_BUCKETS*REUSE_WAYS_PER_SKEW)){
      printf("\n*** Sanity Check Failed, Pr0Count: %u*****\n", count0);
    }
  }
  else{
    if(count1 != (NUM_BUCKETS*BALLS_PER_BUCKET)){
      printf("\n*** Sanity Check Failed, Pr1Count: %u*****\n", count1);
    }
  }

}

/////////////////////////////////////////////////////
// Randomly Initialize all the Buckets with Balls (NumBuckets * BallsPerBucket) 
/////////////////////////////////////////////////////

void init_buckets(bool priority){
  uns64 ii;

  assert(NUM_SKEWS * NUM_BUCKETS_PER_SKEW == NUM_BUCKETS);
  
  for (uns64 i=0; i<MAX_FILL+1; i++){
    bucket_fill_observed0[i] = 0;
  }
  
  if (priority){

    for(ii=0; ii<NUM_BUCKETS; ii++){
      bucket[ii]=0;
    }
    for(ii=0; ii<(NUM_BUCKETS*BALLS_PER_BUCKET); ii++){
      balls[ii] = -1;
      insert_ball(ii, 1);
    }
  }
  else{
    for(ii=0; ii<NUM_BUCKETS; ii++){
      bucket0[ii]=0;
    }
    for(ii=0; ii<(NUM_BUCKETS*REUSE_WAYS_PER_SKEW); ii++){
      balls0[ii] = -1;
      insert_ball(ii, 0);
    }
  }

  for(ii=0; ii<=MAX_FILL; ii++){
    stat_counts[ii]=0;
  }
  sanity_check(priority);
  init_buckets_done = true;
}

/////////////////////////////////////////////////////
// Randomly remove a ball and
// then insert a new ball with Power-of-Choices Install
/////////////////////////////////////////////////////

uns remove_and_insert(bool priority){
  
  uns res = 0;

  uns64 ballID = remove_ball(priority);
  res = insert_ball(ballID, priority);

  if(res <= MAX_FILL){
    stat_counts[res]++;
  }
  else{
    printf("Overflow\n");
    exit(-1);
  }

  //printf("Res: %u\n", res);
  return res;
}

void upgrade_and_downgrade(void){
  // Select Random BallID 0 from all Balls
  uns64 ball0ID, bucket0_index;
  
  ball0ID = mtrand->randInt(NUM_BUCKETS*REUSE_WAYS_PER_SKEW -1);

  // Identify which bucket this ball is in 
  assert(balls0[ball0ID] != (uns64)-1);
  bucket0_index = balls0[ball0ID];

  // Update Ball Tracking
  assert(bucket0[bucket0_index] != 0 );  
  bucket0[bucket0_index]--;
  bucket[bucket0_index]++;
  
   // Select Random BallID 1 from all Balls
  uns64 ball1ID, bucket1_index;
  
  ball1ID = mtrand->randInt(NUM_BUCKETS*BASE_WAYS_PER_SKEW -1);
  
  // Identify which bucket this ball is in 
  assert(balls[ball1ID] != (uns64)-1);
  bucket1_index = balls[ball1ID];
	
  // Update Ball Tracking
  assert(bucket[bucket1_index] != 0);
  bucket[bucket1_index]--;
  bucket0[bucket1_index]++;
  
  balls0[ball0ID] = bucket1_index;
  balls[ball1ID] = bucket0_index;

}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

int main(int argc, char* argv[]){

  //Get arguments:
  assert((argc == 4) && "Need 3 arguments: (EXTRA_BUCKET_CAPACITY:[0-8] BN_BALL_THROWS:[1-10^5] SEED:[1-400])");
  EXTRA_BUCKET_CAPACITY = atoi(argv[1]);
  SPILL_THRESHOLD = BASE_WAYS_PER_SKEW + EXTRA_BUCKET_CAPACITY + REUSE_WAYS_PER_SKEW;
  NUM_BILLION_TRIES  = atoi(argv[2]);
  myseed = atoi(argv[3]);

  printf("Cache Configuration: %d MB, %d skews, %d ways (%d ways/skew)\n",CACHE_SZ_BYTES/1024/1024,NUM_SKEWS,NUM_SKEWS*(BASE_WAYS_PER_SKEW+REUSE_WAYS_PER_SKEW),(BASE_WAYS_PER_SKEW+REUSE_WAYS_PER_SKEW));
  printf("AVG-BALLS-PER-BUCKET:%d, BUCKET-SPILL-THRESHOLD:%d \n",BASE_WAYS_PER_SKEW+REUSE_WAYS_PER_SKEW,SPILL_THRESHOLD);
  printf("Simulation Parameters - BALL_THROWS:%llu, SEED:%d\n\n",(unsigned long long)NUM_BILLION_TRIES*(unsigned long long)BILLION_TRIES,myseed);
  
  uns64 ii;
  mtrand->seed(myseed);

  //Initialize Buckets
  init_buckets(0);
  init_buckets(1);

  //Ensure Total Balls in Buckets is Conserved.
  sanity_check(0);
  sanity_check(1);

  printf("Starting throws --  (Dot printed every 100M Ball throws) \n");

  //N Billion Ball Throws
  for (uns64 bn_i=0 ; bn_i < NUM_BILLION_TRIES; bn_i++) {    
    //1 Billion Ball Throws
    for(uns64 hundred_mn_count=0; hundred_mn_count<10; hundred_mn_count++){
      //In multiples of 100 Million Ball Throws.
      for(ii=0; ii<HUNDRED_MILLION_TRIES; ii++){
        //Insert and Remove Ball
        remove_and_insert(0);
        upgrade_and_downgrade();
        remove_and_insert(1);
      }
      printf(".");fflush(stdout);
    }    
    //Ensure Total Balls in Buckets is Conserved.
    sanity_check(0);
    sanity_check(1);
    //Print count of Balls Thrown.
    printf(" %dBn\n",bn_i+1);fflush(stdout);    
  }

  printf("\n\nBucket-Occupancy Snapshot at End of Experiment\n");
  display_histogram();
  printf("\n\n");

  printf("Distribution of Bucket-Occupancy (Averaged across Ball Throws) => Used for P(Bucket = k balls) calculation \n");
  printf("\nOccupancy: \t\t %16s \t P(Bucket=k balls)","Count");
  for(ii=0; ii<= MAX_FILL; ii++){
    double perc = 100.0 * (double)bucket_fill_observed[ii]/(2*NUM_SKEWS*(double)NUM_BILLION_TRIES*(double)BILLION_TRIES);
    printf("\nBucket[%2u Fill]: \t %16llu \t (%5.3f)", ii, bucket_fill_observed[ii], perc);
  }

  printf("\n\n\n");

  printf("Distribution of Balls-in-Dest-Bucket on Ball-Insertion (Best-Of-2 Indexed-Buckets) => Spill-Count = Spills from Bucket-With-%d-Balls\n",SPILL_THRESHOLD);
  //  printf("\n");
  printf("\nBalls-in-Dest-Bucket (k) \t\t Spills from Bucket-With-k-Balls)\n");
  for(ii=0; ii<MAX_FILL; ii++){
    double perc = 100.0*(double)(stat_counts[ii])/(double)(2*(double)NUM_BILLION_TRIES*(double)BILLION_TRIES);
    printf("%2llu:\t\t\t\t %16llu\t (%5.3f)\n", ii, stat_counts[ii], perc);
  }

  printf("\n\n");

  printf("\nSpill Count: %llu (%5.3f)\n", spill_count,
         100.0* (double)spill_count/(double)(2*(double)NUM_BILLION_TRIES*(double)BILLION_TRIES));
  printf("\nCuckoo Spill Count: %llu (%5.3f)\n", cuckoo_spill_count,
         100.0* (double)cuckoo_spill_count/(double)(2*(double)NUM_BILLION_TRIES*(double)BILLION_TRIES));

  return 0;
}
