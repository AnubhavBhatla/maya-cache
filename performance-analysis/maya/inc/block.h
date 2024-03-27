#ifndef BLOCK_H
#define BLOCK_H

#include "champsim.h"
#include "instruction.h"
#include "set.h"

// CACHE BLOCK
class BLOCK {
  public:
    uint8_t prefetch,
            used;

    int delta,
        depth,
        signature,
        confidence;

    uint64_t address,
             full_addr,
             tag,
             data,
             cpu,
             instr_id,
	     ip; //For Hawkeye
    int tag_set,tag_way,tag_number;
    // replacement state
    uint32_t lru, valid, rrpv, dirty;
    int is_on_remap_recursive_call;
   // CEASER
    bool curr_or_next_key;

    BLOCK() {
        valid = 0;
        prefetch = 0;
        dirty = 0;
        used = 0;

        delta = 0;
        depth = 0;
        signature = 0;
        confidence = 0;

        address = 0;
        full_addr = 0;
        tag = 0;
        data = 0;
        cpu = 0;
        instr_id = 0;
	ip = 0; //For Hawkeye
        tag_number = -1;
	tag_set = -1;
	tag_way = -1;
	lru = 0;
        curr_or_next_key = 0; //CEASER
	is_on_remap_recursive_call = 0;

    };
};

// DRAM CACHE BLOCK
class DRAM_ARRAY {
  public:
    BLOCK **block;

    DRAM_ARRAY() {
        block = NULL;
    };
};

// message packet
class PACKET {
  public:
    uint8_t instruction, 
            tlb_access,
            scheduled,
            translated,
            fetched,
            prefetched,
            drc_tag_read;

    int fill_level, 
        pf_origin_level,
        rob_signal, 
        rob_index, 
        producer,
        delta,
        depth,
        signature,
        confidence;

    uint32_t pf_metadata;

    uint8_t  is_producer, 
             //rob_index_depend_on_me[ROB_SIZE], 
             //lq_index_depend_on_me[ROB_SIZE], 
             //sq_index_depend_on_me[ROB_SIZE], 
             instr_merged,
             load_merged, 
             store_merged,
             returned,
             asid[2],
             type;

    fastset
             rob_index_depend_on_me, 
             lq_index_depend_on_me, 
             sq_index_depend_on_me;

    uint32_t cpu, data_index, lq_index, sq_index;

    uint64_t address, 
             full_addr, 
             instruction_pa,
             data_pa,
             data,
             instr_id,
             ip, 
             event_cycle,
	     mshr_data_return_cycle,
	     //waiting_time_in_queue,
             cycle_enqueued;

     uint64_t add_cycle_count;

    int forL2;//! @Anuj only to be used at router to determine whether to send the packet to L2(forL2=1) or LLC(forL2=0)
    int router_path[NUM_CPUS]; //Stores router id's of routers in packet path
    int router_path_covered; // Used for setting router path

    PACKET() {
        instruction = 0;
        tlb_access = 0;
        scheduled = 0;
        translated = 0;
        fetched = 0;
        prefetched = 0;
        drc_tag_read = 0;

        returned = 0;
        asid[0] = UINT8_MAX;
        asid[1] = UINT8_MAX;
        type = 0;

        fill_level = -1; 
        rob_signal = -1;
        rob_index = -1;
        producer = -1;
        delta = 0;
        depth = 0;
        signature = 0;
        confidence = 0;

        forL2=-1;//@Anuj
        router_path_covered = 0;
        for (int i = 0; i < NUM_CPUS; ++i)
        {
         router_path[i]=-1;
        }


#if 0
        for (uint32_t i=0; i<ROB_SIZE; i++) {
            rob_index_depend_on_me[i] = 0;
            lq_index_depend_on_me[i] = 0;
            sq_index_depend_on_me[i] = 0;
        }
#endif
        is_producer = 0;
        instr_merged = 0;
        load_merged = 0;
        store_merged = 0;

        cpu = NUM_CPUS;
        data_index = 0;
        lq_index = 0;
        sq_index = 0;

        address = 0;
        full_addr = 0;
        instruction_pa = 0;
        data = 0;
        instr_id = 0;
        ip = 0;
        event_cycle = UINT64_MAX;
	mshr_data_return_cycle=0;
	//waiting_time_in_queue=0;
	cycle_enqueued = 0;
	add_cycle_count = 0;
    };
};

// packet queue
class PACKET_QUEUE {
  public:
    string NAME;
    uint32_t SIZE;

    uint8_t  is_RQ, 
             is_WQ,
             write_mode;

    uint32_t cpu, 
             head, 
             tail, 
             occupancy, 
             num_returned, 
             next_fill_index, 
             next_schedule_index, 
             next_process_index;

    uint64_t next_fill_cycle, 
             next_schedule_cycle, 
             next_process_cycle,
             ACCESS,
             FORWARD,
             MERGED,
             TO_CACHE,
             ROW_BUFFER_HIT,
             ROW_BUFFER_MISS,
             FULL;

