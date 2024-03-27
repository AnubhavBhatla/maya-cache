#include <math.h>
#include "cache.h"
#include "set.h"
#include "uncore.h"
#include "ooo_cpu.h"
#include "ceaser.h"
#include "prince.h"
#include <bitset>
uint64_t l2pf_access = 0;
uint32_t is_valid_block_evicted = 0; //this flag is used for doing remap. Call remap only on evictions of valid cache block from the LLC
/*CEASER-S
        // pla: physical  line address
	// ela: encrypted line address
*/
    uint64_t pla, ela, curr_addr,full_addr, set_not_remapped=0;
    AES d1;
    PRINCE p;
/*------------------------------------------------------------------------------------------*/

void CACHE::set_key(uint32_t set,uint32_t way)
{//set the c_or_n bit that tells whether the block uses the current key or next key for ela

	if(cache_type == IS_ITLB || cache_type == IS_DTLB || cache_type == IS_STLB)
        	return;
	if( ( CEASER_S_LLC == 0  && cache_type == IS_LLC) )
                return;
	if(CEASER_S_LLC == 1 && cache_type == IS_LLC)
	{
        	if(set >= Sptr)
                   	c_or_n=0;
         	else 
               		c_or_n=1;
      	}	
}

int CACHE::add_read_to_lower_level(uint32_t mshr_index,uint32_t read_index,uint32_t read_cpu)
	/*When load packets get hit in MSHR but MSHR (type == PREFETCH), forward this load request to lower level cache otherwise load request have to wait due to encryption engine priority */ 
{
	if (cache_type==IS_L2C)
        {
		uint32_t slice_num = get_slice_num(MSHR.entry[mshr_index].address);
		int insert_err =0;
		int pq_index = uncore.LLC[slice_num]->PQ.check_queue(&MSHR.entry[mshr_index]); //Check for the packet in lower level PQ
		
		if(pq_index != -1) //Packet is present
		{
			 if (INTERCONNECT_ON == 0 || get_slice_num(RQ.entry[read_index].address) == read_cpu  )
                         {
                                                                if ((this_router->LLC_MAP[get_slice_num(RQ.entry[read_index].address)])->get_occupancy(1,RQ.entry[read_index].address) == (this_router->LLC_MAP[get_slice_num(RQ.entry[read_index].address)])->get_size(1,RQ.entry[read_index].address))//lower_level rq is full
                                                                {
									return -1;
                                                                }
                                                                else
								{

									uncore.LLC[slice_num]->PQ.remove_queue_at_index(pq_index); //Remove from the PQ 
                                                                        insert_err = (this_router->LLC_MAP[get_slice_num(RQ.entry[read_index].address)])->add_rq(&RQ.entry[read_index]); //Adding the packet to LLC RQ
                                                                }

                        }
			else
                         {
                                                                   int direction = get_direction(read_cpu,slice_num);
                                                                   if (this_router->NI[direction].OUTQ.occupancy == this_router->NI[direction].OUTQ.SIZE) 							 {
                                                                                        insert_err=-2;
                                                                                        // Dead code.
                                                                                        //Router Queue filled
                                                                                        STALL[RQ.entry[read_index].type]++;
                                                                                        DP ( if (warmup_complete[read_cpu]) {
                                                                                        cout << "[" << NAME << "] " << __func__ ;
                                                                                        cout << " Router queue filled!" << " fill_addr: " << hex << RQ.entry[read_index].address;});
											return -1;
                                                                   }
                                                                   else
								   {
									   		uncore.LLC[slice_num]->PQ.remove_queue_at_index(pq_index);
										  	insert_err = this_router->NI[direction].add_outq(&RQ.entry[read_index]);
                                                                   }


                        }

    		}
		return 1;//IF Not in lower level prefetch queue, it might have gone to MSHR of lower level cache
			// ELSE Added to lower level Read queue successfully
 	}
   else if(cache_type == IS_L1D)
   {
		 int pq_index = ooo_cpu[read_cpu].L2C.PQ.check_queue(&MSHR.entry[mshr_index]);
		 	if ((lower_level->get_occupancy(1, RQ.entry[read_index].address) != lower_level->get_size(1, RQ.entry[read_index].address)))
		 	{
                          	 lower_level->add_rq(&RQ.entry[read_index]); //Add to L2C RQ

				 if(pq_index != -1)
				 {
					ooo_cpu[read_cpu].L2C.PQ.remove_queue_at_index(pq_index); //Removed from L2C PQ
				 }
				 return 1;
                 	}
		 	else
			 	return -1; // Lower level cache's Read Queue is Full

   }
	assert (0); 
	// Program Control should never reach here 
	return 1;
	
}

