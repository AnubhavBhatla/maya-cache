#include "champsim.h"

#define threads 1


#define L1I_SET 64
#define L1I_RQ_SIZE 64
#define L1I_WQ_SIZE 64
#define L1I_PQ_SIZE 8
#define L1I_MSHR_SIZE 8

#define L1D_SET 64
#define L1D_RQ_SIZE 64
#define L1D_WQ_SIZE 64
#define L1D_PQ_SIZE 8
#define L1D_MSHR_SIZE 16


#define L2C_SET 1024
#define L2C_RQ_SIZE 32
#define L2C_WQ_SIZE 32
#define L2C_PQ_SIZE 16   //16
#define L2C_MSHR_SIZE 32


#define LLC_SET 2048
#define LLC_RQ_SIZE L2C_MSHR_SIZE*2 //Original Code
#define LLC_WQ_SIZE L2C_MSHR_SIZE*2
#define LLC_PQ_SIZE 32 
#define LLC_MSHR_SIZE 64 



#define NUM_SLICES NUM_CPUS // currently we can have either NUM_SLICES = 1 or NUM_SLICES = NUM_CPUS 


#define DRAM_CHANNELS NUM_CPUS/4  ////   default: assuming one DIMM per one channel 4GB * 1 => 4GB off-chip memory
#define LOG2_DRAM_CHANNELS 2 

