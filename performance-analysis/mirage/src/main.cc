#define _BSD_SOURCE

#include <getopt.h>
#include "ooo_cpu.h"
#include "uncore.h"
#include "ring.h"
#include <fstream>
#include "ceaser.h"



uint8_t warmup_complete[NUM_CPUS], 
        simulation_complete[NUM_CPUS], 
        all_warmup_complete = 0, 
        all_simulation_complete = 0,
        MAX_INSTR_DESTINATIONS = NUM_INSTR_DESTINATIONS,
        knob_cloudsuite = 0,
        knob_low_bandwidth = 0;

uint64_t warmup_instructions     = 1000000,
         simulation_instructions = 10000000,
         champsim_seed,
	 is_rob_stalled[NUM_CPUS];

time_t start_time;

// PAGE TABLE
uint32_t PAGE_TABLE_LATENCY = 0, SWAP_LATENCY = 0;
queue <uint64_t > page_queue;
map <uint64_t, uint64_t> page_table, inverse_table, recent_page, unique_cl[NUM_CPUS];
uint64_t previous_ppage, num_adjacent_page, num_cl[NUM_CPUS], allocated_pages, num_page[NUM_CPUS], minor_fault[NUM_CPUS], major_fault[NUM_CPUS];
uint32_t stall_the_attacker[NUM_CPUS], attacker_delay[NUM_CPUS], Attacker_deprioritize_in_DRAM=2500;
void record_roi_stats(uint32_t cpu, CACHE *cache)
{
    for (uint32_t i=0; i<NUM_TYPES; i++) {
        cache->roi_access[cpu][i] = cache->sim_access[cpu][i];
        cache->roi_hit[cpu][i] = cache->sim_hit[cpu][i];
        cache->roi_miss[cpu][i] = cache->sim_miss[cpu][i];

	cache->roi_miss_penalty[cpu][i] = cache->sim_miss_penalty[cpu][i];
    }
    	cache->roi_miss_latency[cpu] = cache->total_miss_latency[cpu];
//PREFETCHER
	cache->roi_pf_lower_level = cache->pf_lower_level;
	cache->roi_pf_useful = cache->pf_useful;
	cache->roi_pf_useless = cache->pf_useless;
	cache->roi_pf_late = cache->pf_late;
	cache->roi_pf_dropped = cache->pf_dropped;
	cache->roi_pf_requested = cache->pf_requested;
	cache->roi_pf_issued = cache->pf_issued;

}

void print_roi_stats(uint32_t cpu, CACHE *cache)
{
    uint64_t TOTAL_ACCESS = 0, TOTAL_HIT = 0, TOTAL_MISS = 0, TOTAL_MISS_PENALTY = 0;
    for (uint32_t i=0; i<NUM_TYPES; i++) {
        TOTAL_ACCESS += cache->roi_access[cpu][i];
        TOTAL_HIT += cache->roi_hit[cpu][i];
        TOTAL_MISS += cache->roi_miss[cpu][i];
	TOTAL_MISS_PENALTY += cache->roi_miss_penalty[cpu][i];
    }

    cout << cache->NAME;
    cout << " TOTAL     ACCESS: " << setw(5) << TOTAL_ACCESS << "  HIT: " << setw(5) << TOTAL_HIT <<  "  MISS: " << setw(5) << TOTAL_MISS;
    if(TOTAL_ACCESS != 0)
    	cout<< "  HIT %: " << setw(5) << ((double)TOTAL_HIT*100/TOTAL_ACCESS) << "  MISS %: " << setw(5) << ((double)TOTAL_MISS*100/TOTAL_ACCESS);
    else
	cout<< "  HIT %: " << setw(5) << "0" << "  MISS %: " << setw(5) << "0";

    if (TOTAL_MISS != 0)
         cout<< "  AVERAGE MISS PENALTY: " <<  float(TOTAL_MISS_PENALTY)/float(TOTAL_MISS) <<endl;
    else
        cout<< "  AVERAGE MISS PENALTY: 0"<<endl;
    

    cout << cache->NAME;
    cout << " LOAD      ACCESS: " << setw(5) << cache->roi_access[cpu][0] << "  HIT: " << setw(5) << cache->roi_hit[cpu][0] << "  MISS: " << setw(5) << cache->roi_miss[cpu][0];
    if (cache->roi_access[cpu][0] != 0)
   	cout<< "  HIT %: " << setw(5) << ((double)cache->roi_hit[cpu][0]*100/cache->roi_access[cpu][0]) << "  MISS %: " << setw(5) << ((double)cache->roi_miss[cpu][0]*100/cache->roi_access[cpu][0]);
    else
        cout<< "  HIT %: " << setw(5) << "0" << "  MISS %: " << setw(5) << "0";


    if (cache->roi_miss_penalty[cpu][0] != 0)
         cout<< "  AVERAGE MISS PENALTY: " <<  float(cache->roi_miss_penalty[cpu][0])/float(cache->roi_miss[cpu][0]) <<endl;
    else
        cout<< "  AVERAGE MISS PENALTY: 0"<<endl;


    cout << cache->NAME;
    cout << " RFO       ACCESS: " << setw(5) << cache->roi_access[cpu][1] << "  HIT: " << setw(5) << cache->roi_hit[cpu][1] << "  MISS: " << setw(5) << cache->roi_miss[cpu][1]; 
    if (cache->roi_access[cpu][1] != 0)
    	cout<<"  HIT %: " << setw(5) << ((double)cache->roi_hit[cpu][1]*100/cache->roi_access[cpu][1]) << "  MISS %: " << setw(5) << ((double)cache->roi_miss[cpu][1]*100/cache->roi_access[cpu][1]);
    else
        cout<< "  HIT %: " << setw(5) << "0" << "  MISS %: " << setw(5) << "0";

    if (cache->roi_miss_penalty[cpu][1] != 0)
         cout<< "  AVERAGE MISS PENALTY: " <<  float(cache->roi_miss_penalty[cpu][1])/float(cache->roi_miss[cpu][1]) <<endl;

    else
        cout<< "  AVERAGE MISS PENALTY: 0"<<endl;


    cout << cache->NAME;
    cout << " PREFETCH  ACCESS: " << setw(5) << cache->roi_access[cpu][2] << "  HIT: " << setw(5) << cache->roi_hit[cpu][2] << "  MISS: " << setw(5) << cache->roi_miss[cpu][2];
    if (cache->roi_access[cpu][2] != 0)
   cout<< "  HIT %: " << setw(5) << ((double)cache->roi_hit[cpu][2]*100/cache->roi_access[cpu][2]) << "  MISS %: " << setw(5) << ((double)cache->roi_miss[cpu][2]*100/cache->roi_access[cpu][2]);
        else
        cout<< "  HIT %: " << setw(5) << "0" << "  MISS %: " << setw(5) << "0";


    if (cache->roi_miss_penalty[cpu][2] != 0)
         cout<< "  AVERAGE MISS PENALTY: " <<  float(cache->roi_miss_penalty[cpu][2])/float(cache->roi_miss[cpu][2]) <<endl;

    else
        cout<< "  AVERAGE MISS PENALTY: 0"<<endl;

    cout << cache->NAME;
    cout << " WRITEBACK ACCESS: " << setw(5) << cache->roi_access[cpu][3] << "  HIT: " << setw(5) << cache->roi_hit[cpu][3] << "  MISS: " << setw(5) << cache->roi_miss[cpu][3];
    if (cache->roi_access[cpu][3] != 0)
   	cout<< "  HIT %: " << setw(5) << ((double)cache->roi_hit[cpu][3]*100/cache->roi_access[cpu][3]) << "  MISS %: " << setw(5) << ((double)cache->roi_miss[cpu][3]*100/cache->roi_access[cpu][3]);
        else
        cout<< "  HIT %: " << setw(5) << "0" << "  MISS %: " << setw(5) << "0";


    if (cache->roi_miss_penalty[cpu][3] != 0)
         cout<< "  AVERAGE MISS PENALTY: " <<  float(cache->roi_miss_penalty[cpu][3])/float(cache->roi_miss[cpu][3]) <<endl;

    else
        cout<< "  AVERAGE MISS PENALTY: 0"<<endl;

    cout << cache->NAME;
    cout << " PREFETCH  REQUESTED: " << setw(5) << cache->roi_pf_requested << "  ISSUED: " << setw(5) << cache->roi_pf_issued;
    cout << "  USEFUL: " << setw(5) << cache->roi_pf_useful << "  USELESS: " << setw(5) << cache->roi_pf_useless << endl;
        cout << " USEFUL LOAD PREFETCHES: " << setw(10) << cache->roi_pf_useful
         << " PREFETCH ISSUED TO LOWER LEVEL: " << setw(10)
         << cache->roi_pf_lower_level << "  ACCURACY: "
         << ((double)cache->roi_pf_useful * 100 / cache->roi_pf_lower_level) << endl;
    cout << " TIMELY PREFETCHES: " << setw(10) << cache->roi_pf_useful
         << " LATE PREFETCHES: " << cache->roi_pf_late 
         << " DROPPED PREFETCHES: " << cache->roi_pf_dropped << endl;
    
    cout << cache->NAME;
    cout << " AVERAGE MISS LATENCY: " << (1.0*(cache->roi_miss_latency[cpu]))/TOTAL_MISS << " cycles" << endl;

    if (cache->NAME == "LLC0"){
        cout << "Deadblocks : " << cache->counter_deadblock << " Deadblock percentage : " << (100.0*(cache->counter_deadblock))/cache->roi_miss[cpu][0] << endl;
        cout <<endl;
    }
}