void CACHE::handle_fill() //Interconnect done
{
	// handle fill
	uint32_t fill_cpu = (MSHR.next_fill_index == MSHR_SIZE) ? NUM_CPUS : MSHR.entry[MSHR.next_fill_index].cpu;
	if (fill_cpu == NUM_CPUS)
		return;

		int tag_way,tag_number,tag_set_number;
	if (MSHR.next_fill_cycle <= current_core_cycle[fill_cpu]) {

#ifdef SANITY_CHECK
		if (MSHR.next_fill_index >= MSHR.SIZE)
			assert(0);
#endif

		uint32_t mshr_index = MSHR.next_fill_index;
		// find victim
		
		uint32_t set = get_set(MSHR.entry[mshr_index].address), way,set1,way1;
		if(MIRAGE == 1 && cache_type == IS_LLC)
		{
			//cout<<"-------MIRAGE : Handle_Fill Starts-------"<<endl;
			get_tag_set(MSHR.entry[mshr_index].address); //sets tag0 and tag1 set number
			tag_number = llc_find_victim_tag(tag0_set,tag1_set,&tag_way);  		
			set=random_set();
			way=random_way();
			if(tag_number ==0 )
			       	tag_set_number = tag0_set;
			else
				tag_set_number = tag1_set;
			set1=set;
			way1=way;
			//cout<<"tag_number : "<<tag_number<<" tag_set_number : "<<tag_set_number<<"tag_way_number : "<<tag_way<<endl;
			//cout<<"set : "<<set<<" way : "<<way<<endl;	
		}
		else if(CEASER_S_LLC == 1 && cache_type == IS_LLC)
		{
			int part=rand()%partitions;  //Randomly selects a partition
                      	if (ceaser_s_set[part] >= Sptr)
         			set=ceaser_s_set[part];
                     	else
                             	set =ceaser_s_next_set[part];
			//CEASER_S Replacement policy
                      	way = llc_find_victim_ceaser_s(fill_cpu, MSHR.entry[mshr_index].instr_id, set, block[set], MSHR.entry[mshr_index].ip, full_addr, MSHR.entry[mshr_index].type,part);
                }
		else if (cache_type == IS_LLC && CEASER_S_LLC != 1) 
		{
			way = llc_find_victim(fill_cpu, MSHR.entry[mshr_index].instr_id, set, block[set], MSHR.entry[mshr_index].ip, full_addr, MSHR.entry[mshr_index].type);
		}
		else
			way = find_victim(fill_cpu, MSHR.entry[mshr_index].instr_id, set, block[set], MSHR.entry[mshr_index].ip, MSHR.entry[mshr_index].full_addr, MSHR.entry[mshr_index].type);
 
#ifdef LLC_BYPASS
		if ((cache_type == IS_LLC) && (way == LLC_WAY)) { // this is a bypass that does not fill the LLC

		//This check is to determine before-hand if packet will get stuck in network queue and return without processing
		if (INTERCONNECT_ON && MSHR.entry[mshr_index].fill_level < fill_level && this_router->id != fill_cpu && this_router->NI[get_direction(this_router->id,fill_cpu)].OUTQ.occupancy == this_router->NI[get_direction(this_router->id,fill_cpu)].OUTQ.SIZE)
		{
		   	this_router->stall_cycle++;
			STALL[MSHR.entry[mshr_index].type]++;
			cout << "[" << NAME << "] " << __func__ ;
			cout << " Router queue filled!" << " fill_addr: " << hex << MSHR.entry[mshr_index].address;
			DP ( if (warmup_complete[fill_cpu]) {
			cout << "[" << NAME << "] " << __func__ ;
			cout << " Router queue filled!" << " fill_addr: " << hex << MSHR.entry[mshr_index].address;
			cout << " victim_addr: " << block[set][way].tag << dec << endl; });
		return;
		}
			//@Why update repl. state if no fill
			// update replacement policy
			if (cache_type == IS_LLC) {
				llc_update_replacement_state(fill_cpu, set, way, MSHR.entry[mshr_index].full_addr, MSHR.entry[mshr_index].ip, 0, MSHR.entry[mshr_index].type, 0);
			}
			else
				update_replacement_state(fill_cpu, set, way, MSHR.entry[mshr_index].full_addr, MSHR.entry[mshr_index].ip, 0, MSHR.entry[mshr_index].type, 0);

			// COLLECT STATS
			sim_miss[fill_cpu][MSHR.entry[mshr_index].type]++;
			sim_access[fill_cpu][MSHR.entry[mshr_index].type]++;

			// check fill level
			if (MSHR.entry[mshr_index].fill_level < fill_level) 
			{

				if (INTERCONNECT_ON == 0 || this_router->id == fill_cpu)
				{
					if (MSHR.entry[mshr_index].instruction) 
							upper_level_icache[fill_cpu]->return_data(&MSHR.entry[mshr_index]);
						else // data
							upper_level_dcache[fill_cpu]->return_data(&MSHR.entry[mshr_index]);
				}
				else
				{
					int direction = get_direction(this_router->id,fill_cpu);
					if (this_router->NI[direction].OUTQ.occupancy == this_router->NI[direction].OUTQ.SIZE) 
					{
						//Dead code
					   	this_router->stall_cycle++;
						STALL[MSHR.entry[mshr_index].type]++;

						DP ( if (warmup_complete[fill_cpu]) {
						cout << "[" << NAME << "] " << __func__ ;
						cout << " Router queue filled!" << " fill_addr: " << hex << MSHR.entry[mshr_index].address;
						cout << " victim_addr: " << block[set][way].tag << dec << endl; });
					}
					else
					{
						MSHR.entry[mshr_index].forL2=1;
						this_router->NI[direction].add_outq(&MSHR.entry[mshr_index]);  
					}    
					
				}
			}
			if(warmup_complete[fill_cpu])
			  {
				uint64_t current_miss_latency = (current_core_cycle[fill_cpu] - MSHR.entry[mshr_index].cycle_enqueued);	
				total_miss_latency[fill_cpu] += current_miss_latency;
			  } 
				    if(warmup_complete[fill_cpu])
					sim_miss_penalty[fill_cpu][MSHR.entry[mshr_index].type] +=  current_core_cycle[fill_cpu] - MSHR.entry[mshr_index].add_cycle_count;
				
					if((cache_type == IS_L2C || cache_type == IS_LLC) && fill_level > MSHR.entry[mshr_index].fill_level )
					{
						int index = RQ.check_queue(&MSHR.entry[mshr_index]);
						if(index != -1)
						{
							RQ.remove_queue_at_index(index);
						}
					}
				    	MSHR.remove_queue(&MSHR.entry[mshr_index]);
					if(cache_type == IS_LLC &&  all_warmup_complete > NUM_CPUS )
					{
						total_waiting_time_in_mshr[fill_cpu] += (current_core_cycle[fill_cpu] - MSHR.entry[mshr_index].mshr_data_return_cycle);
						total_mshr_packets[MSHR.entry[mshr_index].cpu]++;
					}
					update_fill_cycle();			
			check_llc_access();
			return; // return here, no need to process further in this function
		}
#endif

		uint8_t  do_fill = 1;

// is this dirty?
if (block[set][way].dirty) 
		{
			if (cache_type == IS_L2C) //L2C --> LLC slice can go either via network or without it[ when it is for connected LLC slice or if INTERCONNECT IS OFF]
			{
				if (INTERCONNECT_ON == 0 || get_slice_num(block[set][way].tag) == this_router->id ) //Direct path
				{
					int destination_slice = get_slice_num(block[set][way].tag);
					if (this_router->LLC_MAP[destination_slice]->get_occupancy(2, block[set][way].tag) == this_router->LLC_MAP[destination_slice]->get_size(2, block[set][way].tag)) 
						{// lower level WQ is full, cannot replace this victim
						do_fill = 0;
						this_router->LLC_MAP[destination_slice]->increment_WQ_FULL(block[set][way].tag);
						STALL[MSHR.entry[mshr_index].type]++;

						DP ( if (warmup_complete[fill_cpu]) {
						cout << "[" << NAME << "] " << __func__ << "do_fill: " << do_fill;
						cout << " lower level wq is full!" << " fill_addr: " << hex << MSHR.entry[mshr_index].address;
						cout << " victim_addr: " << block[set][way].tag << dec << endl; });
						}
					else
						{
							PACKET writeback_packet;

							writeback_packet.fill_level = fill_level << 1;
							writeback_packet.cpu = fill_cpu;
				                        curr_addr=getDecryptedAddress(set,way);
				                        if(curr_addr==-1)
					                        do_fill=0;

							writeback_packet.address = curr_addr;
		                                        full_addr       = (curr_addr << LOG2_BLOCK_SIZE) + (block[set][way].full_addr & 0x3F);

							writeback_packet.full_addr = full_addr;
							writeback_packet.data = block[set][way].data;
							writeback_packet.instr_id = MSHR.entry[mshr_index].instr_id;
							writeback_packet.ip = 0; // writeback does not have ip
							writeback_packet.type = WRITEBACK;
							writeback_packet.event_cycle = current_core_cycle[fill_cpu];
				                        if(do_fill==1)
								this_router->LLC_MAP[destination_slice]->add_wq(&writeback_packet);
						}
				}
				else //WB packet need to go to network
				{
					int destination = get_slice_num(block[set][way].tag);
					int direction = get_direction(cpu,destination);
					if (this_router->NI[direction].OUTQ.occupancy == this_router->NI[direction].OUTQ.SIZE) 
					{
					//Router Queue filled
					cout << "[" << NAME << "] " << __func__ ;
					cout << " Router queue filled!" << " fill_addr: " << hex << MSHR.entry[mshr_index].address;
					do_fill = 0;
					STALL[MSHR.entry[mshr_index].type]++;
					DP ( if (warmup_complete[fill_cpu]) {
					cout << "[" << NAME << "] " << __func__ ;
					cout << " Router queue filled!" << " fill_addr: " << hex << MSHR.entry[mshr_index].address;
					cout << " victim_addr: " << block[set][way].tag << dec << endl; });
					}
					else 
					{
					PACKET writeback_packet;
					writeback_packet.fill_level = fill_level << 1;
					writeback_packet.cpu = fill_cpu;
		                        curr_addr=getDecryptedAddress(set,way);
                                        if(curr_addr==-1)
                                                do_fill=0;
                                        full_addr       = (curr_addr << LOG2_BLOCK_SIZE) + (block[set][way].full_addr & 0x3F);

					writeback_packet.address = curr_addr;
					writeback_packet.full_addr = full_addr;
					writeback_packet.data = block[set][way].data;
					writeback_packet.instr_id = MSHR.entry[mshr_index].instr_id;
					writeback_packet.ip = 0; // writeback does not have ip
					writeback_packet.type = WRITEBACK;
					writeback_packet.event_cycle = current_core_cycle[fill_cpu];
					writeback_packet.forL2=0;
					if(do_fill==1)
					this_router->NI[direction].add_outq(&writeback_packet);   
					 }       
				}
			}
			else //For caches other than L2
			{
			if (lower_level) 
				{
				if (lower_level->get_occupancy(2, block[set][way].tag) == lower_level->get_size(2, block[set][way].tag)) 
				{
					// lower level WQ is full, cannot replace this victim
					do_fill = 0;
					lower_level->increment_WQ_FULL(block[set][way].tag);
					STALL[MSHR.entry[mshr_index].type]++;

					DP ( if (warmup_complete[fill_cpu]) {
					cout << "[" << NAME << "] " << __func__ << "do_fill: " << do_fill;
					cout << " lower level wq is full!" << " fill_addr: " << hex << MSHR.entry[mshr_index].address;
					cout << " victim_addr: " << block[set][way].tag << dec << endl; });
				}
				else 
				{
					PACKET writeback_packet;

					writeback_packet.fill_level = fill_level << 1;
					writeback_packet.cpu = fill_cpu;
                                        curr_addr=getDecryptedAddress(set,way);
                                              if(curr_addr==-1)
                                                      do_fill=0;

					writeback_packet.address = curr_addr;
	                                        full_addr       = (curr_addr << LOG2_BLOCK_SIZE) + (block[set][way].full_addr & 0x3F);

					writeback_packet.full_addr = full_addr;
					writeback_packet.data = block[set][way].data;
					writeback_packet.instr_id = MSHR.entry[mshr_index].instr_id;
					writeback_packet.ip = 0; // writeback does not have ip
					writeback_packet.type = WRITEBACK;
					writeback_packet.event_cycle = current_core_cycle[fill_cpu];
					if(do_fill == 1)
						lower_level->add_wq(&writeback_packet);
				}
				}

#ifdef SANITY_CHECK
			else {
				// sanity check
				if (cache_type != IS_STLB)
					assert(0);
			}
#endif
			
		}
	}
		if (do_fill)
		{
			//This check is to determine before-hand if packet will get stuck in network queue and return without processing the packet or making any changes to other variables
			if ( INTERCONNECT_ON && cache_type == IS_LLC && MSHR.entry[mshr_index].fill_level < fill_level && this_router->id!=fill_cpu)
			{
				int direction = get_direction(this_router->id,fill_cpu);
				if (this_router->NI[direction].OUTQ.occupancy == this_router->NI[direction].OUTQ.SIZE) 
				{
				   	this_router->stall_cycle++;
					STALL[MSHR.entry[mshr_index].type]++;
					cout << "[" << NAME << "] " << __func__ ;
					cout << " Router queue filled!" << " fill_addr: " << hex << MSHR.entry[mshr_index].address;
					DP ( if (warmup_complete[fill_cpu]) {
					cout << "[" << NAME << "] " << __func__ ;
					cout << " Router queue filled!" << " fill_addr: " << hex << MSHR.entry[mshr_index].address;
					cout << " victim_addr: " << block[set][way].tag << dec << endl; });
					
					return;		
				}
			}

			// update prefetcher
			if (cache_type == IS_L1D)
				l1d_prefetcher_cache_fill(MSHR.entry[mshr_index].full_addr, set, way, (MSHR.entry[mshr_index].type == PREFETCH) ? 1 : 0, block[set][way].tag<<LOG2_BLOCK_SIZE,MSHR.entry[mshr_index].pf_metadata);
			if  (cache_type == IS_L2C)
				MSHR.entry[mshr_index].pf_metadata = l2c_prefetcher_cache_fill(MSHR.entry[mshr_index].address<<LOG2_BLOCK_SIZE, set, way, (MSHR.entry[mshr_index].type == PREFETCH) ? 1 : 0,block[set][way].tag<<LOG2_BLOCK_SIZE, MSHR.entry[mshr_index].pf_metadata);
			if (cache_type == IS_LLC)
			{
				cpu = fill_cpu;
				MSHR.entry[mshr_index].pf_metadata = llc_prefetcher_cache_fill(MSHR.entry[mshr_index].address<<LOG2_BLOCK_SIZE, set, way, (MSHR.entry[mshr_index].type == PREFETCH) ? 1 : 0,block[set][way].tag<<LOG2_BLOCK_SIZE, MSHR.entry[mshr_index].pf_metadata);
				cpu = 0;
			}  
			// update replacement policy
			if (cache_type == IS_LLC) 
			{
				llc_update_replacement_state(fill_cpu, set, way, MSHR.entry[mshr_index].full_addr, MSHR.entry[mshr_index].ip, block[set][way].full_addr, MSHR.entry[mshr_index].type, 0);
			}
			else
				update_replacement_state(fill_cpu, set, way, MSHR.entry[mshr_index].full_addr, MSHR.entry[mshr_index].ip, block[set][way].full_addr, MSHR.entry[mshr_index].type, 0);

			// COLLECT STATS
			sim_miss[fill_cpu][MSHR.entry[mshr_index].type]++;
			sim_access[fill_cpu][MSHR.entry[mshr_index].type]++;
                	#if remap_on_evictions
                        	if(block[set][way].valid == 1)
                        	{
                                	is_valid_block_evicted = 1;
                        	}
                	#endif
		
			if(MIRAGE == 1 && cache_type == IS_LLC)
			{
				if(block[set][way].valid == 1)
					remove_tag_entry_from_block(set,way);    //(block[set][way].tag_set,block[set][way].tag_way,block[set][way].tag);
				if((tag_number == 0 && tag0[tag_set_number][tag_way].valid == 1) || (tag_number == 1 && tag1[tag_set_number][tag_way].valid == 1))
				{	
					remove_block_based_on_tag_array(tag_number,tag_set_number,tag_way);
					//cout<<"Remove Block based on tag array  tag_number : "<<tag_number<<" tag_set "<<tag_set_number<<" tag_way : "<<tag_way<<endl;
				}
				fill_tag(tag_number,tag_set_number,tag_way,MSHR.entry[mshr_index].address,set,way);	
				//cout<<"Fill_cache in Handle_Fill : set = "<<set<<" way : "<<way<<endl;
				if(set1 != set && way1 != way)
					assert(0);
			}
			if (MSHR.entry[mshr_index].type == LOAD && cache_type == IS_LLC && block[set][way].used == 0){
				counter_deadblock++;
			}

			fill_cache(set, way, &MSHR.entry[mshr_index]);
			// RFO marks cache line dirty
			if (cache_type == IS_L1D) 
			{
				if (MSHR.entry[mshr_index].type == RFO)
					block[set][way].dirty = 1;
			}

			// check fill level
			if (MSHR.entry[mshr_index].fill_level < fill_level) 
			{
				if (cache_type == IS_LLC) //LLC --> L2C, packet can go either via network or without it[ When requesting core is directly connected to LLC slice or if INTERCONNECT IS OFF]
				{
					if (INTERCONNECT_ON == 0 || this_router->id == fill_cpu )
					{
						if (MSHR.entry[mshr_index].instruction) 
							upper_level_icache[fill_cpu]->return_data(&MSHR.entry[mshr_index]);
						else // data
							upper_level_dcache[fill_cpu]->return_data(&MSHR.entry[mshr_index]);						
					}
					else //Add the packet to network
					{
						int direction = get_direction(this_router->id,fill_cpu);//Since this packet is going from LLC, fill_cpu is destination
						if (this_router->NI[direction].OUTQ.occupancy == this_router->NI[direction].OUTQ.SIZE)
						{
							//Dead code 
							this_router->stall_cycle++;							
							STALL[MSHR.entry[mshr_index].type]++;
							cout << "[" << NAME << "] " << __func__ ;
							cout << " Router queue filled!" << " Packet address: " << hex << MSHR.entry[mshr_index].address<<"Router_id"<<this_router->id;

							DP ( if (warmup_complete[fill_cpu]) {
							cout << "[" << NAME << "] " << __func__ ;
							cout << " Router queue filled!" << " Packet address: " << hex << MSHR.entry[mshr_index].address<<"Router_id"<<this_router->id;
							});
							//Dead code
						}
						else
						{
							MSHR.entry[mshr_index].forL2=1;
							this_router->NI[direction].add_outq(&MSHR.entry[mshr_index]);  
						}    
					}
				}
				else
				{
					if (MSHR.entry[mshr_index].instruction) 
						upper_level_icache[fill_cpu]->return_data(&MSHR.entry[mshr_index]);
					else // data
						upper_level_dcache[fill_cpu]->return_data(&MSHR.entry[mshr_index]);
				}
			}

			// update processed packets
			if (cache_type == IS_ITLB) { 
				MSHR.entry[mshr_index].instruction_pa = block[set][way].data;
				if (PROCESSED.occupancy < PROCESSED.SIZE)
					PROCESSED.add_queue(&MSHR.entry[mshr_index]);
			}
			else if (cache_type == IS_DTLB) {
				MSHR.entry[mshr_index].data_pa = block[set][way].data;
				if (PROCESSED.occupancy < PROCESSED.SIZE)
					PROCESSED.add_queue(&MSHR.entry[mshr_index]);
			}
			else if (cache_type == IS_L1I) {
				if (PROCESSED.occupancy < PROCESSED.SIZE)
					PROCESSED.add_queue(&MSHR.entry[mshr_index]);
			}
			//else if (cache_type == IS_L1D) {
			else if ((cache_type == IS_L1D) && (MSHR.entry[mshr_index].type != PREFETCH)) {
				if (PROCESSED.occupancy < PROCESSED.SIZE)
					PROCESSED.add_queue(&MSHR.entry[mshr_index]);
			}

			if(warmup_complete[fill_cpu])
			{
				uint64_t current_miss_latency = (current_core_cycle[fill_cpu] - MSHR.entry[mshr_index].cycle_enqueued);
				total_miss_latency[fill_cpu] += current_miss_latency;

			}
		        if(warmup_complete[fill_cpu])
                		sim_miss_penalty[fill_cpu][MSHR.entry[mshr_index].type] +=  current_core_cycle[fill_cpu] - MSHR.entry[mshr_index].add_cycle_count;

			if( (cache_type == IS_L2C || cache_type == IS_LLC) && fill_level > MSHR.entry[mshr_index].fill_level)
			{
				int index = RQ.check_queue(&MSHR.entry[mshr_index]); //There can be redundent entry as PQ data might be served
				if(index != -1)
				{
					RQ.remove_queue_at_index(index);
				}
			}
			if(cache_type == IS_LLC &&  all_warmup_complete > NUM_CPUS ) //We can remove it
			{
				total_waiting_time_in_mshr[fill_cpu] += (current_core_cycle[fill_cpu] - MSHR.entry[mshr_index].mshr_data_return_cycle);
				total_mshr_packets[MSHR.entry[mshr_index].cpu]++;
			}
			MSHR.remove_queue(&MSHR.entry[mshr_index]);
			MSHR.num_returned--;
	
			update_fill_cycle();
        		#if remap_on_evictions
                		if(is_valid_block_evicted == 1)
                		{
                        		is_valid_block_evicted = 0;
                        		check_llc_access(); //Increment the number of llc accesses/evictions by one 
                		}
        		#endif
        		#if !(remap_on_evictions)
                        	check_llc_access(); //Increment the number of llc accesses by one  
        		#endif
				//if(cache_type == IS_LLC)
				//cout<<" ------Handle_fill ends ---------"<<endl;
		}
	}
}
//add the packet to router in this cycle, simply return without modifying 
void CACHE::handle_writeback() //Interconnect done
{
	// handle write
	uint32_t writeback_cpu = WQ.entry[WQ.head].cpu;
	if (writeback_cpu == NUM_CPUS)
		return;

	// handle the oldest entry
	if ((WQ.entry[WQ.head].event_cycle <= current_core_cycle[writeback_cpu]) && (WQ.occupancy > 0)) 
	{
		int index = WQ.head;
		uint32_t set = get_set(WQ.entry[index].address);
		int way = check_hit(&WQ.entry[index],set);
		int tag_way,tag_number,tag_set_number;
		if(MIRAGE == 1 && cache_type == IS_LLC)
                {
                        get_tag_set(WQ.entry[index].address); //sets tag0 and tag1 set number
			//cout<<" WB address : "<<WQ.entry[index].address<<endl;
			tag_number = check_hit_tag(tag0_set,tag1_set,&tag_way,WQ.entry[index].address); //return tag_number and sets tag_way
			//cout<<"tag_number  : "<<tag_number<<endl;
			if(tag_number >= 0)    //gets a hit
			{
				//cout<<"-------MIRAGE : Handle_WB Starts-------"<<endl;
	
                        	if(tag_number ==0 )
				{
					tag_set_number = tag0_set;
					set = tag0[tag_set_number][tag_way].set; 
					way = tag0[tag_set_number][tag_way].way;
					//cout<<"Set : "<<set<<" way : "<<way<<" tag : "<<block[set][way].tag<<" tag0[tag_set_number][tag_way].tag : "<<tag0[tag_set_number][tag_way].tag<<" WQ Tag "<<WQ.entry[index].address<<endl;
					//cout<<" Block Valid bit :"<<block[set][way].valid<<"block[set][way].tag_number"<<block[set][way].tag_number<<" block[set][way].tag_set " <<block[set][way].tag_set<<" block[set][way].tag_way "<<block[set][way].tag_way<<endl;
					if(tag0[tag_set_number][tag_way].tag != WQ.entry[index].address || block[set][way].tag != tag0[tag_set_number][tag_way].tag  )
                                                        assert(0);
				}
                        	else
				{
                                	tag_set_number = tag1_set;
					set = tag1[tag_set_number][tag_way].set; 
					way = tag1[tag_set_number][tag_way].way;
					//cout<<"Set : "<<set<<" way : "<<way<<" tag : "<<block[set][way].tag<<" tag_set_number "<<tag_set_number<<" tag_way_number : "<<tag_way_number<<" tag1[tag_set_number][tag_way].tag : "<<tag1[tag_set_number][tag_way].tag<<" WQ Tag "<<WQ.entry[index].address<<endl;
                                        //cout<<" Block Valid bit :"<<block[set][way].valid<<"block[set][way].tag_number"<<block[set][way].tag_number<<" block[set][way].tag_set " <<block[set][way].tag_set<<" block[set][way].tag_way "<<block[set][way].tag_way<<endl;
					if(tag1[tag_set_number][tag_way].tag != WQ.entry[index].address || block[set][way].tag != tag1[tag_set_number][tag_way].tag )
                                                        assert(0);
					//cout<<"Set : "<<set<<" way : "<<way<<" tag : "<<block[set][way].tag<<" tag0[tag_set_number][tag_way].tag : "<<tag0[tag_set_number][tag_way].tag<<endl;
				}
			}
			else 
				way = -1;
                }

		// access cache
		else if(CEASER_S_LLC == 1 && cache_type == IS_LLC && way>=0) //Reset set based on the partition
		{
			int part =way/(NUM_WAY/partitions);  
                     	if (ceaser_s_set[part] >= Sptr)
          			set=ceaser_s_set[part];
                    	else
                             	set =ceaser_s_next_set[part];
                              
               	}
		#if multi_step_relocation || bfs_on  //remap_on_evictions
                         if(CEASER_S_LLC == 1 && cache_type == IS_LLC)
                         {
                                uint32_t current_set=0;
                                way = check_hit_for_remap_on_evictions(&WQ.entry[index], &current_set);
                                if(way != -1) 
					set = current_set;
			//	cout << "gotten set: " << set << endl;
                         }
                 #endif
		if (way >= 0) { // writeback hit (or RFO hit for L1D)

			//This check is to determine before-hand if packet will get stuck in network queue and return without processing the packet or making any changes to other variables
			if ( INTERCONNECT_ON && cache_type == IS_LLC && this_router->id != writeback_cpu )
			{					
				int direction = get_direction(this_router->id,writeback_cpu);
				if ( this_router->NI[direction].OUTQ.occupancy == this_router->NI[direction].OUTQ.SIZE )
				{
					STALL[WQ.entry[index].type]++;
					cout << "[" << NAME << "] " << __func__ ;
					cout << " Router queue filled!" << " Packet address: " << hex << WQ.entry[index].address<<"Router_id"<<this_router->id;

					DP ( if (warmup_complete[writeback_cpu]) {
					cout << "[" << NAME << "] " << __func__ ;
					cout << " Router queue filled!" << " Packet address: " << hex << WQ.entry[index].address<<"Router_id"<<this_router->id;
					});
					return;
				}
			}

			if (cache_type == IS_LLC) {
				llc_update_replacement_state(writeback_cpu, set, way, block[set][way].full_addr, WQ.entry[index].ip, 0, WQ.entry[index].type, 1);
			}
			else
			update_replacement_state(writeback_cpu, set, way, block[set][way].full_addr, WQ.entry[index].ip, 0, WQ.entry[index].type, 1);

			// COLLECT STATS
			sim_hit[writeback_cpu][WQ.entry[index].type]++;
			sim_access[writeback_cpu][WQ.entry[index].type]++;

			// mark dirty
			block[set][way].dirty = 1;

			if (cache_type == IS_ITLB)
				WQ.entry[index].instruction_pa = block[set][way].data;
			else if (cache_type == IS_DTLB)
				WQ.entry[index].data_pa = block[set][way].data;
			else if (cache_type == IS_STLB)
				WQ.entry[index].data = block[set][way].data;

			// check fill level
			if (WQ.entry[index].fill_level < fill_level) 
			{
				if (cache_type == IS_LLC)//LLC --> L2C, packet can go either via network or without it[ When requesting core is directly connected to LLC slice or if INTERCONNECT IS OFF]
				{
					if (INTERCONNECT_ON == 0 || this_router->id == writeback_cpu )
					{					
						if (WQ.entry[index].instruction) 
							upper_level_icache[writeback_cpu]->return_data(&WQ.entry[index]);
						else // data
							upper_level_dcache[writeback_cpu]->return_data(&WQ.entry[index]);					
					}
					else //Packet need to go to network
					{
						int direction = get_direction(this_router->id,writeback_cpu);
						if (this_router->NI[direction].OUTQ.occupancy == this_router->NI[direction].OUTQ.SIZE) 
						{
							//dead code
							STALL[WQ.entry[index].type]++;
							DP ( if (warmup_complete[writeback_cpu]) {
							cout << "[" << NAME << "] " << __func__ ;
							cout << " Router queue filled!" << " Packet address: " << hex << WQ.entry[index].address<<"Router_id"<<this_router->id;
							});
						}
						else
						{
							WQ.entry[index].forL2 =1;
							this_router->NI[direction].add_outq(&WQ.entry[index]);  
						}
					}
				}
				else
				{
					if (WQ.entry[index].instruction) 
						upper_level_icache[writeback_cpu]->return_data(&WQ.entry[index]);
					else // data
						upper_level_dcache[writeback_cpu]->return_data(&WQ.entry[index]);
				}
			}

			HIT[WQ.entry[index].type]++;
			ACCESS[WQ.entry[index].type]++;
			
			// remove this entry from WQ
			WQ.remove_queue(&WQ.entry[index]);
			#if !(remap_on_evictions)  //If remap is done based on LLC access
                        	check_llc_access();
			#endif
		}
		else { // writeback miss (or RFO miss for L1D)

			DP ( if (warmup_complete[writeback_cpu]) {
			cout << "[" << NAME << "] " << __func__ << " type: " << +WQ.entry[index].type << " miss";
			cout << " instr_id: " << WQ.entry[index].instr_id << " address: " << hex << WQ.entry[index].address;
			cout << " full_addr: " << WQ.entry[index].full_addr << dec;
			cout << " cycle: " << WQ.entry[index].event_cycle << endl; });

			if (cache_type == IS_L1D) { // RFO miss
				// check mshr
				uint8_t miss_handled = 1;
				int mshr_index = check_mshr(&WQ.entry[index]);

				if ((mshr_index == -1) && (MSHR.occupancy < MSHR_SIZE)) { // this is a new miss

			//*Dead Code we get here only if cache_type == L1D		
		  	if(cache_type == IS_LLC) 
		  	{
		  		// check to make sure the DRAM RQ has room for this LLC RFO miss
			  	if (lower_level->get_occupancy(1, WQ.entry[index].address) == lower_level->get_size(1, WQ.entry[index].address))
				{
				  miss_handled = 0;
				}
				else
				{ 
					  add_mshr(&WQ.entry[index]);
					  lower_level->add_rq(&WQ.entry[index]);
				}
			}
			//*Dead code end			
		  else
			{
				add_mshr(&WQ.entry[index]);
				lower_level->add_rq(&WQ.entry[index]);
			}
				}
				else {
					if ((mshr_index == -1) && (MSHR.occupancy == MSHR_SIZE)) { // not enough MSHR resource
						// cannot handle miss request until one of MSHRs is available
						miss_handled = 0;
						STALL[WQ.entry[index].type]++;
					}
					else if (mshr_index != -1) { // already in-flight miss

						// update fill_level
						if (WQ.entry[index].fill_level < MSHR.entry[mshr_index].fill_level)
							MSHR.entry[mshr_index].fill_level = WQ.entry[index].fill_level;

						// update request
						if (MSHR.entry[mshr_index].type == PREFETCH) {
							uint8_t  prior_returned = MSHR.entry[mshr_index].returned;
							uint64_t prior_event_cycle = MSHR.entry[mshr_index].event_cycle;
							MSHR.entry[mshr_index] = WQ.entry[index];

							// in case request is already returned, we should keep event_cycle and retunred variables
							MSHR.entry[mshr_index].returned = prior_returned;
							MSHR.entry[mshr_index].event_cycle = prior_event_cycle;
						}

						MSHR_MERGED[WQ.entry[index].type]++;

						DP ( if (warmup_complete[writeback_cpu]) {
						cout << "[" << NAME << "] " << __func__ << " mshr merged";
						cout << " instr_id: " << WQ.entry[index].instr_id << " prior_id: " << MSHR.entry[mshr_index].instr_id; 
						cout << " address: " << hex << WQ.entry[index].address;
						cout << " full_addr: " << WQ.entry[index].full_addr << dec;
						cout << " cycle: " << WQ.entry[index].event_cycle << endl; });
					}
					else { // WE SHOULD NOT REACH HERE
						cerr << "[" << NAME << "] MSHR errors" << endl;
						assert(0);
					}
				}

				if (miss_handled) {

					MISS[WQ.entry[index].type]++;
					ACCESS[WQ.entry[index].type]++;

					// remove this entry from WQ
					WQ.remove_queue(&WQ.entry[index]);
				}

			}
			else {
				// find victim
				uint32_t way;
			 	if(cache_type == IS_LLC) 
			 	{
				       	if(MIRAGE == 1 && cache_type == IS_LLC)
                			{
						//cout<<"Handle Writeback : "<<endl;
                        			get_tag_set(WQ.entry[index].address); //sets tag0 and tag1 set number
                        			tag_number = llc_find_victim_tag(tag0_set,tag1_set,&tag_way);
                        			set=random_set();
                        			way=random_way();
                        			if(tag_number ==0 )
                                			tag_set_number = tag0_set;
                        			else
                                			tag_set_number = tag1_set;
						//cout<<"tag_number : "<<tag_number<<" tag_set_number : "<<tag_set_number<<"tag_way_number : "<<tag_way<<endl;
                        			//cout<<"set : "<<set<<" way : "<<way<<endl;

                			}
					else if(CEASER_S_LLC == 1 && cache_type == IS_LLC)
					{
			 			int part=rand()%partitions; //random partition is selected to fill the cache block
                         			if (ceaser_s_set[part] >= Sptr)
          				  		set=ceaser_s_set[part];
                        			else
                                			set =ceaser_s_next_set[part];
                        			way = llc_find_victim_ceaser_s(writeback_cpu, WQ.entry[index].instr_id, set, block[set], WQ.entry[index].ip, WQ.entry[index].full_addr, WQ.entry[index].type,part); 
                	 		}
				 	else //baseline
				   		way = llc_find_victim(writeback_cpu, WQ.entry[index].instr_id, set, block[set], WQ.entry[index].ip, WQ.entry[index].full_addr, WQ.entry[index].type);
				}
				else
					way = find_victim(writeback_cpu, WQ.entry[index].instr_id, set, block[set], WQ.entry[index].ip, WQ.entry[index].full_addr, WQ.entry[index].type);

#ifdef LLC_BYPASS
				if ((cache_type == IS_LLC) && (way == LLC_WAY)) {
					cerr << "LLC bypassing for writebacks is not allowed!" << endl;
					assert(0);
				}
#endif

				uint8_t  do_fill = 1;

				// is this dirty?
				if (block[set][way].dirty) {    

					if (cache_type == IS_L2C) 
						{
							if (INTERCONNECT_ON == 0 || get_slice_num(block[set][way].tag) == this_router->id )
							{
							   int destination_slice = get_slice_num(block[set][way].tag);
							   if (this_router->LLC_MAP[destination_slice]->get_occupancy(2, block[set][way].tag) == this_router->LLC_MAP[destination_slice]->get_size(2, block[set][way].tag)) 
							   {
									// lower level WQ is full, cannot replace this victim
									do_fill = 0;
									this_router->LLC_MAP[destination_slice]->increment_WQ_FULL(block[set][way].tag);
									STALL[WQ.entry[index].type]++;

									DP ( if (warmup_complete[writeback_cpu]) {
									cout << "[" << NAME << "] " << __func__ << "do_fill: " << do_fill;
									cout << " lower level wq is full!" << " fill_addr: " << hex << WQ.entry[index].address;
									cout << " victim_addr: " << block[set][way].tag << dec << endl; });
								}
								else
								{
									PACKET writeback_packet;

									writeback_packet.fill_level = fill_level << 1;
									writeback_packet.cpu = writeback_cpu;
									curr_addr=getDecryptedAddress(set,way);
									if(curr_addr==-1)
										do_fill=0;
				                                        full_addr       = (curr_addr << LOG2_BLOCK_SIZE) + (block[set][way].full_addr & 0x3F);


									writeback_packet.address = curr_addr;
									writeback_packet.full_addr = full_addr;
									writeback_packet.data = block[set][way].data;
									writeback_packet.instr_id = WQ.entry[index].instr_id;
									writeback_packet.ip = 0;
									writeback_packet.type = WRITEBACK;
									writeback_packet.event_cycle = current_core_cycle[writeback_cpu];
									if(do_fill == 1)
										this_router->LLC_MAP[destination_slice]->add_wq(&writeback_packet);
								}									
							}
							else
							{
								//WB packet need to go to network     
								int destination = get_slice_num(block[set][way].tag);
								int direction = get_direction(cpu,destination);
								if (this_router->NI[direction].OUTQ.occupancy == this_router->NI[direction].OUTQ.SIZE) 
								{
									// Network Interface queue is full, cannot add this victim to 
									do_fill = 0;
									STALL[WQ.entry[index].type]++;

									DP ( if (warmup_complete[writeback_cpu]) {
									cout << "[" << NAME << "] " << __func__ << "do_fill: " << do_fill;
									cout << " Router queue is full!" << " fill_addr: " << hex << WQ.entry[index].address;
									cout << " victim_addr: " << block[set][way].tag << dec << endl; });
								}
								else 
								{
									PACKET writeback_packet;
									writeback_packet.fill_level = fill_level << 1;
									writeback_packet.cpu = writeback_cpu;
									curr_addr=getDecryptedAddress(set,way);
                                                        		if(curr_addr==-1)
                                                                		do_fill=0;
				                                        full_addr       = (curr_addr << LOG2_BLOCK_SIZE) + (block[set][way].full_addr & 0x3F);

									writeback_packet.address = curr_addr;
									writeback_packet.full_addr = full_addr;
									writeback_packet.data = block[set][way].data;
									writeback_packet.instr_id = WQ.entry[index].instr_id;
									writeback_packet.ip = 0;
									writeback_packet.type = WRITEBACK;
									writeback_packet.event_cycle = current_core_cycle[writeback_cpu];
									// cout<<"DP WB packet"<<writeback_packet.cpu;                                
									writeback_packet.forL2=0;
									if (do_fill == 1)
										this_router->NI[direction].add_outq(&writeback_packet);   
								}       
							
							}
						}
					else//For other caches, No changes
					{
						// check if the lower level WQ has enough room to keep this writeback request
						if (lower_level) 
						{
							if (lower_level->get_occupancy(2, block[set][way].tag) == lower_level->get_size(2, block[set][way].tag)) 
								{
								// lower level WQ is full, cannot replace this victim
								do_fill = 0;
								lower_level->increment_WQ_FULL(block[set][way].tag);
								STALL[WQ.entry[index].type]++;

								DP ( if (warmup_complete[writeback_cpu]) {
								cout << "[" << NAME << "] " << __func__ << "do_fill: " << do_fill;
								cout << " lower level wq is full!" << " fill_addr: " << hex << WQ.entry[index].address;
								cout << " victim_addr: " << block[set][way].tag << dec << endl; });
								}
							else { 
								PACKET writeback_packet;

								writeback_packet.fill_level = fill_level << 1;
								writeback_packet.cpu = writeback_cpu;
								curr_addr=getDecryptedAddress(set,way);
	                                                        if(curr_addr==-1)
        	                                                        do_fill=0;
			                                        full_addr       = (curr_addr << LOG2_BLOCK_SIZE) + (block[set][way].full_addr & 0x3F);

								writeback_packet.address = curr_addr;
								writeback_packet.full_addr = full_addr;
								writeback_packet.data = block[set][way].data;
								writeback_packet.instr_id = WQ.entry[index].instr_id;
								writeback_packet.ip = 0;
								writeback_packet.type = WRITEBACK;
								writeback_packet.event_cycle = current_core_cycle[writeback_cpu];
								if(do_fill == 1)
									lower_level->add_wq(&writeback_packet);
								}
							}   

#ifdef SANITY_CHECK
					else {
						// sanity check
						if (cache_type != IS_STLB)
							assert(0);
					}
#endif
				 }
				}

				if (do_fill) 
				{
					//This check is to determine before-hand if packet will get stuck in network queue and return without processing
					if (INTERCONNECT_ON && cache_type == IS_LLC && WQ.entry[index].fill_level < fill_level)
					{
						int direction = get_direction(this_router->id,writeback_cpu);
						if (this_router->id != writeback_cpu && this_router->NI[direction].OUTQ.occupancy == this_router->NI[direction].OUTQ.SIZE)
						{
							STALL[WQ.entry[index].type]++;

							DP ( if (warmup_complete[writeback_cpu]) {
							cout << "[" << NAME << "] " << __func__ ;
							cout << " Router queue filled!" << " fill_addr: " << hex << WQ.entry[index].address;
							cout << " victim_addr: " << block[set][way].tag << dec << endl; });			
						return;
						}
					}

					// update prefetcher
					if (cache_type == IS_L1D)
						l1d_prefetcher_cache_fill(WQ.entry[index].full_addr, set, way, 0, block[set][way].tag<<LOG2_BLOCK_SIZE, WQ.entry[index].pf_metadata);
					else if (cache_type == IS_L2C)
						WQ.entry[index].pf_metadata = l2c_prefetcher_cache_fill(WQ.entry[index].address<<LOG2_BLOCK_SIZE, set, way, 0,block[set][way].tag<<LOG2_BLOCK_SIZE, WQ.entry[index].pf_metadata);
					if (cache_type == IS_LLC)
					{
						cpu = writeback_cpu;
						WQ.entry[index].pf_metadata =llc_prefetcher_cache_fill(WQ.entry[index].address<<LOG2_BLOCK_SIZE, set, way, 0,block[set][way].tag<<LOG2_BLOCK_SIZE, WQ.entry[index].pf_metadata);
						cpu = 0;
					}

					// update replacement policy
					if (cache_type == IS_LLC) {
						llc_update_replacement_state(writeback_cpu, set, way, WQ.entry[index].full_addr, WQ.entry[index].ip, block[set][way].full_addr, WQ.entry[index].type, 0);
					}
					else
						update_replacement_state(writeback_cpu, set, way, WQ.entry[index].full_addr, WQ.entry[index].ip, block[set][way].full_addr, WQ.entry[index].type, 0);

					// COLLECT STATS
					sim_miss[writeback_cpu][WQ.entry[index].type]++;
					sim_access[writeback_cpu][WQ.entry[index].type]++;
					#if remap_on_evictions
                                          	if(block[set][way].valid == 1)
                                          	{
                                                 	is_valid_block_evicted = 1;
                                          	}
                                        #endif
					if(MIRAGE == 1 && cache_type == IS_LLC)
                        		{
                                		if(block[set][way].valid == 1)
                                        		remove_tag_entry_from_block(set,way);
						if((tag_number == 0 && tag0[tag_set_number][tag_way].valid == 1) || (tag_number == 1 && tag1[tag_set_number][tag_way].valid == 1))
                                        remove_block_based_on_tag_array(tag_number,tag_set_number,tag_way);
                                		//fill_tag(tag_set_number,tag_way,&MSHR.entry[mshr_index]);
						fill_tag(tag_number,tag_set_number,tag_way,WQ.entry[index].address,set,way);
                        		}

					fill_cache(set, way, &WQ.entry[index]);

					// mark dirty
					block[set][way].dirty = 1; 

					// check fill level
					if (WQ.entry[index].fill_level < fill_level) 
					{
					//Instead of directly passing to L2, we add to router if required
						if (cache_type == IS_LLC)
						{

							if (INTERCONNECT_ON == 0 || this_router->id == writeback_cpu)
							{
								if (WQ.entry[index].instruction)
	 								upper_level_icache[writeback_cpu]->return_data(&WQ.entry[index]);
								else // data
									upper_level_dcache[writeback_cpu]->return_data(&WQ.entry[index]);							
							}
							else
							{
								int direction = get_direction(this_router->id,writeback_cpu);
								if (this_router->NI[direction].OUTQ.occupancy == this_router->NI[direction].OUTQ.SIZE) 
								   {
									//Dead Code. if queue was filled then do_fill ==0 and we were not here
									//What all need to be done when router queue is filled ?
									STALL[WQ.entry[index].type]++;

									DP ( if (warmup_complete[writeback_cpu]) {
									cout << "[" << NAME << "] " << __func__ ;
									cout << " Router queue filled!" << " fill_addr: " << hex << WQ.entry[index].address;
									cout << " victim_addr: " << block[set][way].tag << dec << endl; });
									}
								else
									{
										WQ.entry[index].forL2 =1;
										this_router->NI[direction].add_outq(&WQ.entry[index]);  
									}    
								
							}	
						}
					 	else
					 	{ //For other caches, no change is required
					 		if (WQ.entry[index].instruction) 
								upper_level_icache[writeback_cpu]->return_data(&WQ.entry[index]);
							else // data
								upper_level_dcache[writeback_cpu]->return_data(&WQ.entry[index]);
						}       
					}
					MISS[WQ.entry[index].type]++;
					ACCESS[WQ.entry[index].type]++;

					// remove this entry from WQ
					WQ.remove_queue(&WQ.entry[index]);
					#if remap_on_evictions
                				if(is_valid_block_evicted == 1)
                				{
                        				is_valid_block_evicted = 0;
                        				check_llc_access(); 
                				}
        				#endif
					#if !(remap_on_evictions)
                        			check_llc_access();
					#endif
				}
			}
		}
	}
	//if(cache_type == IS_LLC && MIRAGE == 1)
	//	cout<<"-------MIRAGE : Handle_WB ENDS-------"<<endl;

}

