#include "ring.h"


RING ring;

uint32_t get_slice_num(uint64_t address){
 return (((1 << lg2(NUM_SLICES)) - 1) & (address));
}
/*!
@Anuj
Return value    Action
0:              go anticlockwise i.e, 2-->1-->0-->NUM_SLICES-1
1:              go clockwise i.e, 0-->1-->2 
*/
int get_direction(int source,int destination)
{
    if(source < destination){
        if( (source + (NUM_SLICES - destination)) > (destination - source)  )
            return 1;
        else return 0;
    }else{
        if( ((NUM_SLICES - source) + destination) > (source - destination) )
            return 0;
        else return 1;
    }
}

void ring_DP(PACKET *packet){

cout<<"Debug Print"<<endl;
cout<<"address"<<packet->address<<" forL2 "<< packet->forL2<<"fill_level"<<packet->fill_level<<endl;
cout<<"instr_id"<<packet->instr_id<<" type "<<packet->type; 
cout<<"CPU"<<packet->cpu<<" PATH: ";
for (int i = 0; i < NUM_SLICES; ++i)
{
    cout<<packet->router_path[i]<< " ";
}
cout<<"event_cycle"<<packet->event_cycle<<endl;
}


void RING::operate(){

for (int router_i = 0; router_i < NUM_SLICES; ++router_i)
{
	for (int k = 0 ; k<2 ; k++) // For 2 NI of Router
	{//Handle INQ
    for (int j = 0; j < MAX_READ; ++j)
	{       
    // should there be a check for uninitialised packet? doesn't make much sense since packet in router is added via cache only   
    // PACKET packet_cpu = router[router_i].NI[k].INQ.entry[router[router_i].NI[k].INQ.head];
    uint32_t in_cpu = router[router_i].NI[k].INQ.entry[router[router_i].NI[k].INQ.head].cpu;
    if (in_cpu == NUM_SLICES) //done with this NI's INQ
        break;
    
    //Check if there is a packet in the queue for processing or not
    if ((router[router_i].NI[k].INQ.occupancy > 0) && (router[router_i].NI[k].INQ.entry[router[router_i].NI[k].INQ.head].event_cycle <= current_core_cycle[in_cpu])) 
    {
        int index = router[router_i].NI[k].INQ.head;

        int destination;
        if (router[router_i].NI[k].INQ.entry[index].forL2 == 0)
            destination = get_slice_num(router[router_i].NI[k].INQ.entry[index].address);
        else
            destination = in_cpu;


        if (destination == router_i) //Packet has reached it's destination
        {
            //Resetting network path for this packet
            router[router_i].NI[k].INQ.entry[index].router_path_covered = 0;
            for (int i= 0; i < NUM_SLICES; ++i)
            {
                router[router_i].NI[k].INQ.entry[index].router_path[i] = -1;
            }


            if (router[router_i].NI[k].INQ.entry[index].forL2 == 0) //PACKET is for LLC
            {
                if (router[router_i].NI[k].INQ.entry[index].type == WRITEBACK)
                    {   
                    if (router[router_i].lower_level->WQ.occupancy == router[router_i].lower_level->WQ.SIZE)
                    break;//WB queue full, packet stays in NQ, so we should not process any packet from this queue, but rest queues should be processed. //is //STALL needed?                    
                    else 
                    router[router_i].lower_level->add_wq(&router[router_i].NI[k].INQ.entry[index]);
                    }  
                else if (router[router_i].NI[k].INQ.entry[index].type == PREFETCH) 
                    {
                    if (router[router_i].lower_level->PQ.occupancy == router[router_i].lower_level->PQ.SIZE)
                        break;
                    else router[router_i].lower_level->add_pq(&router[router_i].NI[k].INQ.entry[index]);
                    }
                else if (router[router_i].NI[k].INQ.entry[index].type == LOAD || router[router_i].NI[k].INQ.entry[index].type == RFO)
                    {
                    if (router[router_i].lower_level->RQ.occupancy == router[router_i].lower_level->RQ.SIZE)
                        break;
                    else router[router_i].lower_level->add_rq(&router[router_i].NI[k].INQ.entry[index]);   
                    }      
            }
            else //PACKET is for L2 
            {
                router[router_i].upper_level->return_data(&router[router_i].NI[k].INQ.entry[index]);
            }
        }
        else //Packet has to goto Network. We take the packet from INQ and put it in OUTQ.
        {
            int direction = get_direction(router_i,destination);
            if (k==0) 
            {
                if (direction == 0) 
                {
                    ring_DP(&router[router_i].NI[k].INQ.entry[index]);
                    assert(0);
                }

                if (router[router_i].NI[1].OUTQ.occupancy == router[router_i].NI[1].OUTQ.SIZE)
                {
                    //STALL[router[router_i].NI[k].INQ.entry[index].type]++;
                    cout << " Router OUTQ filled! router_id:" << router_i << "packet address , cpu packet type, forL2   "<<router[router_i].NI[k].INQ.entry[index].address << router[router_i].NI[k].INQ.entry[index].cpu << router[router_i].NI[k].INQ.entry[index].type << router[router_i].NI[k].INQ.entry[index].forL2 ;        
                }
                else{
                router[router_i].NI[1].add_outq(&router[router_i].NI[k].INQ.entry[index]);                                    
                router[router_i].NI[k].INQ.entry[index].router_path[router[router_i].NI[k].INQ.entry[index].router_path_covered++]=router_i;
                }

            }
            else if(k == 1)
            {
                if (direction == 1) //Packet is looping 
                {
                    //cout<<",router_i "<<router_i<<"destination "<<destination<<"direction "<<direction;
                    ring_DP(&router[router_i].NI[k].INQ.entry[index]);
                    assert(0);
                }                

                if (router[router_i].NI[0].OUTQ.occupancy == router[router_i].NI[0].OUTQ.SIZE)
                {
                    //STALL[router[router_i].NI[k].INQ.entry[index].type]++;
                    cout << " Router OUTQ filled! router_id:" << router_i << "packet address, cpu packet type, forL2   "<< router[router_i].NI[k].INQ.entry[index].address<<router[router_i].NI[k].INQ.entry[index].cpu << router[router_i].NI[k].INQ.entry[index].type << router[router_i].NI[k].INQ.entry[index].forL2 ;        
                }
                else{
                router[router_i].NI[k].INQ.entry[index].router_path[router[router_i].NI[k].INQ.entry[index].router_path_covered++]=router_i;
                router[router_i].NI[0].add_outq(&router[router_i].NI[k].INQ.entry[index]);
                }
            }
        }
    router[router_i].NI[k].INQ.remove_queue(&router[router_i].NI[k].INQ.entry[index]); //Removing processed packet from router queue        
        }

    } //MAX READ loop end

    //Handling OUTQ
    for (int j = 0; j < MAX_READ; ++j)
    {
    uint32_t out_cpu = router[router_i].NI[k].OUTQ.entry[router[router_i].NI[k].OUTQ.head].cpu;

    if (out_cpu == NUM_SLICES)
    break;
    //Check if there is a packet in the queue for processing or not
    if ((router[router_i].NI[k].OUTQ.occupancy > 0) && (router[router_i].NI[k].OUTQ.entry[router[router_i].NI[k].OUTQ.head].event_cycle <= current_core_cycle[out_cpu])) 
    {
    
    int destination;    
    int index = router[router_i].NI[k].OUTQ.head;
    
    if (router[router_i].NI[k].OUTQ.entry[index].forL2 == 0)
    destination = get_slice_num(router[router_i].NI[k].OUTQ.entry[index].address);
    else destination = out_cpu;

    if(k==1)
    {
        if (get_direction(router_i,destination) == 0)
            {
                ring_DP(&router[router_i].NI[k].OUTQ.entry[index]);
                //cout<< "DP" << router_i << destination << get_direction(router_i,destination) <<current_core_cycle[router_i] << endl;
                assert(0);       
            }
        if(router_i == NUM_SLICES-1)
            {
                if (router[0].NI[0].INQ.occupancy == router[0].NI[0].INQ.SIZE)
                {
                    //STALL[router[0].NI[0].type]++;
                    cout<< "Router INQ filled. router_id: "<< 0 << "Packet address cpu type forL2 " <<router[router_i].NI[k].OUTQ.entry[index].address << router[router_i].NI[k].OUTQ.entry[index].cpu << router[router_i].NI[k].OUTQ.entry[index].type <<router[router_i].NI[k].OUTQ.entry[index].forL2 ; 
                }
                else{
                links[router_i]++;
                router[0].NI[0].add_inq(&router[router_i].NI[k].OUTQ.entry[index]);                        
                }

            }
            else
            {
                if (router[router_i+1].NI[0].INQ.occupancy == router[router_i+1].NI[0].INQ.SIZE)
                {
                    //STALL[router[router_i].NI[k].OUTQ.entry[index].type]++;
                    cout<< "Router INQ filled. router_id: "<< router_i+1 << "Packet address cpu type forL2 " <<router[router_i].NI[k].OUTQ.entry[index].address << router[router_i].NI[k].OUTQ.entry[index].cpu << router[router_i].NI[k].OUTQ.entry[index].type <<router[router_i].NI[k].OUTQ.entry[index].forL2 ;                    
                }
                else{
                links[router_i]++;
                router[router_i+1].NI[0].add_inq(&router[router_i].NI[k].OUTQ.entry[index]);                    
                }

            }
    }
    else if(k==0){
            if(get_direction(router_i,destination) ==1)//!Packet is looping
            {
                ring_DP(&router[router_i].NI[k].OUTQ.entry[index]);
                //cout<< "DP" << router_i << destination << get_direction(router_i,destination) <<current_core_cycle[router_i]<< endl;
                assert(0);   
            }
            if(router_i == 0)
            {
                links[NUM_SLICES-1]++;
                router[NUM_SLICES-1].NI[1].add_inq(&router[router_i].NI[k].OUTQ.entry[index]);
            }             
            else
            {
                links[router_i-1]++;
                router[router_i-1].NI[1].add_inq(&router[router_i].NI[k].OUTQ.entry[index]);        
            }
    }


    router[router_i].NI[k].OUTQ.remove_queue(&router[router_i].NI[k].OUTQ.entry[index]);
    }
    }

}
}
}
/*!
This function is called by L2/LLC cache or from ring.operate() to inject a packet into router's NI inqueue. 
@param Packet to be inserted
*/
int NetworkInterface::add_inq(PACKET *packet){
    if (!packet)//Invalid packet
    assert(0);
    if (INQ.check_queue(packet) != -1)//Queue already has packet
    {
        //Packet already in NQ, should it fail with assert(0) or continue by doing some merging?
       int index =  INQ.check_queue(packet);
       cout<< "\nPacket already present in Network, index in INQ "<<index<<endl; 
       cout<<"Packet present in Network\n";
       ring_DP(&INQ.entry[index]);
       cout<<"\nPacket Trying to insert in Network\n";
       ring_DP(packet);
       // assert(packet->type != INQ.entry[index].type);
       assert(0);

    }
    // else{        //Add bookeeping info here
    if(INQ.occupancy >= INQ.SIZE)
        return -2;
    int index = INQ.tail;
    INQ.entry[index] = *packet;

    if (INQ.entry[index].event_cycle < current_core_cycle[packet->cpu])
        INQ.entry[index].event_cycle = current_core_cycle[INQ.entry[index].cpu] + QUEUE_PROC_LATENCY;
    else
        INQ.entry[index].event_cycle += QUEUE_PROC_LATENCY;

    INQ.occupancy++;
    INQ.tail++;

    if (INQ.tail >=INQ.SIZE)
    {INQ.tail =0;}


    // }
    return 0;
}