void print_sim_stats(uint32_t cpu, CACHE *cache)
{
    uint64_t TOTAL_ACCESS = 0, TOTAL_HIT = 0, TOTAL_MISS = 0, TOTAL_MISS_PENALTY = 0;

    for (uint32_t i=0; i<NUM_TYPES; i++) {
        TOTAL_ACCESS += cache->sim_access[cpu][i];
        TOTAL_HIT += cache->sim_hit[cpu][i];
        TOTAL_MISS += cache->sim_miss[cpu][i];
	TOTAL_MISS_PENALTY += cache->sim_miss_penalty[cpu][i];
    }

    cout << cache->NAME;
    cout << " TOTAL     ACCESS: " << setw(5) << TOTAL_ACCESS << "  HIT: " << setw(5) << TOTAL_HIT <<  "  MISS: " << setw(5) << TOTAL_MISS;
    if (TOTAL_ACCESS!=0)
    cout << "  HIT %: " << setw(5) << ((double)TOTAL_HIT*100/TOTAL_ACCESS) << "  MISS %: " << setw(5) << ((double)TOTAL_MISS*100/TOTAL_ACCESS);
        else
        cout<< "  HIT %: " << setw(5) << "0" << "  MISS %: " << setw(5) << "0";


    if (TOTAL_MISS != 0)
            cout<< "  AVERAGE MISS PENALTY: " <<  float(TOTAL_MISS_PENALTY)/float(TOTAL_MISS) <<endl;
    else
                cout<< "  AVERAGE MISS PENALTY: 0"<<endl;


    cout << cache->NAME;
    cout << " LOAD      ACCESS: " << setw(5) << cache->sim_access[cpu][0] << "  HIT: " << setw(5) << cache->sim_hit[cpu][0] << "  MISS: " << setw(5) << cache->sim_miss[cpu][0]; 
    if (cache->sim_access[cpu][0]!=0)
    cout << "  HIT %: " << setw(5) << ((double)cache->sim_hit[cpu][0]*100/cache->sim_access[cpu][0]) << "  MISS %: " << setw(5) << ((double)cache->sim_miss[cpu][0]*100/cache->sim_access[cpu][0]);
        else
        cout<< "  HIT %: " << setw(5) << "0" << "  MISS %: " << setw(5) << "0";


    if (cache->sim_miss_penalty[cpu][0] != 0)
         cout<< "  AVERAGE MISS PENALTY: " <<  float(cache->sim_miss_penalty[cpu][0])/float(cache->sim_miss[cpu][0]) <<endl;
    else
        cout<< "  AVERAGE MISS PENALTY: 0"<<endl;

    cout << cache->NAME;
    cout << " RFO       ACCESS: " << setw(5) << cache->sim_access[cpu][1] << "  HIT: " << setw(5) << cache->sim_hit[cpu][1] << "  MISS: " << setw(5) << cache->sim_miss[cpu][1];
    if(cache->sim_access[cpu][1]!=0)
    cout << "  HIT %: " << setw(5) << ((double)cache->sim_hit[cpu][1]*100/cache->sim_access[cpu][1]) << "  MISS %: " << setw(5) << ((double)cache->sim_miss[cpu][1]*100/cache->sim_access[cpu][1]);
        else
        cout<< "  HIT %: " << setw(5) << "0" << "  MISS %: " << setw(5) << "0";


    if (cache->sim_miss_penalty[cpu][1] != 0)
         cout<< "  AVERAGE MISS PENALTY: " <<  float(cache->sim_miss_penalty[cpu][1])/float(cache->sim_miss[cpu][1]) <<endl;
    else
        cout<< "  AVERAGE MISS PENALTY: 0"<<endl;

    cout << cache->NAME;
    cout << " PREFETCH  ACCESS: " << setw(5) << cache->sim_access[cpu][2] << "  HIT: " << setw(5) << cache->sim_hit[cpu][2] << "  MISS: " << setw(5) << cache->sim_miss[cpu][2];
    if (cache->sim_access[cpu][2] != 0)
    cout << "  HIT %: " << setw(5) << ((double)cache->sim_hit[cpu][2]*100/cache->sim_access[cpu][2]) << "  MISS %: " << setw(5) << ((double)cache->sim_miss[cpu][2]*100/cache->sim_access[cpu][2]);
        else
        cout<< "  HIT %: " << setw(5) << "0" << "  MISS %: " << setw(5) << "0";


    if (cache->sim_miss_penalty[cpu][2] != 0)
         cout<< "  AVERAGE MISS PENALTY: " <<  float(cache->sim_miss_penalty[cpu][2])/float(cache->sim_miss[cpu][2]) <<endl;
    else
        cout<< "  AVERAGE MISS PENALTY: 0"<<endl;


    cout << cache->NAME;
    cout << " WRITEBACK ACCESS: " << setw(5) << cache->sim_access[cpu][3] << "  HIT: " << setw(5) << cache->sim_hit[cpu][3] << "  MISS: " << setw(5) << cache->sim_miss[cpu][3];
    if(cache->sim_access[cpu][3]!=0)
   cout << "  HIT %: " << setw(5) << ((double)cache->sim_hit[cpu][3]*100/cache->sim_access[cpu][3]) << "  MISS %: " << setw(5) << ((double)cache->sim_miss[cpu][3]*100/cache->sim_access[cpu][3]);
        else
        cout<< "  HIT %: " << setw(5) << "0" << "  MISS %: " << setw(5) << "0";

    if (cache->sim_miss_penalty[cpu][3] != 0)
         cout<< "  AVERAGE MISS PENALTY: " <<  float(cache->sim_miss_penalty[cpu][3])/float(cache->sim_miss[cpu][3]) <<endl;
    else
        cout<< "  AVERAGE MISS PENALTY: 0"<<endl;
    cout<<endl;

}

void print_branch_stats()
{
    for (uint32_t i=0; i<NUM_CPUS; i++) {
        cout << endl << "CPU " << i << " Branch Prediction Accuracy: ";
        cout << (100.0*(ooo_cpu[i].num_branch - ooo_cpu[i].branch_mispredictions)) / ooo_cpu[i].num_branch;
        cout << "% MPKI: " << (1000.0*ooo_cpu[i].branch_mispredictions)/(ooo_cpu[i].num_retired - ooo_cpu[i].warmup_instructions);
	cout << " Average ROB Occupancy at Mispredict: " << (1.0*ooo_cpu[i].total_rob_occupancy_at_branch_mispredict)/ooo_cpu[i].branch_mispredictions << endl;
    }
}
void print_analysis_for_DOS_stats(){
	
	for(int cpu=0;cpu<NUM_CPUS;cpu++)
	{
		unsigned long long total_wait_time_rq=0,total_wait_time_pq=0,total_wait_time_mshr=0,total_enc_time_rq=0,total_enc_time_pq=0,total_enc_time_mshr=0,total_acc_time_rq=0,total_acc_time_pq=0,total_acc_time_mshr=0,total_read_packet=0,total_pre_packet=0,total_mshr_packet=0;
			cout<<"CPU : " <<cpu<<"\n";
		for(int slice_num=0; slice_num<NUM_SLICES; slice_num++)
		{
			total_wait_time_rq += uncore.LLC[slice_num]->total_waiting_time_in_rq[cpu];
			total_wait_time_pq += uncore.LLC[slice_num]->total_waiting_time_in_pq[cpu];	
			total_wait_time_mshr += uncore.LLC[slice_num]->total_waiting_time_in_mshr[cpu];
			total_enc_time_rq += uncore.LLC[slice_num]->total_encryption_time_rq[cpu];
			total_enc_time_pq += uncore.LLC[slice_num]->total_encryption_time_pq[cpu];
			total_enc_time_mshr += uncore.LLC[slice_num]->total_encryption_time_mshr[cpu];
			total_acc_time_rq += uncore.LLC[slice_num]->total_access_time_rq[cpu];
			total_acc_time_pq += uncore.LLC[slice_num]->total_access_time_pq[cpu];
			total_acc_time_mshr += uncore.LLC[slice_num]->total_access_time_mshr[cpu];
		
			total_read_packet += uncore.LLC[slice_num]->total_read_packets[cpu];
			total_pre_packet += uncore.LLC[slice_num]->total_prefetch_packets[cpu];
			total_mshr_packet += uncore.LLC[slice_num]->total_mshr_packets[cpu];
		}
		cout<<"Total Encryption Time Lapsed in RQ = "<<total_enc_time_rq<<"\n";
		cout<<"Total Encryption Time Lapsed in MSHR = "<<total_enc_time_mshr<<"\n";
		cout<<"Average Time Lapsed in RQ = "<<(float)total_wait_time_rq/(float)total_read_packet<<"\n";
		cout<<"Average Time Lapsed in PQ = "<<(float)total_wait_time_pq/(float)total_pre_packet<<"\n";
		cout<<"Average Time Lapsed in MSHR = "<<(float)total_wait_time_mshr/(float)total_mshr_packet<<"\n";
		cout<<"Average Encryption Time by Read Packet = "<<(float)total_enc_time_rq/(float)total_read_packet<<"\n";
		cout<<"Average Encryption Time by Prefetch Packet = "<<(float)total_enc_time_pq/(float)total_pre_packet<<"\n";
		cout<<"Average Encryption Time by MSHR Packet = "<<(float)total_enc_time_mshr/(float)total_mshr_packet<<"\n";
		cout<<"Average Access Time by Read Packet = "<<(float)total_acc_time_rq/(float)total_read_packet<<"\n";
		cout<<"Average Access Time by Pref Packet = "<<(float)total_acc_time_pq/(float)total_pre_packet<<"\n";
		cout<<"Average Access Time by MSHR Packet = "<<(float)total_acc_time_mshr/(float)total_mshr_packet<<"\n";
	}

}
void print_interconnect_stats(){
    uint64_t max_cycles =0;

    for (int i = 0; i < NUM_SLICES; ++i)
    {
        if (ooo_cpu[i].finish_sim_cycle > max_cycles)
        {
            max_cycles = ooo_cpu[i].finish_sim_cycle; 
        }
    }

cout << "Interconnect Stats "<<endl;
    for (int i = 0; i < NUM_SLICES; ++i)
    {
        cout<<"LINK "<<i<<" BW : " << ((float)ring.links[i] / max_cycles);
    }
}

void print_dram_stats()
{
    cout << endl;
    cout << "DRAM Statistics" << endl;
    for (uint32_t i=0; i<DRAM_CHANNELS; i++) {
        cout << " CHANNEL " << i << endl;
        cout << " RQ ROW_BUFFER_HIT: " << setw(5) << uncore.DRAM.RQ[i].ROW_BUFFER_HIT << "  ROW_BUFFER_MISS: " << setw(5) << uncore.DRAM.RQ[i].ROW_BUFFER_MISS << endl;
        cout << " DBUS_CONGESTED: " << setw(5) << uncore.DRAM.dbus_congested[NUM_TYPES][NUM_TYPES] << endl; 
        cout << " WQ ROW_BUFFER_HIT: " << setw(5) << uncore.DRAM.WQ[i].ROW_BUFFER_HIT << "  ROW_BUFFER_MISS: " << setw(5) << uncore.DRAM.WQ[i].ROW_BUFFER_MISS;
        cout << "  FULL: " << setw(5) << uncore.DRAM.WQ[i].FULL << endl; 
        cout << endl;
    }

    uint64_t total_congested_cycle = 0;
    for (uint32_t i=0; i<DRAM_CHANNELS; i++)
        total_congested_cycle += uncore.DRAM.dbus_cycle_congested[i];
    if (uncore.DRAM.dbus_congested[NUM_TYPES][NUM_TYPES])
        cout << " AVG_CONGESTED_CYCLE: " << (total_congested_cycle / uncore.DRAM.dbus_congested[NUM_TYPES][NUM_TYPES]) << endl;
    else
        cout << " AVG_CONGESTED_CYCLE: -" << endl;
}