void CACHE::handle_read()
{
	// handle read
	for (uint32_t i=0; i<MAX_READ; i++)
	{
		uint32_t read_cpu = RQ.entry[RQ.head].cpu;
	  	if(read_cpu == NUM_CPUS)
		return;
		// handle the oldest entry
		if ((RQ.entry[RQ.head].event_cycle <= current_core_cycle[read_cpu]) && (RQ.occupancy > 0))
		{
			int index = RQ.head;
			unsigned long long cycle_enqued_at_head=RQ.entry[index].cycle_enqueued;		
			uint32_t set = get_set(RQ.entry[index].address);
			int way = check_hit(&RQ.entry[index],set);
			int tag_way,tag_number,tag_set_number;
			if(MIRAGE == 1 && cache_type == IS_LLC)
                	{
                        	get_tag_set(RQ.entry[index].address); //sets tag0 and tag1 set number
                        	tag_number = check_hit_tag(tag0_set,tag1_set,&tag_way,RQ.entry[index].address); //return tag_number and sets tag_way
                        	if(tag_number >= 0)    //gets a hit
                        	{
					if(tag_number ==0 )
                                	{
                                        	tag_set_number = tag0_set;
                                        	set = tag0[tag_set_number][tag_way].set;
                                        	way = tag0[tag_set_number][tag_way].way;
						if(block[set][way].tag != tag0[tag_set_number][tag_way].tag || block[set][way].tag != RQ.entry[index].address )
							assert(0);
                                	}
                                	else
                                	{
                                        	tag_set_number = tag1_set;
                                        	set = tag1[tag_set_number][tag_way].set;
                                        	way = tag1[tag_set_number][tag_way].way;
						if(block[set][way].tag != tag1[tag_set_number][tag_way].tag || block[set][way].tag != RQ.entry[index].address )
                                                        assert(0);

                                	}

                        	}
				else
					way = -1;
                	}
			else if(CEASER_S_LLC == 1 && cache_type == IS_LLC && way>=0)
			{
                   	        int part =way/(NUM_WAY/partitions);
                                if (ceaser_s_set[part] >= Sptr)
          				set=ceaser_s_set[part];
                                else
                                	set =ceaser_s_next_set[part];
                       	}
			#if multi_step_relocation || bfs_on  //remap_on_evictions
			 if(CEASER_S_LLC == 1 && cache_type == IS_LLC)
			 {
				uint32_t current_set=0;
				way = check_hit_for_remap_on_evictions(&RQ.entry[index],  &current_set);
				if(way != -1)
					set = current_set;
				//cout << "gotten set: " << set << endl;
			 }
			#endif
			if (way >= 0) // read hit
			{ 
				//This check is to determine before-hand if packet will get stuck in network queue and return without processing
				if ( INTERCONNECT_ON && cache_type == IS_LLC && RQ.entry[index].fill_level < fill_level )
				{
					int direction = get_direction(this_router->id,read_cpu);
					if (this_router->id != read_cpu && this_router->NI[direction].OUTQ.occupancy == this_router->NI[direction].OUTQ.SIZE)
					{
						STALL[RQ.entry[index].type]++;
						cout << "[" << NAME << "] " << __func__ ;
						cout << " Router queue filled!" << " Read_addr: " << hex << RQ.entry[index].address;
						DP ( if (warmup_complete[read_cpu]) {
								cout << "[" << NAME << "] " << __func__ ;
								cout << " Router queue filled!" << " Read_addr: " << hex << RQ.entry[index].address;
								});
						return;
					}
				}					
				if (cache_type == IS_ITLB) {
					RQ.entry[index].instruction_pa = block[set][way].data;
					if (PROCESSED.occupancy < PROCESSED.SIZE)
						PROCESSED.add_queue(&RQ.entry[index]);
				}
				else if (cache_type == IS_DTLB) {
					RQ.entry[index].data_pa = block[set][way].data;
					if (PROCESSED.occupancy < PROCESSED.SIZE)
						PROCESSED.add_queue(&RQ.entry[index]);
				}
				else if (cache_type == IS_STLB) 
					RQ.entry[index].data = block[set][way].data;
				else if (cache_type == IS_L1I) {
					if (PROCESSED.occupancy < PROCESSED.SIZE)
						PROCESSED.add_queue(&RQ.entry[index]);
				}
				else if ((cache_type == IS_L1D) && (RQ.entry[index].type != PREFETCH)) 
				{
					if (PROCESSED.occupancy < PROCESSED.SIZE)
						PROCESSED.add_queue(&RQ.entry[index]);
				}

				// update prefetcher on load instruction
				if (RQ.entry[index].type == LOAD) 
				{
					if (cache_type == IS_L1D) 
						l1d_prefetcher_operate(RQ.entry[index].full_addr, RQ.entry[index].ip, 1, RQ.entry[index].type);
					else if (cache_type == IS_L2C)
						l2c_prefetcher_operate(block[set][way].tag<<LOG2_BLOCK_SIZE, RQ.entry[index].ip, 1, RQ.entry[index].type, 0);
					else if (cache_type == IS_LLC)
					{
						cpu = read_cpu;
						llc_prefetcher_operate(block[set][way].tag<<LOG2_BLOCK_SIZE, RQ.entry[index].ip, 1, RQ.entry[index].type, 0);
						cpu = 0;
					}
				}

				// update replacement policy
				if (cache_type == IS_LLC) {
					llc_update_replacement_state(read_cpu, set, way, block[set][way].full_addr, RQ.entry[index].ip, 0, RQ.entry[index].type, 1);
				}
				else
					update_replacement_state(read_cpu, set, way, block[set][way].full_addr, RQ.entry[index].ip, 0, RQ.entry[index].type, 1);

				// COLLECT STATS
				sim_hit[read_cpu][RQ.entry[index].type]++;
				sim_access[read_cpu][RQ.entry[index].type]++;

				// check fill level
				if (RQ.entry[index].fill_level < fill_level)
				{
					if (cache_type == IS_LLC)
					{
						if (INTERCONNECT_ON == 0 || this_router->id == read_cpu)
						{
							if (RQ.entry[index].instruction) 
								upper_level_icache[read_cpu]->return_data(&RQ.entry[index]);
							else // data
								upper_level_dcache[read_cpu]->return_data(&RQ.entry[index]);
						}
						else
						{	
							int direction = get_direction(this_router->id,read_cpu);
							if (this_router->NI[direction].OUTQ.occupancy == this_router->NI[direction].OUTQ.SIZE)
							{
								//Dead Code
								STALL[RQ.entry[index].type]++;

								DP ( if (warmup_complete[read_cpu]) {
								cout << "[" << NAME << "] " << __func__ ;
								cout << " Router queue filled!" << " Read_addr: " << hex << RQ.entry[index].address;
								 });
							}
							else
							{
								RQ.entry[index].forL2 =1;
								this_router->NI[direction].add_outq(&RQ.entry[index]);  
							}    
							
						}
						}
				 else  //For other caches, no changes.     
					{    
					if (RQ.entry[index].instruction) 
						upper_level_icache[read_cpu]->return_data(&RQ.entry[index]);
					else // data
						upper_level_dcache[read_cpu]->return_data(&RQ.entry[index]);
					}
				}

				// update prefetch stats and reset prefetch bit
				if (block[set][way].prefetch) {
					pf_useful++;
					block[set][way].prefetch = 0;
				}
				block[set][way].used = 1;

				HIT[RQ.entry[index].type]++;
				ACCESS[RQ.entry[index].type]++;
				
				// remove this entry from RQ
				if(cache_type == IS_LLC &&  all_warmup_complete > NUM_CPUS )
				{
					total_waiting_time_in_rq[RQ.entry[index].cpu] += ( current_core_cycle[RQ.entry[index].cpu] - RQ.entry[index].cycle_enqueued);
					total_read_packets[RQ.entry[index].cpu]++;
				}
				RQ.remove_queue(&RQ.entry[index]);
				reads_available_this_cycle--;
				#if !(remap_on_evictions)
        	                        check_llc_access(); 
	                        #endif
			}
			else { // read miss
				DP ( if (warmup_complete[read_cpu]) {
				cout << "[" << NAME << "] " << __func__ << " read miss";
				cout << " instr_id: " << RQ.entry[index].instr_id << " address: " << hex << RQ.entry[index].address;
				cout << " full_addr: " << RQ.entry[index].full_addr << dec;
				cout << " cycle: " << RQ.entry[index].event_cycle << endl; 
				});
				// check mshr
				uint8_t miss_handled = 1;
				int mshr_index = check_mshr(&RQ.entry[index]);

				if ((mshr_index == -1) && (MSHR.occupancy < MSHR_SIZE)) 
				{ // this is a new miss
					//This check is to determine before-hand if packet will get stuck in network queue and return without processing					
					if (INTERCONNECT_ON && cache_type==IS_L2C) 
					{
						int destination_slice = get_slice_num(RQ.entry[index].address);
						int direction = get_direction(read_cpu,destination_slice);						
						if (destination_slice != read_cpu && this_router->NI[direction].OUTQ.occupancy == this_router->NI[direction].OUTQ.SIZE)
						{
							//Router Queue filled
							cout << "[" << NAME << "] " << __func__ ;
							cout << " Router queue filled!" << " fill_addr: " << hex << RQ.entry[index].address;

							STALL[RQ.entry[index].type]++;
							DP ( if (warmup_complete[read_cpu]) {
							cout << "[" << NAME << "] " << __func__ ;
							cout << " Router queue filled!" << " fill_addr: " << hex << RQ.entry[index].address;
							});
							return;							
						}
					}

					if(cache_type == IS_LLC)
					{
						  // check to make sure the DRAM RQ has room for this LLC read miss
						if (lower_level->get_occupancy(1, RQ.entry[index].address) == lower_level->get_size(1, RQ.entry[index].address))
						{
							miss_handled = 0;
						}
						else
						{       //newly_added 
								add_mshr(&RQ.entry[index]);
								if(lower_level)
								{
									lower_level->add_rq(&RQ.entry[index]);
								}
							
						}
					}
					else
					{
						int insert_err = 0;
							
					  	// add it to the next level's read queue
						if (cache_type==IS_L2C)
						{
							int destination = get_slice_num(RQ.entry[index].address);

							if (INTERCONNECT_ON == 0 || get_slice_num(RQ.entry[index].address) == read_cpu )
							{
								if ((this_router->LLC_MAP[get_slice_num(RQ.entry[index].address)])->get_occupancy(1,RQ.entry[index].address) == (this_router->LLC_MAP[get_slice_num(RQ.entry[index].address)])->get_size(1,RQ.entry[index].address))
								{
									miss_handled=0;
								}
								else{
									add_mshr(&RQ.entry[index]);
									insert_err = (this_router->LLC_MAP[get_slice_num(RQ.entry[index].address)])->add_rq(&RQ.entry[index]);
								}
								
							}	
							else
							{
									//	int destination = get_slice_num(RQ.entry[index].address);
										int direction = get_direction(read_cpu,destination);
										if (this_router->NI[direction].OUTQ.occupancy == this_router->NI[direction].OUTQ.SIZE) {
											insert_err=-2;
											miss_handled=0;
											// Dead code.
											//Router Queue filled
											STALL[RQ.entry[index].type]++;
											DP ( if (warmup_complete[read_cpu]) {
											cout << "[" << NAME << "] " << __func__ ;
											cout << " Router queue filled!" << " fill_addr: " << hex << RQ.entry[index].address;});
										}
										else{
											add_mshr(&RQ.entry[index]);
											RQ.entry[index].forL2=0;
											insert_err = this_router->NI[direction].add_outq(&RQ.entry[index]);   
										}                    
									
							}	   
						}
						else//For caches other than L2 no change is required
						{
							// add it to mshr (read miss)
							add_mshr(&RQ.entry[index]);
							if (lower_level){
								lower_level->add_rq(&RQ.entry[index]);
							}
							else 
							{ // this is the last level
								if (cache_type == IS_STLB) 
								{
									// TODO: need to differentiate page table walk and actual swap
						  			// emulate page table walk
						  			uint64_t pa = va_to_pa(read_cpu, RQ.entry[index].instr_id, RQ.entry[index].full_addr, RQ.entry[index].address);
						  			RQ.entry[index].data = pa >> LOG2_PAGE_SIZE;
						  			RQ.entry[index].event_cycle = current_core_cycle[read_cpu];
						  			return_data(&RQ.entry[index]);
						  		}
						  	}		
						} 
					}
				}
				else {
					if ((mshr_index == -1) && (MSHR.occupancy == MSHR_SIZE)) { // not enough MSHR resource
						
						// cannot handle miss request until one of MSHRs is available
						miss_handled = 0;
						STALL[RQ.entry[index].type]++;
					}
					else if (mshr_index != -1) { // already in-flight miss
						// mark merged consumer
						if (RQ.entry[index].type == RFO) {

							if (RQ.entry[index].tlb_access) {
								uint32_t sq_index = RQ.entry[index].sq_index;
								MSHR.entry[mshr_index].store_merged = 1;
								MSHR.entry[mshr_index].sq_index_depend_on_me.insert (sq_index);
				MSHR.entry[mshr_index].sq_index_depend_on_me.join (RQ.entry[index].sq_index_depend_on_me, SQ_SIZE);
							}

							if (RQ.entry[index].load_merged) {
								//uint32_t lq_index = RQ.entry[index].lq_index; 
								MSHR.entry[mshr_index].load_merged = 1;
								//MSHR.entry[mshr_index].lq_index_depend_on_me[lq_index] = 1;
				MSHR.entry[mshr_index].lq_index_depend_on_me.join (RQ.entry[index].lq_index_depend_on_me, LQ_SIZE);
							}
						}
						else {
							if (RQ.entry[index].instruction) {
								uint32_t rob_index = RQ.entry[index].rob_index;
								MSHR.entry[mshr_index].instr_merged = 1;
								MSHR.entry[mshr_index].rob_index_depend_on_me.insert (rob_index);

								DP (if (warmup_complete[MSHR.entry[mshr_index].cpu]) {
								cout << "[INSTR_MERGED] " << __func__ << " cpu: " << MSHR.entry[mshr_index].cpu << " instr_id: " << MSHR.entry[mshr_index].instr_id;
								cout << " merged rob_index: " << rob_index << " instr_id: " << RQ.entry[index].instr_id << endl; });

								if (RQ.entry[index].instr_merged) {
					MSHR.entry[mshr_index].rob_index_depend_on_me.join (RQ.entry[index].rob_index_depend_on_me, ROB_SIZE);
									DP (if (warmup_complete[MSHR.entry[mshr_index].cpu]) {
									cout << "[INSTR_MERGED] " << __func__ << " cpu: " << MSHR.entry[mshr_index].cpu << " instr_id: " << MSHR.entry[mshr_index].instr_id;
									cout << " merged rob_index: " << i << " instr_id: N/A" << endl; });
								}
							}
							else 
							{
								uint32_t lq_index = RQ.entry[index].lq_index;
								MSHR.entry[mshr_index].load_merged = 1;
								MSHR.entry[mshr_index].lq_index_depend_on_me.insert (lq_index);

								DP (if (warmup_complete[read_cpu]) {
								cout << "[DATA_MERGED] " << __func__ << " cpu: " << read_cpu << " instr_id: " << RQ.entry[index].instr_id;
								cout << " merged rob_index: " << RQ.entry[index].rob_index << " instr_id: " << RQ.entry[index].instr_id << " lq_index: " << RQ.entry[index].lq_index << endl; });
				MSHR.entry[mshr_index].lq_index_depend_on_me.join (RQ.entry[index].lq_index_depend_on_me, LQ_SIZE);
								if (RQ.entry[index].store_merged) {
									MSHR.entry[mshr_index].store_merged = 1;
					MSHR.entry[mshr_index].sq_index_depend_on_me.join (RQ.entry[index].sq_index_depend_on_me, SQ_SIZE);
								}
							}
						}

						// update fill_level
						if (RQ.entry[index].fill_level < MSHR.entry[mshr_index].fill_level)
							MSHR.entry[mshr_index].fill_level = RQ.entry[index].fill_level;
						//PREFETCH_REMOVED
						// update request
						if (MSHR.entry[mshr_index].type == PREFETCH) {
							uint8_t  prior_returned = MSHR.entry[mshr_index].returned;
							uint64_t prior_event_cycle = MSHR.entry[mshr_index].event_cycle;
							//@ Neelu
							++pf_late; //Late Prefetch
							if( MSHR.entry[mshr_index].returned != COMPLETED && cache_type != IS_LLC)
                                                	{
                                                           	int merged = add_read_to_lower_level(mshr_index,index,read_cpu); //If Load request gets a hit in mshr which used to be a prefetch request then this packet is send to read queue of lower level
							   	if(merged == -1) 
							   	{
									miss_handled = 0 ;
								}
							 
							}
							if(miss_handled != 0)
							MSHR.entry[mshr_index] = RQ.entry[index];
							MSHR.entry[mshr_index].returned = prior_returned;
							MSHR.entry[mshr_index].event_cycle = prior_event_cycle;
						}

						MSHR_MERGED[RQ.entry[index].type]++;

						DP ( if (warmup_complete[read_cpu]) {
						cout << "[" << NAME << "] " << __func__ << " mshr merged";
						cout << " instr_id: " << RQ.entry[index].instr_id << " prior_id: " << MSHR.entry[mshr_index].instr_id; 
						cout << " address: " << hex << RQ.entry[index].address;
						cout << " full_addr: " << RQ.entry[index].full_addr << dec;
						cout << " cycle: " << RQ.entry[index].event_cycle << endl; });
					}
					else { // WE SHOULD NOT REACH HERE
						cerr << "[" << NAME << "] MSHR errors" << endl;
						assert(0);
					}
				}

				if (miss_handled) {
					// update prefetcher on load instruction
				if (RQ.entry[index].type == LOAD) 
				{
						if (cache_type == IS_L1D) 
							l1d_prefetcher_operate(RQ.entry[index].full_addr, RQ.entry[index].ip, 0, RQ.entry[index].type);
						if (cache_type == IS_L2C)
			  				l2c_prefetcher_operate(RQ.entry[index].address<<LOG2_BLOCK_SIZE, RQ.entry[index].ip, 0, RQ.entry[index].type, 0);
						if (cache_type == IS_LLC)
			  			{
							cpu = read_cpu;
							llc_prefetcher_operate(RQ.entry[index].address<<LOG2_BLOCK_SIZE, RQ.entry[index].ip, 0, RQ.entry[index].type, 0);
							cpu = 0;
			  			}
				}

					MISS[RQ.entry[index].type]++;
					ACCESS[RQ.entry[index].type]++;
                        if(warmup_complete[RQ.entry[index].cpu])
                        {
                                uint64_t current_miss_latency = (current_core_cycle[RQ.entry[index].cpu] - RQ.entry[index].cycle_enqueued);
                        }
			RQ.entry[index].cycle_enqueued=cycle_enqued_at_head; //Because of add_mshr cycle enqued value has been changed 
			if(cache_type == IS_LLC &&  all_warmup_complete > NUM_CPUS )
			{
					 total_waiting_time_in_rq[RQ.entry[index].cpu] += ( current_core_cycle[RQ.entry[index].cpu] - RQ.entry[index].cycle_enqueued);	
					// remove this entry from RQ
					total_read_packets[RQ.entry[index].cpu]++;
			//		cout<<"Difference : "<< ( current_core_cycle[RQ.entry[index].cpu] - RQ.entry[index].cycle_enqueued);
			}
			RQ.entry[index].cycle_enqueued=current_core_cycle[RQ.entry[index].cpu];
					 RQ.remove_queue(&RQ.entry[index]);

			reads_available_this_cycle--;
				}
			}
		}
	else
	  {
		return;
	  }

	if(reads_available_this_cycle == 0)
	  {
		return;
	  }
	}
}

