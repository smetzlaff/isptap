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
#include "stack_bf_disp_state_maintainer.hpp"

LoggerPtr BFDISPStateMaintainerStack::logger(Logger::getLogger("BF-DISPStateMaintainerStack"));

BFDISPStateMaintainerStack::BFDISPStateMaintainerStack()
{
	state_map.clear();
}

BFDISPStateMaintainerStack::~BFDISPStateMaintainerStack()
{
	for(NodeSTACKDISPStateMap::iterator it = state_map.begin(); it != state_map.end(); it++)
	{
		// free all entries
		delete((it->second)); 
	}

	state_map.clear();
}

BFDISPStateMaintainerStack::BFDISPStateMaintainerStack(disp_params_t params, FunctionCallGraphObject* fcgo)
{
	disp_parameters = params;
	functions = fcgo;
}

void BFDISPStateMaintainerStack::update(MSGVertex predecessor, MSGVertex node, uint32_t function_addr, activation_type_t activation, uint32_t previous_function)
{
	ostringstream os;

	AbsSTACKDISPMemState *new_state = new AbsSTACKDISPMemState();
	*new_state = *getAbsSTACKDISPMemState(predecessor);

	if(logger->isDebugEnabled())
	{
		os << "Updating: ";
		printMemSet(&os, new_state);
		os << " on " << ((activation==CALL)?("call of "):("return to ")) << "0x" << hex << function_addr << " from 0x" << previous_function << ":";
	}

	updateState(new_state, function_addr, activation, previous_function);

	removeDuplicates(new_state);

	// this is for evaluation of memory usage only:
	uint64_t allocated_mem_size = sizeof(AbsSTACKDISPMemState);
	uint64_t maintained_mem_references = 0;
	for(uint32_t i = 0; i < new_state->size(); i++)
	{
		allocated_mem_size += new_state->at(i).getAllocatedSize();
		maintained_mem_references += new_state->at(i).getNumberOfMaintainedReferences();
	}
	allocatedMemoryState(new_state->size(), allocated_mem_size, maintained_mem_references);
	// end of evaluation code

	if(logger->isDebugEnabled())
	{
		printMemSet(&os, new_state);
		LOG_DEBUG(logger, os.str());
	}

	addAbsMemState(node, new_state);
}

void BFDISPStateMaintainerStack::updateState(AbsSTACKDISPMemState *state, uint32_t function_addr, activation_type_t activation, uint32_t previous_function)
{
	if(state->empty())
	{
		STACKDISPMemState *tmp;

		assert(disp_parameters.rpol == STACK);
		tmp = new STACKDISPMemState(disp_parameters.size, disp_parameters.block_size, disp_parameters.mapping_table_size, disp_parameters.ignore_outsized_functions, functions);

		tmp->activateFunction(function_addr, activation, previous_function);
		state->push_back(*tmp);
	}
	else
	{
		for(AbsSTACKDISPMemState::iterator amsit = state->begin(); amsit != state->end(); amsit++)
		{
				amsit->activateFunction(function_addr, activation, previous_function);
		}
	}
}

void BFDISPStateMaintainerStack::join(vector<MSGVertex> *predecessors, MSGVertex node)
{
	AbsSTACKDISPMemState *new_state = new AbsSTACKDISPMemState();
	ostringstream os;

	os << "Joining:";

	for(uint32_t i = 0; i < predecessors->size(); i++)
	{
		// get a copy of the memory set
		AbsSTACKDISPMemState *pre = getAbsSTACKDISPMemState(predecessors->at(i));

		os << " MSGID_" << dec << predecessors->at(i);
		printMemSet(&os, pre);


		// add the post execution state to the join set
		for(AbsSTACKDISPMemState::iterator amsit = pre->begin(); amsit != pre->end(); amsit++)
		{
			new_state->push_back((*amsit));
		}
	}

	LOG_DEBUG(logger, os.str());

//	os << " into ";
//	printMemSet(&os, new_state);
	removeDuplicates(new_state);

	// this is for evaluation of memory usage only:
	uint64_t allocated_mem_size = sizeof(AbsSTACKDISPMemState);
	uint64_t maintained_mem_references = 0;
	for(uint32_t i = 0; i < new_state->size(); i++)
	{
		allocated_mem_size += new_state->at(i).getAllocatedSize();
		maintained_mem_references += new_state->at(i).getNumberOfMaintainedReferences();
	}
	allocatedMemoryState(new_state->size(), allocated_mem_size, maintained_mem_references);
	// end of evaluation code

//	os << " w/o dup: ";
//	printMemSet(&os, new_state);
	addAbsMemState(node, new_state);

//	LOG_DEBUG(logger, os.str());
}