void reset_cache_stats(uint32_t cpu, CACHE *cache)
{
    for (uint32_t i=0; i<NUM_TYPES; i++) {
        cache->ACCESS[i] = 0;
        cache->HIT[i] = 0;
        cache->MISS[i] = 0;
        cache->MSHR_MERGED[i] = 0;
        cache->STALL[i] = 0;

        cache->sim_access[cpu][i] = 0;
        cache->sim_hit[cpu][i] = 0;
        cache->sim_miss[cpu][i] = 0;
	cache->sim_miss_penalty[cpu][i] =0;
    }
    for (uint32_t j=0; j<NUM_CPUS; j++){
	 cache->total_waiting_time_in_mshr[j]=0;
	 cache->total_mshr_packets[j]=0;
	 cache->total_waiting_time_in_rq[j]=0;
	 cache->total_read_packets[j]=0;
	 cache->total_waiting_time_in_pq[j]=0;
	 cache->total_prefetch_packets[j]=0;
	for(uint32_t i=0;i<2048;i++)
	{
		cache->set_access_counter[j][i] = 0;
	}
    }
    cache->total_miss_latency[cpu] = 0;

    cache->RQ.ACCESS = 0;
    cache->RQ.MERGED = 0;
    cache->RQ.TO_CACHE = 0;

    cache->WQ.ACCESS = 0;
    cache->WQ.MERGED = 0;
    cache->WQ.TO_CACHE = 0;
    cache->WQ.FORWARD = 0;
    cache->WQ.FULL = 0;

    cache->pf_requested = 0;
    cache->pf_issued = 0;
    cache->pf_useful = 0;
    cache->pf_useless = 0;
    cache->pf_fill = 0;
    cache->pf_late = 0;
    cache->pf_dropped=0;
    cache->pf_lower_level = 0;

    cache->counter_deadblock = 0;
}

void finish_warmup()
{
	for(int i=0; i<NUM_SLICES; i++)
	{
        	uncore.LLC[i]->total_sets_remapped=0;
       		uncore.LLC[i]->total_llc_stalled=0;
        	uncore.LLC[i]->total_stall_cycle=0;
		uncore.LLC[i]->blocks_less_evicted=0;
	}
    uint64_t elapsed_second = (uint64_t)(time(NULL) - start_time),
             elapsed_minute = elapsed_second / 60,
             elapsed_hour = elapsed_minute / 60;
    elapsed_minute -= elapsed_hour*60;
    elapsed_second -= (elapsed_hour*3600 + elapsed_minute*60);

    // reset core latency
    SCHEDULING_LATENCY = 6;
    EXEC_LATENCY = 1;
    PAGE_TABLE_LATENCY = 100;
    SWAP_LATENCY = 100000;

    cout << endl;
    for (uint32_t i=0; i<NUM_CPUS; i++) {
        cout << "Warmup complete CPU " << i << " instructions: " << ooo_cpu[i].num_retired << " cycles: " << current_core_cycle[i];
        cout << " (Simulation time: " << elapsed_hour << " hr " << elapsed_minute << " min " << elapsed_second << " sec) " << endl;

        ooo_cpu[i].begin_sim_cycle = current_core_cycle[i]; 
        ooo_cpu[i].begin_sim_instr = ooo_cpu[i].num_retired;

        // reset branch stats
        ooo_cpu[i].num_branch = 0;
        ooo_cpu[i].branch_mispredictions = 0;
	ooo_cpu[i].total_rob_occupancy_at_branch_mispredict = 0;

		total_rob_stall_cycles[i]=0;
		total_load_rob_stall_cycles[i]=0;
		rob_stall_count[i]=0;
		load_rob_stall_count[i]=0;
        //reset rob stall counter and cycles
	        ooo_cpu[i].sim_rob_stall_counter = 0;
	        ooo_cpu[i].sim_rob_stall_cycle = 0;
	                
	reset_cache_stats(i, &ooo_cpu[i].ITLB);
        reset_cache_stats(i, &ooo_cpu[i].DTLB);
 	reset_cache_stats(i, &ooo_cpu[i].STLB);

        reset_cache_stats(i, &ooo_cpu[i].L1I);
        reset_cache_stats(i, &ooo_cpu[i].L1D);
        reset_cache_stats(i, &ooo_cpu[i].L2C);
        for (int slice_num = 0; slice_num < NUM_SLICES; slice_num++)
        reset_cache_stats(i, uncore.LLC[slice_num]);
    }
    cout << endl;

    // reset DRAM stats
    for (uint32_t i=0; i<DRAM_CHANNELS; i++) {
        uncore.DRAM.RQ[i].ROW_BUFFER_HIT = 0;
        uncore.DRAM.RQ[i].ROW_BUFFER_MISS = 0;
        uncore.DRAM.WQ[i].ROW_BUFFER_HIT = 0;
        uncore.DRAM.WQ[i].ROW_BUFFER_MISS = 0;
    }

    // set actual cache latency
    for (uint32_t i=0; i<NUM_CPUS; i++) {
        ooo_cpu[i].ITLB.LATENCY = ITLB_LATENCY;
        ooo_cpu[i].DTLB.LATENCY = DTLB_LATENCY;
        ooo_cpu[i].STLB.LATENCY = STLB_LATENCY;
        ooo_cpu[i].L1I.LATENCY  = L1I_LATENCY;
        ooo_cpu[i].L1D.LATENCY  = L1D_LATENCY;
        ooo_cpu[i].L2C.LATENCY  = L2C_LATENCY;
    }
    for (int slice_num = 0; slice_num < NUM_SLICES; slice_num++)
    uncore.LLC[slice_num]->LATENCY = LLC_LATENCY;
}