void CACHE::handle_prefetch()
{
	// handle prefetch
	for (uint32_t i=0; i<MAX_READ; i++) {
	  
	  uint32_t prefetch_cpu = PQ.entry[PQ.head].cpu;
	  if (prefetch_cpu == NUM_CPUS)
		return;

		// handle the oldest entry
		if ((PQ.entry[PQ.head].event_cycle <= current_core_cycle[prefetch_cpu]) && (PQ.occupancy > 0)) {
			int index = PQ.head;
			unsigned long long cycle_enqued_at_head=PQ.entry[index].cycle_enqueued;
			// access cache
			uint32_t set = get_set(PQ.entry[index].address);
			int way = check_hit(&PQ.entry[index],set);
			if(MIRAGE == 1 && cache_type == IS_LLC)
                	{
                        	get_tag_set(PQ.entry[index].address); //sets tag0 and tag1 set number
                        	int tag_way;
                        	int tag_number = check_hit_tag(tag0_set,tag1_set,&tag_way,PQ.entry[index].address); //return tag_number and sets tag_way
                        	uint32_t tag_set_number;
                        	if(tag_number >= 0)    //gets a hit
                        	{
					if(tag_number ==0 )
                                	{
                                        	tag_set_number = tag0_set;
                                        	set = tag0[tag_set_number][tag_way].set;
                                        	way = tag0[tag_set_number][tag_way].way;
                                        	if(block[set][way].tag != tag0[tag_set_number][tag_way].tag || block[set][way].tag != PQ.entry[index].address )
                                                        assert(0);
                                	}
                                	else
                                	{
                                        	tag_set_number = tag1_set;
                                        	set = tag1[tag_set_number][tag_way].set;
                                        	way = tag1[tag_set_number][tag_way].way;
                                        	if(block[set][way].tag != tag1[tag_set_number][tag_way].tag || block[set][way].tag != PQ.entry[index].address )
                                                        assert(0);
                                	}

                        	}
				else
					way = -1;
                	}

			else if(CEASER_S_LLC == 1 && cache_type == IS_LLC && way>=0)
			{
                    		int part =way/(NUM_WAY/partitions);
                                if (ceaser_s_set[part] >= Sptr)
          				  set=ceaser_s_set[part];
                                else
                                set =ceaser_s_next_set[part];
                	}
			#if multi_step_relocation || bfs_on    //remap_on_evictions
                         if(CEASER_S_LLC == 1 && cache_type == IS_LLC)  //Overwrite 
                         {
				 //cout<<"Hi"<<endl;
                                uint32_t current_set=0;
                                way = check_hit_for_remap_on_evictions(&PQ.entry[index],  &current_set);
                                if(way != -1)
					set = current_set;
			//	cout << "gotten set: " << set << endl;
                         }
                        #endif
			if (way >= 0) 
			{ // prefetch hit

				//This check is to determine before-hand if packet will get stuck in network queue and return without processing
				if (INTERCONNECT_ON && cache_type == IS_LLC && PQ.entry[index].fill_level < fill_level )
				{
					int direction = get_direction(this_router->id,prefetch_cpu);
					if (this_router->id != prefetch_cpu && this_router->NI[direction].OUTQ.occupancy == this_router->NI[direction].OUTQ.SIZE)
					{
					STALL[PQ.entry[index].type]++;
					cout << "[" << NAME << "] " << __func__ ;
					cout << " Router queue filled!" << " Prefetch_addr: " << hex << PQ.entry[index].address;

					DP ( if (warmup_complete[prefetch_cpu]) {
					cout << "[" << NAME << "] " << __func__ ;
					cout << " Router queue filled!" << " Prefetch_addr: " << hex << PQ.entry[index].address;
					// cout << " victim_addr: " << block[set][way].tag << dec << endl;
					 });
					return;
					}
				}

				// update replacement policy
				if (cache_type == IS_LLC) {
					llc_update_replacement_state(prefetch_cpu, set, way, block[set][way].full_addr, PQ.entry[index].ip, 0, PQ.entry[index].type, 1);
				}
				else
					update_replacement_state(prefetch_cpu, set, way, block[set][way].full_addr, PQ.entry[index].ip, 0, PQ.entry[index].type, 1);

				// COLLECT STATS
				sim_hit[prefetch_cpu][PQ.entry[index].type]++;
				sim_access[prefetch_cpu][PQ.entry[index].type]++;

				// run prefetcher on prefetches from higher caches
				if(PQ.entry[index].pf_origin_level < fill_level)
				{
					if (cache_type == IS_L1D)
					  l1d_prefetcher_operate(PQ.entry[index].full_addr, PQ.entry[index].ip, 1, PREFETCH);
					else if (cache_type == IS_L2C)
						PQ.entry[index].pf_metadata = l2c_prefetcher_operate(block[set][way].tag<<LOG2_BLOCK_SIZE, PQ.entry[index].ip, 1, PREFETCH, PQ.entry[index].pf_metadata);
					else if (cache_type == IS_LLC)
					{
						cpu = prefetch_cpu;
						PQ.entry[index].pf_metadata = llc_prefetcher_operate(block[set][way].tag<<LOG2_BLOCK_SIZE, PQ.entry[index].ip, 1, PREFETCH, PQ.entry[index].pf_metadata);
						cpu = 0;
					 }
				}
				// check fill level
				if (PQ.entry[index].fill_level < fill_level) 
				{
					if (cache_type == IS_LLC)//LLC --> L2C, packet can go either via network or without it[ When requesting core is directly connected to LLC slice or if INTERCONNECT IS OFF]
					{
						if (INTERCONNECT_ON == 0 || this_router->id == prefetch_cpu )
						{
							if (PQ.entry[index].instruction) 
								upper_level_icache[prefetch_cpu]->return_data(&PQ.entry[index]);
							else // data
								upper_level_dcache[prefetch_cpu]->return_data(&PQ.entry[index]);
						}	
						else
						{
							int direction = get_direction(this_router->id,prefetch_cpu);
							if (this_router->NI[direction].OUTQ.occupancy == this_router->NI[direction].OUTQ.SIZE) 
							   {
								//Dead code
								STALL[PQ.entry[index].type]++;

								DP ( if (warmup_complete[prefetch_cpu]) {
								cout << "[" << NAME << "] " << __func__ ;
								cout << " Router queue filled!" << " Prefetch_addr: " << hex << PQ.entry[index].address;
								// cout << " victim_addr: " << block[set][way].tag << dec << endl;
								 });
								}
							else
							{
								PQ.entry[index].forL2 =1;
								this_router->NI[direction].add_outq(&PQ.entry[index]);  
							}    
						}	
					}
					else //For other caches, no changes
					{
						if (PQ.entry[index].instruction) 
							upper_level_icache[prefetch_cpu]->return_data(&PQ.entry[index]);
						else // data
							upper_level_dcache[prefetch_cpu]->return_data(&PQ.entry[index]);
					}
				}

				HIT[PQ.entry[index].type]++;
				ACCESS[PQ.entry[index].type]++;
				if(cache_type == IS_LLC &&  all_warmup_complete > NUM_CPUS )
				{
				 	total_waiting_time_in_pq[prefetch_cpu] += ( current_core_cycle[prefetch_cpu] - PQ.entry[index].cycle_enqueued);	
					total_prefetch_packets[PQ.entry[index].cpu]++;
				}
				 // remove this entry from PQ
				PQ.remove_queue(&PQ.entry[index]);
				reads_available_this_cycle--;
				#if !(remap_on_evictions)
        	                        check_llc_access(); 
	                        #endif

			}
			else 
			{ // prefetch miss
				++pf_lower_level;
				DP ( if (warmup_complete[prefetch_cpu]) {
				cout << "[" << NAME << "] " << __func__ << " prefetch miss";
				cout << " instr_id: " << PQ.entry[index].instr_id << " address: " << hex << PQ.entry[index].address;
				cout << " full_addr: " << PQ.entry[index].full_addr << dec << " fill_level: " << PQ.entry[index].fill_level;
				cout << " cycle: " << PQ.entry[index].event_cycle << endl; });

				// check mshr
				uint8_t miss_handled = 1;
				int mshr_index = check_mshr(&PQ.entry[index]);

				if ((mshr_index == -1) && (MSHR.occupancy < MSHR_SIZE)) 
				{ // this is a new miss

					DP ( if (warmup_complete[PQ.entry[index].cpu]) 
					{
						cout << "[" << NAME << "_PQ] " <<  __func__ << " want to add instr_id: " << PQ.entry[index].instr_id << " address: " << hex << PQ.entry[index].address;
						cout << " full_addr: " << PQ.entry[index].full_addr << dec;
						if (cache_type == IS_L2C)
						{
							cout << " occupancy: " << this_router->LLC_MAP[get_slice_num(PQ.entry[index].address)]->get_occupancy(3, PQ.entry[index].address) << " SIZE: " << this_router->LLC_MAP[get_slice_num(PQ.entry[index].address)]->get_size(3, PQ.entry[index].address) << endl;						
						}
						else
						{
							cout << " occupancy: " << lower_level->get_occupancy(3, PQ.entry[index].address) << " SIZE: " << lower_level->get_size(3, PQ.entry[index].address) << endl; 
						}
					});

					if (INTERCONNECT_ON && cache_type==IS_L2C && PQ.entry[index].fill_level <= fill_level )
					{
						int destination_slice = get_slice_num(PQ.entry[index].address); 
						int direction = get_direction(prefetch_cpu,destination_slice);
						if (destination_slice != prefetch_cpu && this_router->NI[direction].OUTQ.occupancy == this_router->NI[direction].OUTQ.SIZE)
						{
							cout << "[" << NAME << "] " << __func__ ;
							cout << " Router queue filled!" << "pref_addr: " << hex << PQ.entry[index].address;
							//No need to stall 
							// STALL[PQ.entry[index].type]++;
							DP ( if (warmup_complete[prefetch_cpu]) {
							cout << "[" << NAME << "] " << __func__ ;
							cout << " Router queue filled!" << "pref_addr: " << hex << PQ.entry[index].address;});
							return;							
						}
					}	

					
					// this is possible since multiple prefetchers can exist at each level of caches
					if (cache_type == IS_L2C)
					{
						if (INTERCONNECT_ON == 0 || get_slice_num(PQ.entry[index].address) == prefetch_cpu )
						{
							int destination = get_slice_num(PQ.entry[index].address);
							// first check if the lower level PQ is full or not
							if (this_router->LLC_MAP[destination]->get_occupancy(3, PQ.entry[index].address) == this_router->LLC_MAP[destination]->get_size(3, PQ.entry[index].address))
								miss_handled = 0;
							else 
							{
								// run prefetcher on prefetches from higher caches
								if(PQ.entry[index].pf_origin_level < fill_level)
							    {
							    	PQ.entry[index].pf_metadata = l2c_prefetcher_operate(PQ.entry[index].address<<LOG2_BLOCK_SIZE, PQ.entry[index].ip, 0, PREFETCH, PQ.entry[index].pf_metadata);
							    }
							    // add it to MSHRs if this prefetch miss will be filled to this cache level
							  	if (PQ.entry[index].fill_level <= fill_level)
							    	add_mshr(&PQ.entry[index]);

							  	this_router->LLC_MAP[get_slice_num(PQ.entry[index].address)]->add_pq(&PQ.entry[index]); // add it to the lower_level PQ								}
							}
						}
						else
						{
							int destination = get_slice_num(PQ.entry[index].address);
							int direction = get_direction(prefetch_cpu,destination);
							if (this_router->NI[direction].OUTQ.occupancy == this_router->NI[direction].OUTQ.SIZE)
							{
								miss_handled=0;
								cout << "[" << NAME << "] " << __func__ ;
								cout << " Router queue filled!" << "pref_addr: " << hex << PQ.entry[index].address;								
							}
							else//Packet need to go via network
							{	
								// run prefetcher on prefetches from higher caches
								if(PQ.entry[index].pf_origin_level < fill_level)
							    {
							    	PQ.entry[index].pf_metadata = l2c_prefetcher_operate(PQ.entry[index].address<<LOG2_BLOCK_SIZE, PQ.entry[index].ip, 0, PREFETCH, PQ.entry[index].pf_metadata);
							    }
							    // add it to MSHRs if this prefetch miss will be filled to this cache level
							  	if (PQ.entry[index].fill_level <= fill_level)
							    	add_mshr(&PQ.entry[index]);

								PQ.entry[index].forL2=0;
								this_router->NI[direction].add_outq(&PQ.entry[index]);
							}

						}

					}		
					else if (lower_level) 
					{
						if (cache_type == IS_LLC) 
						{
							if (lower_level->get_occupancy(1, PQ.entry[index].address) == lower_level->get_size(1, PQ.entry[index].address))
								miss_handled = 0;
							else 
							{
								 
											
								// run prefetcher on prefetches from higher caches
									if(PQ.entry[index].pf_origin_level < fill_level) 
										{
											if (cache_type == IS_LLC) 
												{
													//Redundant check
													cpu = prefetch_cpu;
													PQ.entry[index].pf_metadata = llc_prefetcher_operate(PQ.entry[index].address<<LOG2_BLOCK_SIZE, PQ.entry[index].ip, 0, PREFETCH, PQ.entry[index].pf_metadata);
													cpu = 0;
												}
										}
									// add it to MSHRs if this prefetch miss will be filled to this cache level
									if (PQ.entry[index].fill_level <= fill_level)
										add_mshr(&PQ.entry[index]);
									lower_level->add_rq(&PQ.entry[index]); // add it to the DRAM RQ
								
							}
			  			}
   					    else 
   					    {
   					    	if (lower_level->get_occupancy(3, PQ.entry[index].address) == lower_level->get_size(3, PQ.entry[index].address))
   					    		miss_handled = 0;
							else
							{
								// run prefetcher on prefetches from higher caches
								if(PQ.entry[index].pf_origin_level < fill_level)
								{
									if (cache_type == IS_L1D)
										l1d_prefetcher_operate(PQ.entry[index].full_addr, PQ.entry[index].ip, 0, PREFETCH);
									if (cache_type == IS_L2C)//Dead code
										PQ.entry[index].pf_metadata = l2c_prefetcher_operate(PQ.entry[index].address<<LOG2_BLOCK_SIZE, PQ.entry[index].ip, 0, PREFETCH, PQ.entry[index].pf_metadata);
								}

								// add it to MSHRs if this prefetch miss will be filled to this cache level
								if (PQ.entry[index].fill_level <= fill_level)
							  		add_mshr(&PQ.entry[index]);
								lower_level->add_pq(&PQ.entry[index]); 		
							}
						}
					}
				}
				else {
					if ((mshr_index == -1) && (MSHR.occupancy == MSHR_SIZE)) { // not enough MSHR resource

						// TODO: should we allow prefetching with lower fill level at this case?
						
						// cannot handle miss request until one of MSHRs is available
						miss_handled = 0;
						STALL[PQ.entry[index].type]++;
					}
					else if (mshr_index != -1) { // already in-flight miss

						// no need to update request except fill_level
						// update fill_level
						if (PQ.entry[index].fill_level < MSHR.entry[mshr_index].fill_level)
							MSHR.entry[mshr_index].fill_level = PQ.entry[index].fill_level;
						
						//Late Prefetch
						if(MSHR.entry[mshr_index].type != PREFETCH)
							++pf_late; 
						MSHR_MERGED[PQ.entry[index].type]++;

						DP ( if (warmup_complete[prefetch_cpu]) {
						cout << "[" << NAME << "] " << __func__ << " mshr merged";
						cout << " instr_id: " << PQ.entry[index].instr_id << " prior_id: " << MSHR.entry[mshr_index].instr_id; 
						cout << " address: " << hex << PQ.entry[index].address;
						cout << " full_addr: " << PQ.entry[index].full_addr << dec << " fill_level: " << MSHR.entry[mshr_index].fill_level;
						cout << " cycle: " << MSHR.entry[mshr_index].event_cycle << endl; });
					}
					else { // WE SHOULD NOT REACH HERE
						cerr << "[" << NAME << "] MSHR errors" << endl;
						assert(0);
					}
				}

				if (miss_handled) {

					DP ( if (warmup_complete[prefetch_cpu]) {
					cout << "[" << NAME << "] " << __func__ << " prefetch miss handled";
					cout << " instr_id: " << PQ.entry[index].instr_id << " address: " << hex << PQ.entry[index].address;
					cout << " full_addr: " << PQ.entry[index].full_addr << dec << " fill_level: " << PQ.entry[index].fill_level;
					cout << " cycle: " << PQ.entry[index].event_cycle << endl; });

					MISS[PQ.entry[index].type]++;
					ACCESS[PQ.entry[index].type]++;
					PQ.entry[index].cycle_enqueued =  cycle_enqued_at_head;
					if(cache_type == IS_LLC &&  all_warmup_complete > NUM_CPUS)
					{
						total_waiting_time_in_pq[prefetch_cpu] += ( current_core_cycle[prefetch_cpu] - PQ.entry[index].cycle_enqueued);
						total_prefetch_packets[PQ.entry[index].cpu]++;
					}
					 PQ.remove_queue(&PQ.entry[index]);
			reads_available_this_cycle--;
				}
			}
		}
	else
	  {
		return;
	  }

	if(reads_available_this_cycle == 0)
	  {
		return;
	  }
	}
}

int CACHE::check_llc_stall() //Checks whether the LLC is stalled 
{
	if(cache_type == IS_ITLB || cache_type == IS_DTLB || cache_type == IS_STLB)
        	return 1;
	if( 	(cache_type == IS_L1I) || ( cache_type == IS_L1D) || (cache_type == IS_L2C) || (CEASER_S_LLC == 0 && cache_type == IS_LLC) )
        	return 1;
        assert(cache_type == IS_LLC);
      	if(cache_stall_cycle >0 || is_remap_complete == 0)
	{	
              	return 0;
	}
        return 1;
}
void CACHE::continue_remap(){ //It continues remapping if remapping is stopped due to WQ occupancy 
	if (is_remap_complete ==0 && CEASER_S_LLC ==1 && cache_type == IS_LLC)
	{
	        remap_set_ceaser_s();
		return;
	}
	assert(0);
   
}
void CACHE::add_stall_cycle_to_queues()
{ //Remapping stalls the LLC requests till it is finished
	//newly_added
        assert(cache_type == IS_LLC);
	for(int index=0;index<RQ.SIZE;index++)
       	{
		if(RQ.entry[index].address != 0 && RQ.entry[index].event_cycle < UINT64_MAX)
			RQ.entry[index].event_cycle = UINT64_MAX;
       	}
	for(int index=0;index<WQ.SIZE;index++)
       	{
		if(WQ.entry[index].address != 0 && WQ.entry[index].event_cycle < UINT64_MAX)
                        WQ.entry[index].event_cycle = UINT64_MAX;
      	} 
	for(int index=0;index<PQ.SIZE;index++)
       	{
		if(PQ.entry[index].address != 0 && PQ.entry[index].event_cycle < UINT64_MAX)
                        PQ.entry[index].event_cycle = UINT64_MAX;
      	}
	for(int index=0;index<MSHR.SIZE;index++)
       	{
                if(MSHR.entry[index].address != 0 && MSHR.entry[index].event_cycle < UINT64_MAX && MSHR.entry[index].returned == COMPLETED)
                        MSHR.entry[index].event_cycle = UINT64_MAX;
       	}
		update_fill_cycle();
	
}

