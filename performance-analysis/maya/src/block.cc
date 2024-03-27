#include "block.h"

int PACKET_QUEUE::check_queue(PACKET *packet)
{
    if ((head == tail) && occupancy == 0)
        return -1;

    if (head < tail) {
        for (uint32_t i=head; i<tail; i++) {
            if (NAME == "L1D_WQ") {
                if (entry[i].full_addr == packet->full_addr) {
                    DP (if (warmup_complete[packet->cpu]) {
                    cout << "[" << NAME << "] " << __func__ << " cpu: " << packet->cpu << " instr_id: " << packet->instr_id << " same address: " << hex << packet->address;
                    cout << " full_addr: " << packet->full_addr << dec << " by instr_id: " << entry[i].instr_id << " index: " << i;
                    cout << " cycle " << packet->event_cycle << endl; });
                    return i;
                }
            }
            else {
                if (entry[i].address == packet->address) {
                    DP (if (warmup_complete[packet->cpu]) {
                    cout << "[" << NAME << "] " << __func__ << " cpu: " << packet->cpu << " instr_id: " << packet->instr_id << " same address: " << hex << packet->address;
                    cout << " full_addr: " << packet->full_addr << dec << " by instr_id: " << entry[i].instr_id << " index: " << i;
                    cout << " cycle " << packet->event_cycle << endl; });
                    return i;
                }
            }
        }
    }
    else {
        for (uint32_t i=head; i<SIZE; i++) {
            if (NAME == "L1D_WQ") {
                if (entry[i].full_addr == packet->full_addr) {
                    DP (if (warmup_complete[packet->cpu]) {
                    cout << "[" << NAME << "] " << __func__ << " cpu: " << packet->cpu << " instr_id: " << packet->instr_id << " same address: " << hex << packet->address;
                    cout << " full_addr: " << packet->full_addr << dec << " by instr_id: " << entry[i].instr_id << " index: " << i;
                    cout << " cycle " << packet->event_cycle << endl; });
                    return i;
                }
            }
            else {
                if (entry[i].address == packet->address) {
                    DP (if (warmup_complete[packet->cpu]) {
                    cout << "[" << NAME << "] " << __func__ << " cpu: " << packet->cpu << " instr_id: " << packet->instr_id << " same address: " << hex << packet->address;
                    cout << " full_addr: " << packet->full_addr << dec << " by instr_id: " << entry[i].instr_id << " index: " << i;
                    cout << " cycle " << packet->event_cycle << endl; });
                    return i;
                }
            }
        }
        for (uint32_t i=0; i<tail; i++) {
            if (NAME == "L1D_WQ") {
                if (entry[i].full_addr == packet->full_addr) {
                    DP (if (warmup_complete[packet->cpu]) {
                    cout << "[" << NAME << "] " << __func__ << " cpu: " << packet->cpu << " instr_id: " << packet->instr_id << " same address: " << hex << packet->address;
                    cout << " full_addr: " << packet->full_addr << dec << " by instr_id: " << entry[i].instr_id << " index: " << i;
                    cout << " cycle " << packet->event_cycle << endl; });
                    return i;
                }
            }
            else {
                if (entry[i].address == packet->address) {
                    DP (if (warmup_complete[packet->cpu]) {
                    cout << "[" << NAME << "] " << __func__ << " cpu: " << packet->cpu << " instr_id: " << packet->instr_id << " same address: " << hex << packet->address;
                    cout << " full_addr: " << packet->full_addr << dec << " by instr_id: " << entry[i].instr_id << " index: " << i;
                    cout << " cycle " << packet->event_cycle << endl; });
                    return i;
                }
            }
        }
    }

    return -1;
}