void print_deadlock(uint32_t i)
{
    cout << "DEADLOCK! CPU " << i << " instr_id: " << ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].instr_id;
    cout << " translated: " << +ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].translated;
    cout << " fetched: " << +ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].fetched;
    cout << " scheduled: " << +ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].scheduled;
    cout << " executed: " << +ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].executed;
    cout << " is_memory: " << +ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].is_memory;
    cout << " event: " << ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].event_cycle;
    cout << " current: " << current_core_cycle[i] << endl;
    cout << "Stall" << stall_cycle[i] << endl;
    // print LQ entry
    cout << endl << "Load Queue Entry" << endl;
    for (uint32_t j=0; j<LQ_SIZE; j++) {
        cout << "[LQ] entry: " << j << " instr_id: " << ooo_cpu[i].LQ.entry[j].instr_id << " address: " << hex << ooo_cpu[i].LQ.entry[j].physical_address << dec << " translated: " << +ooo_cpu[i].LQ.entry[j].translated << " fetched: " << +ooo_cpu[i].LQ.entry[i].fetched << endl;
    }

    // print SQ entry
    cout << endl << "Store Queue Entry" << endl;
    for (uint32_t j=0; j<SQ_SIZE; j++) {
        cout << "[SQ] entry: " << j << " instr_id: " << ooo_cpu[i].SQ.entry[j].instr_id << " address: " << hex << ooo_cpu[i].SQ.entry[j].physical_address << dec << " translated: " << +ooo_cpu[i].SQ.entry[j].translated << " fetched: " << +ooo_cpu[i].SQ.entry[i].fetched << endl;
    }

    // print L1D MSHR entry
    PACKET_QUEUE *queue;
    queue = &ooo_cpu[i].L1D.MSHR;
    cout << endl << queue->NAME << " Entry" << endl;
    for (uint32_t j=0; j<queue->SIZE; j++) {
        cout << "[" << queue->NAME << "] entry: " << j << " instr_id: " << queue->entry[j].instr_id << " rob_index: " << queue->entry[j].rob_index;
        cout << " address: " << hex << queue->entry[j].address << " full_addr: " << queue->entry[j].full_addr << dec << " type: " << +queue->entry[j].type;
        cout<<"forL2"<<queue->entry[j].forL2<<endl;
        cout << " fill_level: " << queue->entry[j].fill_level << " lq_index: " << queue->entry[j].lq_index << " sq_index: " << queue->entry[j].sq_index << endl; 
    }

    queue = &ooo_cpu[i].L1D.RQ;
    cout << endl << queue->NAME << "L1DRQ Entry" << endl;
    for (uint32_t j=0; j<queue->SIZE; j++) {
        if (ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].instr_id == queue->entry[j].instr_id)
        {
        cout << "[" << queue->NAME << "] entry: " << j << " instr_id: " << queue->entry[j].instr_id << " rob_index: " << queue->entry[j].rob_index;
        cout << " address: " << hex << queue->entry[j].address << " full_addr: " << queue->entry[j].full_addr << dec << " type: " << +queue->entry[j].type;
        cout<<"forL2"<<queue->entry[j].forL2<<endl;
        cout << " fill_level: " << queue->entry[j].fill_level << " lq_index: " << queue->entry[j].lq_index << " sq_index: " << queue->entry[j].sq_index << endl;                         
        }
    }
    queue = &ooo_cpu[i].L1D.WQ;
    cout << endl << queue->NAME << "L1DWQ Entry" << endl;
    for (uint32_t j=0; j<queue->SIZE; j++) {
        if (ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].instr_id == queue->entry[j].instr_id)
        {
        cout << "[" << queue->NAME << "] entry: " << j << " instr_id: " << queue->entry[j].instr_id << " rob_index: " << queue->entry[j].rob_index;
        cout << " address: " << hex << queue->entry[j].address << " full_addr: " << queue->entry[j].full_addr << dec << " type: " << +queue->entry[j].type;
        cout<<"forL2"<<queue->entry[j].forL2<<endl;
        cout << " fill_level: " << queue->entry[j].fill_level << " lq_index: " << queue->entry[j].lq_index << " sq_index: " << queue->entry[j].sq_index << endl;                         
        }
    }
    queue = &ooo_cpu[i].L2C.MSHR;
    cout << endl << queue->NAME << "L2MSHR Entry" << endl;
    for (uint32_t j=0; j<queue->SIZE; j++) {
        if (ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].instr_id == queue->entry[j].instr_id)
        {
        cout << "[" << queue->NAME << "] entry: " << j << " instr_id: " << queue->entry[j].instr_id << " rob_index: " << queue->entry[j].rob_index;
        cout << " address: " << hex << queue->entry[j].address << " full_addr: " << queue->entry[j].full_addr << dec << " type: " << +queue->entry[j].type;
        cout<<"forL2"<<queue->entry[j].forL2<<endl;
        cout << " fill_level: " << queue->entry[j].fill_level << " lq_index: " << queue->entry[j].lq_index << " sq_index: " << queue->entry[j].sq_index << endl;                         
        }
    }
    queue = &ooo_cpu[i].L2C.RQ;
    cout << endl << queue->NAME << "L2CRQ Entry" << endl;
    for (uint32_t j=0; j<queue->SIZE; j++) {
        if (ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].instr_id == queue->entry[j].instr_id)
        {
        cout << "[" << queue->NAME << "] entry: " << j << " instr_id: " << queue->entry[j].instr_id << " rob_index: " << queue->entry[j].rob_index;
        cout << " address: " << hex << queue->entry[j].address << " full_addr: " << queue->entry[j].full_addr << dec << " type: " << +queue->entry[j].type;
        cout<<"forL2"<<queue->entry[j].forL2<<endl;
        cout << " fill_level: " << queue->entry[j].fill_level << " lq_index: " << queue->entry[j].lq_index << " sq_index: " << queue->entry[j].sq_index << endl;                         
        }
    }
    queue = &ooo_cpu[i].L2C.PQ;
    cout << endl << queue->NAME << "L2CPQ Entry" << endl;
    for (uint32_t j=0; j<queue->SIZE; j++) {
        if (ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].instr_id == queue->entry[j].instr_id)
        {
        cout << "[" << queue->NAME << "] entry: " << j << " instr_id: " << queue->entry[j].instr_id << " rob_index: " << queue->entry[j].rob_index;
        cout << " address: " << hex << queue->entry[j].address << " full_addr: " << queue->entry[j].full_addr << dec << " type: " << +queue->entry[j].type;
        cout<<"forL2"<<queue->entry[j].forL2<<endl;
        cout << " fill_level: " << queue->entry[j].fill_level << " lq_index: " << queue->entry[j].lq_index << " sq_index: " << queue->entry[j].sq_index << endl;                         
        }
    }
    for (int j = 0; j < NUM_SLICES; ++j)
    {
    queue = &(uncore.LLC[j]->MSHR);
    cout << endl << queue->NAME<<"LLCMSHR"<<j << " Entry" << endl;
    for (uint32_t j=0; j<queue->SIZE; j++) {
        if (ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].instr_id == queue->entry[j].instr_id)
        {
        cout << "[" << queue->NAME << "] entry: " << j << " instr_id: " << queue->entry[j].instr_id << " rob_index: " << queue->entry[j].rob_index;
        cout << " address: " << hex << queue->entry[j].address << " full_addr: " << queue->entry[j].full_addr << dec << " type: " << +queue->entry[j].type;
        cout<<"forL2"<<queue->entry[j].forL2<<endl;
        cout << " fill_level: " << queue->entry[j].fill_level << " lq_index: " << queue->entry[j].lq_index << " sq_index: " << queue->entry[j].sq_index << endl;                         
        }
    }
    queue = &(uncore.LLC[j]->RQ);
    cout << endl << queue->NAME<<"LLCRQ"<<j << " Entry" << endl;
    for (uint32_t j=0; j<queue->SIZE; j++) {
        if (ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].instr_id == queue->entry[j].instr_id)
        {
        cout << "[" << queue->NAME << "] entry: " << j << " instr_id: " << queue->entry[j].instr_id << " rob_index: " << queue->entry[j].rob_index;
        cout << " address: " << hex << queue->entry[j].address << " full_addr: " << queue->entry[j].full_addr << dec << " type: " << +queue->entry[j].type;
        cout<<"forL2"<<queue->entry[j].forL2<<endl;
        cout << " fill_level: " << queue->entry[j].fill_level << " lq_index: " << queue->entry[j].lq_index << " sq_index: " << queue->entry[j].sq_index << endl;                         
        }
    }
    queue = &(uncore.LLC[j]->WQ);
    cout << endl << queue->NAME<<"LLCWQ"<<j << " Entry" << endl;
    for (uint32_t j=0; j<queue->SIZE; j++) {
        if (ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].instr_id == queue->entry[j].instr_id)
        {
        cout << "[" << queue->NAME << "] entry: " << j << " instr_id: " << queue->entry[j].instr_id << " rob_index: " << queue->entry[j].rob_index;
	cout << " address: " << hex << queue->entry[j].address << " full_addr: " << queue->entry[j].full_addr << dec << " type: " << +queue->entry[j].type;
        cout<<"forL2"<<queue->entry[j].forL2<<endl;
        cout << " fill_level: " << queue->entry[j].fill_level << " lq_index: " << queue->entry[j].lq_index << " sq_index: " << queue->entry[j].sq_index << endl;                         
        }
    }
    queue = &(uncore.LLC[j]->PQ);
    cout << endl << queue->NAME<<"LLCPQ"<<j << " Entry" << endl;
    for (uint32_t j=0; j<queue->SIZE; j++) {
        if (ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].instr_id == queue->entry[j].instr_id)
        {
        cout << "[" << queue->NAME << "] entry: " << j << " instr_id: " << queue->entry[j].instr_id << " rob_index: " << queue->entry[j].rob_index;
        cout << " address: " << hex << queue->entry[j].address << " full_addr: " << queue->entry[j].full_addr << dec << " type: " << +queue->entry[j].type;
        cout<<"forL2"<<queue->entry[j].forL2<<endl;
        cout << " fill_level: " << queue->entry[j].fill_level << " lq_index: " << queue->entry[j].lq_index << " sq_index: " << queue->entry[j].sq_index << endl;                         
        }
    }
    static int flag=0;
  for(int i=0;i<NUM_CPUS && flag==0 ;i++){
	  cout<<"CPU number "<<i<<"\n";
	  cout<<"\n*****************L1 **********************\n";
      queue = &ooo_cpu[i].L1D.MSHR;
      cout<<"\nMSHR occupancy: "<<queue->occupancy<<" Size: "<<queue->SIZE<<"\n\n";
      for (uint32_t j=0; j<queue->SIZE; j++) {
        if(queue->entry[j].address != 0)
        cout << " address: " << hex << queue->entry[j].address << " full_addr: " << queue->entry[j].full_addr << dec << " instr_id: " << queue->entry[j].instr_id<<" returned "<<(uint32_t)queue->entry[j].returned<< "\n";
	 }
      queue = &ooo_cpu[i].L1D.RQ;

      cout<<"\nRQ occupancy: "<<queue->occupancy<<" Size: "<<queue->SIZE<<"\n";
     for (uint32_t j=0; j<queue->SIZE; j++) {
      if(queue->entry[j].address != 0)
        cout << " address: " << hex << queue->entry[j].address << " full_addr: " << queue->entry[j].full_addr << dec << " instr_id: " << queue->entry[j].instr_id<<"\n";
         }
      queue = &ooo_cpu[i].L1D.WQ;
      cout<<"\nWQ occupancy: "<<queue->occupancy<<" Size: "<<queue->SIZE<<"\n";
      for (uint32_t j=0; j<queue->SIZE; j++) {
	if(queue->entry[j].address != 0)
        cout << " address: " << hex << queue->entry[j].address << " full_addr: " << queue->entry[j].full_addr << dec << " instr_id: " << queue->entry[j].instr_id<<"\n";
         }
      queue = &ooo_cpu[i].L1D.PQ;
      cout<<"\nPQ occupancy: "<<queue->occupancy<<" Size: "<<queue->SIZE<<"\n";
      cout<<"\n*****************L2 **********************\n";
      queue = &ooo_cpu[i].L2C.MSHR;
      cout<<"\nMSHR occupancy: "<<queue->occupancy<<" Size: "<<queue->SIZE<<"\n\n";
      for (uint32_t j=0; j<queue->SIZE; j++) {
	      if(queue->entry[j].address != 0)
        cout << " address: " << hex << queue->entry[j].address << " full_addr: " << queue->entry[j].full_addr << dec << " instr_id: " << queue->entry[j].instr_id<<" returned "<<(uint32_t)queue->entry[j].returned<<"\n";
         }
     queue = &ooo_cpu[i].L2C.RQ;
      cout<<"\nRQ occupancy: "<<queue->occupancy<<" Size: "<<queue->SIZE<<"\n";
     for (uint32_t j=0; j<queue->SIZE; j++) {
	     if(queue->entry[j].address != 0)
        cout << " address: " << hex << queue->entry[j].address << " full_addr: " << queue->entry[j].full_addr << dec << " instr_id: " << queue->entry[j].instr_id<<"\n";
         }
      queue = &ooo_cpu[i].L2C.WQ;
      cout<<"\nWQ occupancy: "<<queue->occupancy<<" Size: "<<queue->SIZE<<"\n";
      for (uint32_t j=0; j<queue->SIZE; j++) 
      {
	      if(queue->entry[j].address != 0)
        cout << " address: " << hex << queue->entry[j].address << " full_addr: " << queue->entry[j].full_addr << dec << " instr_id: " << queue->entry[j].instr_id<<"\n";
         }
      queue = &ooo_cpu[i].L2C.PQ;
      cout<<"\nPQ occupancy: "<<queue->occupancy<<" Size: "<<queue->SIZE<<"\n";
       cout<<"\n*****************LLC***********************\n";
      queue = &(uncore.LLC[i]->MSHR);
       cout<<"\nMSHR occupancy: "<<queue->occupancy<<" Size: "<<queue->SIZE<<"\n\n";
      for (uint32_t j=0; j<queue->SIZE; j++) {
	      if(queue->entry[j].address != 0)
        cout << " address: " << hex << queue->entry[j].address << " full_addr: " << queue->entry[j].full_addr << dec << " instr_id: " << queue->entry[j].instr_id<<" returned "<<(uint32_t)queue->entry[j].returned<<"\n";
         }
       queue = &(uncore.LLC[i]->RQ);
      cout<<"\nRQ occupancy: "<<queue->occupancy<<" Size: "<<queue->SIZE<<"\n";
     for (uint32_t j=0; j<queue->SIZE; j++) {
	     if(queue->entry[j].address != 0)
        cout << " address: " << hex << queue->entry[j].address << " full_addr: " << queue->entry[j].full_addr << dec << " instr_id: " << queue->entry[j].instr_id<<" returned "<<(uint32_t)queue->entry[j].returned<<"\n";
         }
      queue = &(uncore.LLC[i]->WQ);
      cout<<"\nWQ occupancy: "<<queue->occupancy<<" Size: "<<queue->SIZE<<"\n";
      for (uint32_t j=0; j<queue->SIZE; j++) {
	      if(queue->entry[j].address != 0)
        cout << " address: " << hex << queue->entry[j].address << " full_addr: " << queue->entry[j].full_addr << dec << " instr_id: " << queue->entry[j].instr_id<<"\n";
         }
      queue = &(uncore.LLC[i]->PQ);
      cout<<"\nPQ occupancy: "<<queue->occupancy<<" Size: "<<queue->SIZE<<"\n";
	cout<<"-----------------------------------------------------------------------------------------------\n\n";
  }
  flag=1;
}



    assert(0);
}

void signal_handler(int signal) 
{
	cout << "Caught signal: " << signal << endl;
	exit(1);
}

// log base 2 function from efectiu
int lg2(int n)
{
    int i, m = n, c = -1;
    for (i=0; m; i++) {
        m /= 2;
        c++;
    }
    return c;
}

uint64_t rotl64 (uint64_t n, unsigned int c)
{
    const unsigned int mask = (CHAR_BIT*sizeof(n)-1);

    assert ( (c<=mask) &&"rotate by type width or more");
    c &= mask;  // avoid undef behaviour with NDEBUG.  0 overhead for most types / compilers
    return (n<<c) | (n>>( (-c)&mask ));
}

uint64_t rotr64 (uint64_t n, unsigned int c)
{
    const unsigned int mask = (CHAR_BIT*sizeof(n)-1);

    assert ( (c<=mask) &&"rotate by type width or more");
    c &= mask;  // avoid undef behaviour with NDEBUG.  0 overhead for most types / compilers
    return (n>>c) | (n<<( (-c)&mask ));
}