void CACHE::check_encryption_engine_for_queue_entries() //Gives encryption engine to the LLC request and response packets
	/* Priority for giving encryption engine is given based on the variable turn 	If turn is even: RQ, PQ, MSHR, WQ
	 										If turn is odd : MSHR, RQ, PQ, WQ   	
											If turn value is 21(CEASER_rq_wq_ratio) : WQ, RQ, PQ, MSHR (The value is found emperically)
							*/
{
   	int turn=llc_queue_turn,is_encryption_engine_busy=0;
   	if(encryption_stall_cycle != 0 || cache_stall_cycle != 0 ) 
	     assert(0);
   	if((cache_type == IS_LLC ) && (CEASER_S_LLC == 1) && all_warmup_complete > NUM_CPUS )
   	{  
      		for(int x=0; x<2 && is_encryption_engine_busy == 0;x++)
		{ //If 1st iteration don't have any entry in the queue which is supposed to use the encryption engine then in 2nd iteration it allows other queues to use the encryption engine

		//Request queue
			if((turn < CEASER_rq_wq_ratio && (turn%2) == 0 && is_encryption_engine_busy == 0 ) || x==1) //x=1 signifies 2nd iteration
			{	//RQ
		 		if(RQ.occupancy > 0)
				{
        	              		int index = RQ.first_element_which_need_encryption_engine();
        	                	if(index != -1)
					{
						#if Pipelined_Encryption_Engine 
                               	  			encryption_stall_cycle += 1;
							total_encryption_time_rq[RQ.entry[index].cpu] += (CEASER_LATENCY);
						#else 
							encryption_stall_cycle += CEASER_LATENCY;
							total_encryption_time_rq[RQ.entry[index].cpu] += (CEASER_LATENCY);
						#endif
						RQ.entry[index].event_cycle = current_core_cycle[RQ.entry[index].cpu] + LATENCY;
                           			total_access_time_rq[RQ.entry[index].cpu]+= LATENCY;		
						is_encryption_engine_busy=1;
						break;
                                	}

                     		}	
				//PQ
		   		if(PQ.occupancy > 0  && is_encryption_engine_busy == 0)
				{
          	      			int index = PQ.first_element_which_need_encryption_engine();
           				if(index != -1)
					{
						#if Pipelined_Encryption_Engine
                                			encryption_stall_cycle += 1;
                                        		total_encryption_time_rq[RQ.entry[index].cpu] += 1;
                               	 		#else
                                        		encryption_stall_cycle += CEASER_LATENCY;
                                        		total_encryption_time_rq[RQ.entry[index].cpu] += (CEASER_LATENCY);
                                		#endif
                				PQ.entry[index].event_cycle = current_core_cycle[PQ.entry[index].cpu] + LATENCY;
                				is_encryption_engine_busy=1;
						total_access_time_pq[PQ.entry[index].cpu]+= (LATENCY);
               					break;
                			}
        	   		}

          		}
	 		//WQ 
	       		if(WQ.occupancy > 0 && (turn == CEASER_rq_wq_ratio || x==1) && is_encryption_engine_busy == 0 )
			{
               	 		int index = WQ.first_element_which_need_encryption_engine();
                    		if(index != -1)
				{
					#if Pipelined_Encryption_Engine 
                                 		encryption_stall_cycle += 1;
                                        	total_encryption_time_rq[RQ.entry[index].cpu] += 1;
                                	#else
                                    		encryption_stall_cycle += CEASER_LATENCY;
                                     		total_encryption_time_rq[RQ.entry[index].cpu] += (CEASER_LATENCY);
                                	#endif
                        		WQ.entry[index].event_cycle = current_core_cycle[WQ.entry[index].cpu] + LATENCY;
                        		is_encryption_engine_busy=1;

                        		break;
                        	}

               		}
     //Request Queue ends
     //Response Queue
	  //MSHR
	   	if( ( (is_encryption_engine_busy == 0 && turn<CEASER_rq_wq_ratio) || x==1) && MSHR.occupancy > 0 )
	   	{
			uint32_t min_index = MSHR.SIZE;
                	uint64_t min_enqued_cycle = UINT64_MAX;
           		for (uint32_t index=0; index<MSHR_SIZE; index++)
			{
                		if(MSHR.entry[index].address != 0 && min_enqued_cycle  > MSHR.entry[index].cycle_enqueued && (MSHR.entry[index].returned == COMPLETED) && MSHR.entry[index].event_cycle == UINT64_MAX )
                		{ 
			       		min_enqued_cycle = MSHR.entry[index].cycle_enqueued;
                   			min_index = index;
                		}
			}
                	if(min_index != MSHR.SIZE)
			{
				#if Pipelined_Encryption_Engine 
                               		encryption_stall_cycle += 1;
                        	#else
                                	encryption_stall_cycle += CEASER_LATENCY;
                        	#endif
				total_access_time_mshr[MSHR.entry[min_index].cpu]+= (LATENCY);
                        	MSHR.entry[min_index].event_cycle = current_core_cycle[MSHR.entry[min_index].cpu] + LATENCY;
				is_encryption_engine_busy=1;
				update_fill_cycle();
				break;
                	}
		}//MSHR Ends
	   //Response queue implementation end.

    	}//x for loop end
        llc_queue_turn = ((llc_queue_turn + 1 ) % (CEASER_rq_wq_ratio +1) );
   }
}
void CACHE::operate()
{
	/*if(cache_type ==IS_LLC) 
	{
		for(int i=0;i<NUM_SET;i++)
                for(int j=0;j<NUM_WAY;j++)
                {
                        if(block[i][j].valid == 1 )
                        {
                                if(block[i][j].tag_number == 0 && block[i][j].tag != tag0[block[i][j].tag_set][block[i][j].tag_way].tag)
                                {
                                        cout<<"Set : "<<i<<" way :"<<j<<" tag : "<<block[i][j].tag<<endl;
                                        cout<<"tag_num : "<<block[i][j].tag_number<<"tag0[block[i][j].tag_set][block[i][j].tag_way].tag "<<tag0[block[i][j].tag_set][block[i][j].tag_way].tag<<endl;
                                        assert(0);
                                }
                                if(block[i][j].tag_number == 1 && block[i][j].tag != tag1[block[i][j].tag_set][block[i][j].tag_way].tag)
                                {
                                        cout<<"tag_num : "<<block[i][j].tag_number<<"tag1[block[i][j].tag_set][block[i][j].tag_way].tag "<<tag1[block[i][j].tag_set][block[i][j].tag_way].tag<<endl;
                                        cout<<"Set : "<<i<<" way :"<<j<<endl;
                                        assert(0);
                                }
                        }
                }

		for(int i=0;i<NUM_SET;i++)
		{
			for(int j=0;j<extra_tag_ways;j++)
			{
				if(tag0[i][j].valid == 1 && tag0[i][j].tag != block[tag0[i][j].set][tag0[i][j].way].tag)
				{
					cout<<"tag_num : 0 "<<"tag0[i][j].set"<<tag0[i][j].set<<endl;
					assert(0);
				
				}
				if(tag1[i][j].valid == 1 && tag1[i][j].tag != block[tag1[i][j].set][tag1[i][j].way].tag)
                                        assert(0);

			}
		}

	} */
	if ( is_remap_complete ==0 && (CEASER_S_LLC == 1) && cache_type == IS_LLC && cache_stall_cycle == 0)//Continue remappin if remap is not completed due to write_queue occupancy
         	continue_remap();
//newly_added
      	if((cache_type == IS_LLC ) && (CEASER_S_LLC == 1) && all_warmup_complete > NUM_CPUS && encryption_stall_cycle == 0 && cache_stall_cycle == 0) 
      	{
		check_encryption_engine_for_queue_entries();
      	}
    	if(check_llc_stall())
		handle_fill();
    	if(check_llc_stall())
		handle_writeback();
		reads_available_this_cycle = MAX_READ;
    	if(check_llc_stall())
		handle_read();
	if (PQ.occupancy && (reads_available_this_cycle > 0)  &&  check_llc_stall())
		 handle_prefetch();

}
uint32_t CACHE::random_set()
{
	return (rand()%NUM_SET);
}
uint32_t CACHE::random_way()
{
        return (rand()%NUM_WAY);
}
int CACHE::remove_block_based_on_tag_array(int tag_number,int tag_set_number,int tag_way)
{
	cout << "SAE" << endl;
	//cout<<"Remove tag from tag array corresponding to tag_number "<<tag_number<<" tag_set : "<<tag_set_number<<" and tag_way : "<<tag_way<<endl;
	uint32_t block_set;
	int block_way,tag_valid=0;

	if(tag_number == 0)
	{
		block_set = tag0[tag_set_number][tag_way].set;
		block_way = tag0[tag_set_number][tag_way].way;
		tag_valid = tag0[tag_set_number][tag_way].valid;
	}
	else if(tag_number == 1 )
	{
		block_set = tag1[tag_set_number][tag_way].set;
                block_way = tag1[tag_set_number][tag_way].way;
		tag_valid = tag1[tag_set_number][tag_way].valid;
	}
	if(block[block_set][block_way].valid != 1 || tag_valid != 1)
       		assert(0);	
	if(block[block_set][block_way].dirty == 1 )
	{//add in lower_level WQ
		PACKET writeback_packet;
         	writeback_packet.fill_level = FILL_DRAM;
          	writeback_packet.cpu = block[block_set][block_way].cpu;
            	writeback_packet.address = block[block_set][block_way].tag;
          	writeback_packet.full_addr = block[block_set][block_way].full_addr;
         	writeback_packet.data = block[block_set][block_way].data;
        	writeback_packet.instr_id = block[block_set][block_way].instr_id;
           	writeback_packet.ip = 0; // writeback does not have ip
             	writeback_packet.type = WRITEBACK;
           	writeback_packet.event_cycle = current_core_cycle[block[block_set][block_way].cpu];
            	int channel = uncore.DRAM.dram_get_channel(block[block_set][block_way].tag);
           	if(uncore.DRAM.WQ[channel].occupancy == uncore.DRAM.WQ[channel].SIZE) //If WQ of DRAM is full then remap will stop and it starts again when the WQ have occupancy
                                                         return 0;
             	lower_level->add_wq(&writeback_packet);

	}
	block[block_set][block_way].valid = 0;
	block[block_set][block_way].tag=0;
	block[block_set][block_way].tag_number=0;
	block[block_set][block_way].tag_set=0;
	if(tag_number == 0)
	{
		tag0[tag_set_number][tag_way].valid = 0;
		tag0[tag_set_number][tag_way].set=-1;
	}
	else
	{
		tag1[tag_set_number][tag_way].valid = 0;
                tag1[tag_set_number][tag_way].set=-1;
	}

}
void CACHE::remove_tag_entry_from_block(uint32_t set,int way) //(block[set][way].tag_number,block[set][way].tag_set,block[set][way].tag_way,block[set][way].tag)
{
	//cout<<"Remove block corresponding to set : "<<set<<" and way : "<<way<<endl;
	if(block[set][way].tag_number != 0 && block[set][way].tag_number != 1)
		assert(0);
	int tag_num=block[set][way].tag_number, tag_set = block[set][way].tag_set,tag_way=block[set][way].tag_way;
	if((tag_num == 0 && tag0[tag_set][tag_way].tag != block[set][way].tag) || (tag_num == 1 && tag1[tag_set][tag_way].tag != block[set][way].tag) )
		assert(0);

	if(tag_num == 0)
	{
		if(tag0[tag_set][tag_way].valid == 0)
			assert(0);
		tag0[tag_set][tag_way].tag = 0;
		tag0[tag_set][tag_way].valid = 0;
	}
	else if(tag_num == 1)
	{
		if(tag1[tag_set][tag_way].valid == 0)
			assert(0);
		tag1[tag_set][tag_way].tag = 0;
		tag1[tag_set][tag_way].valid = 0;
	}
	//cout<<"Remove tag whose tag_number : "<<tag_num<<" tag_set : "<<tag_set<<" tag_way : "<<tag_way<<endl;
	
}
int CACHE::llc_find_victim_tag(uint32_t tag0_set,uint32_t tag1_set,int* tag_way) //returns tag_number and set tag_way
{
	int count1=0,count2=0;
        int tag=-1;
        for(int way=0;way<extra_tag_ways;way++)
        {
                if(tag0[tag0_set][way].valid == 0)
                        count1++;
                if(tag1[tag1_set][way].valid == 0)
                        count2++;
        }
	//cout<<"Tag Table 0 invalid block count : "<<count1<<" Table 1 invalid block count "<<count2<<endl;
// Load Aware
        if(count1 > count2)
        {
                tag=0;
        }
        else if(count2 > count1)
        {
                tag=1;
        }
        else if(count1 == count2)
        {
                tag=rand()%2;
		if(count1 ==  0)
		{
			 //cout<<"-------------- Returned tag from llc_find_victim_tag : "<<tag;
			*tag_way=random_way();
			return tag;
		}
		
        }
//Load Aware ends
        int flag=0;
        if(tag == 0)
        {
                for(int way=0;way<extra_tag_ways;way++)
                {
                        if(tag0[tag0_set][way].valid == 0)
                        {
                                *tag_way=way;
				flag=1;
				//cout<<" Returned tag from llc_find_victim_tag : "<<tag; 
                                return tag;
                        }
		}
        }
	else if(tag == 1)
	{
                for(int way=0;way<extra_tag_ways;way++)
                {
                        if(tag1[tag1_set][way].valid == 0)
                        {
                                *tag_way=way;
				flag=1;
				 //cout<<" Returned tag from llc_find_victim_tag : "<<tag;
                                return tag;
                        }
		}
        }
	assert(0);


}
void CACHE::fill_tag(int tag_number,int tag_set,int tag_way,uint64_t tag,uint32_t block_set,int block_way)
{
//	cout<<"Fill Tag of tag_number :" <<tag_number<<" tag_set :"<<tag_set<<" tag_way : " <<tag_way<<" tag : "<<tag<<endl;
	//cout<<"Fill Tag to block block_set : "<<block_set<<" block_way : "<< block_way<<endl;
	/*for(int i=0;i<NUM_SET;i++)
		for(int j=0;j<NUM_WAY;j++)
		{
			if(block[i][j].valid == 1 && i!= block_set && j != block_way)
			{
				if(block[i][j].tag_number == 0 && block[i][j].tag != tag0[block[i][j].tag_set][block[i][j].tag_way].tag)
				{
					cout<<"Set : "<<i<<" way :"<<j<<" tag : "<<block[i][j].tag<<endl;
					cout<<"tag_num : "<<block[i][j].tag_number<<"tag0[block[i][j].tag_set][block[i][j].tag_way].tag "<<tag0[block[i][j].tag_set][block[i][j].tag_way].tag<<endl;
					assert(0);
				}
				if(block[i][j].tag_number == 1 && block[i][j].tag != tag1[block[i][j].tag_set][block[i][j].tag_way].tag)
                        	{
					cout<<"tag_num : "<<block[i][j].tag_number<<"tag1[block[i][j].tag_set][block[i][j].tag_way].tag "<<tag1[block[i][j].tag_set][block[i][j].tag_way].tag<<endl;
					cout<<"Set : "<<i<<" way :"<<j<<endl;
                               	 	assert(0);
                        	}
			}
		}*/
	if(tag_number == 0 )
	{
		tag0[tag_set][tag_way].tag = tag;
		tag0[tag_set][tag_way].valid = 1;
		tag0[tag_set][tag_way].set = block_set;
		tag0[tag_set][tag_way].way = block_way;
		block[block_set][block_way].tag_number = tag_number;
		block[block_set][block_way].tag_set = tag_set;
		block[block_set][block_way].tag_way = tag_way;
		return;
	}
	else if(tag_number == 1 )
	{
		tag1[tag_set][tag_way].tag = tag;
		tag1[tag_set][tag_way].valid = 1;
		tag1[tag_set][tag_way].set = block_set;
		tag1[tag_set][tag_way].way = block_way;
		block[block_set][block_way].tag_number = tag_number;
                block[block_set][block_way].tag_set = tag_set;
                block[block_set][block_way].tag_way = tag_way;
		return;
	}
	assert(0);

}
int CACHE::check_hit_tag(uint32_t tag0_set,uint32_t tag1_set,int* tag_way,uint64_t tag) // return tag_number and set tag_way
{
	if(cache_type == IS_LLC && MIRAGE == 1){
	//get_tag_set(address);
        for(int way=0;way<extra_tag_ways;way++)
        {
                if(tag0[tag0_set][way].tag == tag && tag0[tag0_set][way].valid == 1)
                {
			//cout<<"Tag is matched in Tag : 0 Tag_Set : "<<tag0_set<<" Tag_way : "<<way<<" Tag = "<<tag<<" tag0[tag0_set][way].tag : "<<tag0[tag0_set][way].tag<<endl;
                        *tag_way = way;
                        return 0;
                }
                if(tag1[tag1_set][way].tag == tag && tag1[tag1_set][way].valid == 1)
                {
			//cout<<"Tag is matched in Tag : 1 Tag_Set : "<<tag1_set<<" Tag_way : "<<way<<" Tag = "<<tag<<" tag1[tag1_set][way].tag : "<<tag1[tag1_set][way].tag<<endl;
                        *tag_way =way;
                        return 1;
                }

        }
        return -1;
	}
	else
		assert(0);
}
uint32_t CACHE::get_tag_set(uint64_t address)
{// Gives set number of the block in each tag array table 

        uint32_t llcset1,llcset2,llcset,llc_l_set,llc_r_set;
        #ifdef No_Randomization   
                for(int part=0;part<partitions;part++){
                        llcset1 = ((uint32_t) (address & ((1 << lg2(NUM_SET*NUM_SLICES)) - 1)))>>lg2(NUM_SLICES);
                        llcset2 = ((uint32_t) (address & ((1 << lg2(NUM_SET*NUM_SLICES)) - 1)))>>lg2(NUM_SLICES);
                        tag0_set=llcset1;
                        tag1_set=llcset2;}
                        return ((uint32_t) (address & ((1 << lg2(NUM_SET*NUM_SLICES)) - 1)))>>lg2(NUM_SLICES);

        #endif
        uint64_t addr1,addr2;
        uint8_t *cur_k,*next_k;
        int encrypt_cost=0,part=0;
                cur_k=curr_keys[part];
                next_k=next_keys[part];
                addr1 = getEncryptedAddress(address,cpu,cur_k,encrypt_cost);
                addr2 = getEncryptedAddress(address,cpu,next_k,encrypt_cost);
                llcset1 = ((uint32_t) (addr1 & ((1 << lg2(NUM_SET*NUM_SLICES)) - 1)))>>lg2(NUM_SLICES);
                llcset2 = ((uint32_t) (addr2 & ((1 << lg2(NUM_SET*NUM_SLICES)) - 1)))>>lg2(NUM_SLICES);
                tag0_set=llcset1;
                tag1_set=llcset2;
		//cout<<"get_tag_set -> tag0_set : "<<tag0_set<<" tag1_set "<<tag1_set<<endl; 
         return llcset1;  //Return arbitary set. ceaser_s_set[],ceaser_s_next_set[] are used to get the set numbers.
}

uint32_t CACHE::get_ceaser_s_set(uint64_t address)
{// Gives set number of the block in each partition with current and next key

	uint32_t llcset1,llcset2,llcset,llc_l_set,llc_r_set;
       	#ifdef No_Randomization   
		for(int part=0;part<partitions;part++){
			llcset1 = ((uint32_t) (address & ((1 << lg2(NUM_SET*NUM_SLICES)) - 1)))>>lg2(NUM_SLICES);
                	llcset2 = ((uint32_t) (address & ((1 << lg2(NUM_SET*NUM_SLICES)) - 1)))>>lg2(NUM_SLICES);
                	ceaser_s_set[part]=llcset1;
                	ceaser_s_next_set[part]=llcset2;}
			return ((uint32_t) (address & ((1 << lg2(NUM_SET*NUM_SLICES)) - 1)))>>lg2(NUM_SLICES);
		
       	#endif
       	uint64_t addr1,addr2;
	uint8_t *cur_k,*next_k;
      	int encrypt_cost=0;
       	for(int part=0;part<partitions;part++)
	{
  		cur_k=curr_keys[part];
             	next_k=next_keys[part];
         	addr1 = getEncryptedAddress(address,cpu,cur_k,encrypt_cost);
         	addr2 = getEncryptedAddress(address,cpu,next_k,encrypt_cost); 
               	llcset1 = ((uint32_t) (addr1 & ((1 << lg2(NUM_SET*NUM_SLICES)) - 1)))>>lg2(NUM_SLICES);
         	llcset2 = ((uint32_t) (addr2 & ((1 << lg2(NUM_SET*NUM_SLICES)) - 1)))>>lg2(NUM_SLICES);
                ceaser_s_set[part]=llcset1;
                ceaser_s_next_set[part]=llcset2;
     	}
         return llcset1;  //Return arbitary set. ceaser_s_set[],ceaser_s_next_set[] are used to get the set numbers.
}
uint32_t CACHE::get_set(uint64_t address)
{
 
	if(CEASER_S_LLC == 1 && cache_type == IS_LLC)
           	return get_ceaser_s_set(address);  
	if (cache_type == IS_LLC) //Since LLC is sliced, thus we need to return set number in that slice.
	{
		//[First find the set number assuming single LLC, then remove lg2(NUM_SLICES) bits to get set number in particular slice]
		//return get_ceaser_set(address);
		return ((uint32_t) (address & ((1 << lg2(NUM_SET*NUM_SLICES)) - 1)))>>lg2(NUM_SLICES); 
	}
	return (uint32_t) (address & ((1 << lg2(NUM_SET)) - 1)); 
}

uint32_t CACHE::get_way(uint64_t address, uint32_t set)
{
	for (uint32_t way=0; way<NUM_WAY; way++) {
		if (block[set][way].valid && (block[set][way].tag == address)) 
			return way;
	}

	return NUM_WAY;
}

void CACHE::fill_cache(uint32_t set, uint32_t way, PACKET *packet)
{
//if(cache_type == IS_LLC){
//int x = llc_find_victim(packet->cpu, packet->instr_id, set, block[set], packet->ip, packet->full_addr, packet->type);
//cout<<"X :"<<x<<endl;}
#ifdef SANITY_CHECK
	if (cache_type == IS_ITLB) {
		if (packet->data == 0)
			assert(0);
	}

	if (cache_type == IS_DTLB) {
		if (packet->data == 0)
			assert(0);
	}

	if (cache_type == IS_STLB) {
		if (packet->data == 0)
			assert(0);
	}
#endif
	if (block[set][way].prefetch && (block[set][way].used == 0))
		pf_useless++;

	if (block[set][way].valid == 0)
		block[set][way].valid = 1;
	block[set][way].dirty = 0;
	block[set][way].prefetch = (packet->type == PREFETCH) ? 1 : 0;
	block[set][way].used = 0;

	if (block[set][way].prefetch)
		pf_fill++;

	block[set][way].delta = packet->delta;
	block[set][way].depth = packet->depth;
	block[set][way].signature = packet->signature;
	block[set][way].confidence = packet->confidence;

	block[set][way].tag = packet->address;
	set_key(set,way);
	block[set][way].address = packet->address;
	block[set][way].full_addr = packet->full_addr;
	block[set][way].data = packet->data;
	block[set][way].cpu = packet->cpu;
	block[set][way].instr_id = packet->instr_id;
	block[set][way].ip = packet->ip; 
	if(cache_type == IS_LLC && (CEASER_S_LLC == 1))
             block[set][way].curr_or_next_key = c_or_n;//which key is used cur_key or next_key

	//if(cache_type == IS_LLC && MIRAGE == 1)
	//	cout<<"block[set][way].tag_number :" <<block[set][way].tag_number<<"block[set][way].tag_set : "<<block[set][way].tag_set<<"block[set][way].tag_way : "<<block[set][way].tag_way<<" block[set][way].tag : "<<block[set][way].tag<<endl;
	if( (block[set][way].tag_number == 0 && tag0[block[set][way].tag_set][block[set][way].tag_way].tag != block[set][way].tag) || (block[set][way].tag_number == 1 && tag1[block[set][way].tag_set][block[set][way].tag_way].tag != block[set][way].tag))
			assert(0);

	DP ( if (warmup_complete[packet->cpu]) {
	cout << "[" << NAME << "] " << __func__ << " set: " << set << " way: " << way;
	cout << " lru: " << block[set][way].lru << " tag: " << hex << block[set][way].tag << " full_addr: " << block[set][way].full_addr;
	cout << " data: " << block[set][way].data << dec << endl; });
}
int CACHE::check_hit_for_remap_on_evictions(PACKET *packet, uint32_t * current_set)
{

	int flag2 = 0;

        /*if (cache_type == IS_LLC)
        {      
                assert(this_router->id == get_slice_num(packet->address, packet->cpu));
        }*/
        //uint32_t set = get_set(packet->address);
        
	int match_way = -1;
	uint32_t set1,set2;
        int flag=0;
        if(CEASER_S_LLC != 1 || cache_type != IS_LLC)
                partitions=1;
      	if((cache_type != IS_LLC || (cache_type == IS_LLC && CEASER_S_LLC !=1)) && partitions != 1)
       	{
                  cout<<"Partitions "<<partitions<<"\n"<<endl;
                  assert(0);
       	}
      	for(int cur_part=0;cur_part<partitions;cur_part++)
      	{
        	for(uint32_t way=(cur_part * (NUM_WAY/partitions)); way<NUM_WAY && (way < ((cur_part+1) * (NUM_WAY/partitions))); way++)
                {
                    if(CEASER_S_LLC == 1 && cache_type == IS_LLC)
                    {
                        if (ceaser_s_set[cur_part] >= Sptr)
			{
				set1=ceaser_s_set[cur_part];
				set2=ceaser_s_next_set[cur_part];
				if (block[set1][way].valid && (block[set1][way].tag == packet->address) )
                     		{
                            		flag2 = 1;
                            		*current_set = set1;
			//		cout << *current_set << " inside if" << endl;
                     		}
				else if (block[set2][way].valid && (block[set2][way].tag == packet->address) )
                                {
                                        flag2 = 1;
                                        *current_set = set2;
			//		cout << *current_set << " inside elseif" << endl;

                                }

			}
                        else
			{
				set2 =ceaser_s_next_set[cur_part];
				if (block[set2][way].valid && (block[set2][way].tag == packet->address))
                     		{
                             		flag2 = 1;
                             		*current_set = set2;
			//		cout << *current_set << " inside else" << endl;

                     		}
			}
                     }
		    if(flag2 != 0)
                    {
                        match_way = way;
                        flag=1;
                        DP ( if (warmup_complete[packet->cpu]) {
                        if(packet->type==PREFETCH)
                                cout<<"PQ ";
                        else
                                cout<<"RQ ";
                        cout << "[" << NAME << "] " << __func__ << " instr_id: " << packet->instr_id << " type: " << +packet->type << hex << " addr: " << packet->address;
                        cout << " full_addr: " << packet->full_addr << " tag: " << block[set][way].tag << " data: " << block[set][way].data << dec;
                        cout << " set: " << set << " way: " << way << " lru: " << block[set][way].lru;
                        cout << " event: " << packet->event_cycle << " cycle: " << current_core_cycle[cpu] << endl; });
                        return match_way;
                    }
        }
       }

        return match_way;
}
int CACHE::check_hit(PACKET *packet,uint32_t set)
{
	if (cache_type == IS_LLC)
	{	//If we're checking hit for a packet of different slice
		assert(this_router->id == get_slice_num(packet->address)); 
	}
	int match_way = -1;

	if (cache_type == IS_LLC) //Total sets in LLC are NUM_SET*NUM_SLICES
	{
		if (NUM_SET*NUM_SLICES < set) 
		{
			cerr << "[" << NAME << "_ERROR] " << __func__ << " invalid set index: " << set << " NUM_SET: " << NUM_SET;
			cerr << " address: " << hex << packet->address << " full_addr: " << packet->full_addr << dec;
			cerr << " event: " << packet->event_cycle << endl;
			assert(0);
		}
	}
	else //For other caches no chnages 
	{
		if (NUM_SET < set) 
		{
			cerr << "[" << NAME << "_ERROR] " << __func__ << " invalid set index: " << set << " NUM_SET: " << NUM_SET;
			cerr << " address: " << hex << packet->address << " full_addr: " << packet->full_addr << dec;
			cerr << " event: " << packet->event_cycle << endl;
			assert(0);
		}
	}
       	int cache_hit=0;
	// hit
	if((cache_type == IS_LLC && CEASER_S_LLC !=1)) partitions=1;
     	if((cache_type != IS_LLC || (cache_type == IS_LLC && CEASER_S_LLC !=1)) && partitions != 1)
	{
		  cout<<"Partitions "<<partitions<<"\n"<<endl;
		  assert(0);
	}
       	for(int cur_part=0;cur_part<partitions;cur_part++) //Search in each partition
	{
		if(CEASER_S_LLC == 1 && cache_type == IS_LLC)
                {
                        if (ceaser_s_set[cur_part] >= Sptr)
                                set=ceaser_s_set[cur_part]; //Reset set number based on the key in the CEASER-S
                        else
                                set =ceaser_s_next_set[cur_part];
                }
		for(uint32_t way=(cur_part * (NUM_WAY/partitions)); way<NUM_WAY && (way < ((cur_part+1) * (NUM_WAY/partitions))); way++) 
		{
		     if (block[set][way].valid && (block[set][way].tag == packet->address)) 
		     {

			match_way = way;
			cache_hit=1;
			DP ( if (warmup_complete[packet->cpu]) {
			if(packet->type==PREFETCH)
				cout<<"PQ ";
			else
				cout<<"RQ ";
			cout << "[" << NAME << "] " << __func__ << " instr_id: " << packet->instr_id << " type: " << +packet->type << hex << " addr: " << packet->address;
			cout << " full_addr: " << packet->full_addr << " tag: " << block[set][way].tag << " data: " << block[set][way].data << dec;
			cout << " set: " << set << " way: " << way << " lru: " << block[set][way].lru;
			cout << " event: " << packet->event_cycle << " cycle: " << current_core_cycle[cpu] << endl; });
			return match_way;
			break;
		    }
		}
           if(cache_hit==1) break;
       	}

/*	if(CEASER_S_LLC == 1 && cache_type == IS_LLC && match_way == -1) //searching in victim_cache
        {
		//cout<<"Hi I am at victim cAche "<<endl;
                int victim_cache_match_way=-1;
                for(int way=0;way< NUM_WAY ; way++)
		{
                        if(victim_cache[0][way].valid && (victim_cache[0][way].tag == packet->address))
                        {
                                victim_cache_match_way = way;
                                break;
                        }
		}
                if(victim_cache_match_way != -1) //fill block in the LLC
                {
                        int cur_part=0;
			//cout<<"Set "<<set<<endl;
                        match_way = llc_find_victim_ceaser_s(victim_cache[0][victim_cache_match_way].cpu, victim_cache[0][victim_cache_match_way].instr_id, set, block[set], 0*ip*, ela, LOAD,cur_part);
                        //COPY The Block from victim_cache to LLC

                        if (block[set][match_way].valid == 0)
                                block[set][match_way].valid = 1;
                        block[set][match_way].dirty = 0;
                        block[set][match_way].prefetch = victim_cache[0][victim_cache_match_way].prefetch;
                        block[set][match_way].used = victim_cache[0][victim_cache_match_way].used;
                        block[set][match_way].delta = victim_cache[0][victim_cache_match_way].delta;
                        block[set][match_way].depth = victim_cache[0][victim_cache_match_way].depth;
                        block[set][match_way].signature = victim_cache[0][victim_cache_match_way].signature;
                        block[set][match_way].confidence = victim_cache[0][victim_cache_match_way].confidence;

                        block[set][match_way].tag = victim_cache[0][victim_cache_match_way].address;
                        block[set][match_way].address = victim_cache[0][victim_cache_match_way].address;
                        block[set][match_way].full_addr = victim_cache[0][victim_cache_match_way].full_addr;
                        block[set][match_way].data = victim_cache[0][victim_cache_match_way].data;
                        block[set][match_way].cpu = victim_cache[0][victim_cache_match_way].cpu;
                        block[set][match_way].instr_id = victim_cache[0][victim_cache_match_way].instr_id;
                        block[set][match_way].ip = victim_cache[0][victim_cache_match_way].ip;
                       	block[set][match_way].curr_or_next_key = victim_cache[0][victim_cache_match_way].curr_or_next_key; 
			victim_hit++;
                        victim_cache[0][victim_cache_match_way].valid = 0;
			  cout<<"Victim Hit"<<victim_hit;
                }
		//cout<<"Hi Ends Here"<<endl;
        } */

	return match_way;
}

