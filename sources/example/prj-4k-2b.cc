////////////////////////////////////////////
//                                        //
//     SRRIP [Jaleel et al. ISCA' 10]     //
//     Jinchun Kim, cienlux@tamu.edu      //
//                                        //
////////////////////////////////////////////
//
#include "../inc/champsim_crc2.h"
#include <stdlib.h>
#include <time.h>

#define NUM_CORE 1
#define MAX_LLC_SETS NUM_CORE*2048
#define LLC_WAYS 16

#define SAT_INC(x,max)  (x<max)?x+1:x
#define SAT_DEC(x)      (x>0)?x-1:x
#define TRUE 1
#define FALSE 0

#define RRIP_OVERRIDE_PERC   0



// The base policy is SRRIP. SHIP needs the following on a per-line basis
#define maxRRPV 3
uint32_t line_rrpv[MAX_LLC_SETS][LLC_WAYS];
uint32_t is_prefetch[MAX_LLC_SETS][LLC_WAYS];
uint32_t fill_core[MAX_LLC_SETS][LLC_WAYS];

// These two are only for sampled sets (we use 64 sets)
#define NUM_LEADER_SETS   64

uint32_t ship_sample[MAX_LLC_SETS];
uint32_t line_reuse[MAX_LLC_SETS][LLC_WAYS];
uint64_t line_sig[MAX_LLC_SETS][LLC_WAYS];
	
// SHCT. Signature History Counter Table
// per-core 16K entry. 14-bit signature = 16k entry. 3-bit per entry
#define maxSHCTR 3
#define SHCT_SIZE (1<<12)
uint32_t SHCT[NUM_CORE][SHCT_SIZE];
bool bypass_enable[NUM_CORE][SHCT_SIZE];

// Statistics
uint64_t insertion_distrib[NUM_TYPES][maxRRPV+1];
uint64_t total_prefetch_downgrades;

// initialize replacement state
void InitReplacementState()
{
    int LLC_SETS = (get_config_number() <= 2) ? 2048 : MAX_LLC_SETS;

    srand(time(NULL));
    cout << "Initialize SRRIP state" << endl;

    for (int i=0; i<MAX_LLC_SETS; i++) {
        for (int j=0; j<LLC_WAYS; j++) {
            line_rrpv[i][j] = maxRRPV;
            line_reuse[i][j] = FALSE;
            is_prefetch[i][j] = FALSE;
            line_sig[i][j] = 0;
        }
    }

    for (int i=0; i<NUM_CORE; i++) {
        for (int j=0; j<SHCT_SIZE; j++) {
            bypass_enable[i][j] = false;
            SHCT[i][j] = 1; // Assume weakly re-use start
        }
    }

    int leaders=0;

    while(leaders<NUM_LEADER_SETS){
      int randval = rand()%LLC_SETS;
      
      if(ship_sample[randval]==0){
	ship_sample[randval]=1;
	leaders++;
      }
    }
}

// find replacement victim
// return value should be 0 ~ 15 or 16 (bypass)
uint32_t GetVictimInSet (uint32_t cpu, uint32_t set, const BLOCK *current_set, uint64_t PC, uint64_t paddr, uint32_t type)
{
    // look for the maxRRPV line
    uint64_t use_PC = (type == PREFETCH ) ? ((PC << 1) + 1) : (PC<<1);
    uint32_t new_sig = use_PC%SHCT_SIZE;
    uint32_t possible_vic = 0;
    uint32_t bypass_it = 14;

    for (int i = 0; i<LLC_WAYS; i++){
        if (line_rrpv[set][i] < (maxRRPV - 1)){
            bypass_it = SAT_DEC(bypass_it);
        }
    }
    
    if ((ship_sample[set] == 0) && (type != WRITEBACK) && (type != PREFETCH) && (SHCT[cpu][new_sig]  == 0) && (bypass_it == 0)){
        return 16;
    }

    while (1)
    {
        bool got = false;
        for (int i=0; i<LLC_WAYS; i++){
            if (line_rrpv[set][i] == maxRRPV) { // found victim
                possible_vic = i;
                got = true;
                break;
            }
        }
        if (got){
            break;
        }
        for (int i=0; i<LLC_WAYS; i++)
            line_rrpv[set][i]++;
    }
    return possible_vic;

    // WE SHOULD NOT REACH HERE
    assert(0);
    return 0;
}

