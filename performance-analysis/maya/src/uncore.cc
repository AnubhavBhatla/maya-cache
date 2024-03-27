#include "uncore.h"

// uncore
UNCORE uncore;

// constructor
UNCORE::UNCORE() {
	//CACHE LLC{"LLC", LLC_SET, LLC_WAY, LLC_SET*LLC_WAY, LLC_WQ_SIZE, LLC_RQ_SIZE, LLC_PQ_SIZE, LLC_MSHR_SIZE};
	// for (int i = 0; i < NUM_SLICES; ++i) 
	// {
	// 	// LLC[i]=CACHE("LLC", LLC_SET, LLC_WAY, LLC_SET*LLC_WAY, LLC_WQ_SIZE, LLC_RQ_SIZE, LLC_PQ_SIZE, LLC_MSHR_SIZE);
	// 	// sprintf(LLC[i].NAME,"LLC_%d",i);
	// 	LLC[i].NUM_SET = LLC_SET/4;
	// 	LLC[i].NUM_WAY = LLC_WAY;
	// 	LLC[i].NUM_LINE = LLC[i].NUM_SET * LLC[i].NUM_WAY;
	// 	LLC[i].WQ_SIZE = LLC_WQ_SIZE/4;
	// 	LLC[i].RQ_SIZE = LLC_RQ_SIZE/4;
	// 	LLC[i].PQ_SIZE = LLC_PQ_SIZE/4;
	// 	LLC[i].MSHR_SIZE = LLC_MSHR_SIZE/4;


	// }

for (int i = 0; i < NUM_SLICES; ++i){
	LLC[i]= new CACHE("LLC"+ to_string(i), LLC_SET, LLC_WAY, LLC_SET*LLC_WAY, LLC_WQ_SIZE, LLC_RQ_SIZE, LLC_PQ_SIZE, LLC_MSHR_SIZE );
    // LLC.push_back(*temp);
    // cout<<"\nsize:"<<LLC.size()<<endl<<NUM_SLICES<<"\n";
}

}
