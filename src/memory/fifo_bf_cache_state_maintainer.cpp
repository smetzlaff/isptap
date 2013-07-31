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
#include "fifo_bf_cache_state_maintainer.hpp"

LoggerPtr FIFOBFCacheStateMaintainer::logger(Logger::getLogger("FIFO_CSM"));

FIFOBFCacheStateMaintainer::FIFOBFCacheStateMaintainer()
{
	state_map.clear();
}


FIFOBFCacheStateMaintainer::~FIFOBFCacheStateMaintainer()
{
	for(NodeMemSetMap::iterator it = state_map.begin(); it != state_map.end(); it++)
	{
		// free all entries
		delete(((*it).second)); // if transfer() is used one address could be multiple times in the map, then this line may cause an exception
	}
	state_map.clear();
}



FIFOBFCacheStateMaintainer::FIFOBFCacheStateMaintainer(cache_params_t params)
{
	cache_parameters = params;
	assert((cache_parameters.size != 0) && (cache_parameters.line_size != 0) && (cache_parameters.line_size_bit != 0));
}

void FIFOBFCacheStateMaintainer::update(MSGVertex predecessor, MSGVertex node, vector<uint32_t> *line_addrs)
{
	AbsMemSet *new_state;
	new_state = new AbsMemSet();

	uint32_t size_pred = getAbsMemState(predecessor)->size();

	*new_state = *getAbsMemState(predecessor);

	for(uint32_t i=0; i < line_addrs->size(); i++)
	{
		updateState(new_state, line_addrs->at(i));
	}

	removeDuplicates(new_state);


	// this is for evaluation of memory usage only:
	uint64_t allocated_mem_size = sizeof(AbsMemSet);
	uint64_t maintained_mem_references = 0;
	for(uint32_t i = 0; i < new_state->size(); i++)
	{
		allocated_mem_size += new_state->at(i).getAllocatedSize();
		maintained_mem_references += new_state->at(i).getNumberOfMaintainedReferences();
	}
	allocatedMemoryState(new_state->size(), allocated_mem_size, maintained_mem_references);
	// end of evaluation code
	
	addAbsMemState(node, new_state);

	assert(size_pred == getAbsMemState(predecessor)->size());
}

void FIFOBFCacheStateMaintainer::join(vector<MSGVertex> *predecessors, vector< vector< uint32_t > > *predecessor_cache_lines, MSGVertex node)
{
	AbsMemSet *new_state;
	new_state = new AbsMemSet();
	ostringstream os;

	os << "Joining:";

	for(uint32_t i = 0; i < predecessors->size(); i++)
	{
		// get a copy of the memory set
		AbsMemSet pre = AbsMemSet(*getAbsMemState(predecessors->at(i)));

		os << " MSGID_" << dec << predecessors->at(i);
		printMemSet(&os, &pre);


		// update the copy of the set to obtain a post execution state, which represents the initial state of the joined node
		vector<uint32_t> line_addrs = predecessor_cache_lines->at(i);
		for(uint32_t j=0; j < line_addrs.size(); j++)
		{
			updateState(&pre, line_addrs.at(j));
		}

		os << " /";
		printMemSet(&os, &pre);

		// add the post execution state to the join set
		for(AbsMemSet::iterator amsit = pre.begin(); amsit != pre.end(); amsit++)
		{
			new_state->push_back((*amsit));
		}
	}

	LOG_DEBUG(logger, os.str());

	removeDuplicates(new_state);

	// this is for evaluation of memory usage only:
	uint64_t allocated_mem_size = sizeof(AbsMemSet);
	uint64_t maintained_mem_references = 0;
	for(uint32_t i = 0; i < new_state->size(); i++)
	{
		allocated_mem_size += new_state->at(i).getAllocatedSize();
		maintained_mem_references += new_state->at(i).getNumberOfMaintainedReferences();
	}
	allocatedMemoryState(new_state->size(), allocated_mem_size, maintained_mem_references);
	// end of evaluation code

	addAbsMemState(node, new_state);

}

void FIFOBFCacheStateMaintainer::printMemSet(ostringstream *os, MSGVertex node)
{
	AbsMemSet *state = getAbsMemState(node);
	*os << "printing States for node MSGID_" << node << ":";

	printMemSet(os, state);
}

void FIFOBFCacheStateMaintainer::printMemSet(ostringstream *os, AbsMemSet *state)
{
	for(AbsMemSet::iterator it = state->begin(); it != state->end(); it++)
	{
		*os << " [ ";
		it->print(os);
		*os << "]";
	}
}

AbsMemSet *FIFOBFCacheStateMaintainer::getAbsMemState(MSGVertex node)
{
//	ostringstream s;
//	s << "MSGID_" << node;
//	LOG_DEBUG(logger, "Obtaining state for: " << s.str());

	NodeMemSetMap::iterator pos;
	pos = state_map.find(node);
	assert(pos != state_map.end());
	return pos->second;
}

bool FIFOBFCacheStateMaintainer::addAbsMemState(MSGVertex node, AbsMemSet *state)
{
	NodeMemSetMap::iterator pos;
	bool ins_bool = false;

	LOG_DEBUG(logger, "Inserting MSGID_" << node);

//	ostringstream os;
//	os << "Storing MSGID_" << node << " ";
//	printMemSet(&os, state);
//	LOG_DEBUG(logger, os.str());

	tie(pos, ins_bool) = state_map.insert(make_pair(node, state));
	assert(ins_bool);

	return ins_bool;
}