void PACKET_QUEUE::print_queue()
{ //Prints Queue (used for debugging)
	
  	if ((head == tail) && occupancy == 0)
      	{ 
	       	cout<<"Occupancy of this queue is 0\n ";
          	return;
	}
    	if (head < tail) 
	{
		cout<<"Entries are : \n";
        	for (uint32_t i=head; i<tail; i++) 
		{
           		cout<<hex<<entry[i].address<<" "<<entry[i].ip<<" "<<dec<<entry[i].event_cycle<<" "<<entry[i].cycle_enqueued<<"\n";
        	}
      	}    
    	else 
	{
        	cout<<"Entries are : \n";
           	for (uint32_t i=head; i<SIZE; i++)
		 	cout<<hex<<entry[i].address<<" "<<entry[i].ip<<" "<<dec<<entry[i].event_cycle<<"\n";
           	for (uint32_t i=0; i<tail; i++) 
		      	cout<<hex<<entry[i].address<<" "<<entry[i].ip<<" "<<dec<<entry[i].event_cycle<<"\n";
    	}

}
void PACKET_QUEUE::remove_queue_at_index(int index)
{ //Remaove the element in the given indxe
	assert(entry[index].address != 0);
	//cout<<"BEFORE -->";
	//cout<<"occupancy: "<<occupancy<<" head: "<<head<<" tail: "<<tail<<" Index : "<<index<<"\n";
	if(head == index)
	{
		//PQ.remove_queue(&PQ.entry[pq_index]);
		PACKET empty_packet;
                entry[index]=empty_packet;
	        head++;
		if(entry[index].address !=0 )
                                assert(0);
    		if (head >= SIZE)
        	head = 0;

	}
	else
	{
		int dec_tail = tail;
		if(tail == 0)
			dec_tail = SIZE -1;
		else
			dec_tail = tail -1;
		if (head < tail)
			{
				int i;
				for( i=index;i<dec_tail;i++)
				{
					entry[i]=entry[i+1];
				}
				
			}
		else
		{
			int i;
			if( index > head)
			{
				for(i=index;i<SIZE;i++)
				{
					entry[i]=entry[(i+1)%SIZE];
				}
				if(tail != 0)
				for(i=0;i<dec_tail;i++)
				{
					entry[i]=entry[i+1];
				}
			}
			else
			{
				for(i=index;i<dec_tail;i++)
                                {
                                        entry[i]=entry[i+1];
                                }
			}
			
		}
		
		if (tail == 0)
		  tail = SIZE-1;
		else 
		  tail--;
		PACKET empty_packet;
		entry[tail]=empty_packet;
	}
		occupancy--;
	 if (entry[tail].address != 0)
	 {
		 cout<<"PQ . address "<<entry[tail].address<<" tail = "<<tail<<" SIZE = "<<SIZE<<"\n";
		 cout<<endl<<endl;
		 assert(0);
	 }
	if(head>= SIZE || tail >= SIZE)
	{
		
		cout<<"occupancy: "<<occupancy<<" head: "<<head<<" tail: "<<tail<<"\n";
		cout<<endl<<endl;
		assert(0);
	}

	if (head < tail && occupancy != 0) 
	{
        	for (uint32_t i=head; i<tail; i++) {
        		if(entry[i].address ==0)
			{
		 		cout<<" Occupancy "<<occupancy<<" Size : "<<SIZE<<" Head : "<<head<<" Tail: "<<tail<<endl;
                     		cout<<endl;
		     		assert(0);

			}
        	}
      	}
    else if (occupancy != 0){
           for (uint32_t i=head; i<SIZE; i++)
             if(entry[i].address ==0)
	     {
		     cout<<" Occupancy "<<occupancy<<" Size : "<<SIZE<<" Head : "<<head<<" Tail: "<<tail<<endl;
		     cout<<endl;
		     assert(0);
	     }
           for (uint32_t i=0; i<tail; i++)
             if(entry[i].address ==0)
		     assert(0);
    }

}
int PACKET_QUEUE::first_element_which_need_encryption_engine()
{ //Return index of the first element in the queue that need encryption engine to access LLC
    if ((head == tail) && occupancy == 0)
        return -1;

    if (head < tail) 
    {
        for (uint32_t i=head; i<tail; i++) 
	{
        	if(entry[i].address !=0 && entry[i].event_cycle == UINT64_MAX)
            		return i;
        }
    }
    else 
    {
               
           for (uint32_t i=head; i<SIZE; i++)
              if(entry[i].address!=0 && entry[i].event_cycle == UINT64_MAX)
           	 return i;
           for (uint32_t i=0; i<tail; i++)
             if(entry[i].address !=0 && entry[i].event_cycle == UINT64_MAX)
	            return i;
    }
return -1;
}

void PACKET_QUEUE::add_queue(PACKET *packet)
{
#ifdef SANITY_CHECK
    if (occupancy && (head == tail))
        assert(0);
#endif

    // add entry
    entry[tail] = *packet;

    DP ( if (warmup_complete[packet->cpu]) {
    cout << "[" << NAME << "] " << __func__ << " cpu: " << packet->cpu << " instr_id: " << packet->instr_id;
    cout << " address: " << hex << entry[tail].address << " full_addr: " << entry[tail].full_addr << dec;
    cout << " head: " << head << " tail: " << tail << " occupancy: " << occupancy << " event_cycle: " << entry[tail].event_cycle << endl; });

    occupancy++;
    tail++;
    if (tail >= SIZE)
        tail = 0;
}

void PACKET_QUEUE::remove_queue(PACKET *packet)
{
#ifdef SANITY_CHECK
    if ((occupancy == 0) && (head == tail))
        assert(0);
#endif

    DP ( if (warmup_complete[packet->cpu]) {
    cout << "[" << NAME << "] " << __func__ << " cpu: " << packet->cpu << " instr_id: " << packet->instr_id;
    cout << " address: " << hex << packet->address << " full_addr: " << packet->full_addr << dec << " fill_level: " << packet->fill_level;
    cout << " head: " << head << " tail: " << tail << " occupancy: " << occupancy << " event_cycle: " << packet->event_cycle << endl; });

    // reset entry
    PACKET empty_packet;
    *packet = empty_packet;

    occupancy--;
    head++;
    if (head >= SIZE)
        head = 0;
}