RANDOM champsim_rand(champsim_seed);
uint64_t va_to_pa(uint32_t cpu, uint64_t instr_id, uint64_t va, uint64_t unique_vpage)
{
#ifdef SANITY_CHECK
    if (va == 0) 
        assert(0);
#endif

    uint8_t  swap = 0;
    uint64_t high_bit_mask = rotr64(cpu, lg2(NUM_CPUS)),
             unique_va = va | high_bit_mask;
    //uint64_t vpage = unique_va >> LOG2_PAGE_SIZE,
    uint64_t vpage = unique_vpage | high_bit_mask,
             voffset = unique_va & ((1<<LOG2_PAGE_SIZE) - 1);

    // smart random number generator
    uint64_t random_ppage;
    uint64_t page_begin = 100;
    map <uint64_t, uint64_t>::iterator pr = page_table.begin();
    map <uint64_t, uint64_t>::iterator ppage_check = inverse_table.begin();

    // check unique cache line footprint
    map <uint64_t, uint64_t>::iterator cl_check = unique_cl[cpu].find(unique_va >> LOG2_BLOCK_SIZE);
    if (cl_check == unique_cl[cpu].end()) { // we've never seen this cache line before
        unique_cl[cpu].insert(make_pair(unique_va >> LOG2_BLOCK_SIZE, 0));
        num_cl[cpu]++;
    }
    else
        cl_check->second++;

    pr = page_table.find(vpage);
    if (pr == page_table.end()) { // no VA => PA translation found 

        if (allocated_pages >= DRAM_PAGES) { // not enough memory

            // TODO: elaborate page replacement algorithm
            // here, ChampSim randomly selects a page that is not recently used and we only track 32K recently accessed pages
            uint8_t  found_NRU = 0;
            uint64_t NRU_vpage = 0; // implement it
            map <uint64_t, uint64_t>::iterator pr2 = recent_page.begin();
            for (pr = page_table.begin(); pr != page_table.end(); pr++) {

                NRU_vpage = pr->first;
                if (recent_page.find(NRU_vpage) == recent_page.end()) {
                    found_NRU = 1;
                    break;
                }
            }
#ifdef SANITY_CHECK
            if (found_NRU == 0)
                assert(0);

            if (pr == page_table.end())
                assert(0);
#endif
            DP ( if (warmup_complete[cpu]) {
            cout << "[SWAP] update page table NRU_vpage: " << hex << pr->first << " new_vpage: " << vpage << " ppage: " << pr->second << dec << endl; });

            // update page table with new VA => PA mapping
            // since we cannot change the key value already inserted in a map structure, we need to erase the old node and add a new node
            uint64_t mapped_ppage = pr->second;
            page_table.erase(pr);
            page_table.insert(make_pair(vpage, mapped_ppage));

            // update inverse table with new PA => VA mapping
            ppage_check = inverse_table.find(mapped_ppage);
#ifdef SANITY_CHECK
            if (ppage_check == inverse_table.end())
                assert(0);
#endif
            ppage_check->second = vpage;

            DP ( if (warmup_complete[cpu]) {
            cout << "[SWAP] update inverse table NRU_vpage: " << hex << NRU_vpage << " new_vpage: ";
            cout << ppage_check->second << " ppage: " << ppage_check->first << dec << endl; });

            // update page_queue
            page_queue.pop();
            page_queue.push(vpage);

            // invalidate corresponding vpage and ppage from the cache hierarchy
            ooo_cpu[cpu].ITLB.invalidate_entry(NRU_vpage);
            ooo_cpu[cpu].DTLB.invalidate_entry(NRU_vpage);
            ooo_cpu[cpu].STLB.invalidate_entry(NRU_vpage);

            for (uint32_t i=0; i<BLOCK_SIZE; i++) {
                uint64_t cl_addr = (mapped_ppage << 6) | i;
                ooo_cpu[cpu].L1I.invalidate_entry(cl_addr);
                ooo_cpu[cpu].L1D.invalidate_entry(cl_addr);
                ooo_cpu[cpu].L2C.invalidate_entry(cl_addr);
                
                uncore.LLC[get_slice_num(cl_addr)]->invalidate_entry(cl_addr);
            }

            // swap complete
            swap = 1;
        } else {
            uint8_t fragmented = 0;
            //if (num_adjacent_page > 0)
             //   random_ppage = ++previous_ppage;
            //else {
                //random_ppage = champsim_rand.draw_rand();
		//@Pratik
		random_ppage = page_begin%DRAM_PAGES;
	       	page_begin++;	
                fragmented = 1;
            //}

            // encoding cpu number 
            // this allows ChampSim to run homogeneous multi-programmed workloads without VA => PA aliasing
            // (e.g., cpu0: astar  cpu1: astar  cpu2: astar  cpu3: astar...)
            //random_ppage &= (~((NUM_CPUS-1)<< (32-LOG2_PAGE_SIZE)));
            //random_ppage |= (cpu<<(32-LOG2_PAGE_SIZE)); 

            while (1) { // try to find an empty physical page number
                ppage_check = inverse_table.find(random_ppage); // check if this page can be allocated 
                if (ppage_check != inverse_table.end()) { // random_ppage is not available
                    DP ( if (warmup_complete[cpu]) {
                    cout << "vpage: " << hex << ppage_check->first << " is already mapped to ppage: " << random_ppage << dec << endl; }); 
                    
                    if (num_adjacent_page > 0)
                        fragmented = 1;

                    // try one more time
                    //random_ppage = champsim_rand.draw_rand();
                   //@Pratik
		     random_ppage = page_begin%DRAM_PAGES;
		     page_begin++;
                    // encoding cpu number 
                    //random_ppage &= (~((NUM_CPUS-1)<<(32-LOG2_PAGE_SIZE)));
                    //random_ppage |= (cpu<<(32-LOG2_PAGE_SIZE)); 
                }
                else
                    break;
            }

            // insert translation to page tables
            //printf("Insert  num_adjacent_page: %u  vpage: %lx  ppage: %lx\n", num_adjacent_page, vpage, random_ppage);
            page_table.insert(make_pair(vpage, random_ppage));
            inverse_table.insert(make_pair(random_ppage, vpage));
            page_queue.push(vpage);
            previous_ppage = random_ppage;
            num_adjacent_page--;
            num_page[cpu]++;
            allocated_pages++;

            // try to allocate pages contiguously
            if (fragmented) {
                num_adjacent_page = 1 << (rand() % 10);
                DP ( if (warmup_complete[cpu]) {
                cout << "Recalculate num_adjacent_page: " << num_adjacent_page << endl; });
            }
        }

        if (swap)
            major_fault[cpu]++;
        else
            minor_fault[cpu]++;
    }
    else {
        //printf("Found  vpage: %lx  random_ppage: %lx\n", vpage, pr->second);
    }

    pr = page_table.find(vpage);
#ifdef SANITY_CHECK
    if (pr == page_table.end())
        assert(0);
#endif
    uint64_t ppage = pr->second;

    uint64_t pa = ppage << LOG2_PAGE_SIZE;
    pa |= voffset;

    DP ( if (warmup_complete[cpu]) {
    cout << "[PAGE_TABLE] instr_id: " << instr_id << " vpage: " << hex << vpage;
    cout << " => ppage: " << (pa >> LOG2_PAGE_SIZE) << " vadress: " << unique_va << " paddress: " << pa << dec << endl; });

    if (swap)
        stall_cycle[cpu] = current_core_cycle[cpu] + SWAP_LATENCY;
    else
        stall_cycle[cpu] = current_core_cycle[cpu] + PAGE_TABLE_LATENCY;

    //cout << "cpu: " << cpu << " allocated unique_vpage: " << hex << unique_vpage << " to ppage: " << ppage << dec << endl;

    return pa;
}

int main(int argc, char** argv)
{

	// interrupt signal hanlder
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = signal_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);

    cout << endl << "*** ChampSim Multicore Out-of-Order Simulator ***" << endl << endl;
