#include "Block.h"
#include "../inc/champsim_crc2.h"
#include "TagEntry.h"
#include <deque>

#define LLC_SETS (NUM_CORE * 2048)
#define LLC_WAYS 16

#define TABLE_SIZE (128*NUM_CORE)
#define TABLE_WAY_NUM (16)
#define ep 0.03125

int get_hit_count(uint64_t tag);
void action(uint64_t tag, int hit_count);
void add_tag(uint64_t tag, int hit_count);

int default_value = 0;
Block blocks[LLC_SETS][LLC_WAYS];
std::deque <TagEntry*> table[TABLE_SIZE];
int counter = 1;

int hash_tag_index(uint64_t tag) {
	uint64_t hashed_tag = tag % TABLE_SIZE;
	return hashed_tag;
}
uint64_t hash_tag_tag(uint64_t tag) {
	uint64_t hashed_tag = tag / TABLE_SIZE;
	hashed_tag &= 0xfffff;
	return hashed_tag;
}

// initialize replacement state
void InitReplacementState() {
	srand (time(NULL));
	default_value = 2;
	for (int i = 0; i < LLC_SETS; i++) {
		for (int j = 0; j < LLC_WAYS; j++) {
			blocks[i][j].rrpv = maxRRPV;
			blocks[i][j].hit_count = 0;
		}
	}

}

int get_hit_count(uint64_t tag) {
	uint64_t table_tag = hash_tag_tag(tag);
	int table_index = hash_tag_index(tag);
	int index = -1;
	for(int i = 0; i < (int)table[table_index].size(); i++)
		if(table[table_index][i]->tag == table_tag) {
			index = i;
			break;
		}

	if(index != -1){
		return table[table_index][index]->get_hit_count();
	}
	else {
		return default_value;
	}
}

// find replacement victim
// return value should be 0 ~ 15 or 16 (bypass)
uint32_t GetVictimInSet(uint32_t cpu, uint32_t set, const BLOCK *current_set,
		uint64_t PC, uint64_t paddr, uint32_t type) {
	for(int i = 0; i < LLC_WAYS; i++)
		if(current_set[i].valid == false)
			return i;

	uint64_t tag = (paddr >> (logarithm2(LLC_SETS) + 6));

	int victim_index = -1, min_value = INF;
	double candidate = double(get_hit_count(tag))/HISTORY_SIZE;
	if(candidate < 2){
		if(type!= WRITEBACK){
			//std::cout<<"!\n";
			double p=double(rand())/RAND_MAX;
			if(p>0.2)
				return LLC_WAYS;
		}
	}
	bool flag = true;
	int array_backup[LLC_WAYS];
	for(int i=0;i<LLC_WAYS;i++){
		array_backup[i]=blocks[set][i].rrpv;
	}
	while(flag) {
		for(int i = 0; i < LLC_WAYS; i++)
			if(blocks[set][i].rrpv == maxRRPV) {
				flag = false;
				break;
			}
		if(flag) {
			for(int i = 0; i < LLC_WAYS; i++)
				blocks[set][i].rrpv++;
		}
	}
	int dest_rrip;
	if(set<32){
		dest_rrip = maxRRPV-1;
	}else if(set >= 32 && set < 64){
		double p = double(rand())/RAND_MAX;
		if(p < ep) {
			dest_rrip = maxRRPV-1;
		} else {
			dest_rrip = maxRRPV;
		}
	} else {

		if(counter<0) {
			dest_rrip = maxRRPV-1;
		} else {
			double p=double(rand())/RAND_MAX;
			if(p < ep){
				dest_rrip = maxRRPV-1;
			}else{
				dest_rrip = maxRRPV;
			}

		}

	}
	if(victim_index == -1) {
		for (int i = 0; i < LLC_WAYS; i++) {
			double tmp =double(get_hit_count(current_set[i].tag))/HISTORY_SIZE;
			double value=tmp - blocks[set][i].hit_count;
			if(value<0){
				value=-value;		
			}
			value = value + (maxRRPV - blocks[set][i].rrpv);
			if (value < min_value) {
				victim_index = i;
				min_value = value;
			}
		}
	}
	double incoming_value = double(get_hit_count(tag))/HISTORY_SIZE + (maxRRPV - dest_rrip);
	if (incoming_value < min_value) {
		if(type != WRITEBACK) {
			for(int i=0;i<LLC_WAYS;i++){	
				blocks[set][i].rrpv=array_backup[i];
			}
			return LLC_WAYS;	
		}
	}
	if(blocks[set][victim_index].hit_count != maxHITCOUNT)
		action(current_set[victim_index].tag, blocks[set][victim_index].hit_count);

	blocks[set][victim_index].rrpv = dest_rrip;
	blocks[set][victim_index].hit_count = 0;

	return victim_index;
}

void add_tag(uint64_t tag, int hit_count) {
	uint64_t table_tag=hash_tag_tag(tag);
	int table_index=hash_tag_index(tag);
	TagEntry * temp = new TagEntry();
// 	if (table_index == 0) {
// 		for (int i=0; i<table[table_index].size(); i++) {
// 			std::cout<<table[table_index][i]->tag<<" ,";
// 		}
// 		std::cout<<"\n";
// }
	temp->tag = table_tag;
	temp->add(hit_count);
	if (table[table_index].size() < TABLE_WAY_NUM) {
		table[table_index].push_back(temp);
	} else {
		table[table_index].erase(table[table_index].begin());
		table[table_index].push_back(temp);
	}

}


void action(uint64_t tag, int hit_count) {
	uint64_t table_tag = hash_tag_tag(tag);
	int table_index = hash_tag_index(tag);
	int index = -1;
	for(int i = 0; i < (int)table[table_index].size(); i++)
		if(table[table_index][i]->tag == table_tag) {
			TagEntry * tag_entry = table[table_index][i];
			table[table_index].erase(table[table_index].begin() + i);
			table[table_index].push_back(tag_entry);
			index = table[table_index].size() - 1;
			break;
		}
	if(index != -1) {
		table[table_index][index]->add(hit_count);
	}
	else {
		add_tag(tag, hit_count);
		return;
	}
}

// called on every cache hit and cache fill
void UpdateReplacementState(uint32_t cpu, uint32_t set, uint32_t way,
		uint64_t paddr, uint64_t PC, uint64_t victim_addr, uint32_t type,
		uint8_t hit) {
	if (hit) {
		blocks[set][way].rrpv = 0;
		if (blocks[set][way].hit_count < maxHITCOUNT - 1)
			blocks[set][way].hit_count++;
		else if (blocks[set][way].hit_count == maxHITCOUNT - 1) {
			blocks[set][way].hit_count++;
			uint64_t tag = paddr >> (logarithm2(LLC_SETS) + 6); // 6 bit for block offset
			action(tag, blocks[set][way].hit_count);
		}
	} else {
		if (set < 32) {
			if (counter < 7)
				counter++;
		} else if (set >=32 && set < 64) {
			if(counter > -8)			
				counter--;
		}
		blocks[set][way].hit_count = 0;
	}
}

// use this function to print out your own stats on every heartbeat 
void PrintStats_Heartbeat() {

}

// use this function to print out your own stats at the end of simulation
void PrintStats() {

}
