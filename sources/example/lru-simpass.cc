////////////////////////////////////////////
//                                        //
//        LRU replacement policy          //
//                                        //
////////////////////////////////////////////

#include "../inc/champsim_crc2.h"
#include <stdlib.h>
#include <time.h>


#define NUM_CORE 1
#define LLC_SETS NUM_CORE*2048
#define LLC_WAYS 16
#define ENTRIES 4096
#define TABLE_MAX_VALUE 7

uint32_t bp_table[ENTRIES];

uint32_t lru[LLC_SETS][LLC_WAYS];

/*
 bool dice(float p){
    int r = rand();
    float v = (float)r / RAND_MAX;

    if (v <= p){
        return true;
    }
    else{
        return false;
    }
    
}
*/

uint32_t Sat_INC(uint32_t val){
    if (val < TABLE_MAX_VALUE){
        val++;
    }
    else{
        val = val;
    }
    return val;
}

uint32_t Sat_DEC(uint32_t val){
    if (val > 0){
        val--;
    }
    else{
        val = 0;
    }
    return val;
}

// initialize replacement state
void InitReplacementState()
{
    cout << "Initialize LRU replacement state" << endl;

    for (int i=0; i<LLC_SETS; i++) {
        for (int j=0; j<LLC_WAYS; j++) {
            lru[i][j] = j;
        }
    }
    for (int i = 0; i < ENTRIES; i++){
        bp_table[i] = 3;
    }


    srand(time(NULL));
}

// find replacement victim
// victim block's lru counter is 15 (LLC_WAYS - 1).
// return value should be 0 ~ 15 or 16 (bypass)
uint32_t GetVictimInSet (uint32_t cpu, uint32_t set, const BLOCK *current_set, uint64_t PC, uint64_t paddr, uint32_t type)
{
    if (type != WRITEBACK){
        uint32_t addr = PC & 0xFFF; // 4096 entries has 12 address bits
        uint32_t val = bp_table[addr];
        if (val >= 6){
            return 16;
        }
    }
    for (int i=0; i<LLC_WAYS; i++)
        if (lru[set][i] == (LLC_WAYS-1))
            return i;

    return 0;
}

// called on every cache hit and cache fill
void UpdateReplacementState (uint32_t cpu, uint32_t set, uint32_t way, uint64_t paddr, uint64_t PC, uint64_t victim_addr, uint32_t type, uint8_t hit)
{
    uint32_t addr = PC & 0xFFF;
    uint32_t val = bp_table[addr];
    if (type != WRITEBACK){
        if (!hit){
            val = Sat_INC(val);
        }
        else{
            val = Sat_DEC(val);
        }
    }

    bp_table[addr] = val;

    // update lru replacement state
    if(way < 16){
        for (uint32_t i=0; i<LLC_WAYS; i++) {
            if (lru[set][i] < lru[set][way]) {
                lru[set][i]++;

                if (lru[set][i] == LLC_WAYS)
                    assert(0);
            }
        }
        lru[set][way] = 0; // promote to the MRU position
    }
}

// use this function to print out your own stats on every heartbeat 
void PrintStats_Heartbeat()
{

}

// use this function to print out your own stats at the end of simulation
void PrintStats()
{

}
