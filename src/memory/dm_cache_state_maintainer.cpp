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
#include "dm_cache_state_maintainer.hpp"

#define get_block_addr(a) (a >> cache_parameters.line_size_bit)

DirectMappedCacheStateMaintainer::DirectMappedCacheStateMaintainer(cache_params_t params) : CacheStateMaintainer(params)
{
}

abs_mem_set_t DirectMappedCacheStateMaintainer::join(vector<abs_mem_set_t> states)
{
	return join(&states);
}

abs_mem_set_t DirectMappedCacheStateMaintainer::join(vector<abs_mem_set_t> *states)
{
	abs_mem_set_t new_state;
	abs_mem_set_t a,b;
	if(states->size() > 2)
	{
		// if there are more than 2 memory states: recursive approach:
		// first join the first n states and then join the resulting state with the last memory state of the parameters
		// TODO Check if this recursive handling is correct and safe!
		LOG_DEBUG(logger,  states->size() << " mem sets, joining " << states->size()-1 << " left hand side mem sets recursively");
		b = states->back();
		states->pop_back();
		a = join(states);
		LOG_DEBUG(logger,  "Joined the " << states->size() << " left hand side mem sets recursively.");
	}
	else
	{
		assert(states->size() == 2);
		a = states->at(0);
		b = states->at(1);
	}

	if(logger->isDebugEnabled())
	{
		ostringstream s1, s2;

		s1 << "Must set A: ";
		printMemSet(&s1, &a.must_set);

		
		s2 << "Must set B: ";
		printMemSet(&s2, &b.must_set);

		LOG_DEBUG(logger, "Joining must sets for node");
		LOG_DEBUG(logger, s1.str());
		LOG_DEBUG(logger, s2.str());

		ostringstream s3, s4;

		s3 << "May set A: ";
		printMemSet(&s3, &a.may_set);

		
		s4 << "May set B: ";
		printMemSet(&s4, &b.may_set);

		LOG_DEBUG(logger, "Joining may sets for node");
		LOG_DEBUG(logger, s3.str());
		LOG_DEBUG(logger, s4.str());
	}

	///////////////////////////////////////////////////////////
	// must analysis: intersection of states 
	// (positions in mem state represent the calculated index and cannot be moved)
	///////////////////////////////////////////////////////////
	
	vector<uint32_t> joined_must_mem_addrs;
	joined_must_mem_addrs = getUniqueMemAddrsOfSets(&a.must_set, &b.must_set);

	for(uint32_t i = 0; i < joined_must_mem_addrs.size(); i++)
	{
		if((isInSet(joined_must_mem_addrs[i], &a.must_set)) && (isInSet(joined_must_mem_addrs[i], &b.must_set)))
		{
			addAddrToMemSet(joined_must_mem_addrs[i], &new_state.must_set, false);
		}
	}
	sortMemSetByPosition(&new_state.must_set);

	///////////////////////////////////////////////////////////
	// may analysis: union of states
	// (positions in mem state represent the calculated index and cannot be moved)
	///////////////////////////////////////////////////////////
	

	vector<uint32_t> joined_may_mem_addrs;
	joined_may_mem_addrs = getUniqueMemAddrsOfSets(&a.may_set, &b.may_set);

	// if it is in the list it is either in set a or in set b, so add every unique address
	for(uint32_t i = 0; i < joined_may_mem_addrs.size(); i++)
	{
		addAddrToMemSet(joined_may_mem_addrs[i], &new_state.may_set, false);
	}
	sortMemSetByPosition(&new_state.may_set);


	// this is for evaluation of memory usage only:
	uint64_t allocated_mem_size = sizeof(abs_mem_set_t) + ((new_state.may_set.size()+new_state.must_set.size())*sizeof(abs_mem_entry_t));
	uint64_t maintained_mem_references = new_state.may_set.size() + new_state.must_set.size();
	allocatedMemoryState(2, allocated_mem_size, maintained_mem_references);
	// end of evaluation code

	return new_state;
}


abs_mem_set_t DirectMappedCacheStateMaintainer::update(abs_mem_set_t state, vector<uint32_t> line_addrs, bool use_stats)
{
	return update(&state, &line_addrs, use_stats);
}


