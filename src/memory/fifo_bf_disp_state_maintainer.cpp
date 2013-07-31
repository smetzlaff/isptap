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
#include "fifo_bf_disp_state_maintainer.hpp"

LoggerPtr BFDISPStateMaintainerFIFO::logger(Logger::getLogger("BF-DISPStateMaintainerFIFO"));

BFDISPStateMaintainerFIFO::BFDISPStateMaintainerFIFO()
{
	state_map.clear();
}

BFDISPStateMaintainerFIFO::~BFDISPStateMaintainerFIFO()
{
	for(NodeFIFODISPStateMap::iterator it = state_map.begin(); it != state_map.end(); it++)
	{
		// free all entries
		delete((it->second)); 
	}

	state_map.clear();
}

BFDISPStateMaintainerFIFO::BFDISPStateMaintainerFIFO(disp_params_t params, FunctionCallGraphObject* fcgo)
{
	disp_parameters = params;
	functions = fcgo;
}

void BFDISPStateMaintainerFIFO::update(MSGVertex predecessor, MSGVertex node, uint32_t function_addr, activation_type_t activation, uint32_t previous_function)
{
	ostringstream os;

	AbsFIFODISPMemState *new_state = new AbsFIFODISPMemState();
	*new_state = *getAbsFIFODISPMemState(predecessor);

	if(logger->isDebugEnabled())
	{
		os << "Updating: ";
		printMemSet(&os, new_state);
		os << " on " << ((activation==CALL)?("call of "):("return to ")) << "0x" << hex << function_addr << ":";
	}

	updateState(new_state, function_addr, activation, previous_function);

	removeDuplicates(new_state);

	// this is for evaluation of memory usage only:
	uint64_t allocated_mem_size = sizeof(AbsFIFODISPMemState);
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

void BFDISPStateMaintainerFIFO::updateState(AbsFIFODISPMemState *state, uint32_t function_addr, activation_type_t activation, uint32_t previous_function)
{
	if(state->empty())
	{
		FIFODISPMemState *tmp;

		assert(disp_parameters.rpol == FIFO);
		tmp = new FIFODISPMemState(disp_parameters.size, disp_parameters.block_size, disp_parameters.mapping_table_size, disp_parameters.ignore_outsized_functions, functions);

		tmp->activateFunction(function_addr, activation, previous_function);
		state->push_back(*tmp);
	}
	else
	{
		for(AbsFIFODISPMemState::iterator amsit = state->begin(); amsit != state->end(); amsit++)
		{
				amsit->activateFunction(function_addr, activation, previous_function);
		}
	}
}

void BFDISPStateMaintainerFIFO::join(vector<MSGVertex> *predecessors, MSGVertex node)
{
	AbsFIFODISPMemState *new_state = new AbsFIFODISPMemState();
	ostringstream os;

	os << "Joining:";

	for(uint32_t i = 0; i < predecessors->size(); i++)
	{
		// get a copy of the memory set
		AbsFIFODISPMemState *pre = getAbsFIFODISPMemState(predecessors->at(i));

		os << " MSGID_" << dec << predecessors->at(i);
		printMemSet(&os, pre);


		// add the post execution state to the join set
		for(AbsFIFODISPMemState::iterator amsit = pre->begin(); amsit != pre->end(); amsit++)
		{
			new_state->push_back((*amsit));
		}
	}

	LOG_DEBUG(logger, os.str());

	removeDuplicates(new_state);

	// this is for evaluation of memory usage only:
	uint64_t allocated_mem_size = sizeof(AbsFIFODISPMemState);
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

void BFDISPStateMaintainerFIFO::transfer(MSGVertex predecessor, MSGVertex node)
{
	// do not copy the abstract memory state, just register the node to the same reference
	AbsFIFODISPMemState *transfered_state = new AbsFIFODISPMemState(*getAbsFIFODISPMemState(predecessor));
	addAbsMemState(node, transfered_state);
}

void BFDISPStateMaintainerFIFO::printMemSet(ostringstream *os, MSGVertex node)
{
	AbsFIFODISPMemState *state = getAbsFIFODISPMemState(node);
	*os << "printing States for node MSGID_" << node << ":";

	printMemSet(os, state);
}

void BFDISPStateMaintainerFIFO::printMemSet(ostringstream *os, AbsFIFODISPMemState *state)
{
	for(AbsFIFODISPMemState::iterator it = state->begin(); it != state->end(); it++)
	{
		*os << " [ ";
		it->print(os);
		*os << "]";
	}
}

void BFDISPStateMaintainerFIFO::setBlankMemState(MSGVertex node)
{
	ostringstream s;
	s << "MSGID_" << node;
	LOG_DEBUG(logger, "Creating empty set for initial state: " << s.str());

	AbsFIFODISPMemState *empty;
	empty= new AbsFIFODISPMemState();

	// this is for evaluation of memory usage only:
	allocatedMemoryState(1, sizeof(AbsFIFODISPMemState), 0);
	// end of evaluation code
	
	addAbsMemState(node, empty);
}


AbsFIFODISPMemState *BFDISPStateMaintainerFIFO::getAbsFIFODISPMemState(MSGVertex node)
{
//	ostringstream s;
//	s << "MSGID_" << node;
//	LOG_DEBUG(logger, "Obtaining state for: " << s.str());

	NodeFIFODISPStateMap::iterator pos;
	pos = state_map.find(node);
	assert(pos != state_map.end());
	return pos->second;
}

bool BFDISPStateMaintainerFIFO::addAbsMemState(MSGVertex node, AbsFIFODISPMemState *state)
{
	NodeFIFODISPStateMap::iterator pos;
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

void BFDISPStateMaintainerFIFO::removeDuplicates(AbsFIFODISPMemState *state)
{
	// erase duplicates
	for(AbsFIFODISPMemState::iterator ita = state->begin(); ita != state->end(); ita++)
	{
		for(AbsFIFODISPMemState::iterator itb = ita+1; itb != state->end();)
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

bool BFDISPStateMaintainerFIFO::isInAllSets(uint32_t addr, MSGVertex node)
{
	AbsFIFODISPMemState *state;
	state = getAbsFIFODISPMemState(node);
	
	return isInAllSets(addr, state);
}

bool BFDISPStateMaintainerFIFO::isInAllSets(uint32_t addr, AbsFIFODISPMemState *state)
{
	if(state->empty())
	{
		return false;
	}

	for(AbsFIFODISPMemState::iterator amsit = state->begin(); amsit != state->end(); amsit++)
	{
		if(!(*amsit).isInState(addr))
		{
			return false;
		}
	}
	return true;
}

bool BFDISPStateMaintainerFIFO::isInAnySet(uint32_t addr, MSGVertex node)
{
	AbsFIFODISPMemState *state;
	state = getAbsFIFODISPMemState(node);
	
	return isInAnySet(addr, state);
}


bool BFDISPStateMaintainerFIFO::isInAnySet(uint32_t addr, AbsFIFODISPMemState *state)
{
	if(state->empty())
	{
		return false;
	}

	for(AbsFIFODISPMemState::iterator amsit = state->begin(); amsit != state->end(); amsit++)
	{
		if((*amsit).isInState(addr))
		{
			return true;
		}
	}
	return false;
}

bool BFDISPStateMaintainerFIFO::isInNoSet(uint32_t addr, MSGVertex node)
{
	AbsFIFODISPMemState *state;
	state = getAbsFIFODISPMemState(node);


	return isInNoSet(addr, state);
}

bool BFDISPStateMaintainerFIFO::isInNoSet(uint32_t addr, AbsFIFODISPMemState *state)
{
	for(AbsFIFODISPMemState::iterator amsit = state->begin(); amsit != state->end(); amsit++)
	{
		if((*amsit).isInState(addr))
		{
			return false;
		}
	}
	return true;
}