for(int i=0; i<NUM_CPUS; i++)                                
      	is_rob_stalled[i] = 0;
    // initialize knobs
    uint8_t show_heartbeat = 1;

    uint32_t seed_number = 0;

    // check to see if knobs changed using getopt_long()
    int c;
    while (1) {
        static struct option long_options[] =
        {
            {"warmup_instructions", required_argument, 0, 'w'},
            {"simulation_instructions", required_argument, 0, 'i'},
            {"hide_heartbeat", no_argument, 0, 'h'},
            {"cloudsuite", no_argument, 0, 'c'},
            {"low_bandwidth",  no_argument, 0, 'b'},
            {"traces",  no_argument, 0, 't'},
            {0, 0, 0, 0}      
        };

        int option_index = 0;

        c = getopt_long_only(argc, argv, "wihsb", long_options, &option_index);

        // no more option characters
        if (c == -1)
            break;

        int traces_encountered = 0;

        switch(c) {
            case 'w':
                warmup_instructions = atol(optarg);
                break;
            case 'i':
                simulation_instructions = atol(optarg);
                break;
            case 'h':
                show_heartbeat = 0;
                break;
            case 'c':
                knob_cloudsuite = 1;
                MAX_INSTR_DESTINATIONS = NUM_INSTR_DESTINATIONS_SPARC;
                break;
            case 'b':
                knob_low_bandwidth = 1;
                break;
            case 't':
                traces_encountered = 1;
                break;
            default:
                abort();
        }

        if (traces_encountered == 1)
            break;
    }

    // consequences of knobs
    cout << "Warmup Instructions: " << warmup_instructions << endl;
    cout << "Simulation Instructions: " << simulation_instructions << endl;
    cout << "Number of CPUs: " << NUM_CPUS << endl;
    cout<<"threads:" <<threads <<endl;
    cout<<endl<<"L1I Configuration:"<<endl;
    cout<<"L1I_SET:"<<L1I_SET<<endl;
    cout<<"L1I_WAY:"<<L1I_WAY<<endl;
    cout<<"L1I_RQ_SIZE:"<<L1I_RQ_SIZE<<endl;
    cout<<"L1I_WQ_SIZE:"<<L1I_WQ_SIZE<<endl;
    cout<<"L1I_PQ_SIZE:"<<L1I_PQ_SIZE<<endl;
    cout<<"L1I_MSHR_SIZE:"<<L1I_MSHR_SIZE<<endl;

    cout<<endl<<"L1D Configuration:"<<endl;
    cout<<"L1D_SET:"<<L1D_SET<<endl;
    cout<<"L1D_WAY:"<<L1D_WAY<<endl;
    cout<<"L1D_RQ_SIZE:"<<L1D_RQ_SIZE<<endl;
    cout<<"L1D_WQ_SIZE:"<<L1D_WQ_SIZE<<endl;
    cout<<"L1D_PQ_SIZE:"<<L1D_PQ_SIZE<<endl;
    cout<<"L1D_MSHR_SIZE:"<<L1D_MSHR_SIZE<<endl;

    cout<<endl<<"L2C Configuration:"<<endl;
    cout<<"L2C_SET:"<<L2C_SET<<endl;
    cout<<"L2C_WAY:"<<L2C_WAY<<endl;
    cout<<"L2C_RQ_SIZE:"<<L2C_RQ_SIZE<<endl;
    cout<<"L2C_WQ_SIZE:"<<L2C_WQ_SIZE<<endl;
    cout<<"L2C_PQ_SIZE:"<<L2C_PQ_SIZE<<endl;
    cout<<"L2C_MSHR_SIZE:"<<L2C_MSHR_SIZE<<endl;

    cout<<endl<<"LLC Configuration:"<<endl;
    cout<<"LLC_SET:"<<LLC_SET<<endl;
    cout<<"LLC_WAY:"<<LLC_WAY<<endl;
    cout << "LLC SLICES:" << NUM_SLICES << endl;
    cout<<"LLC_RQ_SIZE:"<<LLC_RQ_SIZE<<endl;
    cout<<"LLC_WQ_SIZE:"<<LLC_WQ_SIZE<<endl;
    cout<<"LLC_PQ_SIZE:"<<LLC_PQ_SIZE<<endl;
    cout<<"LLC_MSHR_SIZE:"<<LLC_MSHR_SIZE<<endl<<endl;


    if (INTERCONNECT_ON)
        cout << "Interconnect ON"<<endl;
    else
        cout<< "Interconnect OFF"<<endl;

    if (Deprioritize_attacker)
        cout << "Attacker deprioritize is: ON " << "for "<<Attacker_deprioritize_in_DRAM <<" cycles"<<endl;
    else
        cout << "Attacker deprioritize is: OFF" << endl;


    if (knob_low_bandwidth)
        DRAM_MTPS = DRAM_IO_FREQ/4;
    else
        DRAM_MTPS = DRAM_IO_FREQ;

    // DRAM access latency
    tRP  = (uint32_t)((1.0 * tRP_DRAM_NANOSECONDS  * CPU_FREQ) / 1000); 
    tRCD = (uint32_t)((1.0 * tRCD_DRAM_NANOSECONDS * CPU_FREQ) / 1000); 
    tCAS = (uint32_t)((1.0 * tCAS_DRAM_NANOSECONDS * CPU_FREQ) / 1000); 

    // default: 16 = (64 / 8) * (3200 / 1600)
    // it takes 16 CPU cycles to tranfser 64B cache block on a 8B (64-bit) bus 
    // note that dram burst length = BLOCK_SIZE/DRAM_CHANNEL_WIDTH
    DRAM_DBUS_RETURN_TIME = (BLOCK_SIZE / DRAM_CHANNEL_WIDTH) * (CPU_FREQ / DRAM_MTPS);

    printf("Off-chip DRAM Size: %u MB Channels: %u Width: %u-bit Data Rate: %u MT/s\n",
            DRAM_SIZE, DRAM_CHANNELS, 8*DRAM_CHANNEL_WIDTH, DRAM_MTPS);

    printf("PAGE SIZE in KB: %lf \n", uint64_t(PAGE_SIZE)/1024.0);
    printf("PAGE SIZE in MB: %lf \n", uint64_t(PAGE_SIZE)/(1024.0*1024.0));

    printf("PAGE offset in bits: %u \n",LOG2_PAGE_SIZE);



            cout<<"CEASER-S_LLC:"<<CEASER_S_LLC<<endl;
	    #if MIRAGE
	    	cout<<"MIRAGE : 1"<<endl;
	    #else 
		    cout<<"MIRAGE : 0"<<endl;
	    #endif
	    cout<<"CEASER_LATENCY:" <<CEASER_LATENCY<<endl;
            int remap=0;
	    #ifdef No_Remapping
            		remap=0;
	    		cout<<"REMAP : OFF"<<endl; //without remap
	    #else
	    	#if CEASER_S_LLC == 1
             		cout<<"REMAP : ON"<<endl;
	     		cout<<"APLR : "<<uncore.LLC[0]->APLR;
	    	#endif
	   #endif
	    cout<<"Partitions : "<<uncore.LLC[0]->partitions<<endl;

	    #ifdef multi_step_relocation 
	    	cout<<"Multi-Step : ON"<<endl;
	    #else
		cout<<"Multi-Step : OFF"<<endl;
	    #endif

	    #ifdef bfs_on
                cout<<"BFS : ON"<<endl;
            #else
                cout<<"BFS : OFF"<<endl;
            #endif
    // end consequence of knobs

    // search through the argv for "-traces"
    int found_traces = 0;
    int count_traces = 0;
    cout << endl;
    for (int i=0; i<argc; i++) {
        if (found_traces) {
            printf("CPU %d runs %s\n", count_traces, argv[i]);

            sprintf(ooo_cpu[count_traces].trace_string, "%s", argv[i]);

            char *full_name = ooo_cpu[count_traces].trace_string,
                 *last_dot = strrchr(ooo_cpu[count_traces].trace_string, '.');

			ifstream test_file(full_name);
			if(!test_file.good()){
				printf("TRACE FILE DOES NOT EXIST\n");
				assert(false);
			}
				

            if (full_name[last_dot - full_name + 1] == 'g') // gzip format
                sprintf(ooo_cpu[count_traces].gunzip_command, "gunzip -c %s", argv[i]);
            else if (full_name[last_dot - full_name + 1] == 'x') // xz
                sprintf(ooo_cpu[count_traces].gunzip_command, "xz -dc %s", argv[i]);
            else {
                cout << "ChampSim does not support traces other than gz or xz compression!" << endl; 
                assert(0);
            }

            char *pch[100];
            int count_str = 0;
            pch[0] = strtok (argv[i], " /,.-");
            while (pch[count_str] != NULL) {
                //printf ("%s %d\n", pch[count_str], count_str);
                count_str++;
                pch[count_str] = strtok (NULL, " /,.-");
            }

            //printf("max count_str: %d\n", count_str);
            //printf("application: %s\n", pch[count_str-3]);

            int j = 0;
            while (pch[count_str-3][j] != '\0') {
                seed_number += pch[count_str-3][j];
                //printf("%c %d %d\n", pch[count_str-3][j], j, seed_number);
                j++;
            }

            ooo_cpu[count_traces].trace_file = popen(ooo_cpu[count_traces].gunzip_command, "r");
            if (ooo_cpu[count_traces].trace_file == NULL) {
                printf("\n*** Trace file not found: %s ***\n\n", argv[i]);
                assert(0);
            }

            count_traces++;
            if (count_traces > NUM_CPUS) {
                printf("\n*** Too many traces for the configured number of cores ***\n\n");
                assert(0);
            }
        }
        else if(strcmp(argv[i],"-traces") == 0) {
            found_traces = 1;
        }
    }

    if (count_traces != NUM_CPUS) {
        printf("\n*** Not enough traces for the configured number of cores ***\n\n");
        assert(0);
    }
    // end trace file setup

    // TODO: can we initialize these variables from the class constructor?
    srand(seed_number);
    champsim_seed = seed_number;
    for (int i=0; i<NUM_CPUS; i++) {

        ooo_cpu[i].cpu = i; 
        ooo_cpu[i].warmup_instructions = warmup_instructions;
        ooo_cpu[i].simulation_instructions = simulation_instructions;
        ooo_cpu[i].begin_sim_cycle = 0; 
        ooo_cpu[i].begin_sim_instr = warmup_instructions;

        // ROB
        ooo_cpu[i].ROB.cpu = i;

        // BRANCH PREDICTOR
        ooo_cpu[i].initialize_branch_predictor();

        // TLBs
        ooo_cpu[i].ITLB.cpu = i;
        ooo_cpu[i].ITLB.cache_type = IS_ITLB;
        ooo_cpu[i].ITLB.fill_level = FILL_L1;
        ooo_cpu[i].ITLB.extra_interface = &ooo_cpu[i].L1I;
        ooo_cpu[i].ITLB.lower_level = &ooo_cpu[i].STLB; 
	ooo_cpu[i].ITLB.partitions=1;
	
        ooo_cpu[i].DTLB.cpu = i;
        ooo_cpu[i].DTLB.cache_type = IS_DTLB;
        ooo_cpu[i].DTLB.MAX_READ = (2 > MAX_READ_PER_CYCLE) ? MAX_READ_PER_CYCLE : 2;
        ooo_cpu[i].DTLB.fill_level = FILL_L1;
        ooo_cpu[i].DTLB.extra_interface = &ooo_cpu[i].L1D;
        ooo_cpu[i].DTLB.lower_level = &ooo_cpu[i].STLB;
	ooo_cpu[i].DTLB.partitions=1;
        
	ooo_cpu[i].STLB.cpu = i;
        ooo_cpu[i].STLB.cache_type = IS_STLB;
        ooo_cpu[i].STLB.fill_level = FILL_L2;
        ooo_cpu[i].STLB.upper_level_icache[i] = &ooo_cpu[i].ITLB;
        ooo_cpu[i].STLB.upper_level_dcache[i] = &ooo_cpu[i].DTLB;
	ooo_cpu[i].STLB.partitions=1;

        // PRIVATE CACHE
        ooo_cpu[i].L1I.cpu = i;
        ooo_cpu[i].L1I.cache_type = IS_L1I;
        ooo_cpu[i].L1I.MAX_READ = (FETCH_WIDTH > MAX_READ_PER_CYCLE) ? MAX_READ_PER_CYCLE : FETCH_WIDTH;
        ooo_cpu[i].L1I.fill_level = FILL_L1;
        ooo_cpu[i].L1I.lower_level = &ooo_cpu[i].L2C; 
	ooo_cpu[i].L1I.partitions=1;

        ooo_cpu[i].L1D.cpu = i;
        ooo_cpu[i].L1D.cache_type = IS_L1D;
        ooo_cpu[i].L1D.MAX_READ = (2 > MAX_READ_PER_CYCLE) ? MAX_READ_PER_CYCLE : 2;
        ooo_cpu[i].L1D.fill_level = FILL_L1;
        ooo_cpu[i].L1D.lower_level = &ooo_cpu[i].L2C; 
        ooo_cpu[i].L1D.l1d_prefetcher_initialize();
	ooo_cpu[i].L1D.partitions=1;

        ooo_cpu[i].L2C.cpu = i;
        ooo_cpu[i].L2C.cache_type = IS_L2C;
        ooo_cpu[i].L2C.fill_level = FILL_L2;
        ooo_cpu[i].L2C.upper_level_icache[i] = &ooo_cpu[i].L1I;
        ooo_cpu[i].L2C.upper_level_dcache[i] = &ooo_cpu[i].L1D;
	ooo_cpu[i].L2C.partitions=1;
// ##################################

        if (NUM_SLICES == NUM_CPUS)
        {
            ooo_cpu[i].L2C.this_router = &ring.router[i];
        }
        else
        {
            assert(INTERCONNECT_ON == 0); //Interconnect only works if NUM_CPUS == NUM_SLICES
            ooo_cpu[i].L2C.this_router = &ring.router[0];
        }

// ##################################
        ooo_cpu[i].L2C.l2c_prefetcher_initialize();


        // SHARED CACHE
        //@Anuj
        // uncore.LLC.cache_type = IS_LLC;
        // uncore.LLC.fill_level = FILL_LLC;
        // uncore.LLC.MAX_READ = NUM_CPUS;
        // uncore.LLC.upper_level_icache[i] = &ooo_cpu[i].L2C;
        // uncore.LLC.upper_level_dcache[i] = &ooo_cpu[i].L2C;
        // uncore.LLC.lower_level = &uncore.DRAM;

        // uncore.LLC[slice_num]->upper_level_icache[i] = &ooo_cpu[i].L2C;
        // uncore.LLC[slice_num]->upper_level_dcache[i] = &ooo_cpu[i].L2C;

        // OFF-CHIP DRAM
        // uncore.DRAM.upper_level_icache[i] = &uncore.LLC;
        // uncore.DRAM.upper_level_dcache[i] = &uncore.LLC;

         for (int slice_num = 0; slice_num < NUM_SLICES; slice_num++)
        {
        uncore.LLC[slice_num]->cache_type = IS_LLC;
        uncore.LLC[slice_num]->fill_level = FILL_LLC;
        uncore.LLC[slice_num]->MAX_READ = NUM_CPUS;
        uncore.LLC[slice_num]->this_router = &ring.router[slice_num];       
        uncore.LLC[slice_num]->lower_level = &uncore.DRAM;

        uncore.LLC[slice_num]->SLICE_NUM = slice_num;    
        
	uncore.LLC[slice_num]->upper_level_icache[i] = &ooo_cpu[i].L2C;
        uncore.LLC[slice_num]->upper_level_dcache[i] = &ooo_cpu[i].L2C;

        uncore.LLC[slice_num]->llc_initialize_replacement();
        uncore.LLC[slice_num]->llc_prefetcher_initialize();
        
        uncore.DRAM.upper_level_slice[slice_num] = uncore.LLC[slice_num];
        }

        uncore.DRAM.fill_level = FILL_DRAM;

        for (uint32_t i=0; i<DRAM_CHANNELS; i++) {
            uncore.DRAM.RQ[i].is_RQ = 1;
            uncore.DRAM.WQ[i].is_WQ = 1;
        }

        warmup_complete[i] = 0;
        //all_warmup_complete = NUM_CPUS;
        simulation_complete[i] = 0;
        current_core_cycle[i] = 0;
        stall_cycle[i] = 0;
        
        previous_ppage = 0;
        num_adjacent_page = 0;
        num_cl[i] = 0;
        allocated_pages = 0;
        num_page[i] = 0;
        minor_fault[i] = 0;
        major_fault[i] = 0;
    }

    //!Initialising Interconnect
    //********NEED to change this
    ring.MAX_READ = MAX_READ_PER_CYCLE ; 
    //#######Need to change this
    ring.LATENCY = 1;
    //********
    for (int router_i = 0; router_i < NUM_SLICES; router_i++)
    {
        ring.router[router_i].upper_level = &ooo_cpu[router_i].L2C;
        ring.router[router_i].lower_level = uncore.LLC[router_i];
        ring.router[router_i].id = router_i; // Is this correct
    
    	ring.links[router_i] = 0;
    	//to add prefetch packets in correct LLC slice
    	for (int slice_i = 0; slice_i < NUM_SLICES; slice_i++)
    	{
        	ring.router[router_i].LLC_MAP[slice_i] = uncore.LLC[slice_i];

    	}

    }

    // simulation entry point
    start_time = time(NULL);
    uint8_t run_simulation = 1;
    while (run_simulation) {

        uint64_t elapsed_second = (uint64_t)(time(NULL) - start_time),
                 elapsed_minute = elapsed_second / 60,
                 elapsed_hour = elapsed_minute / 60;
        elapsed_minute -= elapsed_hour*60;
        elapsed_second -= (elapsed_hour*3600 + elapsed_minute*60);

        for (int i=0; i<NUM_CPUS; i++) {
            // proceed one cycle
            current_core_cycle[i]++;
            /*if(ooo_cpu[i].L1I.cache_stall_cycle>0)
                ooo_cpu[i].L1I.cache_stall_cycle--;

            if(ooo_cpu[i].L1D.cache_stall_cycle>0)
                ooo_cpu[i].L1D.cache_stall_cycle--;

            if(ooo_cpu[i].L2C.cache_stall_cycle>0)
                ooo_cpu[i].L2C.cache_stall_cycle--;*/


#if Deprioritize_attacker
            if(attacker_delay[i] <= 10000 && /*isolated_cpu_per_slice[1][i] == 1  &&*/ all_warmup_complete > NUM_CPUS && i ==0)  // 1 is the slice number
            {
                   //cout<<"attacker_delay is :" << attacker_delay[i]<<endl;
                    if(attacker_delay[i] == Attacker_deprioritize_in_DRAM /*5000*/)
                    {
                            int simulations_completed_core = 0;
                            //To de-restrict the attacker once all the cpus have completed simulation.
                        for(int core_id = 1; core_id < NUM_CPUS; core_id++)
                        {
                                if(simulation_complete[core_id] == 1)
                                    simulations_completed_core++;

                        }
                        if(simulations_completed_core != NUM_CPUS-1)
                              stall_the_attacker[i] = 1;
                /*      if(simulations_completed_core == NUM_CPUS-1)
                                cout << "attacker_deprioritizing is off" << endl;*/
                    }

                    if(attacker_delay[i] == 10000)
                    {
                        stall_the_attacker[i] = 0;
                        attacker_delay[i] = 0;
                    }
                    attacker_delay[i]++;
            }
#endif


	    for(int slice_num=0;slice_num<NUM_SLICES;slice_num++)
	    { //Decreasing LLC stall cycles caused due to remapping
            	if(uncore.LLC[slice_num]->cache_stall_cycle > 0 && all_warmup_complete > NUM_CPUS && i== (NUM_CPUS-1))
			uncore.LLC[slice_num]->cache_stall_cycle--;
                  
		if(uncore.LLC[slice_num]->encryption_stall_cycle > 0 && all_warmup_complete > NUM_CPUS && i == (NUM_CPUS-1) )
                               uncore.LLC[slice_num]-> encryption_stall_cycle--;
                
		if(uncore.LLC[slice_num]->decryption_stall_cycle > 0 && all_warmup_complete > NUM_CPUS && i == (NUM_CPUS-1) )
			uncore.LLC[slice_num]->decryption_stall_cycle--;
            }
            /*if(ooo_cpu[i].L1I.decryption_stall_cycle > 0)
                ooo_cpu[i].L1I.decryption_stall_cycle--;

            if(ooo_cpu[i].L1D.decryption_stall_cycle > 0)
                ooo_cpu[i].L1D.decryption_stall_cycle--;

            if(ooo_cpu[i].L2C.decryption_stall_cycle>0)
                ooo_cpu[i].L2C.decryption_stall_cycle--;

		*/

            // core might be stalled due to page fault or branch misprediction
            if (stall_cycle[i] <= current_core_cycle[i]) {

                // fetch unit
                if (ooo_cpu[i].ROB.occupancy < ooo_cpu[i].ROB.SIZE) {
                    // handle branch
		    is_rob_stalled[i]=0;
                    if (ooo_cpu[i].fetch_stall == 0) 
                        ooo_cpu[i].handle_branch();
                }
		else
		{
                        ooo_cpu[i].sim_rob_stall_cycle++;
                        if(is_rob_stalled[i] == 0)
                        {
                                is_rob_stalled[i] = 1;
                                ooo_cpu[i].sim_rob_stall_counter++;
                        }

		}

                // fetch
                ooo_cpu[i].fetch_instruction();


                // schedule (including decode latency)
                uint32_t schedule_index = ooo_cpu[i].ROB.next_schedule;
                if ((ooo_cpu[i].ROB.entry[schedule_index].scheduled == 0) && (ooo_cpu[i].ROB.entry[schedule_index].event_cycle <= current_core_cycle[i]))
                    ooo_cpu[i].schedule_instruction();

                // execute
                ooo_cpu[i].execute_instruction();

                // memory operation
                ooo_cpu[i].schedule_memory_instruction();
                ooo_cpu[i].execute_memory_instruction();

                // complete 
                ooo_cpu[i].update_rob();

                // retire
               // if ((ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].executed == COMPLETED) && (ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].event_cycle <= current_core_cycle[i]))
                    ooo_cpu[i].retire_rob();
            }

            // heartbeat information
            if (show_heartbeat && (ooo_cpu[i].num_retired >= ooo_cpu[i].next_print_instruction)) {
                float cumulative_ipc;
                if (warmup_complete[i])
                    cumulative_ipc = (1.0*(ooo_cpu[i].num_retired - ooo_cpu[i].begin_sim_instr)) / (current_core_cycle[i] - ooo_cpu[i].begin_sim_cycle);
                else
                    cumulative_ipc = (1.0*ooo_cpu[i].num_retired) / current_core_cycle[i];
                float heartbeat_ipc = (1.0*ooo_cpu[i].num_retired - ooo_cpu[i].last_sim_instr) / (current_core_cycle[i] - ooo_cpu[i].last_sim_cycle);
		
                cout << "Heartbeat CPU " << i << " instructions: " << ooo_cpu[i].num_retired << " cycles: " << current_core_cycle[i];
                cout << " heartbeat IPC: " << heartbeat_ipc << " cumulative IPC: " << cumulative_ipc; 
                cout << " (Simulation time: " << elapsed_hour << " hr " << elapsed_minute << " min " << elapsed_second << " sec) " << endl;
                int counter=0;
		for(int set=0;set< LLC_SET;set++)
			{
				for(int way=0;way<LLC_WAY;way++)
					{
						for(int slice_num=0;slice_num<NUM_SLICES;slice_num++)
                        				if(uncore.LLC[slice_num]->block[set][way].valid == 0)
								counter++;
					}
			}
        
		cout<<" Total Blocks : "<< (LLC_SET * LLC_WAY * NUM_SLICES ) << " Invalid Blocks : "<<counter<<endl;
		ooo_cpu[i].next_print_instruction += STAT_PRINTING_PERIOD;

                ooo_cpu[i].last_sim_instr = ooo_cpu[i].num_retired;
                ooo_cpu[i].last_sim_cycle = current_core_cycle[i];
            }

            // check for deadlock
            if (ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].ip && (ooo_cpu[i].ROB.entry[ooo_cpu[i].ROB.head].event_cycle + DEADLOCK_CYCLE) <= current_core_cycle[i])
              print_deadlock(i);

            // check for warmup
            // warmup complete
            if ((warmup_complete[i] == 0) && (ooo_cpu[i].num_retired > warmup_instructions)) {
                warmup_complete[i] = 1;
                all_warmup_complete++;
		//cout<<"ALL warmup complete "<<all_warmup_complete<<"\n";
            }
            if (all_warmup_complete == NUM_CPUS) { // this part is called only once when all cores are warmed up
                all_warmup_complete++;
                finish_warmup();
            }

            /*
            if (all_warmup_complete == 0) { 
                all_warmup_complete = 1;
                finish_warmup();
            }
            if (ooo_cpu[1].num_retired > 0)
                warmup_complete[1] = 1;
            */
            


	    // simulation complete
            if ((all_warmup_complete > NUM_CPUS) && (simulation_complete[i] == 0) && (ooo_cpu[i].num_retired >= (ooo_cpu[i].begin_sim_instr + ooo_cpu[i].simulation_instructions))) {
                simulation_complete[i] = 1;
                ooo_cpu[i].finish_sim_instr = ooo_cpu[i].num_retired - ooo_cpu[i].begin_sim_instr;
                ooo_cpu[i].finish_sim_cycle = current_core_cycle[i] - ooo_cpu[i].begin_sim_cycle;
                
		ooo_cpu[i].roi_rob_stall_cycle =total_rob_stall_cycles[i];                               //ooo_cpu[i].sim_rob_stall_cycle;
		ooo_cpu[i].roi_rob_stall_counter =rob_stall_count[i];
			
		ooo_cpu[i].roi_load_rob_stall_cycle =total_load_rob_stall_cycles[i];                        //ooo_cpu[i].sim_rob_stall_cycle;
                ooo_cpu[i].roi_load_rob_stall_counter = load_rob_stall_count[i];
                
		cout << "Finished CPU " << i << " instructions: " << ooo_cpu[i].finish_sim_instr << " cycles: " << ooo_cpu[i].finish_sim_cycle;
                
		cout << " cumulative IPC: " << ((float) ooo_cpu[i].finish_sim_instr / ooo_cpu[i].finish_sim_cycle);
                cout << " (Simulation time: " << elapsed_hour << " hr " << elapsed_minute << " min " << elapsed_second << " sec) " << endl;
		//cout<<" TOTAL HI *****************************"<<total_rob_stall_cycles[i]<<"\n";	
	
                record_roi_stats(i, &ooo_cpu[i].ITLB);
                record_roi_stats(i, &ooo_cpu[i].DTLB);
                record_roi_stats(i, &ooo_cpu[i].STLB);
		record_roi_stats(i, &ooo_cpu[i].L1D);
                record_roi_stats(i, &ooo_cpu[i].L1I);
                record_roi_stats(i, &ooo_cpu[i].L2C);
                for (int slice_num = 0; slice_num < NUM_SLICES; slice_num++)
                {
                record_roi_stats(i, uncore.LLC[slice_num]); 
                }

                all_simulation_complete++;
            }

            if (all_simulation_complete == NUM_CPUS)
                run_simulation = 0;
        }
        //****** TODO: Is it the right place to call or should it be from ooo_cpu
         ring.operate(); //!perform Interconnect operations every cycle

        // TODO: should it be backward?
        for (int slice_num = 0; slice_num < NUM_SLICES; slice_num++)
	{
	        uint32_t run;
        	run=1;
        	for(uint32_t i=0;i<NUM_CPUS;i++)
                	if(uncore.LLC[slice_num]->cache_stall_cycle > 0)
			{
                        	run=0;
				//cout<<"LLC IS stalled \n";
			}
	    if(run == 1 || all_warmup_complete <= NUM_CPUS)
            	uncore.LLC[slice_num]->operate();
	    if (run == 0 && all_warmup_complete > NUM_CPUS)
                uncore.LLC[slice_num]->total_llc_stalled++;

	}
            uncore.DRAM.operate();
    }