abs_mem_set_t DirectMappedCacheStateMaintainer::update(abs_mem_set_t *state, vector<uint32_t> *line_addrs, bool use_stats)
{
	abs_mem_set_t new_state = *state;
	for(uint32_t i = 0; i < line_addrs->size(); i++)
	{
		// update must set, if addr not already in a line
		if(!isInSet(line_addrs->at(i), &new_state.must_set))
		{
			addAddrToMemSet(line_addrs->at(i), &new_state.must_set, true);
		}
		else
		{
			// this is not necessary, because one index can contain only have one possible address
			addAddrToMemSet(line_addrs->at(i), &new_state.must_set, true);
		}

		// update may set
		if(!isInSet(line_addrs->at(i), &new_state.may_set))
		{
			addAddrToMemSet(line_addrs->at(i), &new_state.may_set, true);
		}
		else
		{
			// if multiple addresses may in the same cache line, the addresses that are not accessed are definitely evicted
			addAddrToMemSet(line_addrs->at(i), &new_state.may_set, true);
		}
	}

	// this is for evaluation of memory usage only:
	if(use_stats)
	{
		uint64_t allocated_mem_size = sizeof(abs_mem_set_t) + ((new_state.may_set.size()+new_state.must_set.size())*sizeof(abs_mem_entry_t));
		uint64_t maintained_mem_references = new_state.may_set.size() + new_state.must_set.size();
		allocatedMemoryState(2, allocated_mem_size, maintained_mem_references);
	}
	// end of evaluation code
	
	return new_state;
}

abs_mem_set_t DirectMappedCacheStateMaintainer::update(abs_mem_set_t *state, uint32_t line_addr, bool use_stats)
{
	abs_mem_set_t new_state = *state;
	// update must set, if addr not already in a line
	if(!isInSet(line_addr, &new_state.must_set))
	{
		addAddrToMemSet(line_addr, &new_state.must_set, true);
	}
	else
	{
		// this is not necessary, because one index can contain only have one possible address
		addAddrToMemSet(line_addr, &new_state.must_set, true);
	}

	// update may set
	if(!isInSet(line_addr, &new_state.may_set))
	{
		addAddrToMemSet(line_addr, &new_state.may_set, true);
	}
	else
	{
		// if multiple addresses may in the same cache line, the addresses that are not accessed are definitely evicted
		addAddrToMemSet(line_addr, &new_state.may_set, true);
	}

	// this is for evaluation of memory usage only:
	if(use_stats)
	{
		uint64_t allocated_mem_size = sizeof(abs_mem_set_t) + ((new_state.may_set.size()+new_state.must_set.size())*sizeof(abs_mem_entry_t));
		uint64_t maintained_mem_references = new_state.may_set.size() + new_state.must_set.size();
		allocatedMemoryState(2, allocated_mem_size, maintained_mem_references);
	}
	// end of evaluation code
	
	return new_state;
}

vector<uint32_t> DirectMappedCacheStateMaintainer::addAddrToMemSet(uint32_t addr, vector<abs_mem_entry_t> *mem_set, bool evict)
{
	uint32_t index = get_block_addr(addr) % cache_parameters.line_no;
	vector<uint32_t> evicted_addrs;
	abs_mem_entry_t tmp;
	tmp.position = index;
	tmp.address = addr;

	//	LOG_DEBUG(logger, "Using cache line: " << index << " of " << cache_parameters.line_no);

	if(evict)
	{
		// erase all entries with that index of the cache line to add (it is not distinguished if the cache line is already present in cache or not)
		vector<abs_mem_entry_t>::iterator it;
		it = mem_set->begin();

		while(it != mem_set->end())
		{
			if((*it).position == index)
			{
				evicted_addrs.push_back((*it).address);
				it = mem_set->erase(it);
			}
			else
			{
				it++;
			}
		}

		// (re-)add the cache line
		mem_set->push_back(tmp);

		// sort:
		sortMemSetByPosition(mem_set);
	}
	else
	{
		// just add the cache line if it is not already present.
		if(!isInSet(addr, mem_set))
		{
			mem_set->push_back(tmp);
			// sort:
			sortMemSetByPosition(mem_set);
		}
	}

	if(!evicted_addrs.empty())
	{
		ostringstream str;
		printAddresses(&str, &evicted_addrs);

		LOG_DEBUG(logger, "Evicted cache lines: " << str.str());
	}

	return evicted_addrs;
}