int NetworkInterface::add_outq(PACKET *packet){
    if (!packet)//Invalid packet
    assert(0);

    if(OUTQ.check_queue(packet) != -1)
    {
       int index =  OUTQ.check_queue(packet);
       cout<< "\nPacket already present in Network, index in OUTQ "<<index<<endl; 
       cout<<"Packet present in Network\n";
       ring_DP(&OUTQ.entry[index]);
       cout<<"\nPacket Trying to insert in Network\n";
       ring_DP(packet);
       // assert(packet->type != OUTQ.entry[index].type); //Packet already in NQ, should it fail with assert(0) or continue.       
       assert(0);
    }
    // else{        //Add bookeeping information here
    if(OUTQ.occupancy >= OUTQ.SIZE)
        return -2;    
    int index = OUTQ.tail;
    OUTQ.entry[index] = *packet;
    //Add OUTQ processing latency
    // if (OUTQ.entry[index].event_cycle < current_core_cycle[packet->cpu])
    //     OUTQ.entry[index].event_cycle = current_core_cycle[packet->cpu] + QUEUE_PROC_LATENCY;
    // else
    //     OUTQ.entry[index].event_cycle += QUEUE_PROC_LATENCY;

    OUTQ.occupancy++;
    OUTQ.tail++;

    if (OUTQ.tail >=OUTQ.SIZE)
    {OUTQ.tail =0;}

    // }    
    return 0;
}