// called on every cache hit and cache fill
void UpdateReplacementState (uint32_t cpu, uint32_t set, uint32_t way, uint64_t paddr, uint64_t PC, uint64_t victim_addr, uint32_t type, uint8_t hit)
{
  uint32_t sig   = line_sig[set][way];

    if (hit) { // update to REREF on hit
        if( type != WRITEBACK ) 
        {

            if( (type == PREFETCH) && is_prefetch[set][way] )
            {
//                line_rrpv[set][way] = 0;
                
                if( (ship_sample[set] == 1) && ((rand()%100 <5) || (get_config_number()==4))) 
                {
                    uint32_t fill_cpu = fill_core[set][way];

                    SHCT[fill_cpu][sig] = SAT_INC(SHCT[fill_cpu][sig], maxSHCTR);
                    line_reuse[set][way] = TRUE;
                }
            }
            else 
            {
                line_rrpv[set][way] = 0;

                if( is_prefetch[set][way] )
                {
                    line_rrpv[set][way] = maxRRPV;
                    is_prefetch[set][way] = FALSE;
                    total_prefetch_downgrades++;
                }
//

                if( (ship_sample[set] == 1) && (line_reuse[set][way]==0) ) 
                {
                    uint32_t fill_cpu = fill_core[set][way];

                    SHCT[fill_cpu][sig] = SAT_INC(SHCT[fill_cpu][sig], maxSHCTR);
                    line_reuse[set][way] = TRUE;
                }
            }
        }
        
	return;
    }
    
    //--- All of the below is done only on misses -------
    // remember signature of what is being inserted
    uint64_t use_PC = (type == PREFETCH ) ? ((PC << 1) + 1) : (PC<<1);
    uint32_t new_sig = use_PC%SHCT_SIZE;
    
    if( ship_sample[set] == 1 ) 
    {
        uint32_t fill_cpu = fill_core[set][way];
        
        // update signature based on what is getting evicted
        if (line_reuse[set][way] == FALSE) { 
            SHCT[fill_cpu][sig] = SAT_DEC(SHCT[fill_cpu][sig]);
        }
        else 
        {
            SHCT[fill_cpu][sig] = SAT_INC(SHCT[fill_cpu][sig], maxSHCTR);
        }

        line_reuse[set][way] = FALSE;
        line_sig[set][way]   = new_sig;  
        fill_core[set][way]  = cpu;
    }



    is_prefetch[set][way] = (type == PREFETCH);

    // Now determine the insertion prediciton

    uint32_t priority_RRPV = maxRRPV-1 ; // default SHIP

    if( type == WRITEBACK )
    {
        line_rrpv[set][way] = maxRRPV;
    }
    else if (SHCT[cpu][new_sig] == 0) {
      line_rrpv[set][way] = (rand()%100>=RRIP_OVERRIDE_PERC)?  maxRRPV: priority_RRPV; //LowPriorityInstallMostly
    }
    else if (SHCT[cpu][new_sig] == maxSHCTR) {
        line_rrpv[set][way] = (type == PREFETCH) ? 1 : 0; // HighPriority Install
    }
    else {
        line_rrpv[set][way] = priority_RRPV; // HighPriority Install 
    }

    // Stat tracking for what insertion it was at
    insertion_distrib[type][line_rrpv[set][way]]++;

}

// use this function to print out your own stats on every heartbeat 
void PrintStats_Heartbeat()
{
}

string names[] = 
{
    "LOAD", "RFO", "PREF", "WRITEBACK" 
};

// use this function to print out your own stats at the end of simulation
void PrintStats()
{
    cout<<"Insertion Distribution: "<<endl;
    for(uint32_t i=0; i<NUM_TYPES; i++) 
    {
        cout<<"\t"<<names[i]<<" ";
        for(uint32_t v=0; v<maxRRPV+1; v++) 
        {
            cout<<insertion_distrib[i][v]<<" ";
        }
        
        cout<<endl;
    }

    cout<<"Total Prefetch Downgrades: "<<total_prefetch_downgrades<<endl;
    
    printf("SHCT values\n");
    for (int i = 0;i < SHCT_SIZE; i++){
        printf("SHCT[%04d]:\t %d\n", i, SHCT[0][i]);
    }
}