int CACHE::invalidate_entry(uint64_t inval_addr)
{
	uint32_t set = get_set(inval_addr);
	int match_way = -1;

	if (NUM_SET < set) {
		cerr << "[" << NAME << "_ERROR] " << __func__ << " invalid set index: " << set << " NUM_SET: " << NUM_SET;
		cerr << " inval_addr: " << hex << inval_addr << dec << endl;
		assert(0);
	}

	// invalidate
        int inval_hit=0;
	if(CEASER_S_LLC != 1 || cache_type != IS_LLC) partitions=1;
       	for(int cur_part=0;cur_part<partitions;cur_part++) // Search in each partition
	{
            	if(CEASER_S_LLC == 1 && cache_type == IS_LLC)
		{
                    	if (ceaser_s_set[cur_part] >= Sptr)
          			set=ceaser_s_set[cur_part]; //Reset set number based on the Key in the CEASER-S
                        else
                                set =ceaser_s_next_set[cur_part];
               	}
		for (uint32_t way=(cur_part * (NUM_WAY/partitions)); way<NUM_WAY && (way < ((cur_part+1) * (NUM_WAY/partitions))) ; way++) 
		{
		   	if (block[set][way].valid && (block[set][way].tag == inval_addr)) 
			{

				block[set][way].valid = 0;
				match_way = way;
		 		inval_hit=1;
				DP ( if (warmup_complete[cpu]) {
				cout << "[" << NAME << "] " << __func__ << " inval_addr: " << hex << inval_addr;  
				cout << " tag: " << block[set][way].tag << " data: " << block[set][way].data << dec;
				cout << " set: " << set << " way: " << way << " lru: " << block[set][way].lru << " cycle: " << current_core_cycle[cpu] << endl; });
				break;
		  	}
           	}
            	if(inval_hit==1)
               		break;
	}

	return match_way;
}

int CACHE::add_rq(PACKET *packet)
{
	// check for the latest wirtebacks in the write queue
	int wq_index = WQ.check_queue(packet);
	if (wq_index != -1) {
		
		// check fill level
		if (packet->fill_level < fill_level) 
		{
			packet->data = WQ.entry[wq_index].data;

			if (cache_type == IS_LLC) //For LLC we have to add the pacekt to router with forL2=1.
			{
				if (INTERCONNECT_ON == 0 || this_router->id == packet->cpu)
				{
					if (packet->instruction) 
						upper_level_icache[packet->cpu]->return_data(packet);
					else // data
						upper_level_dcache[packet->cpu]->return_data(packet);
				}
				else
				{		
					int direction = get_direction(this_router->id,packet->cpu);
					if (this_router->NI[direction].OUTQ.occupancy == this_router->NI[direction].OUTQ.SIZE) 
					{
						cout << "[" << NAME << "] " << __func__ <<endl;
						cout << " Router queue filled!" << " Packet address: " << hex << packet->address<<"Router_id"<<this_router->id;
						
						STALL[packet->type]++;

						DP ( if (warmup_complete[packet->cpu]) {
						cout << "[" << NAME << "] " << __func__ <<endl;
						cout << " Router queue filled!" << " Packet address: " << hex << packet->address<<"Router_id"<<this_router->id;
						
						 });
					}
					else
					{
						packet->forL2=1;
						this_router->NI[direction].add_outq(packet);  
					}    
				}
			}
			else
			{   //For other caches no changes. 
				if (packet->instruction) 
					upper_level_icache[packet->cpu]->return_data(packet);
				else // data
					upper_level_dcache[packet->cpu]->return_data(packet);
			}
		}
#ifdef SANITY_CHECK
		if (cache_type == IS_ITLB)
			assert(0);
		else if (cache_type == IS_DTLB)
			assert(0);
		else if (cache_type == IS_L1I)
			assert(0);
#endif
		// update processed packets
		if ((cache_type == IS_L1D) && (packet->type != PREFETCH)) {
			if (PROCESSED.occupancy < PROCESSED.SIZE)
				PROCESSED.add_queue(packet);

			DP ( if (warmup_complete[packet->cpu]) {
			cout << "[" << NAME << "_RQ] " << __func__ << " instr_id: " << packet->instr_id << " found recent writebacks";
			cout << hex << " read: " << packet->address << " writeback: " << WQ.entry[wq_index].address << dec;
			cout << " index: " << MAX_READ << " rob_signal: " << packet->rob_signal << endl; });
		}

		HIT[packet->type]++;
		ACCESS[packet->type]++;

		WQ.FORWARD++;
		RQ.ACCESS++;

		return -1;
	}

	// check for duplicates in the read queue
	int index = RQ.check_queue(packet);
	if (index != -1) {
		
		if (packet->instruction) {
			uint32_t rob_index = packet->rob_index;
			RQ.entry[index].rob_index_depend_on_me.insert (rob_index);
			RQ.entry[index].instr_merged = 1;

			DP (if (warmup_complete[packet->cpu]) {
			cout << "[INSTR_MERGED] " << __func__ << " cpu: " << packet->cpu << " instr_id: " << RQ.entry[index].instr_id;
			cout << " merged rob_index: " << rob_index << " instr_id: " << packet->instr_id << endl; });
		}
		else 
		{
			// mark merged consumer
			if (packet->type == RFO) {

				uint32_t sq_index = packet->sq_index;
				RQ.entry[index].sq_index_depend_on_me.insert (sq_index);
				RQ.entry[index].store_merged = 1;
			}
			else {
				uint32_t lq_index = packet->lq_index; 
				RQ.entry[index].lq_index_depend_on_me.insert (lq_index);
				RQ.entry[index].load_merged = 1;

				DP (if (warmup_complete[packet->cpu]) {
				cout << "[DATA_MERGED] " << __func__ << " cpu: " << packet->cpu << " instr_id: " << RQ.entry[index].instr_id;
				cout << " merged rob_index: " << packet->rob_index << " instr_id: " << packet->instr_id << " lq_index: " << packet->lq_index << endl; });
			}
		}

		RQ.MERGED++;
		RQ.ACCESS++;

		return index; // merged index
	}

	// check occupancy
	if (RQ.occupancy == RQ_SIZE) {
		RQ.FULL++;
		if (cache_type == IS_LLC)
		{
			cout<<"LLC "<<this_router->id<<" RQ full"<<endl;
		}
		
		return -2; // cannot handle this request
	}

	// if there is no duplicate, add it to RQ
	index = RQ.tail;

#ifdef SANITY_CHECK
	if (RQ.entry[index].address != 0) {
		cerr << "[" << NAME << "_ERROR] " << __func__ << " is not empty index: " << index;
		cerr << " address: " << hex << RQ.entry[index].address;
		cerr << " full_addr: " << RQ.entry[index].full_addr << dec << endl;
		assert(0);
	}
#endif

	RQ.entry[index] = *packet;

	RQ.entry[index].cycle_enqueued = current_core_cycle[packet->cpu]; //@Sujeet
	// ADD LATENCY
	if(cache_type == IS_LLC && ( CEASER_S_LLC == 1) && all_warmup_complete > NUM_CPUS )//As the encryption engine is shared among all the queues, the event cycle will be set as UINT_64. It is reduced when the entry will get encryption engine in the function giving_encryption_engine_to_queue_entries.
		RQ.entry[index].event_cycle = UINT64_MAX;
	else{
		if (RQ.entry[index].event_cycle < current_core_cycle[packet->cpu])
			RQ.entry[index].event_cycle = current_core_cycle[packet->cpu] + LATENCY;
		else
			RQ.entry[index].event_cycle += LATENCY;
           }
	

	RQ.occupancy++;
	RQ.tail++;
	if (RQ.tail >= RQ.SIZE)
		RQ.tail = 0;

	DP ( if (warmup_complete[RQ.entry[index].cpu]) {
	cout << "[" << NAME << "_RQ] " <<  __func__ << " instr_id: " << RQ.entry[index].instr_id << " address: " << hex << RQ.entry[index].address;
	cout << " full_addr: " << RQ.entry[index].full_addr << dec;
	cout << " type: " << +RQ.entry[index].type << " head: " << RQ.head << " tail: " << RQ.tail << " occupancy: " << RQ.occupancy;
	cout << " event: " << RQ.entry[index].event_cycle << " current: " << current_core_cycle[RQ.entry[index].cpu] << endl; });

	if (packet->address == 0)
		assert(0);

	RQ.TO_CACHE++;
	RQ.ACCESS++;

	return -1;
}

int CACHE::add_wq(PACKET *packet)
{
	// check for duplicates in the write queue
	int index = WQ.check_queue(packet);
	if (index != -1) {

		WQ.MERGED++;
		WQ.ACCESS++;

		return index; // merged index
	}

	// sanity check
	if (WQ.occupancy >= WQ.SIZE)
		assert(0);

	// if there is no duplicate, add it to the write queue
	index = WQ.tail;
	if (WQ.entry[index].address != 0) {
		cerr << "[" << NAME << "_ERROR] " << __func__ << " is not empty index: " << index;
		cerr << " address: " << hex << WQ.entry[index].address;
		cerr << " full_addr: " << WQ.entry[index].full_addr << dec << endl;
		assert(0);
	}

	WQ.entry[index] = *packet;

	// ADD LATENCY
        if(cache_type == IS_LLC && (CEASER_S_LLC == 1) && all_warmup_complete > NUM_CPUS) //As the encryption engine is shared among all the queues, the event cycle will be set as UINT_64. It is reduced when it will get encryption engine in the function giving_encryption_engine_to_queue_entries.
                WQ.entry[index].event_cycle = UINT64_MAX;
        else
	{
		if (WQ.entry[index].event_cycle < current_core_cycle[packet->cpu])
                	WQ.entry[index].event_cycle = current_core_cycle[packet->cpu] + LATENCY;
        	else
                	WQ.entry[index].event_cycle += LATENCY;
	}

	WQ.occupancy++;
	WQ.tail++;
	if (WQ.tail >= WQ.SIZE)
		WQ.tail = 0;

	DP (if (warmup_complete[WQ.entry[index].cpu]) {
	cout << "[" << NAME << "_WQ] " <<  __func__ << " instr_id: " << WQ.entry[index].instr_id << " address: " << hex << WQ.entry[index].address;
	cout << " full_addr: " << WQ.entry[index].full_addr << dec;
	cout << " head: " << WQ.head << " tail: " << WQ.tail << " occupancy: " << WQ.occupancy;
	cout << " data: " << hex << WQ.entry[index].data << dec;
	cout << " event: " << WQ.entry[index].event_cycle << " current: " << current_core_cycle[WQ.entry[index].cpu] << endl; });

	WQ.TO_CACHE++;
	WQ.ACCESS++;

	return -1;
}

int CACHE::prefetch_line(uint64_t ip, uint64_t base_addr, uint64_t pf_addr, int pf_fill_level, uint32_t prefetch_metadata)
{
	pf_requested++;

	if (PQ.occupancy < PQ.SIZE) {
		if ((base_addr>>LOG2_PAGE_SIZE) == (pf_addr>>LOG2_PAGE_SIZE)) {
			
			PACKET pf_packet;
			pf_packet.fill_level = pf_fill_level;
		pf_packet.pf_origin_level = fill_level;
		pf_packet.pf_metadata = prefetch_metadata;
			pf_packet.cpu = cpu;
			//pf_packet.data_index = LQ.entry[lq_index].data_index;
			//pf_packet.lq_index = lq_index;
			pf_packet.address = pf_addr >> LOG2_BLOCK_SIZE;
			pf_packet.full_addr = pf_addr;
			//pf_packet.instr_id = LQ.entry[lq_index].instr_id;
			//pf_packet.rob_index = LQ.entry[lq_index].rob_index;
			pf_packet.ip = ip;
			pf_packet.type = PREFETCH;
			pf_packet.event_cycle = current_core_cycle[cpu];

			// give a dummy 0 as the IP of a prefetch
			if (cache_type == IS_LLC)
			{
				int packet_slice = get_slice_num(pf_packet.address);
				if (this_router->id != packet_slice)
				{
					// cout<<"DP: LLC slice "<<this_router->id<<" making prefetch request for slice "<<packet_slice<<endl; 
				}
				this_router->LLC_MAP[packet_slice]->add_pq(&pf_packet); 
			}
		   	else
				add_pq(&pf_packet); // For other caches, no change

			pf_issued++;

			return 1;
		}
	}
	pf_dropped++;
	return 0;
}

int CACHE::kpc_prefetch_line(uint64_t base_addr, uint64_t pf_addr, int pf_fill_level, int delta, int depth, int signature, int confidence, uint32_t prefetch_metadata)
{
	if (PQ.occupancy < PQ.SIZE) {
		if ((base_addr>>LOG2_PAGE_SIZE) == (pf_addr>>LOG2_PAGE_SIZE)) {
			
			PACKET pf_packet;
			pf_packet.fill_level = pf_fill_level;
		pf_packet.pf_origin_level = fill_level;
		pf_packet.pf_metadata = prefetch_metadata;
			pf_packet.cpu = cpu;
			//pf_packet.data_index = LQ.entry[lq_index].data_index;
			//pf_packet.lq_index = lq_index;
			pf_packet.address = pf_addr >> LOG2_BLOCK_SIZE;
			pf_packet.full_addr = pf_addr;
			//pf_packet.instr_id = LQ.entry[lq_index].instr_id;
			//pf_packet.rob_index = LQ.entry[lq_index].rob_index;
			pf_packet.ip = 0;
			pf_packet.type = PREFETCH;
			pf_packet.delta = delta;
			pf_packet.depth = depth;
			pf_packet.signature = signature;
			pf_packet.confidence = confidence;
			pf_packet.event_cycle = current_core_cycle[cpu];

			// give a dummy 0 as the IP of a prefetch
			//It is possible that (say) LLC slice 0 generates a prefetch request for slice 1 
			if (cache_type == IS_LLC)
			{
				int packet_slice = get_slice_num(pf_packet.address);
				if (this_router->id != packet_slice)
				{
					// cout<<"DP: LLC slice "<<this_router->id<<" making prefetch request for slice "<<packet_slice<<endl; 
				}
				 this_router->LLC_MAP[packet_slice]->add_pq(&pf_packet); 
			}
		   else
			add_pq(&pf_packet); // For other caches, no change
			
			pf_issued++;

			return 1;
		}
	}

	return 0;
}

int CACHE::add_pq(PACKET *packet)
{
	// check for the latest wirtebacks in the write queue
	int wq_index = WQ.check_queue(packet);
	if (wq_index != -1) {
		
		// check fill level
		if (packet->fill_level < fill_level) {

			packet->data = WQ.entry[wq_index].data;

			if (cache_type == IS_LLC) //For LLC we have to add the pacekt to router with forL2=1.
			{
				if (INTERCONNECT_ON == 0 || this_router->id == packet->cpu)
				{
					if (packet->instruction) 
						upper_level_icache[packet->cpu]->return_data(packet);
					else // data
						upper_level_dcache[packet->cpu]->return_data(packet);
				}
				else
				{
					int direction = get_direction(this_router->id,packet->cpu);
					if (this_router->NI[direction].OUTQ.occupancy == this_router->NI[direction].OUTQ.SIZE)
					{
						cout << "[" << NAME << "] " << __func__ <<endl;
						cout << " Router queue filled!" << " Packet address: " << hex << packet->address<<"Router_id"<<this_router->id;
						
						STALL[packet->type]++;

						DP ( if (warmup_complete[packet->cpu]) {
						cout << "[" << NAME << "] " << __func__ <<endl;
						cout << " Router queue filled!" << " Packet address: " << hex << packet->address<<"Router_id"<<this_router->id;
						
						return -2; //Is this correct way of handling?
						 });
					}
					else
					{
						packet->forL2=1;
						this_router->NI[direction].add_outq(packet);  	
					}                
				}	
			}
			else
			{   //For other caches no changes. 
				if (packet->instruction) 
					upper_level_icache[packet->cpu]->return_data(packet);
				else // data
					upper_level_dcache[packet->cpu]->return_data(packet);
			}

		}

		HIT[packet->type]++;
		ACCESS[packet->type]++;

		WQ.FORWARD++;
		PQ.ACCESS++;

		return -1;
	}

	// check for duplicates in the PQ
	int index = PQ.check_queue(packet);
	if (index != -1) {
		if (packet->fill_level < PQ.entry[index].fill_level)
			PQ.entry[index].fill_level = packet->fill_level;

		PQ.MERGED++;
		PQ.ACCESS++;

		return index; // merged index
	}

	// check occupancy
	if (PQ.occupancy == PQ_SIZE) {
		PQ.FULL++;

		DP ( if (warmup_complete[packet->cpu]) {
		cout << "[" << NAME << "] cannot process add_pq since it is full" << endl; });
		return -2; // cannot handle this request
	}

	// if there is no duplicate, add it to PQ
	index = PQ.tail;

#ifdef SANITY_CHECK
	if (PQ.entry[index].address != 0) {
		cout<<" Occupancy "<<PQ.occupancy<<" Size : "<<PQ.SIZE<<" Head : "<<PQ.head<<" Tail: "<<PQ.tail<<endl;
		PQ.print_queue();
		cout<<endl<<endl;
		cerr << "[" << NAME << "_ERROR] " << __func__ << " is not empty index: " << index;
		cerr << " address: " << hex << PQ.entry[index].address;
		cerr << " full_addr: " << PQ.entry[index].full_addr << dec << endl;
		assert(0);
	}
#endif

	PQ.entry[index] = *packet;

	// ADD LATENCY
        if(cache_type == IS_LLC && ( CEASER_S_LLC == 1) && all_warmup_complete > NUM_CPUS)
                PQ.entry[index].event_cycle = UINT64_MAX;
        else{
                if (PQ.entry[index].event_cycle < current_core_cycle[packet->cpu])
                        PQ.entry[index].event_cycle = current_core_cycle[packet->cpu] + LATENCY;
                else
                        PQ.entry[index].event_cycle += LATENCY;
        }

	PQ.occupancy++;
	PQ.tail++;
	if (PQ.tail >= PQ.SIZE)
		PQ.tail = 0;

	DP ( if (warmup_complete[PQ.entry[index].cpu]) {
	cout << "[" << NAME << "_PQ] " <<  __func__ << " instr_id: " << PQ.entry[index].instr_id << " address: " << hex << PQ.entry[index].address;
	cout << " full_addr: " << PQ.entry[index].full_addr << dec;
	cout << " type: " << +PQ.entry[index].type << " head: " << PQ.head << " tail: " << PQ.tail << " occupancy: " << PQ.occupancy;
	cout << " event: " << PQ.entry[index].event_cycle << " current: " << current_core_cycle[PQ.entry[index].cpu] << endl; });

	if (packet->address == 0)
		assert(0);

	PQ.TO_CACHE++;
	PQ.ACCESS++;

	return -1;
}

