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
#include "lru_disp_state_maintainer.hpp"

LoggerPtr DISPStateMaintainerLRU::logger(Logger::getLogger("DISPStateMaintainer"));

DISPStateMaintainerLRU::DISPStateMaintainerLRU(disp_params_t params, FunctionCallGraphObject* fcgo)
{
	disp_parameters = params;
	functions = fcgo;
}

abs_mem_set_t DISPStateMaintainerLRU::update(abs_mem_set_t state, uint32_t function_addr, node_type_t activation_node_type, uint32_t dummy)
{
	return update(&state, function_addr, activation_node_type, dummy);
}

abs_mem_set_t DISPStateMaintainerLRU::update(abs_mem_set_t *state, uint32_t function_addr, node_type_t UNUSED_PARAMETER(activation_node_type), uint32_t UNUSED_PARAMETER(dummy))
{
	abs_mem_set_t new_state = *state;

	LOG_DEBUG(logger, "Updating memory sets for function address: 0x" << hex << function_addr); 

	// update must set, if addr not already in a line
	if(!isInSet(function_addr, &new_state.must_set))
	{
		LOG_DEBUG(logger, "adding 0x" << hex << function_addr << " to must set");
		addAddrToMemSet(function_addr, &(new_state.must_set));
	}
	else
	{
		LOG_DEBUG(logger, "disp hit, moving 0x" << hex << function_addr << " in must set to front.")
		moveAddrToFront(function_addr, &(new_state.must_set), MUST);
	}
	
	// this is not really usefull:
	sortMemSetByAge(&new_state.must_set);

	// update may set
	if(!isInSet(function_addr, &new_state.may_set))
	{
		// TODO: is this method to add/access functions lines correct? 
		LOG_DEBUG(logger, "adding 0x" << hex << function_addr << " to may set");
		addAddrToMemSet(function_addr, &new_state.may_set);
	}
	else
	{
		LOG_DEBUG(logger, "disp hit, moving 0x" << hex << function_addr << " in may set to front.")
		moveAddrToFront(function_addr, &(new_state.may_set), MAY);
	}

	// this is not really useful:
	sortMemSetByAge(&new_state.may_set);

	// this is for evaluation of memory usage only:
	uint64_t allocated_mem_size = sizeof(abs_mem_set_t) + ((new_state.may_set.size()+new_state.must_set.size())*sizeof(abs_mem_entry_t));
	uint64_t maintained_mem_references = new_state.may_set.size() + new_state.must_set.size();
	allocatedMemoryState(2, allocated_mem_size, maintained_mem_references);
	// end of evaluation code

	return new_state;
}

abs_mem_set_t DISPStateMaintainerLRU::join(vector<abs_mem_set_t> states, uint32_t dummy)
{
	return join(&states, dummy);
}