#ifndef CRC2_COMPILE
    print_branch_stats();
#endif
    uint64_t elapsed_second = (uint64_t)(time(NULL) - start_time),
             elapsed_minute = elapsed_second / 60,
             elapsed_hour = elapsed_minute / 60;
    elapsed_minute -= elapsed_hour*60;
    elapsed_second -= (elapsed_hour*3600 + elapsed_minute*60);
    
    cout << endl << "ChampSim completed all CPUs" << endl;
    if (NUM_CPUS > 1) {
        cout << endl << "Total Simulation Statistics (not including warmup)" << endl;
        for (uint32_t i=0; i<NUM_CPUS; i++) {
            cout << endl << "CPU " << i << " cumulative IPC: " << (float) (ooo_cpu[i].num_retired - ooo_cpu[i].begin_sim_instr) / (current_core_cycle[i] - ooo_cpu[i].begin_sim_cycle); 
            cout << " instructions: " << ooo_cpu[i].num_retired - ooo_cpu[i].begin_sim_instr << " cycles: " << current_core_cycle[i] - ooo_cpu[i].begin_sim_cycle << endl;
#ifndef CRC2_COMPILE
	    //ooo_cpu[i].core_final_stats();
	    print_sim_stats(i, &ooo_cpu[i].ITLB);
            print_sim_stats(i, &ooo_cpu[i].DTLB);
            print_sim_stats(i, &ooo_cpu[i].STLB);
            print_sim_stats(i, &ooo_cpu[i].L1D);
            print_sim_stats(i, &ooo_cpu[i].L1I);
            print_sim_stats(i, &ooo_cpu[i].L2C);
            ooo_cpu[i].L1D.l1d_prefetcher_final_stats();
            ooo_cpu[i].L2C.l2c_prefetcher_final_stats();
#endif
            for (int slice_num = 0; slice_num < NUM_SLICES; slice_num++)
            	print_sim_stats(i, uncore.LLC[slice_num]);
       	    cout<<"Total Rob stall cycles : \t"<<(total_rob_stall_cycles[i])<<endl;
	    cout<<"Total Load Rob Stall cycles : \t"<<total_load_rob_stall_cycles[i]<<endl;
       	    cout<<"Average Rob Stall cycles : \t "<<total_rob_stall_cycles[i]/rob_stall_count[i]<<endl;
            cout<<"Average Load Rob Stall cycles : \t "<<total_load_rob_stall_cycles[i]/load_rob_stall_count[i]<<endl;

	}
        for (int slice_num = 0; slice_num < NUM_SLICES; slice_num++)
        uncore.LLC[slice_num]->llc_prefetcher_final_stats();
    }

    cout << endl << "Region of Interest Statistics" << endl;
    for (uint32_t i=0; i<NUM_CPUS; i++) {
        cout << endl << "CPU " << i << " cumulative IPC: " << ((float) ooo_cpu[i].finish_sim_instr / ooo_cpu[i].finish_sim_cycle); 
        cout << " instructions: " << ooo_cpu[i].finish_sim_instr << " cycles: " << ooo_cpu[i].finish_sim_cycle << endl;
#ifndef CRC2_COMPILE
	//ooo_cpu[i].core_final_stats();
	print_roi_stats(i, &ooo_cpu[i].ITLB);
        print_roi_stats(i, &ooo_cpu[i].DTLB);
        print_roi_stats(i, &ooo_cpu[i].STLB);
        print_roi_stats(i, &ooo_cpu[i].L1D);
        print_roi_stats(i, &ooo_cpu[i].L1I);
        print_roi_stats(i, &ooo_cpu[i].L2C);
#endif
        for (int slice_num = 0; slice_num < NUM_SLICES; slice_num++)
        print_roi_stats(i, uncore.LLC[slice_num]);
        cout << "ROI Average ROB Stall Cycles:\t" << (ooo_cpu[i].roi_rob_stall_cycle*1.0)/ooo_cpu[i].roi_rob_stall_counter<<endl;
	cout << "ROI Total ROB Stalls Cycles:\t"<< ooo_cpu[i].roi_rob_stall_cycle<<endl;
	cout<<  "ROI TOTAL ROB counter      :\t"<<ooo_cpu[i].roi_rob_stall_counter<<endl;

	cout << "ROI Average Load ROB Stall Cycles:\t" << (ooo_cpu[i].roi_load_rob_stall_cycle*1.0)/ooo_cpu[i].roi_load_rob_stall_counter<<endl;
        cout << "ROI Total Load ROB Stalls Cycle  :\t"<< ooo_cpu[i].roi_load_rob_stall_cycle<<endl;
	cout << "ROI Total Load ROB stalls Counter:\t"<<ooo_cpu[i].roi_load_rob_stall_counter<<endl;
        cout << "Major fault: " << major_fault[i] << " Minor fault: " << minor_fault[i] << endl;
    }

    for (uint32_t i=0; i<NUM_CPUS; i++) {
        ooo_cpu[i].L1D.l1d_prefetcher_final_stats();
        ooo_cpu[i].L2C.l2c_prefetcher_final_stats();
    }
    for (int slice_num = 0; slice_num < NUM_SLICES; slice_num++)
    uncore.LLC[slice_num]->llc_prefetcher_final_stats();