bool FIFOBFCacheStateMaintainer::isInMust(vector<uint32_t> *prev_addrs, uint32_t addr, MSGVertex node)
{
	return isInAllSets(prev_addrs, addr, node);
}

bool FIFOBFCacheStateMaintainer::isInAllSets(vector<uint32_t> *prev_addrs, uint32_t addr, MSGVertex node)
{
	// get a copy of the object
	AbsMemSet state = AbsMemSet(*getAbsMemState(node));

	// consider the previous addresses
	for(vector<uint32_t>::iterator it = prev_addrs->begin(); it != prev_addrs->end(); it++)
	{
		updateState(&state, (*it));
	}

	return isInAllSets(addr, &state);
}

bool FIFOBFCacheStateMaintainer::isInMust(uint32_t addr, MSGVertex node)
{
	return isInAllSets(addr, node);
}

bool FIFOBFCacheStateMaintainer::isInAllSets(uint32_t addr, MSGVertex node)
{
	AbsMemSet *state;
	state = getAbsMemState(node);
	
	return isInAllSets(addr, state);
}

bool FIFOBFCacheStateMaintainer::isInAllSets(uint32_t addr, AbsMemSet *state)
{
	if(state->empty())
	{
		return false;
	}

	for(AbsMemSet::iterator amsit = state->begin(); amsit != state->end(); amsit++)
	{
		if(!(*amsit).isInState(addr))
		{
			return false;
		}
	}
	return true;
}

bool FIFOBFCacheStateMaintainer::isInMay(vector<uint32_t> *prev_addrs, uint32_t addr, MSGVertex node)
{
	return isInAnySet(prev_addrs,addr,node);
}

bool FIFOBFCacheStateMaintainer::isInAnySet(vector<uint32_t> *prev_addrs, uint32_t addr, MSGVertex node)
{
	// get a copy of the object
	AbsMemSet state = AbsMemSet(*getAbsMemState(node));

	// consider the previous addresses
	for(vector<uint32_t>::iterator it = prev_addrs->begin(); it != prev_addrs->end(); it++)
	{
		updateState(&state, (*it));
	}

	return isInAnySet(addr, &state);
}



bool FIFOBFCacheStateMaintainer::isInMay(uint32_t addr, MSGVertex node)
{
	return isInAnySet(addr,node);
}


bool FIFOBFCacheStateMaintainer::isInAnySet(uint32_t addr, MSGVertex node)
{
	AbsMemSet *state;
	state = getAbsMemState(node);
	
	return isInAnySet(addr, state);
}


bool FIFOBFCacheStateMaintainer::isInAnySet(uint32_t addr, AbsMemSet *state)
{
	if(state->empty())
	{
		return false;
	}

	for(AbsMemSet::iterator amsit = state->begin(); amsit != state->end(); amsit++)
	{
		if((*amsit).isInState(addr))
		{
			return true;
		}
	}
	return false;
}



bool FIFOBFCacheStateMaintainer::isInNoSet(vector<uint32_t> *prev_addrs, uint32_t addr, MSGVertex node)
{
	// get a copy of the object
	AbsMemSet state = AbsMemSet(*getAbsMemState(node));

	// consider the previous addresses
	for(vector<uint32_t>::iterator it = prev_addrs->begin(); it != prev_addrs->end(); it++)
	{
		updateState(&state, (*it));
	}

	return isInNoSet(addr, &state);

}

bool FIFOBFCacheStateMaintainer::isInNoSet(uint32_t addr, MSGVertex node)
{
	AbsMemSet *state;
	state = getAbsMemState(node);


	return isInNoSet(addr, state);
}

bool FIFOBFCacheStateMaintainer::isInNoSet(uint32_t addr, AbsMemSet *state)
{
	for(AbsMemSet::iterator amsit = state->begin(); amsit != state->end(); amsit++)
	{
		if((*amsit).isInState(addr))
		{
			return false;
		}
	}
	return true;
}


void FIFOBFCacheStateMaintainer::setBlankMemState(MSGVertex node)
{
	ostringstream s;
	s << "MSGID_" << node;
	LOG_DEBUG(logger, "Creating empty set for initial state: " << s.str());

	AbsMemSet *empty;
	empty= new AbsMemSet();

	// this is for evaluation of memory usage only:
	allocatedMemoryState(1, sizeof(AbsMemSet), 0);
	// end of evaluation code
	
	addAbsMemState(node, empty);
}

void FIFOBFCacheStateMaintainer::transfer(MSGVertex predecessor, MSGVertex node)
{
	assert(false); // untested
	AbsMemSet *transfered_state = getAbsMemState(predecessor);
	addAbsMemState(node, transfered_state);
}


void FIFOBFCacheStateMaintainer::updateState(AbsMemSet *state, uint32_t line_addr)
{
	if(state->empty())
	{
		FIFOCacheMemState tmp(cache_parameters.line_no);
		tmp.accessAddress(line_addr);
		state->push_back(tmp);
	}
	else
	{
		for(AbsMemSet::iterator amsit = state->begin(); amsit != state->end(); amsit++)
		{
				amsit->accessAddress(line_addr);
		}
	}
}



void FIFOBFCacheStateMaintainer::removeDuplicates(AbsMemSet *state)
{
	// erase duplicates
	for(AbsMemSet::iterator ita = state->begin(); ita != state->end(); ita++)
	{
		for(AbsMemSet::iterator itb = ita+1; itb != state->end();)
		{
			if((*ita) == (*itb))
			{
				itb = state->erase(itb);
			}
			else
			{
				itb++;
			}
		}
	}
}