void CACHE::return_data(PACKET *packet)
{
	// check MSHR information
	int mshr_index = check_mshr(packet);
	// sanity check
	if (mshr_index == -1) {
		cerr << "[" << NAME << "_MSHR] " << __func__ << " instr_id: " << packet->instr_id << " cannot find a matching entry!";
		cerr << " full_addr: " << hex << packet->full_addr;
		cerr << " address: " << packet->address << dec;
		cerr << " event: " << packet->event_cycle << " current: " << current_core_cycle[packet->cpu] << endl;
		cerr << "packet type" << packet->type << " slice" << get_slice_num(packet->address);
		cerr << "packet->cpu" << packet->cpu;

		for (int i = 0; i < NUM_SLICES; ++i)
		{
			cerr<<packet->router_path[i]<< " ";
		}

		assert(0);
	}

	// MSHR holds the most updated information about this request
	// no need to do memcpy
	MSHR.num_returned++;
	MSHR.entry[mshr_index].returned = COMPLETED;
	MSHR.entry[mshr_index].data = packet->data;
	MSHR.entry[mshr_index].pf_metadata = packet->pf_metadata;
	if(cache_type == IS_LLC && all_warmup_complete > NUM_CPUS)
		MSHR.entry[mshr_index].mshr_data_return_cycle=current_core_cycle[packet->cpu];
	// ADD LATENCY
	 if(cache_type == IS_LLC && all_warmup_complete > NUM_CPUS && ( CEASER_S_LLC == 1) && (cache_stall_cycle ==0 && encryption_stall_cycle == 0) && is_remap_complete == 1)
	 {
		 #if Pipelined_Encryption_Engine 
                      	encryption_stall_cycle += 1;
                 #else
                        encryption_stall_cycle += CEASER_LATENCY;
                 #endif

		 total_encryption_time_mshr[MSHR.entry[mshr_index].cpu] += (CEASER_LATENCY);
                 total_access_time_mshr[MSHR.entry[mshr_index].cpu]+= (LATENCY);		
                 MSHR.entry[mshr_index].event_cycle = current_core_cycle[packet->cpu] + LATENCY;

        } //return_data for LLC gets called from DRAM, we should start filling into LLC at the same time if encryption engine is free
        else if(cache_type == IS_LLC && (CEASER_S_LLC == 1) && all_warmup_complete > NUM_CPUS)
                MSHR.entry[mshr_index].event_cycle = UINT64_MAX;
        else{
                if (MSHR.entry[mshr_index].event_cycle < current_core_cycle[packet->cpu])
                        MSHR.entry[mshr_index].event_cycle = current_core_cycle[packet->cpu] + LATENCY;
                else
                        MSHR.entry[mshr_index].event_cycle += LATENCY;
        }

	update_fill_cycle();

	DP (if (warmup_complete[packet->cpu]) {
	if(MSHR.entry[mshr_index].returned == COMPLETED)
	cout<<"STATUS COMPLETED ";
	else
	cout<<"STATUS INCOMPLETED ";
	cout << "[" << NAME << "_MSHR] " <<  __func__ << " instr_id: " << MSHR.entry[mshr_index].instr_id;
	cout << " address: " << hex << MSHR.entry[mshr_index].address << " full_addr: " << MSHR.entry[mshr_index].full_addr;
	cout << " data: " << MSHR.entry[mshr_index].data << dec << " num_returned: " << MSHR.num_returned;
	cout << " index: " << mshr_index << " occupancy: " << MSHR.occupancy;
	cout << " event: " << MSHR.entry[mshr_index].event_cycle << " current: " << current_core_cycle[packet->cpu] << " next: " << MSHR.next_fill_cycle << endl; });
}

void CACHE::update_fill_cycle()
{
	// update next_fill_cycle
	uint64_t min_cycle = UINT64_MAX;
	uint32_t min_index = MSHR.SIZE;
	for (uint32_t i=0; i<MSHR.SIZE; i++) {
		if ((MSHR.entry[i].returned == COMPLETED) && (MSHR.entry[i].event_cycle < min_cycle)) {
			min_cycle = MSHR.entry[i].event_cycle;
			min_index = i;
		}

		DP (if (warmup_complete[MSHR.entry[i].cpu]) {
		cout << "[" << NAME << "_MSHR] " <<  __func__ << " checking instr_id: " << MSHR.entry[i].instr_id;
		cout << " address: " << hex << MSHR.entry[i].address << " full_addr: " << MSHR.entry[i].full_addr;
		cout << " data: " << MSHR.entry[i].data << dec << " returned: " << +MSHR.entry[i].returned << " fill_level: " << MSHR.entry[i].fill_level;
		cout << " index: " << i << " occupancy: " << MSHR.occupancy;
		cout << " event: " << MSHR.entry[i].event_cycle << " current: " << current_core_cycle[MSHR.entry[i].cpu] << " next: " << MSHR.next_fill_cycle << endl; });
	}
	
	MSHR.next_fill_cycle = min_cycle;
	MSHR.next_fill_index = min_index;
	if (min_index < MSHR.SIZE) {

		DP (if (warmup_complete[MSHR.entry[min_index].cpu]) {
		cout << "[" << NAME << "_MSHR] " <<  __func__ << " instr_id: " << MSHR.entry[min_index].instr_id;
		cout << " address: " << hex << MSHR.entry[min_index].address << " full_addr: " << MSHR.entry[min_index].full_addr;
		cout << " data: " << MSHR.entry[min_index].data << dec << " num_returned: " << MSHR.num_returned;
		cout << " event: " << MSHR.entry[min_index].event_cycle << " current: " << current_core_cycle[MSHR.entry[min_index].cpu] << " next: " << MSHR.next_fill_cycle << endl; });
	}
}

int CACHE::check_mshr(PACKET *packet)
{
	// search mshr
	for (uint32_t index=0; index<MSHR_SIZE; index++) {
		if (MSHR.entry[index].address == packet->address) {
			
			DP ( if (warmup_complete[packet->cpu]) {
			cout << "[" << NAME << "_MSHR] " << __func__ << " same entry instr_id: " << packet->instr_id << " prior_id: " << MSHR.entry[index].instr_id;
			if(MSHR.entry[index].returned == COMPLETED)
		        	cout<<"STATUS COMPLETED\n";
        		else
        			cout<<"STATUS INCOMPLETED\n";

			cout << " address: " << hex << packet->address;
			cout << " full_addr: " << packet->full_addr << dec << endl; });

			return index;
		}
	}

	DP ( if (warmup_complete[packet->cpu]) {
	cout << "[" << NAME << "_MSHR] " << __func__ << " new address: " << hex << packet->address;
	cout << " full_addr: " << packet->full_addr << dec << endl; });

	DP ( if (warmup_complete[packet->cpu] && (MSHR.occupancy == MSHR_SIZE)) { 
	cout << "[" << NAME << "_MSHR] " << __func__ << " mshr is full";
	cout << " instr_id: " << packet->instr_id << " mshr occupancy: " << MSHR.occupancy;
	cout << " address: " << hex << packet->address;
	cout << " full_addr: " << packet->full_addr << dec;
	cout << " cycle: " << current_core_cycle[packet->cpu] << endl; });

	return -1;
}

void CACHE::add_mshr(PACKET *packet)
{
	uint32_t index = 0;
	packet->cycle_enqueued = current_core_cycle[packet->cpu];
	// search mshr
	for (index=0; index<MSHR_SIZE; index++) 
	{
		if (MSHR.entry[index].address == 0) {
			
			MSHR.entry[index] = *packet;
			MSHR.entry[index].returned = INFLIGHT;
			MSHR.occupancy++;
	            	if(all_warmup_complete > NUM_CPUS)
        		    MSHR.entry[index].add_cycle_count = current_core_cycle[packet->cpu];
			DP ( if (warmup_complete[packet->cpu]) {
			cout << "[" << NAME << "_MSHR] " << __func__ << " instr_id: " << packet->instr_id;
			cout << " address: " << hex << packet->address << " full_addr: " << packet->full_addr << dec;
			cout << " index: " << index << " occupancy: " << MSHR.occupancy << endl; });

			break;
		}
	}
}

uint32_t CACHE::get_occupancy(uint8_t queue_type, uint64_t address)
{
	if (queue_type == 0)
		return MSHR.occupancy;
	else if (queue_type == 1)
		return RQ.occupancy;
	else if (queue_type == 2)
		return WQ.occupancy;
	else if (queue_type == 3)
		return PQ.occupancy;

	return 0;
}
uint32_t CACHE::get_size(uint8_t queue_type, uint64_t address)
{
	if (queue_type == 0)
		return MSHR.SIZE;
	else if (queue_type == 1)
		return RQ.SIZE;
	else if (queue_type == 2)
		return WQ.SIZE;
	else if (queue_type == 3)
		return PQ.SIZE;

	return 0;
}

void CACHE::increment_WQ_FULL(uint64_t address)
{
	WQ.FULL++;
}


uint64_t CACHE::bitset42_to_uint64(bitset<42> b)
{  //Convert bitset to uint64 it is used for PRINCE
	uint64_t number=0;
	for(int z=0;z<42;z++){
		if(b[z])
			number+=pow(2,z);
	}
	return number;
}

