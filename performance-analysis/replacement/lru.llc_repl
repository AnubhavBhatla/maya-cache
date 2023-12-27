#include "cache.h"
#include "ceaser.h"
// initialize replacement state
void CACHE::llc_initialize_replacement()
{

}

// find replacement victim
uint32_t CACHE::llc_find_victim(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK *current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
    // baseline LRU
    if(CEASER_S_LLC == 1)
	assert(0);
    return lru_victim(cpu, instr_id, set, current_set, ip, full_addr, type); 
}
// LRU FOR CEASER_S
uint32_t CACHE::llc_find_victim_ceaser_s(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK *current_set, uint64_t ip, uint64_t full_addr, uint32_t type,int part)
{
    uint32_t way = 0;

    // fill invalid line first
  //  for (way=0; way<NUM_WAY; way++) {
      for ( way=(part * (NUM_WAY/partitions)); way<NUM_WAY && (way < ((part+1) * (NUM_WAY/partitions))) ; way++)
      {
        if (block[set][way].valid == false) {

            DP ( if (warmup_complete[cpu]) {
            cout << "[" << NAME << "] " << __func__ << " instr_id: " << instr_id << " invalid set: " << set << " way: " << way;
            cout << hex << " address: " << (full_addr>>LOG2_BLOCK_SIZE) << " victim address: " << block[set][way].address << " data: " << block[set][way].data;
            cout << dec << " lru: " << block[set][way].lru << endl; });

            break;
        }
     }

    // LRU victim
 //   if (way == NUM_WAY) {
      if (way == ((part+1) * (NUM_WAY/partitions))) {
        //for (way=0; way<NUM_WAY; way++) {
        for ( way=(part * (NUM_WAY/partitions)); way<NUM_WAY && (way < ((part+1) * (NUM_WAY/partitions))) ; way++)
        { 
//	   if (block[set][way].lru == NUM_WAY-1) {
	   if (block[set][way].lru == (NUM_WAY/partitions)-1) { //lru value must be with in (NUM_WAY/partition)-1).

                DP ( if (warmup_complete[cpu]) {
                cout << "[" << NAME << "] " << __func__ << " instr_id: " << instr_id << " replace set: " << set << " way: " << way;
                cout << hex << " address: " << (full_addr>>LOG2_BLOCK_SIZE) << " victim address: " << block[set][way].address << " data: " << block[set][way].data;
                cout << dec << " lru: " << block[set][way].lru << endl; });

                break;
            }
            }
    	}

    if (way == ((part+1) * (NUM_WAY/partitions))) {
        cout<<"\nLRU values are for part are : " << part<<endl;
        for ( way=(part * (NUM_WAY/partitions)); way<NUM_WAY && (way < ((part+1) * (NUM_WAY/partitions))) ; way++)
            cout<<" "<<block[set][way].lru;
	cout<<endl;
        cerr << "[" << NAME << "] " << __func__ << " no victim! set: " << set << endl;
        assert(0);
    }

    return way;
}

// called on every cache hit and cache fill
void CACHE::llc_update_replacement_state(uint32_t cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type, uint8_t hit)
{
    string TYPE_NAME;
    if (type == LOAD)
        TYPE_NAME = "LOAD";
    else if (type == RFO)
        TYPE_NAME = "RFO";
    else if (type == PREFETCH)
        TYPE_NAME = "PF";
    else if (type == WRITEBACK)
        TYPE_NAME = "WB";
    else
        assert(0);

    if (hit)
        TYPE_NAME += "_HIT";
    else
        TYPE_NAME += "_MISS";

    if ((type == WRITEBACK) && ip)
        assert(0);

    // uncomment this line to see the LLC accesses
    // cout << "CPU: " << cpu << "  LLC " << setw(9) << TYPE_NAME << " set: " << setw(5) << set << " way: " << setw(2) << way;
    // cout << hex << " paddr: " << setw(12) << paddr << " ip: " << setw(8) << ip << " victim_addr: " << victim_addr << dec << endl;

    // baseline LRU
    if (hit && (type == WRITEBACK)) // writeback hit does not update LRU state
        return;
    if(CEASER_S_LLC == 1)
	return llc_update_ceaser_s(set,way);
    return lru_update(set, way);
}
void CACHE::llc_update_ceaser_s(uint32_t set, uint32_t way)
{
    // update lru replacement state
    int part =way/(NUM_WAY/partitions);
    //for (uint32_t i=0; i<NUM_WAY; i++) {
      for (uint32_t i=(part * (NUM_WAY/partitions)); i<NUM_WAY && (i < ((part+1) * (NUM_WAY/partitions))) ; i++) {
	
	  if (block[set][i].lru < block[set][way].lru) {
            block[set][i].lru++;
	    if(block[set][i].lru >= (NUM_WAY/partitions))
		assert(0);
           }
    }
    block[set][way].lru = 0; // promote to the MRU position
}
void CACHE::remap_llc_update_replacement_state(uint32_t oldset, uint32_t way,uint32_t newset,uint32_t newway,uint64_t tag)
{
	llc_update_ceaser_s(newset, newway);
}
void CACHE::llc_replacement_final_stats()
{

}
int CACHE::rrpv_value(uint32_t set, uint32_t way)
{
                return 0; //rrpv[set][way];
}

