/*******************************************************************************
 * ISPTAP - Instruction Scratchpad Timing Analysis Program
 * Copyright (C) 2013 Stefan Metzlaff, University of Augsburg, Germany
 * URL: <https://github.com/smetzlaff/isptap>
 * 
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program, see <LICENSE>. If not, see
 * <http://www.gnu.org/licenses/>.
 ******************************************************************************/
#include "mem_state_maintainer.hpp"

// for setw() and setfill()
using std::setw;
using std::setfill;

bool sort_by_age(abs_mem_entry_t a, abs_mem_entry_t b){bool ret_val; if(a.age != b.age) {ret_val = a.age < b.age;} else {ret_val = a.address < b.address;} return ret_val;}

bool MemStateMaintainer::isInSet(uint32_t addr_to_find, vector<abs_mem_entry_t> mem_set)
{
	return isInSet(addr_to_find, &mem_set);
}

bool MemStateMaintainer::isInSet(uint32_t addr_to_find, vector<abs_mem_entry_t> *mem_set)
{
	bool found = false;
	for(uint32_t i = 0; (i < mem_set->size())&&(!found); i++)
	{
		if(addr_to_find == mem_set->at(i).address)
		{
			found = true;
		}
	}
	return found;
}



void MemStateMaintainer::printMemSet(ostringstream *os, vector<abs_mem_entry_t> mem_state)
{
	printMemSet(os, &mem_state);
}

void MemStateMaintainer::printMemSet(ostringstream *os, vector<abs_mem_entry_t> *mem_state)
{
	for(uint32_t i = 0; i < mem_state->size(); i++)
	{
		*os << "(0x" << hex << setw(8) << setfill('0') << mem_state->at(i).address << " " << dec << mem_state->at(i).position <<  ")"; 
	}
}


void MemStateMaintainer::printAddresses(ostringstream *os, vector<uint32_t> addrs)
{
	printAddresses(os, &addrs);
}

void MemStateMaintainer::printAddresses(ostringstream *os, vector<uint32_t> *addrs)
{
	for(uint32_t i = 0; i < addrs->size(); i++)
	{
		*os << "(0x" << hex << setw(8) << setfill('0') << addrs->at(i) <<  ")"; 
	}
}

vector<uint32_t> MemStateMaintainer::getUniqueMemAddrsOfSets(vector<abs_mem_entry_t> set_a, vector<abs_mem_entry_t> set_b)
{
	return getUniqueMemAddrsOfSets(&set_a, &set_b);
}

vector<uint32_t> MemStateMaintainer::getUniqueMemAddrsOfSets(vector<abs_mem_entry_t> *set_a, vector<abs_mem_entry_t> *set_b)
{
	vector<uint32_t> result;
	uint32_t i;
	for(i = 0; i < set_a->size(); i++)
	{
		if(set_a->at(i).address != 0) // ignore blank element
		{
			result.push_back(set_a->at(i).address);
		}
	}
	for(i = 0; i < set_b->size(); i++)
	{
		if(set_b->at(i).address != 0) // ignore blank element
		{
			// ensure that an address is not twice in the vector
			bool addr_already_in_result = false;
			for(uint32_t j = 0; (j< result.size())&&(!addr_already_in_result); j++)
			{
				if(set_b->at(i).address == result[j])
				{
					addr_already_in_result = true;
				}
			}
			if(!addr_already_in_result)
			{
				result.push_back(set_b->at(i).address);
			}
		}
	}
	return result;
}



void MemStateMaintainer::addUniqueMemAddrsToList(vector<uint32_t> *addr_set, vector<abs_mem_entry_t> mem_set)
{
	return addUniqueMemAddrsToList(addr_set, &mem_set);
}

void MemStateMaintainer::addUniqueMemAddrsToList(vector<uint32_t> *addr_set, vector<abs_mem_entry_t> *mem_set)
{
	vector<uint32_t> result;

	for(uint32_t i = 0; i < mem_set->size(); i++)
	{
		if(mem_set->at(i).address != 0) // ignore blank element
		{
			// ensure that an address is not twice in the vector
			bool addr_already_in_result = false;
			for(uint32_t j = 0; (j< addr_set->size())&&(!addr_already_in_result); j++)
			{
				if(mem_set->at(i).address == addr_set->at(j))
				{
					addr_already_in_result = true;
				}
			}
			if(!addr_already_in_result)
			{
				addr_set->push_back(mem_set->at(i).address);
			}
		}
	}
}

void MemStateMaintainer::sortMemSetByAge(vector<abs_mem_entry_t> *mem_set)
{
	sort(mem_set->begin(), mem_set->end(), sort_by_age);
}

void MemStateMaintainer::sortMemSetByPosition(vector<abs_mem_entry_t> *mem_set)
{
	sortMemSetByAge(mem_set);
}

void MemStateMaintainer::addAddrAndAgeToMap(AddrAgeMap *map, vector<abs_mem_entry_t> mem_set)
{
	addAddrAndAgeToMap(map, &mem_set);
}

void MemStateMaintainer::addAddrAndAgeToMap(AddrAgeMap *map, vector<abs_mem_entry_t> *mem_set)
{
	bool ins;
	AddrAgeMap::iterator pos;
	for(uint32_t i = 0; i < mem_set->size(); i++)
	{
		if(mem_set->at(i).address != 0) // ignore blank element
		{
			tie(pos,ins) = map->insert(make_pair(mem_set->at(i).address, mem_set->at(i).age));
			assert(ins);
		}
	}
}

void MemStateMaintainer::setBlankMemState(vector<abs_mem_entry_t> *initial_set_must, vector<abs_mem_entry_t> *initial_set_may)
{
	initial_set_must->clear();
	initial_set_may->clear();
	// this is for evaluation of memory usage only:
	allocatedMemoryState(2, 2*sizeof(vector<abs_mem_entry_t>), 0);
	// end of evaluation code
}