uint64_t CACHE::getEncryptedAddress(uint64_t pla,uint32_t current_cpu, uint8_t *key,uint32_t add_latency)
{ //Uses PRINCE to encrypte pla based on the key

	#ifdef No_Randomization
		return pla;//No randomisation
	#endif
	uint64_t ela=0;
	if( (cache_type == IS_L1I) || ( cache_type == IS_L1D) || (cache_type == IS_L2C) || (CEASER_S_LLC==0 && MIRAGE == 0 && cache_type == IS_LLC) )
        	return pla;
	if(cache_type == IS_ITLB || cache_type == IS_DTLB || cache_type == IS_STLB)
        	return pla;

	if(add_latency == 1 && all_warmup_complete > NUM_CPUS)
	{
		#if Pipelined_Encryption_Engine
           		encryption_stall_cycle += 1;
        	#else
                	encryption_stall_cycle += CEASER_LATENCY;
        	#endif
	}

	for (int i=0; i <8 ; i++)
	{
		in[i] = pla >> (7-i)*8;
		in[i] = in[i] % 256;
	}
	out = p.cipher(in,key);
	for(uint32_t i=0; i<8; i++)
	{
		ela +=  uint64_t(out[7-i]) << 8*i; 
	}
	return ela;
}
uint64_t CACHE::getDecryptedAddress(uint32_t Sptr,uint32_t way)
{
	return block[Sptr][way].tag;
}
int CACHE::remap_block(uint32_t set, uint32_t way)
{
	if( block[set][way].valid==0 || block[set][way].curr_or_next_key==1 /*|| rrpv_value(set , way) == 3*/  ) //Our idea of no remap if rrpv == 3
	{
		if(block[set][way].valid==0)
			blocks_less_evicted++;
		 // block is already encrypted with the new key and valid and dirty.
       	     if(block[set][way].valid == 1 && block[set][way].dirty == 1)
         	{

	 		int channel = uncore.DRAM.dram_get_channel(block[set][way].tag);
			//check WQ occupancy, if 0, return 0 i.e. vacancy is not available for remapping
         		if (uncore.DRAM.WQ[channel].occupancy >= uncore.DRAM.WQ[channel].SIZE)  //cache level is not checked
         		{
                		// watermark =way;
                 		block[set][way].is_on_remap_recursive_call = 0;
				return 0;
         		}
                	PACKET writeback_packet;
                	writeback_packet.fill_level = FILL_DRAM; // where are we checking the cache level ?
                	writeback_packet.cpu = block[set][way].cpu;
                	curr_addr=block[set][way].tag;
                	writeback_packet.address = curr_addr;
                	full_addr       = (curr_addr << LOG2_BLOCK_SIZE) + (block[set][way].full_addr & 0x3F);
                	writeback_packet.full_addr = full_addr;
                	writeback_packet.data = block[set][way].data;
                	writeback_packet.instr_id = block[set][way].instr_id;
                	writeback_packet.ip = 0; // writeback does not have ip
                	writeback_packet.type = WRITEBACK;
                	writeback_packet.event_cycle = current_core_cycle[block[set][way].cpu];
                	lower_level->add_wq(&writeback_packet);
			count_remap++;
          	}
	        block[set][way].valid = 0;
	     	block[set][way].is_on_remap_recursive_call = 0;
		//remap_llc_update_replacement_state(Sptr,way,Sptr,way,block[Sptr][way].tag); //rrpv is set to max way
		return 1;
	}
	pla = block[set][way].tag;
        ela = getEncryptedAddress(pla,block[set][way].cpu,next_key,0);
	int cur_part=(way/(NUM_WAY/partitions));
	block[set][way].is_on_remap_recursive_call = 1;
	uint32_t newset = ((uint32_t) (ela & ((1 << lg2(NUM_SET*NUM_SLICES)) - 1)))>>lg2(NUM_SLICES),newway;
	if(cache_type==IS_LLC)
	{
                     //newway = remap_find_victim(block[Sptr][way].cpu, block[Sptr][way].instr_id, newset, block[newset], 0/*ip*/, ela, 0/*type*/,cur_part);
			newway = llc_find_victim_ceaser_s(block[Sptr][way].cpu, block[Sptr][way].instr_id, newset, block[newset], 0/*ip*/, ela, 0/*type*/,cur_part);
	}
	if(Sptr==newset && set == Sptr)  // if the block pointed by sptr gets its own block-address after encryption
     	{
		blocks_less_evicted++;
               block[Sptr][way].valid = 1;
               block[Sptr][way].curr_or_next_key = 1;
	       block[set][way].is_on_remap_recursive_call = 0;
               return 1;
       	}
	if(Sptr == newset) // cyclic mapping, like block 2 is pointed by sptr and  2->3->2 , 2->5->4->2 ('->' means encrypts to)
	{
		if(block[set][way].valid == 1 && block[set][way].dirty == 1)
                {

                        int channel = uncore.DRAM.dram_get_channel(block[set][way].tag);   //cache type is not checked
                        if (uncore.DRAM.WQ[channel].occupancy >= uncore.DRAM.WQ[channel].SIZE)
                        {
                                // watermark =way;
                                block[set][way].is_on_remap_recursive_call = 0;
				return 0;
                        }
                        PACKET writeback_packet;
                        writeback_packet.fill_level = FILL_DRAM;  //cache type is not checked
                        writeback_packet.cpu = block[set][way].cpu;
                        curr_addr=block[set][way].tag;
                        writeback_packet.address = curr_addr;
                        full_addr       = (curr_addr << LOG2_BLOCK_SIZE) + (block[set][way].full_addr & 0x3F);
                        writeback_packet.full_addr = full_addr;
                        writeback_packet.data = block[set][way].data;
                        writeback_packet.instr_id = block[set][way].instr_id;
                        writeback_packet.ip = 0; // writeback does not have ip
                        writeback_packet.type = WRITEBACK;
                        writeback_packet.event_cycle = current_core_cycle[block[set][way].cpu];
                        lower_level->add_wq(&writeback_packet);
                }
		block[set][way].is_on_remap_recursive_call = 0;
		return 1;
	}
	if(block[newset][newway].is_on_remap_recursive_call == 1)
	{
		block[set][way].is_on_remap_recursive_call = 0;
		return 1;
	}
	int flag=remap_block(newset , newway);
	if(flag == 0 &&  block[set][way].dirty == 1) //can't remap 'block[set][way]' to 'block[newset][newway]' & 'block[set][way]' is dirty
	{
		block[set][way].is_on_remap_recursive_call = 0;
		if(block[set][way].valid == 1 && block[set][way].dirty == 1)
         	{

	 		int channel = uncore.DRAM.dram_get_channel(block[set][way].tag);
			//check WQ occupancy, if 0, return 0 i.e. vacancy is not available for remapping
         		if (uncore.DRAM.WQ[channel].occupancy >= uncore.DRAM.WQ[channel].SIZE)  //cache level is not checked
         		{
                		// watermark =way;
                 		block[set][way].is_on_remap_recursive_call = 0;
				return 0;
         		}
                	PACKET writeback_packet;
                	writeback_packet.fill_level = FILL_DRAM; // where are we checking the cache level ?
                	writeback_packet.cpu = block[set][way].cpu;
                	curr_addr=block[set][way].tag;
                	writeback_packet.address = curr_addr;
                	full_addr       = (curr_addr << LOG2_BLOCK_SIZE) + (block[set][way].full_addr & 0x3F);
                	writeback_packet.full_addr = full_addr;
                	writeback_packet.data = block[set][way].data;
                	writeback_packet.instr_id = block[set][way].instr_id;
                	writeback_packet.ip = 0; // writeback does not have ip
                	writeback_packet.type = WRITEBACK;
                	writeback_packet.event_cycle = current_core_cycle[block[set][way].cpu];
                	lower_level->add_wq(&writeback_packet);
			count_remap++;
          	}
		 return 1;
	}
	else if(flag == 0 && set == Sptr) //can't remap 'block[set][way]' to 'block[newset][newway]' & 'block[set][way]' is pointed by Sptr hence regardless of whether it is dirty or not, return should be zero. As, it is a valid block which is not remapped yet.
	 {
		 block[set][way].is_on_remap_recursive_call = 0;
		 return 0;
	 }
	 else if(flag == 0 && set != Sptr) //can't remap 'block[set][way]' to 'block[newset][newway]' but 'block[set][way]' is neither pointed by the Sptr and nor dirty so it is simply discarded to make the room for remapping the block which has it as its encrypted location.
	 {	
		block[set][way].is_on_remap_recursive_call = 0;
		 return 1;
	 }
	 if(newset >= NUM_SET || newway >= NUM_WAY)
		 assert(0);
         block[set][way].valid = 0; //only the remap block is not valid
	block[newset][newway].valid = 1;
         block[newset][newway].dirty = block[set][way].dirty;
         block[newset][newway].prefetch = block[set][way].prefetch;
         block[newset][newway].used = 0;
         block[newset][newway].delta = block[set][way].delta;
         block[newset][newway].depth = block[set][way].depth;
         block[newset][newway].signature = block[set][way].signature;
         block[newset][newway].confidence = block[set][way].confidence;
         block[newset][newway].tag = block[set][way].tag;
         block[newset][newway].data = block[set][way].data;
         block[newset][newway].cpu = block[set][way].cpu;
         block[newset][newway].instr_id = block[set][way].instr_id;
         block[newset][newway].full_addr = (ela <<  LOG2_BLOCK_SIZE) + (block[set][way].full_addr & 0x3F);
         block[newset][newway].address = ela;
         block[newset][newway].curr_or_next_key = 1;
	 total_blocks_remapped++;
	 if (cache_type == IS_LLC)
	 {
		 remap_llc_update_replacement_state(set,way,newset,newway,block[newset][newway].tag);
           //      llc_update_replacement_state(block[newset][newway].cpu, newset,newway, block[newset][newway].full_addr, 1, 0, 0, 1);
         }

	  /*if(cache_type == IS_LLC && all_warmup_complete > NUM_CPUS)
          {
	//	  cout << "adding extra latency in remap_block" << endl;
                cache_stall_cycle += (  (2*LATENCY)-(2*CEASER_LATENCY)  );
                total_stall_cycle += (   (2*LATENCY)-(2*CEASER_LATENCY)   );
                                                        //In get_Encrypted() we have added CEASER_LATENCY but we have taken LLC_LATENCY as 28 so we should substract what we have added in getEncrypted().
//28 -> 20 : Accessin the LLC 8: Decryping address
//28 -> 8 : encrypting address with new key 20 : Accessing the newer LLC set

          }*/
	  block[set][way].is_on_remap_recursive_call = 0;
	  return 1;
}
/*
int CACHE::remap_block(uint32_t set, uint32_t way)
{
        if( block[set][way].valid==0 || block[set][way].curr_or_next_key==1  ) //Our idea of no remap if rrpv == 3
        {
                if(block[set][way].valid==0)
                        blocks_less_evicted++;
                 // block is already encrypted with the new key and valid and dirty.
             if(block[set][way].valid == 1 && block[set][way].dirty == 1)
                {

                        int channel = uncore.DRAM.dram_get_channel(block[set][way].tag);
                        //check WQ occupancy, if 0, return 0 i.e. vacancy is not available for remapping
                        if (uncore.DRAM.WQ[channel].occupancy >= uncore.DRAM.WQ[channel].SIZE)  //cache level is not checked
                        {
                                // watermark =way;
                                block[set][way].is_on_remap_recursive_call = 0;
                                return 0;
                        }
                        PACKET writeback_packet;
                        writeback_packet.fill_level = FILL_DRAM; // where are we checking the cache level ?
                        writeback_packet.cpu = block[set][way].cpu;
                        curr_addr=block[set][way].tag;
                        writeback_packet.address = curr_addr;
                        full_addr       = (curr_addr << LOG2_BLOCK_SIZE) + (block[set][way].full_addr & 0x3F);
                        writeback_packet.full_addr = full_addr;
                        writeback_packet.data = block[set][way].data;
                        writeback_packet.instr_id = block[set][way].instr_id;
                        writeback_packet.ip = 0; // writeback does not have ip
                        writeback_packet.type = WRITEBACK;
                        writeback_packet.event_cycle = current_core_cycle[block[set][way].cpu];
                        lower_level->add_wq(&writeback_packet);
                        count_remap++;
                }
                block[set][way].valid = 0;
                block[set][way].is_on_remap_recursive_call = 0;
                remap_llc_update_replacement_state(way,Sptr,way,block[Sptr][way].tag); //rrpv is set to max way
                return 1;
        }
        pla = block[set][way].tag;
        ela = getEncryptedAddress(pla,block[set][way].cpu,next_key,0);
        int cur_part=(way/(NUM_WAY/partitions));
        block[set][way].is_on_remap_recursive_call = 1;
        uint32_t newset = ((uint32_t) (ela & ((1 << lg2(NUM_SET*NUM_SLICES)) - 1)))>>lg2(NUM_SLICES),newway;
        if(cache_type==IS_LLC)
        {
        }
        if(Sptr==newset && set == Sptr)  // if the block pointed by sptr gets its own block-address after encryption
        {
                blocks_less_evicted++;
               block[Sptr][way].valid = 1;
               block[Sptr][way].curr_or_next_key = 1;
               block[set][way].is_on_remap_recursive_call = 0;
               return 1;
        }
        if(Sptr == newset) // cyclic mapping, like block 2 is pointed by sptr and  2->3->2 , 2->5->4->2 ('->' means encrypts to)
        {
                if(block[set][way].valid == 1 && block[set][way].dirty == 1)
                {

                        int channel = uncore.DRAM.dram_get_channel(block[set][way].tag);   //cache type is not checked
                        if (uncore.DRAM.WQ[channel].occupancy >= uncore.DRAM.WQ[channel].SIZE)
                        {
                                // watermark =way;
                                block[set][way].is_on_remap_recursive_call = 0;
                                return 0;
                        }
                        PACKET writeback_packet;
                        writeback_packet.fill_level = FILL_DRAM;  //cache type is not checked
                        writeback_packet.cpu = block[set][way].cpu;
                        curr_addr=block[set][way].tag;
                        writeback_packet.address = curr_addr;
                        full_addr       = (curr_addr << LOG2_BLOCK_SIZE) + (block[set][way].full_addr & 0x3F);
                        writeback_packet.full_addr = full_addr;
                        writeback_packet.data = block[set][way].data;
                        writeback_packet.instr_id = block[set][way].instr_id;
                        writeback_packet.ip = 0; // writeback does not have ip
                        writeback_packet.type = WRITEBACK;
                        writeback_packet.event_cycle = current_core_cycle[block[set][way].cpu];
                        lower_level->add_wq(&writeback_packet);
                }
                block[set][way].is_on_remap_recursive_call = 0;
                return 1;
        }
        if(block[newset][newway].is_on_remap_recursive_call == 1)
        {
                block[set][way].is_on_remap_recursive_call = 0;
                return 1;
        }
        int flag=remap_block(newset , newway);
        if(flag == 0 &&  block[set][way].dirty == 1) //can't remap 'block[set][way]' to 'block[newset][newway]' & 'block[set][way]' is dirty
        {
                block[set][way].is_on_remap_recursive_call = 0;
                if(block[set][way].valid == 1 && block[set][way].dirty == 1)
                {

                        int channel = uncore.DRAM.dram_get_channel(block[set][way].tag);
                        //check WQ occupancy, if 0, return 0 i.e. vacancy is not available for remapping
                        if (uncore.DRAM.WQ[channel].occupancy >= uncore.DRAM.WQ[channel].SIZE)  //cache level is not checked
                        {
                                // watermark =way;
                                block[set][way].is_on_remap_recursive_call = 0;
                                return 0;
                        }
                        PACKET writeback_packet;
                        writeback_packet.fill_level = FILL_DRAM; // where are we checking the cache level ?
                        writeback_packet.cpu = block[set][way].cpu;
                        curr_addr=block[set][way].tag;
                        writeback_packet.address = curr_addr;
                        full_addr       = (curr_addr << LOG2_BLOCK_SIZE) + (block[set][way].full_addr & 0x3F);
                        writeback_packet.full_addr = full_addr;
                        writeback_packet.data = block[set][way].data;
                        writeback_packet.instr_id = block[set][way].instr_id;
                        writeback_packet.ip = 0; // writeback does not have ip
                        writeback_packet.type = WRITEBACK;
                        writeback_packet.event_cycle = current_core_cycle[block[set][way].cpu];
                        lower_level->add_wq(&writeback_packet);
                        count_remap++;
                }
                 return 1;
        }
        else if(flag == 0 && set == Sptr) //can't remap 'block[set][way]' to 'block[newset][newway]' & 'block[set][way]' is pointed by Sptr hence regardless of whether it is dirty or not, return should be zero. As, it is a valid block which is not remapped yet.
         {
                 block[set][way].is_on_remap_recursive_call = 0;
                 return 0;
         }
         else if(flag == 0 && set != Sptr) //can't remap 'block[set][way]' to 'block[newset][newway]' but 'block[set][way]' is neither pointed by the Sptr and nor dirty so it is simply discarded to make the room for remapping the block which has it as its encrypted location.
         {
                block[set][way].is_on_remap_recursive_call = 0;
                 return 1;
         }
         if(newset >= NUM_SET || newway >= NUM_WAY)
                 assert(0);
         block[set][way].valid = 0; //only the remap block is not valid
        block[newset][newway].valid = 1;
         block[newset][newway].dirty = block[set][way].dirty;
         block[newset][newway].prefetch = block[set][way].prefetch;
         block[newset][newway].used = 0;
         block[newset][newway].delta = block[set][way].delta;
         block[newset][newway].depth = block[set][way].depth;
         block[newset][newway].signature = block[set][way].signature;
         block[newset][newway].confidence = block[set][way].confidence;
         block[newset][newway].tag = block[set][way].tag;
         block[newset][newway].data = block[set][way].data;
         block[newset][newway].cpu = block[set][way].cpu;
         block[newset][newway].instr_id = block[set][way].instr_id;
         block[newset][newway].full_addr = (ela <<  LOG2_BLOCK_SIZE) + (block[set][way].full_addr & 0x3F);
         block[newset][newway].address = ela;
         block[newset][newway].curr_or_next_key = 1;
         if (cache_type == IS_LLC)
         {
                 llc_update_replacement_state(block[newset][newway].cpu, newset,newway, block[newset][newway].full_addr, 1, 0, 0, 1);
         }
          block[set][way].is_on_remap_recursive_call = 0;
          return 1;
}
*/
int CACHE::empty_blocks()
{
	int count_blocks=0;
	for(int i =0;i<NUM_SET;i++)
		for(int j=0;j<NUM_WAY;j++)
			if(block[i][j].valid == 0)
				count_blocks++;

	return count_blocks;
}
void CACHE::remap_set_ceaser_s()
{
	assert(cache_type != IS_ITLB || cache_type != IS_DTLB || cache_type != IS_STLB);
	assert(cache_type==IS_LLC && (CEASER_S_LLC==1)); //Change following line for other caches
	int is_encryption_used_1st_time=0; 
	invalid_blocks_before_remapping += empty_blocks();
	//cout<<"Invalid Blocks Before Remapping : "<<empty_blocks()<<" Sptr : "<<Sptr<<endl;
        for(int way=watermark; way<NUM_WAY; way++)
	{
                			int cur_part=(way/(NUM_WAY/partitions));                
                                        uint32_t newway,newset;
					for(int i=0;i<16;i++)
						next_key[i]=next_keys[cur_part][i];//Copying CEASER_S key in to next_key		
					if(block[Sptr][way].valid==0 || block[Sptr][way].curr_or_next_key==1 )
					{
						//total_blocks_remapped++;
						blocks_less_evicted++;
						continue;
					}
					#if multi_step_relocation
                                        if(remap_block(Sptr,way)==0)
                                        {
                                                watermark =way;
                                                return;
                                        }
					continue;
                                        #endif //*/
					//cout<<"HI \n";
					//BFS on fire
					#if bfs_on
					//cout<<"BFS"<<endl;
					int front = bfs(Sptr,way);

					ela = getEncryptedAddress(pla,block[Sptr][way].cpu,next_key,0);
					newset = ((uint32_t) (ela & ((1 << lg2(NUM_SET*NUM_SLICES)) - 1)))>>lg2(NUM_SLICES);
					
					//if(newset != queue[front].child_set)
					//	exit(0);
					if(front != -1)
						relocation(front);
					else
					{
						front = 0;
						newway = remap_find_victim(block[Sptr][way].cpu, block[Sptr][way].instr_id, queue[front].child_set, block[queue[front].child_set], 0/*ip*/, ela, 0/*type*/,cur_part); 
						//newway=llc_find_victim_ceaser_s(block[Sptr][way].cpu, block[Sptr][way].instr_id, newset, block[newset], 0/*ip*/, ela, 0/*type*/,cur_part);
						copy_block(Sptr,way,queue[front].child_set,newway);	
						
					}
					continue;
					#endif
					//exit(0);
					//cout<<"Hi"<<endl;
					pla = block[Sptr][way].tag;
					if(all_warmup_complete > NUM_CPUS)
                        				decryption_stall_cycle += CEASER_LATENCY;
					ela = getEncryptedAddress(pla,block[Sptr][way].cpu,next_key,0);
					 if(is_encryption_used_1st_time==0 && (all_warmup_complete > NUM_CPUS ))
                                                        {//Only first time encryption and decryption latency will be considered as from the next time the decryption and encryption can happen in parallel with access of blocks
                                                               // cache_stall_cycle +=( 2* CEASER_LATENCY);
							        cache_stall_cycle=0;
                                                                is_encryption_used_1st_time=1;
                                                                total_stall_cycle += (2* CEASER_LATENCY);
                                                        }

					newset =   ((uint32_t) (ela & ((1 << lg2(NUM_SET*NUM_SLICES)) - 1)))>>lg2(NUM_SLICES);
					if(cache_type==IS_LLC && CEASER_S_LLC == 1) 
						//newway = remap_find_victim(block[Sptr][way].cpu, block[Sptr][way].instr_id, newset, block[newset], 0/*ip*/, ela, 0/*type*/,cur_part);
						newway = llc_find_victim_ceaser_s(block[Sptr][way].cpu, block[Sptr][way].instr_id, newset, block[newset], 0/*ip*/, ela, 0/*type*/,cur_part);	
					  //newway = remap_find_victim(block[Sptr][way].cpu, block[Sptr][way].instr_id, newset, block[newset], 0/*ip*/, ela, 0/*type*/,cur_part);  
				       	
					/*if(rrpv_value(Sptr , way) > rrpv_value(newset,newway) &&  rrpv_value(Sptr , way) == 3   && block[Sptr][way].dirty != 1)
					{
						block[Sptr][way].valid=0;
						remap_llc_update_replacement_state(way,Sptr,way,block[Sptr][way].tag); //rrpv is set to max way
						continue;	
					} *///
				     //newway = llc_find_victim_ceaser_s(block[Sptr][way].cpu, block[Sptr][way].instr_id, newset, block[newset], 0/*ip*/, ela, 0/*type*/,cur_part);
					//else
 					//	newway = find_victim(block[Sptr][way].cpu, block[Sptr][way].instr_id, newset, block[newset], 0/*ip*/, ela, 0/*type*/);//Please note that ip and type paramters are not used in find_victim.
					assert(newway!=NUM_WAY);
				#ifdef INCLUSIVE
					if(call_make_inclusive(newset,newway)==0)
					{
						cout<<"OldSet"<<Sptr<<"NewSet"<<newset<<endl;
						cout<<"NewWay"<<newway<<endl;
						assert(0);
					}
				#endif
                    if(Sptr==newset)
                    {
			blocks_less_evicted++;
			block[Sptr][way].valid = 1;
                        block[Sptr][way].curr_or_next_key = 1;
			is_encryption_used_1st_time=0;
                        continue;
                    }
					if(block[newset][newway].valid == 1 && block[newset][newway].dirty == 1)
					{
						PACKET writeback_packet;
						writeback_packet.fill_level = FILL_DRAM;
						writeback_packet.cpu = block[newset][newway].cpu;
						curr_addr=block[newset][newway].tag;
						writeback_packet.address = curr_addr;
						full_addr       = (curr_addr << LOG2_BLOCK_SIZE) + (block[newset][newway].full_addr & 0x3F);
						writeback_packet.full_addr = full_addr;
						writeback_packet.data = block[newset][newway].data;
						writeback_packet.instr_id = block[newset][newway].instr_id;
						writeback_packet.ip = 0; // writeback does not have ip
						writeback_packet.type = WRITEBACK;
						writeback_packet.event_cycle = current_core_cycle[block[newset][newway].cpu];
						int channel = uncore.DRAM.dram_get_channel(block[newset][newway].tag);
						if(uncore.DRAM.WQ[channel].occupancy == uncore.DRAM.WQ[channel].SIZE) //If WQ of DRAM is full then remap will stop and it starts again when the WQ have occupancy
						{
							 watermark =way;
                                                         return ;
						}
						lower_level->add_wq(&writeback_packet);
					}
					#if victim_cache_is_on
                                        //copy_evicted_block to_victim_cache
                                        if(block[newset][newway].valid == 1 && block[newset][newway].dirty != 1)
                                        {
					//	cout<<"WAY : "<<way<<"Valid : " << victim_cache[0][way].valid<<endl;
                                                victim_cache[0][way].valid      = block[newset][newway].valid;
                                                victim_cache[0][way].dirty      = block[newset][newway].dirty;
                                                victim_cache[0][way].prefetch   = block[newset][newway].prefetch;
                                                victim_cache[0][way].used       = block[newset][newway].used;
                                                victim_cache[0][way].delta      = block[newset][newway].delta;
                                                victim_cache[0][way].depth      = block[newset][newway].depth;
                                                victim_cache[0][way].signature  = block[newset][newway].signature;
                                                victim_cache[0][way].confidence = block[newset][newway].confidence;
                                                victim_cache[0][way].tag        = block[newset][newway].tag;
                                                victim_cache[0][way].data       = block[newset][newway].data;
                                                victim_cache[0][way].cpu        = block[newset][newway].cpu;
                                                victim_cache[0][way].instr_id   = block[newset][newway].instr_id;
                                                victim_cache[0][way].full_addr  = block[newset][newway].full_addr;
                                                victim_cache[0][way].address    = block[newset][newway].address;
                                                victim_cache[0][way].ip         = block[newset][newway].ip;
						victim_cache[0][way].curr_or_next_key = block[newset][newway].curr_or_next_key;
					//	cout<<"End"<<endl;
                                        }
                                        #endif	
		    			//Copying the block from old location to new location
                    			block[Sptr][way].valid = 0;
					block[newset][newway].valid = 1;
					block[newset][newway].dirty = block[Sptr][way].dirty;
					block[newset][newway].prefetch = block[Sptr][way].prefetch;
					block[newset][newway].used = 0;
					block[newset][newway].delta = block[Sptr][way].delta;
	                                block[newset][newway].depth = block[Sptr][way].depth;
  				    	block[newset][newway].signature = block[Sptr][way].signature;
					block[newset][newway].confidence = block[Sptr][way].confidence;
				    	block[newset][newway].tag = block[Sptr][way].tag;
					block[newset][newway].data = block[Sptr][way].data;
					block[newset][newway].cpu = block[Sptr][way].cpu;
					block[newset][newway].instr_id = block[Sptr][way].instr_id;
					block[newset][newway].full_addr = (ela <<  LOG2_BLOCK_SIZE) + (block[Sptr][way].full_addr & 0x3F);
					block[newset][newway].address = ela;
					block[newset][newway].curr_or_next_key = 1; //Now its encrypted with next_key
					block[newset][newway].ip = block[Sptr][way].ip; //IP is needed in case of Hawkeye & ShiP replacement policies.
					total_blocks_remapped++;
					if (cache_type == IS_LLC) {
						remap_llc_update_replacement_state(Sptr,way,newset,newway,block[newset][newway].tag);
                                        }
					if(cache_type == IS_LLC && all_warmup_complete > NUM_CPUS)
					{
						cache_stall_cycle=0;
						// cache_stall_cycle += (  (2*LATENCY)-(2*CEASER_LATENCY)  );
						 total_stall_cycle += (   (2*LATENCY)-(2*CEASER_LATENCY)   );
					}
					else if(all_warmup_complete > NUM_CPUS)
					{
						cache_stall_cycle += 2*LATENCY;
					}

	}		//Remap finish
	//accesses_after_remapping[Sptr].full=0;//accesses_after_remapping_that_fills_the_cache_set_again
	//accesses_after_remapping[i].llc_access=0;
        //accesses_after_remapping[i].llc_eviction=0;
	for(int i=0;i<NUM_WAY;i++)
	{
		//cout<<"SPTR : "<<Sptr<<" Way : "<<i<<endl;
		assert(block[Sptr][i].valid == 0 || block[Sptr][i].curr_or_next_key == 1);
	}
	total_sets_remapped++;
       	Actr=Actr-APLR*NUM_WAY;
       	Sptr++;
	//cout<<"Invalid Blocks: "<<empty_blocks()<<" Sptr:"<<Sptr<<endl;
	if(Sptr == NUM_SET) //Resetting the key and Sptr for the next epoch as all sets are remapped
        {
                Sptr = 0;
		for(uint32_t i=0; i<partitions; i++)
        	{
                	for(uint32_t j=0; j<16; j++)
                	{
                        	curr_keys[i][j] = next_keys[i][j];  
                        	next_keys[i][j] = rand() % 256;
                	}
		}
        	for(uint32_t i=0; i<NUM_SET; i++)
                	for(uint32_t j=0; j <NUM_WAY; j++)
                        	block[i][j].curr_or_next_key = 0;
        }
	is_remap_complete=1; //Remap Successful
	//if(Sptr == 15) 	
		//exit(0);
        if(all_warmup_complete > NUM_CPUS)
        	add_stall_cycle_to_queues(); //As the cache is stalled due to remapping all the entries in the response and resquest queues need to wait
	return; 
}

int CACHE::bfs(uint32_t set,int way)
{
	//if (marked_set[set] == 1 || block[set][way].c_or_n == 1 || set < Sptr) 
	//	return 0;
	int front = -1;
	int rear  = -1;
	int threshold = 280000;
	queue[++rear].tag = block[set][way].tag;
	queue[rear].set = set;
	queue[rear].way = way;
	queue[rear].number_of_remap = 1;
	queue[rear].parent_tag = 0;
	
//	cout<<" BFS : "<<endl;
	
	//cout<<"Tag : "<<block[set][way].tag<<" Sptr : "<<Sptr<<" set: "<<set<<" way : "<<way<<endl;
	for(int i=0;i<NUM_SET;i++)
		marked[i]=0;
	uint32_t newset,new_block_set;
	int count=0;
	while( front != rear )
	{
		if(front > threshold)
			return -1;
		pla = queue[++front].tag;
		ela = getEncryptedAddress(pla,0 /*cpu*/,next_key,0);
		newset =   ((uint32_t) (ela & ((1 << lg2(NUM_SET*NUM_SLICES)) - 1)))>>lg2(NUM_SLICES);
		//cout<< " Tag : "<<pla<<" Parent Tag : "<< queue[front].parent_tag<<" newset ----"<<newset<<endl;
		if(marked[newset] == 1)
		{
		//	cout<<"This set is marked : "<<newset<<endl;
		//	cout<<"--------------------------------------"<<endl;
			continue;
		}	
		if(newset < Sptr)
		{
		//	cout<<"newset "<<newset<<" is less than Sptr "<<Sptr<<endl;
		  //     	cout<<"--------------------------------------"<<endl;
			queue[front].child_set = newset;
			continue;	
		}
		if(newset == Sptr)
		{
		//	cout<<"Sptr "<<Sptr<<" and newset "<<newset<<" are same " <<endl;
		//	cout<<"--------------------------------------"<<endl;
			blocks_less_evicted++;
			return front; //got it
		}
		for(int way=0;way < NUM_WAY;way++)
		{
			//cout<<"way : "<<way<<endl;
			if (block[newset][way].valid == 0 || rrpv_value(newset, way) == 3  ) //search ends
			{
				if(block[newset][way].valid == 0)
					blocks_less_evicted++;
				queue[front].child_set = newset;
				queue[front].child_way = way;
				//queue[front].child_tag = block[newset][way].tag;
				//queue[front].parent_tag = pla;

				//cout<<"We find a invalid Block in way "<<way<<endl;
				//cout<<"Number  of remaps required "<<queue[front].number_of_remap<<endl;	
				//cout<<"--------------------------------------"<<endl;
				return front; //got it
			}
			else if(block[newset][way].curr_or_next_key == 1)
				continue;
			else
			{
				//pla = block[newset][way].tag;
                		//ela = getEncryptedAddress(pla,0 /*cpu*/,next_key,0);
               			//new_block_set =   ((uint32_t) (ela & ((1 << lg2(NUM_SET*NUM_SLICES)) - 1)))>>lg2(NUM_SLICES);
				//if(marked[new_block_set] == 1)
				//	continue;
				queue[++rear].set = newset;
				queue[rear].way = way;
				queue[rear].parent_tag = pla;
				queue[rear].tag = block[newset][way].tag;
				queue[rear].number_of_remap  = queue[front].number_of_remap  + 1;
				//cout<<"Added in the queue set : "<<set<< " Way : "<<way<<" Tag : "<<block[newset][way].tag<<endl;
				//cout<<"Front : "<<front<<" element in the queues are : "<<endl;
			//	for(int i=front;i<=rear;i++)
			//		cout<<queue[i].tag<<" ";
			//	cout<<endl;
			}
		}
		marked[newset] = 1;
		//cout<<"New set is marked "<<endl;
		count = 0;
		for(int i=0;i<NUM_SET;i++)
			if(marked[i]==1 || i < Sptr)
				count++;
		if(count == NUM_SET)
			return -1;
	//		cout<<" Marked Set is : "<<i<<endl;
		//cout<<"--------------------------------------"<<endl;
	} 
	return -1;	
}

void CACHE::copy_block(uint32_t oldset,int oldway,uint32_t newset,int newway)
{
//	cout<<"Copy block : ";
//	cout<<"Tag : "<<block[oldset][oldway].tag<<" from set "<<oldset<<" and way : "<<oldway<<" to set "<<newset<<" and way: "<<newway;
        
	block[newset][newway].valid 	= 1;
     	block[newset][newway].dirty 	= block[oldset][oldway].dirty;
     	block[newset][newway].prefetch 	= block[oldset][oldway].prefetch;
      	block[newset][newway].used 	= block[oldset][oldway].used;
      	block[newset][newway].delta 	= block[oldset][oldway].delta;
     	block[newset][newway].depth 	= block[oldset][oldway].depth;
        block[newset][newway].signature = block[oldset][oldway].signature;
       	block[newset][newway].confidence = block[oldset][oldway].confidence;
     	block[newset][newway].tag 	= block[oldset][oldway].tag;
    	block[newset][newway].data 	= block[oldset][oldway].data;
      	block[newset][newway].cpu 	= block[oldset][oldway].cpu;
     	block[newset][newway].instr_id 	= block[oldset][oldway].instr_id;
     	block[newset][newway].full_addr = block[oldset][oldway].full_addr;
      	block[newset][newway].address 	= block[oldset][oldway].address;
     	block[newset][newway].curr_or_next_key = 1; //Now its encrypted with next_key
      	block[newset][newway].ip 	= block[oldset][oldway].ip; //IP is needed in case of Hawkeye & ShiP replacement policies.
	block[oldset][oldway].valid 	= 0;
	remap_llc_update_replacement_state(oldset,oldway,newset,newway,block[newset][newway].tag);
	total_blocks_remapped++;	
//	cout<<"Copy block ends "<<endl;
}
int CACHE::relocation(int front)
{
	/*queue[front].tag 
	queue[front].parent_tag
	queue[front].set
	queue[front].way */
//	cout<<"*****************Relocation Starts********************"<<endl;
	copy_block(queue[front].set,queue[front].way,queue[front].child_set,queue[front].child_way);
	int i=front-1;
        	
	while(i >= 0)
	{
		//cout<<"Tag of i : "<<queue[i].tag<<" Parent Tag of front : "<<queue[front].parent_tag<<endl;;
		if(queue[front].parent_tag == queue[i].tag)
		{
			//cout<<"Front = : "<<front<<" i = : "<<i<<endl;	
			copy_block(queue[i].set,queue[i].way,queue[front].set,queue[front].way);
			front = i;
		}
		i--;
	}
	//cout<<"******************Relocation Ends**********************"<<endl;
}

void CACHE::check_llc_access() //Remaps cache set when LLC accesses/evictions reach a threshold of APLR * NUM_WAYS
{
	#ifdef No_Remapping
		return; //without remap
	#endif
	if(cache_type == IS_ITLB || cache_type == IS_DTLB || cache_type == IS_STLB)
		return;
	if( (cache_type == IS_L1I) || (cache_type == IS_L1D) || (cache_type == IS_L2C) || (CEASER_S_LLC == 0 && cache_type == IS_LLC) )
		return;
	if( all_warmup_complete > NUM_CPUS)
		Actr++;
	//assert(Actr <= APLR*NUM_WAY);
	if(Actr >= 40)     //APLR*NUM_WAY)
	{ 	
		watermark = 0;	
		is_remap_complete = 0;
		if(CEASER_S_LLC ==1 && cache_type == IS_LLC)
			remap_set_ceaser_s();
                                                
	}
}