    PACKET *entry, processed_packet[2*MAX_READ_PER_CYCLE];

    // constructor
    PACKET_QUEUE(string v1, uint32_t v2) : NAME(v1), SIZE(v2) {
        is_RQ = 0;
        is_WQ = 0;
        write_mode = 0;

        cpu = 0; 
        head = 0;
        tail = 0;
        occupancy = 0;
        num_returned = 0;
        next_fill_index = 0;
        next_schedule_index = 0;
        next_process_index = 0;

        next_fill_cycle = UINT64_MAX;
        next_schedule_cycle = UINT64_MAX;
        next_process_cycle = UINT64_MAX;

        ACCESS = 0;
        FORWARD = 0;
        MERGED = 0;
        TO_CACHE = 0;
        ROW_BUFFER_HIT = 0;
        ROW_BUFFER_MISS = 0;
        FULL = 0;

        entry = new PACKET[SIZE]; 
    };

    PACKET_QUEUE() {
        is_RQ = 0;
        is_WQ = 0;

        cpu = 0; 
        head = 0;
        tail = 0;
        occupancy = 0;
        num_returned = 0;
        next_fill_index = 0;
        next_schedule_index = 0;
        next_process_index = 0;

        next_fill_cycle = UINT64_MAX;
        next_schedule_cycle = UINT64_MAX;
        next_process_cycle = UINT64_MAX;

        ACCESS = 0;
        FORWARD = 0;
        MERGED = 0;
        TO_CACHE = 0;
        ROW_BUFFER_HIT = 0;
        ROW_BUFFER_MISS = 0;
        FULL = 0;

        //entry = new PACKET[SIZE]; 
    };

    // destructor
    ~PACKET_QUEUE() {
        delete[] entry;
    };

    // functions
    int check_queue(PACKET* packet);
    void remove_queue_at_index(int index);
    int first_element_which_need_encryption_engine();
	//find(uint64_t addr);

    void add_queue(PACKET* packet),print_queue(),
         remove_queue(PACKET* packet);
};

// reorder buffer
class CORE_BUFFER {
  public:
    const string NAME;
    const uint32_t SIZE;
    uint32_t cpu, 
             head, 
             tail,
             occupancy,
             last_read, last_fetch, last_scheduled, 
             inorder_fetch[2],
             next_fetch[2],
             next_schedule;
    uint64_t event_cycle,
             fetch_event_cycle,
             schedule_event_cycle,
             execute_event_cycle,
             lsq_event_cycle,
             retire_event_cycle;

    ooo_model_instr *entry;

    // constructor
    CORE_BUFFER(string v1, uint32_t v2) : NAME(v1), SIZE(v2) {
        head = 0;
        tail = 0;
        occupancy = 0;

        last_read = SIZE-1;
        last_fetch = SIZE-1;
        last_scheduled = 0;

        inorder_fetch[0] = 0;
        inorder_fetch[1] = 0;
        next_fetch[0] = 0;
        next_fetch[1] = 0;
        next_schedule = 0;

        event_cycle = 0;
        fetch_event_cycle = UINT64_MAX;
        schedule_event_cycle = UINT64_MAX;
        execute_event_cycle = UINT64_MAX;
        lsq_event_cycle = UINT64_MAX;
        retire_event_cycle = UINT64_MAX;

        entry = new ooo_model_instr[SIZE];
    };

    // destructor
    ~CORE_BUFFER() {
        delete[] entry;
    };
};

// load/store queue 
class LSQ_ENTRY {
  public:
    uint64_t instr_id,
             producer_id,
             virtual_address,
             physical_address,
             ip,
             event_cycle;

    uint32_t rob_index, data_index, sq_index;

    uint8_t translated,
            fetched,
            asid[2];
// forwarding_depend_on_me[ROB_SIZE];
    fastset
		forwarding_depend_on_me;

    // constructor
    LSQ_ENTRY() {
        instr_id = 0;
        producer_id = UINT64_MAX;
        virtual_address = 0;
        physical_address = 0;
        ip = 0;
        event_cycle = 0;

        rob_index = 0;
        data_index = 0;
        sq_index = UINT32_MAX;

        translated = 0;
        fetched = 0;
        asid[0] = UINT8_MAX;
        asid[1] = UINT8_MAX;

#if 0
        for (uint32_t i=0; i<ROB_SIZE; i++)
            forwarding_depend_on_me[i] = 0;
#endif
    };
};

class LOAD_STORE_QUEUE {
  public:
    const string NAME;
    const uint32_t SIZE;
    uint32_t occupancy, head, tail;

    LSQ_ENTRY *entry;

    // constructor
    LOAD_STORE_QUEUE(string v1, uint32_t v2) : NAME(v1), SIZE(v2) {
        occupancy = 0;
        head = 0;
        tail = 0;

        entry = new LSQ_ENTRY[SIZE];
    };

    // destructor
    ~LOAD_STORE_QUEUE() {
        delete[] entry;
    };
};
#endif