void BFDISPStateMaintainerStack::transfer(MSGVertex predecessor, MSGVertex node)
{
	// do not copy the abstract memory state, just register the node to the same reference
	AbsSTACKDISPMemState *transfered_state = new AbsSTACKDISPMemState(*getAbsSTACKDISPMemState(predecessor));
	addAbsMemState(node, transfered_state);
}

void BFDISPStateMaintainerStack::printMemSet(ostringstream *os, MSGVertex node)
{
	AbsSTACKDISPMemState *state = getAbsSTACKDISPMemState(node);
	*os << "printing States for node MSGID_" << node << ":";

	printMemSet(os, state);
}

void BFDISPStateMaintainerStack::printMemSet(ostringstream *os, AbsSTACKDISPMemState *state)
{
	for(AbsSTACKDISPMemState::iterator it = state->begin(); it != state->end(); it++)
	{
		*os << " [ ";
		it->print(os);
		*os << "]";
	}
}

void BFDISPStateMaintainerStack::setBlankMemState(MSGVertex node)
{
	ostringstream s;
	s << "MSGID_" << node;
	LOG_DEBUG(logger, "Creating empty set for initial state: " << s.str());

	AbsSTACKDISPMemState *empty;
	empty= new AbsSTACKDISPMemState();

	// this is for evaluation of memory usage only:
	allocatedMemoryState(1, sizeof(AbsSTACKDISPMemState), 0);
	// end of evaluation code
	
	addAbsMemState(node, empty);
}


AbsSTACKDISPMemState *BFDISPStateMaintainerStack::getAbsSTACKDISPMemState(MSGVertex node)
{
//	ostringstream s;
//	s << "MSGID_" << node;
//	LOG_DEBUG(logger, "Obtaining state for: " << s.str());

	NodeSTACKDISPStateMap::iterator pos;
	pos = state_map.find(node);
	assert(pos != state_map.end());
	return pos->second;
}

bool BFDISPStateMaintainerStack::addAbsMemState(MSGVertex node, AbsSTACKDISPMemState *state)
{
	NodeSTACKDISPStateMap::iterator pos;
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

void BFDISPStateMaintainerStack::removeDuplicates(AbsSTACKDISPMemState *state)
{
	// normalize all concrete states before comparison
	for(AbsSTACKDISPMemState::iterator it = state->begin(); it != state->end(); it++)
	{
			it->normalize();
	}
	
	// erase duplicates
	for(AbsSTACKDISPMemState::iterator ita = state->begin(); ita != state->end(); ita++)
	{
		for(AbsSTACKDISPMemState::iterator itb = ita+1; itb != state->end();)
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

bool BFDISPStateMaintainerStack::isInAllSets(uint32_t addr, MSGVertex node)
{
	AbsSTACKDISPMemState *state;
	state = getAbsSTACKDISPMemState(node);
	
	return isInAllSets(addr, state);
}

bool BFDISPStateMaintainerStack::isInAllSets(uint32_t addr, AbsSTACKDISPMemState *state)
{
	if(state->empty())
	{
		return false;
	}

	for(AbsSTACKDISPMemState::iterator amsit = state->begin(); amsit != state->end(); amsit++)
	{
		if(!(*amsit).isInState(addr))
		{
			return false;
		}
	}
	return true;
}

bool BFDISPStateMaintainerStack::isInAnySet(uint32_t addr, MSGVertex node)
{
	AbsSTACKDISPMemState *state;
	state = getAbsSTACKDISPMemState(node);
	
	return isInAnySet(addr, state);
}


bool BFDISPStateMaintainerStack::isInAnySet(uint32_t addr, AbsSTACKDISPMemState *state)
{
	if(state->empty())
	{
		return false;
	}

	for(AbsSTACKDISPMemState::iterator amsit = state->begin(); amsit != state->end(); amsit++)
	{
		if((*amsit).isInState(addr))
		{
			return true;
		}
	}
	return false;
}

bool BFDISPStateMaintainerStack::isInNoSet(uint32_t addr, MSGVertex node)
{
	AbsSTACKDISPMemState *state;
	state = getAbsSTACKDISPMemState(node);


	return isInNoSet(addr, state);
}

bool BFDISPStateMaintainerStack::isInNoSet(uint32_t addr, AbsSTACKDISPMemState *state)
{
	for(AbsSTACKDISPMemState::iterator amsit = state->begin(); amsit != state->end(); amsit++)
	{
		if((*amsit).isInState(addr))
		{
			return false;
		}
	}
	return true;
}
