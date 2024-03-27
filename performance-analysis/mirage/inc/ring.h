#ifndef RING_H
#define RING_H

#include "block.h"
#include "memory_class.h"

#define QUEUE_PROC_LATENCY 1

uint32_t get_slice_num(uint64_t address);
int get_direction(int source,int destination);


class NetworkInterface{
public:
	//!size of router queues should depend on NUM_SLICES, but is this correct?
	//PACKET_QUEUE INQ {"INQ",NUM_SLICES* 8};//!Size of Inqueue = NUM_SLICES*8
	//PACKET_QUEUE OUTQ {"OUTQ",NUM_SLICES* 8};
        PACKET_QUEUE INQ {"INQ",NUM_SLICES*8};//!Size of Inqueue = NUM_SLICES*8
	PACKET_QUEUE OUTQ {"OUTQ",NUM_SLICES*8};
	int add_inq(PACKET *packet);
	int add_outq(PACKET *packet);

};
/*!	Each Router is connected to two other routers via NI, router i is connected to
	i+1 via NI[0] and i-1(for 0 Num_router-1)th router via NI[1], Insert packets in Inqueue of router  
*/
class ROUTER
{
public:
	unsigned int id;
	NetworkInterface NI[2];///NI[0] connects with Clockwise next router, NI[1] connects with Anti-clockwise next router 
	// bool IS_BRIDGE;
	MEMORY *lower_level=NULL,*upper_level=NULL;
	
	uint64_t stall_cycle;	//stat variable
	// int add_nq(int dir,PACKET *packet); //should it be generic like this?

	MEMORY *LLC_MAP[NUM_SLICES];

	ROUTER(){
		stall_cycle =0;
		// cout<<"DP::Router Constructor";
	}
	// ~ROUTER(){
	// 	cout<<"DP::Router Destructor\n";
	// };
	
};
class RING
{
public:
	uint32_t LATENCY;
	ROUTER router[NUM_SLICES];
	uint64_t links[NUM_SLICES];
	int MAX_READ = 4; //####### WHAT should be its value
	RING(){
	// cout<<"DP::Ring Constructor\n";
	};
	// ~RING(){
	// 	cout<<"DP::Ring Destructor\n";
	// };
	void operate();
};

extern RING ring;

#endif
