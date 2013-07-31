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
#include "fifo_disp_mem_state.hpp"

LoggerPtr FIFODISPMemState::logger(Logger::getLogger("FIFODISPMemState"));

FIFODISPMemState::FIFODISPMemState()
{
}

FIFODISPMemState::FIFODISPMemState(uint32_t memSize, uint32_t blockSize, uint32_t maxFuncs, bool ignoreOutsizedFunctions, FunctionCallGraphObject* fcgo)
{
	mem_size = memSize;
	block_size = blockSize;
	max_functions = maxFuncs;
	ignore_outsized_functions = ignoreOutsizedFunctions;
	functions = fcgo;
}

FIFODISPMemState::FIFODISPMemState(const FIFODISPMemState& copy)
{
	mem_size = copy.mem_size;
	block_size = copy.block_size;
	max_functions = copy.max_functions;
	ignore_outsized_functions = copy.ignore_outsized_functions;
	functions = copy.functions;
	content = copy.content;
}

FIFODISPMemState::~FIFODISPMemState()
{
	content.clear();
	functions = NULL;
}

void FIFODISPMemState::activateFunction(uint32_t func_addr, activation_type_t UNUSED_PARAMETER(act), uint32_t UNUSED_PARAMETER(predecessor))
{
	if(getFunctionMemSize(func_addr) > mem_size)
	{
		if(ignore_outsized_functions)
		{
			LOG_INFO(logger, "function: 0x" << hex << func_addr << " has size of " << dec << getFunctionMemSize(func_addr) << " bytes, but memory has size of " << mem_size << " bytes - ignoring it.");
			return;
		}
		else
		{
			LOG_ERROR(logger, "function: 0x" << hex << func_addr << " has size of " << dec << getFunctionMemSize(func_addr) << " bytes, but memory has size of " << mem_size << " bytes!!");
			assert(false);
		}
		assert(false); // remove assert, if this is tested
	}

	if(!isInState(func_addr))
	{
		DISPMemEntryFIFO tmp;
		tmp.address = func_addr;

		content.insert(content.begin(), tmp);

		// check if the mapping table overflows
		if(content.size() > max_functions)
		{
			LOG_DEBUG(logger, "Deleting 0x" << hex << content.back().address << " due to mapping table size restrictions. " << content.size() << " > " << max_functions);
			content.pop_back();
		}

		bool eviction = false;
		ostringstream os;
		// check if the scratchpad size overflows
		while(getUsedSize() > mem_size)
		{
			eviction = true;
			os << hex << " 0x" << content.back().address;
			content.pop_back();
		}
		if(eviction)
		{
			LOG_DEBUG(logger, "Evicting: " << os.str() << " due to size restriction of the DISP");
		}

		assert(content.size() >= 1);
	}
	else
	{
		// do nothing.
	}
}

bool FIFODISPMemState::isInState(uint32_t func_addr)
{
	for(DISPMemSetFIFO::iterator it = content.begin(); it != content.end(); it++)
	{
		if((*it).address == func_addr)
		{
			return true;
		}
	}
	return false;
}

void FIFODISPMemState::print(ostringstream *os)
{
	for(DISPMemSetFIFO::iterator it = content.begin(); it != content.end(); it++)
	{
		*os << "0x" << hex << it->address << " ";
	}
	*os << " (" << dec << getUsedSize() << ")";
}

FIFODISPMemState& FIFODISPMemState::operator=(const FIFODISPMemState& other)
{
	if(this != &other)
	{

		// clear content of overwritten object
		this->content.clear();
		this->functions = NULL;

		this->mem_size = other.mem_size;
		this->block_size = other.block_size;
		this->max_functions = other.max_functions;
		this->ignore_outsized_functions = other.ignore_outsized_functions;
		this->functions = other.functions;
		this->content = other.content;
	}
	return *this;
}

bool FIFODISPMemState::operator== (const FIFODISPMemState& other)
{
	if(this->content.size() != other.content.size())
	{
		return false;
	}

	for(uint32_t i=0; i < this->content.size(); i++)
	{
		if(this->content[i].address != other.content[i].address)
		{
			return false;
		}
	}

	return true;
}

uint32_t FIFODISPMemState::getUsedSize()
{

	uint32_t mem_set_size = 0;
	for(DISPMemSetFIFO::iterator it = content.begin(); it != content.end(); it++)
	{
		mem_set_size += getFunctionMemSize(it->address);
	}
	return mem_set_size;
}

uint32_t FIFODISPMemState::getAllocatedSize(void)
{
	return ((content.size() * sizeof(DISPMemEntryFIFO)) + sizeof(FIFODISPMemState));
}

uint32_t FIFODISPMemState::getNumberOfMaintainedReferences(void)
{
	return content.size();
}