print_interconnect_stats();


#ifndef CRC2_COMPILE
    for (int slice_num = 0; slice_num < NUM_SLICES; slice_num++)
    uncore.LLC[slice_num]->llc_replacement_final_stats();
    print_dram_stats();
#endif

    cout<<"DRAM PAGES: "<<DRAM_PAGES<<endl;
    cout<<"Allocated PAGES: "<<allocated_pages<<endl;

if( CEASER_S_LLC == 1)
{
       	cout<<endl<<"CEASER-S Statistics"<<endl;     	                                                           
        for(int x=0; x<NUM_SLICES; x++)
	{
		  cout<<"SLICE NO: "<<x<<endl;
		  uint64_t total_llc_access =0;
                  for(uint32_t cpu=0; cpu<NUM_CPUS; cpu++)
                          for (uint32_t i=0; i<NUM_TYPES; i++) {
                                  total_llc_access += uncore.LLC[x]->sim_access[cpu][i];
                          }
                  
                  cout<<"Total sets remapped: "<<uncore.LLC[x]->total_sets_remapped<<endl;
                  cout<<"Total LLC Stalled: "<<uncore.LLC[x]->total_llc_stalled<<endl;
                  cout<<"Total Stall Cycles: "<<uncore.LLC[x]->total_stall_cycle<<endl;
		  cout<<"Total Blocks Saved Per set: "<<(float)uncore.LLC[x]->blocks_less_evicted/(float)uncore.LLC[x]->total_sets_remapped<<endl;
		  cout<<"Total Invalid Blocks Before Remapping Per set: "<<(float)uncore.LLC[x]->invalid_blocks_before_remapping/(float)uncore.LLC[x]->total_sets_remapped<<endl;
		  cout<<"Total Blocks Remapped : "<<uncore.LLC[x]->total_blocks_remapped<<" Average number of blocks remapped per set : "<<(float)uncore.LLC[x]->total_blocks_remapped/(float)uncore.LLC[x]->total_sets_remapped<<endl;
		  cout<<endl;
	}
}		
	
		 // cout<<endl<<"DoS Statistics"<<endl;
                 // print_analysis_for_DOS_stats();

    return 0;
}
