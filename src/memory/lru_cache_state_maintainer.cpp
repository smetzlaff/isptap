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
#include "lru_cache_state_maintainer.hpp"


LRUCacheStateMaintainer::LRUCacheStateMaintainer(cache_params_t params) : CacheStateMaintainer(params)
{
}

abs_mem_set_t LRUCacheStateMaintainer::update(abs_mem_set_t state, vector<uint32_t> line_addrs, bool use_stats)
{
	return update(&state, &line_addrs, use_stats);
}

abs_mem_set_t LRUCacheStateMaintainer::update(abs_mem_set_t *state, vector<uint32_t> *line_addrs, bool use_stats)
{
	abs_mem_set_t new_state = *state;
	for(uint32_t i = 0; i < line_addrs->size(); i++)
	{
		// update must set, if addr not already in a line
		if(!isInSet(line_addrs->at(i), &new_state.must_set))
		{
			addAddrToMemSet(line_addrs->at(i), &new_state.must_set);
		}
		else
		{
			moveAddrToFront(line_addrs->at(i), &new_state.must_set, MUST);
		}

		// update may set
		if(!isInSet(line_addrs->at(i), &new_state.may_set))
		{
			// TODO: is this method to add/access cache lines correct? (is a hit in the may set (isInSet() == true) a valid reason for not adding a new line with the address? - or should the line pulled to the front?)
			addAddrToMemSet(line_addrs->at(i), &new_state.may_set);
		}
		else
		{
			moveAddrToFront(line_addrs->at(i), &new_state.may_set, MAY);
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

abs_mem_set_t LRUCacheStateMaintainer::update(abs_mem_set_t *state, uint32_t line_addr, bool use_stats)
{
	abs_mem_set_t new_state = *state;
	// update must set, if addr not already in a line
	if(!isInSet(line_addr, &new_state.must_set))
	{
		addAddrToMemSet(line_addr, &new_state.must_set);
	}
	else
	{
		moveAddrToFront(line_addr, &new_state.must_set, MUST);
	}

	// update may set
	if(!isInSet(line_addr, &new_state.may_set))
	{
		// TODO: is this method to add/access cache lines correct? (is a hit in the may set (isInSet() == true) a valid reason for not adding a new line with the address? - or should the line pulled to the front?)
		addAddrToMemSet(line_addr, &new_state.may_set);
	}
	else
	{
		moveAddrToFront(line_addr, &new_state.may_set, MAY);
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


abs_mem_set_t LRUCacheStateMaintainer::join(vector<abs_mem_set_t> states)
{
	return join(&states);
}


abs_mem_set_t LRUCacheStateMaintainer::join(vector<abs_mem_set_t> *states)
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

	uint32_t i;

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
	// must analysis: add intersection of states with max age.
	///////////////////////////////////////////////////////////
	// create set of lines that are in both cache states
	AddrAgeMap mem_state_must_a, mem_state_must_b;
	vector<uint32_t> joined_must_mem_addrs;
	addAddrAndAgeToMap(&mem_state_must_a, &a.must_set);
	addAddrAndAgeToMap(&mem_state_must_b, &b.must_set);

	// merge all unique addresses of both must sets into one vector
	joined_must_mem_addrs = getUniqueMemAddrsOfSets(&a.must_set, &b.must_set);

	if(logger->isDebugEnabled())
	{
		ostringstream s;
		s << "Joined mem addrs for must analysis:";
		printAddresses(&s, &joined_must_mem_addrs);
		LOG_DEBUG(logger, s.str());
	}

	// select cache lines for new mem set that are common in both sets and use the oldest (smalles number) age of both entries
	for(i=0; i < joined_must_mem_addrs.size(); i++)
	{
		AddrAgeMap::iterator pos_a, pos_b;
		abs_mem_entry_t line;
		line.address = joined_must_mem_addrs[i];
		pos_a = mem_state_must_a.find(joined_must_mem_addrs[i]);
		pos_b = mem_state_must_b.find(joined_must_mem_addrs[i]);

		if((pos_a != mem_state_must_a.end()) && (pos_b != mem_state_must_b.end()))
		{
			if(pos_a->second > pos_b->second)
			{
				line.age = pos_a->second;
			}
			else
			{
				line.age = pos_b->second;
			}
			new_state.must_set.push_back(line);
		}
	}

	// sort the abstract cache lines by their max age, to load them into the new memory state in the right order
	sortMemSetByAge(&new_state.must_set);

	if(logger->isDebugEnabled())
	{
		ostringstream s;
		for(i=0; i<new_state.must_set.size(); i++)
		{
			s << "0x" << hex << new_state.must_set[i].address << " " << new_state.must_set[i].age << ", ";
		}
		LOG_DEBUG(logger, "Abstract MUST cache lines are: " << s.str());
	}

	///////////////////////////////////////////////////////////
	// may analysis: add union of states with min age.
	///////////////////////////////////////////////////////////
	// create set of lines that are in at least in one of both cache states
	AddrAgeMap mem_state_may_a, mem_state_may_b;
	vector<uint32_t> joined_may_mem_addrs;
	addAddrAndAgeToMap(&mem_state_may_a, &a.may_set);
	addAddrAndAgeToMap(&mem_state_may_b, &b.may_set);

	// merge all unique addresses of both may sets into one vector
	joined_may_mem_addrs = getUniqueMemAddrsOfSets(&a.may_set, &b.may_set);

	if(logger->isDebugEnabled())
	{
		ostringstream s;
		s << "Joined mem addrs for may analysis:";
		printAddresses(&s, &joined_may_mem_addrs);
		LOG_DEBUG(logger, s.str());
	}

	for(i = 0; i < joined_may_mem_addrs.size(); i++)
	{
		AddrAgeMap::iterator pos_a, pos_b;
		abs_mem_entry_t line;
		line.address = joined_may_mem_addrs[i];
		pos_a = mem_state_may_a.find(joined_may_mem_addrs[i]);
		pos_b = mem_state_may_b.find(joined_may_mem_addrs[i]);

		bool hit_a = (pos_a != mem_state_may_a.end());
		bool hit_b = (pos_b != mem_state_may_b.end());

		// select the min age of the cache line (if a line in in both sets)
		// and push each line with the min age into a vector
		if(hit_a && hit_b)
		{
			if(pos_a->second > pos_b->second)
			{
				line.age = pos_b->second;
			}
			else
			{
				line.age = pos_a->second;
			}
		}
		else if(hit_a)
		{
			line.age = pos_a->second;
		}
		else if(hit_b)
		{
			line.age = pos_b->second;
		}
		else
		{
			assert(false); // cannot happen, since there has to be a hit somewhere
		}
		new_state.may_set.push_back(line);

	}

	// sort the abstract cache lines by their max age, to load them into the new memory state in the right order
	sortMemSetByAge(&new_state.may_set);

	if(logger->isDebugEnabled())
	{
		ostringstream s;
		for(i=0; i<new_state.may_set.size(); i++)
		{
			s << "0x" << hex << new_state.may_set[i].address << " " << new_state.may_set[i].age << ", ";
		}
		LOG_DEBUG(logger, "Abstract MAY cache lines are: " << s.str());
	}

	// this is for evaluation of memory usage only:
	// TODO implement
//	if(use_stats)
	{
		if(states->size() == 2) // allow the counting of the memory states only ONCE per join (when more than 2 states are joined also only one state hast to be counted!
		{
			uint64_t allocated_mem_size = sizeof(abs_mem_set_t) + ((new_state.may_set.size()+new_state.must_set.size())*sizeof(abs_mem_entry_t));
			uint64_t maintained_mem_references = new_state.may_set.size() + new_state.must_set.size();
			allocatedMemoryState(2, allocated_mem_size, maintained_mem_references);
		}
	}
	// end of evaluation code
	
	return new_state;
}

void LRUCacheStateMaintainer::moveAddrToFront(uint32_t addr, vector<abs_mem_entry_t> *mem_set, analysis_type_t analysis)
{
	vector<abs_mem_entry_t>::iterator it;
	it = mem_set->begin();

	uint32_t age_of_addr = 0;
	bool found = false;

	//assert(isInSet(addr, mem_set));

	while((it != mem_set->end()) && !found)
	{
		if((*it).address == addr)
		{
			found = true;
			age_of_addr = (*it).age;
			// delete entry that is changed to age 0
			it = mem_set->erase(it);
		}
		else
		{
			it++;
		}
	}

	assert(found); // should not happen, since this function can be called if the address is in the abstract memory state!

	it = mem_set->begin();
	while(it != mem_set->end())
	{
		// have to distinguish between MUST and MAY, since addresses have different semantics (maximal age vs. minimal possible age)
		if(((analysis == MUST) && ((*it).age < age_of_addr)) || ((analysis == MAY) && ((*it).age <= age_of_addr)))
		{
			(*it).age++;
		}
		it++;
	}

	abs_mem_entry_t tmp;
	tmp.address = addr;
	tmp.age = 0;
	// insert entry with age 0
	mem_set->push_back(tmp);
}

void LRUCacheStateMaintainer::addAddrToMemSet(uint32_t addr, vector<abs_mem_entry_t> *mem_set)
{
	vector<abs_mem_entry_t>::iterator it;
	it = mem_set->begin();

	while(it != mem_set->end())
	{
		(*it).age++;
		if((*it).age >= cache_parameters.line_no)
		{
			it = mem_set->erase(it);
		}
		else
		{
			it++;
		}
	}

	abs_mem_entry_t tmp;
	tmp.address = addr;
	tmp.age = 0;
	mem_set->push_back(tmp);
}