abs_mem_set_t DISPStateMaintainerLRU::join(vector<abs_mem_set_t> *states, uint32_t dummy)
{
	abs_mem_set_t new_state;

	abs_mem_set_t a,b;
	uint32_t no_of_states = states->size();
	if(no_of_states > 2)
	{
		// if there are more than 2 memory states: recursive approach:
		// first join the first n states and then join the resulting state with the last memory state of the parameters
		// TODO Check if this recursive handling is correct and safe!
		LOG_DEBUG(logger,  no_of_states << " mem sets, joining " << (no_of_states-1) << " left hand side mem sets recursively");
		b = states->back();
		states->pop_back();
		a = join(states, dummy);
		LOG_DEBUG(logger,  "Joined the " << (no_of_states-1) << " left hand side mem sets recursively.");
	}
	else
	{
		assert(no_of_states == 2);
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
	// must analysis: add intersection of states with max age.
	///////////////////////////////////////////////////////////
	
	AddrAgeMap mem_state_must_a, mem_state_must_b;
	addAddrAndAgeToMap(&mem_state_must_a, &a.must_set);
	addAddrAndAgeToMap(&mem_state_must_b, &b.must_set);
	
	// merge all unique addresses of both must sets into one vector
	vector<uint32_t> joined_must_mem_addrs;
	joined_must_mem_addrs = getUniqueMemAddrsOfSets(&a.must_set, &b.must_set);
	
	// select the oldest function if it is in both sets, otherwise ignore it in new mem set.
	for(uint32_t i=0; i < joined_must_mem_addrs.size(); i++)
	{
		AddrAgeMap::iterator pos_a, pos_b;
		abs_mem_entry_t entry;
		entry.address = joined_must_mem_addrs[i];
		pos_a = mem_state_must_a.find(joined_must_mem_addrs[i]);
		pos_b = mem_state_must_b.find(joined_must_mem_addrs[i]);

		if((pos_a != mem_state_must_a.end()) && (pos_b != mem_state_must_b.end()))
		{
			if(pos_a->second > pos_b->second)
			{
				entry.age = pos_a->second;
			}
			else
			{
				entry.age = pos_b->second;
			}
			new_state.must_set.push_back(entry);
		}
	}

	// sort the abstract memory set by the max age of the function, to load them into the new memory state in the right order
	sortMemSetByAge(&new_state.must_set);

	if(logger->isDebugEnabled())
	{
		ostringstream s;
		for(uint32_t i=0; i<new_state.must_set.size(); i++)
		{
			s << "0x" << hex << new_state.must_set[i].address << " " << dec << new_state.must_set[i].age << " - " << (new_state.must_set[i].age + getFunctionMemSize(new_state.must_set[i].address)) << ", ";
		}
		LOG_DEBUG(logger, "Abstract MUST functions are: " << s.str());
	}

	///////////////////////////////////////////////////////////
	// may analysis: add union of states with min age.
	///////////////////////////////////////////////////////////

	AddrAgeMap mem_state_may_a, mem_state_may_b;
	addAddrAndAgeToMap(&mem_state_may_a, &a.may_set);
	addAddrAndAgeToMap(&mem_state_may_b, &b.may_set);
	
	// merge all unique addresses of both may sets into one vector
	vector<uint32_t> joined_may_mem_addrs;
	joined_may_mem_addrs = getUniqueMemAddrsOfSets(&a.may_set, &b.may_set);
	
	// select the oldest function if it is in both sets, otherwise ignore it in new mem set.
	for(uint32_t i=0; i < joined_may_mem_addrs.size(); i++)
	{
		AddrAgeMap::iterator pos_a, pos_b;
		abs_mem_entry_t entry;
		entry.address = joined_may_mem_addrs[i];
		pos_a = mem_state_may_a.find(joined_may_mem_addrs[i]);
		pos_b = mem_state_may_b.find(joined_may_mem_addrs[i]);

		bool hit_a = (pos_a != mem_state_may_a.end());
		bool hit_b = (pos_b != mem_state_may_b.end());

		// build up union with min age
		if(hit_a && hit_b)
		{
			if(pos_a->second > pos_b->second)
			{
				entry.age = pos_b->second;
			}
			else
			{
				entry.age = pos_a->second;
			}
		}
		else if(hit_a)
		{
				entry.age = pos_a->second;
		}
		else if(hit_b)
		{
				entry.age = pos_b->second;
		}
		else
		{
			assert(false); // cannot happen, since there has to be a hit somewhere
		}
		new_state.may_set.push_back(entry);
	}

	// sort the abstract memory set by the max age of the function, to load them into the new memory state in the right order
	sortMemSetByAge(&new_state.may_set);

	if(logger->isDebugEnabled())
	{
		ostringstream s;
		for(uint32_t i=0; i<new_state.may_set.size(); i++)
		{
			s << "0x" << hex << new_state.may_set[i].address << " " << dec << new_state.may_set[i].age << " - " << (new_state.may_set[i].age + getFunctionMemSize(new_state.may_set[i].address)) << ", ";
		}
		LOG_DEBUG(logger, "Abstract MAY functions are: " << s.str());
	}

	// this is for evaluation of memory usage only:
	if(no_of_states == 2) // allow the counting of the memory states only ONCE per join (when more than 2 states are joined also only one state hast to be counted!
	{
		uint64_t allocated_mem_size = sizeof(abs_mem_set_t) + ((new_state.may_set.size()+new_state.must_set.size())*sizeof(abs_mem_entry_t));
		uint64_t maintained_mem_references = new_state.may_set.size() + new_state.must_set.size();
		allocatedMemoryState(2, allocated_mem_size, maintained_mem_references);
	}
	// end of evaluation code

	return new_state;
}

uint32_t DISPStateMaintainerLRU::getFunctionMemSize(uint32_t address)
{
	uint32_t f_size = functions->getFunctionSize(address);

	// no function can be found for this entry
	if(f_size == 0)
	{
		LOG_ERROR(logger, "No function found for address ... stopping.");
		assert(false);
	}


	uint32_t f_size_in_mem = ((f_size/disp_parameters.block_size)*disp_parameters.block_size) + ((f_size % disp_parameters.block_size == 0)?(0):(disp_parameters.block_size));

	return f_size_in_mem;
}


void DISPStateMaintainerLRU::printMemSet(ostringstream *os, vector<abs_mem_entry_t> mem_state)
{
	for(uint32_t i = 0; i < mem_state.size(); i++)
	{
		*os << "(0x" << hex << setw(8) << setfill('0') << mem_state[i].address << " " << dec << mem_state[i].age << "-" << mem_state[i].age+getFunctionMemSize(mem_state[i].address) << "/" << getFunctionMemSize(mem_state[i].address) << ")"; 
	}
}

void DISPStateMaintainerLRU::printMemSet(ostringstream *os, vector<abs_mem_entry_t> *mem_state)
{
	for(uint32_t i = 0; i < mem_state->size(); i++)
	{
		*os << "(0x" << hex << setw(8) << setfill('0') << mem_state->at(i).address << " " << dec << mem_state->at(i).age << "-" << mem_state->at(i).age+getFunctionMemSize(mem_state->at(i).address) << "/" << getFunctionMemSize(mem_state->at(i).address) << ")"; 
	}
}

void DISPStateMaintainerLRU::addAddrToMemSet(uint32_t addr, vector<abs_mem_entry_t> *mem_set)
{
	if(getFunctionMemSize(addr) > disp_parameters.size)
	{
		if(disp_parameters.ignore_outsized_functions)
		{
			LOG_INFO(logger, "function: 0x" << hex << addr << " has size of " << dec << getFunctionMemSize(addr) << " bytes, but memory has size of " << disp_parameters.size << " bytes - ignoring it.");
			return;
		}
		else
		{
			LOG_ERROR(logger, "function: 0x" << hex << addr << " has size of " << dec << getFunctionMemSize(addr) << " bytes, but memory has size of " << disp_parameters.size << " bytes!!");
			assert(false);
		}
		assert(false); // remove assert, if this is tested
	}

	uint32_t size_of_new_function = getFunctionMemSize(addr);
	// eviction
	vector<abs_mem_entry_t>::iterator it;
	it = mem_set->begin();
	while(it != mem_set->end())
	{
		LOG_DEBUG(logger, "updating function age from: " << (*it).age << " by " << size_of_new_function);
		(*it).age += size_of_new_function;

		if(((*it).age + getFunctionMemSize((*it).address)) > disp_parameters.size)
		{
			LOG_DEBUG(logger, "Evicting: 0x" << hex << (*it).address << " with age: " << dec << (*it).age << " and size: " << getFunctionMemSize((*it).address));
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

	sortMemSetByAge(mem_set);

	uint32_t used_size = 0;
	for(uint32_t i = 0; i < mem_set->size(); i++)
	{
		used_size += getFunctionMemSize(mem_set->at(i).address);
	}

	LOG_DEBUG(logger, "used size is: " << used_size << " memory size is: " << disp_parameters.size << " bytes");


}

void DISPStateMaintainerLRU::moveAddrToFront(uint32_t f_addr, vector<abs_mem_entry_t> *mem_set, analysis_type_t analysis)
{
	vector<abs_mem_entry_t>::iterator it;
	it = mem_set->begin();

	uint32_t age_of_function = 0;
	uint32_t size_of_function = getFunctionMemSize(f_addr);
	bool found = false;

	//assert(isInSet(f_addr, mem_set));


	// first delete the function that shall be moved front
	while((it != mem_set->end()) && !found)
	{
		if((*it).address == f_addr)
		{
			found = true;
			age_of_function = (*it).age;
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
		uint32_t it_size = getFunctionMemSize((*it).address);
		// have to distinguish between MUST and MAY, since addresses have different semantics (maximal age vs. minimal possible age)
		if(checkFunctionIntersection(age_of_function, age_of_function+size_of_function, (*it).age, (*it).age+it_size))
		{
//			ostringstream os, os2;
//			printMemSet(&os, mem_set);
//			LOG_DEBUG(logger, "Found intersection in mem set: " << os.str() << " with: (0x" << hex << f_addr << dec << " " << age_of_function << "-" << age_of_function+size_of_function << "/" << size_of_function << ") Analysis is: " << ((analysis==MUST)?("MUST"):("MAY")));

			(*it).age = getIntersectionalAge(analysis, age_of_function, size_of_function, (*it).age, it_size);

//			printMemSet(&os2, mem_set);
//			LOG_DEBUG(logger, "New mem set is: " << os2.str() << " mem size=" << disp_parameters.size);
		}
		else if((*it).age < age_of_function)
		{
			(*it).age += size_of_function;
		}
		else 
		{
			// do nothing since the age of the function is higher than the inserted one and there is no intersection
		}


		if(((*it).age + it_size) <= disp_parameters.size)
		{
			it++;
		}
		else
		{
			assert(analysis == MAY); // eviction is only possible in may set and only for the overlapping functions! (REALLY? SEE EQUATION 5.14/5.17 in thesis!) -> case is not observed yet
			it = mem_set->erase(it);
		}
	}

	abs_mem_entry_t tmp;
	tmp.address = f_addr;
	tmp.age = 0;
	mem_set->push_back(tmp);
}

bool inline DISPStateMaintainerLRU::checkFunctionIntersection(uint32_t a_begin, uint32_t a_end, uint32_t b_begin, uint32_t b_end)
{
	bool intersect = false;

	if(a_begin > a_end)
	{
		assert(false);
	}

	if(b_begin > b_end)
	{
		assert(false);
	}

	if((a_begin >= b_begin) && (a_begin < b_end))
	{
		intersect = true;
	}
	else if((b_begin >= a_begin) && (b_begin < a_end))
	{
		intersect = true;
	}

	return intersect;
}


uint32_t DISPStateMaintainerLRU::getIntersectionalAge(analysis_type_t analysis, uint32_t f_age, uint32_t f_size, uint32_t g_age, uint32_t g_size)
{
	// A requirement of this function is that both functions intersect. Otherwise the new age of g has to be calculated differently.
	assert(checkFunctionIntersection(f_age, f_age+f_size, g_age, g_age+g_size));

	uint32_t new_age;


	if(analysis == MUST)
	{
		// max(g_age, f_size + max(f_age - g_size, 0))
		if(f_age > g_size)
		{
			new_age = f_age-g_size;
		}
		else
		{
			new_age = 0;
		}
		new_age += f_size;

		if(new_age < g_age)
		{
			new_age = g_age;
		}
	}
	else if(analysis == MAY)
	{
		// min(f_age + f_size, g_age + f_size)
		if(f_age > g_age)
		{
			new_age = g_age;
		}
		else
		{
			new_age = f_age;
		}
		new_age += f_size;
	}
	else
	{
		assert(false);
	}

	return new_age;


}
