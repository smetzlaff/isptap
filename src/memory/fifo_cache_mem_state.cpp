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
#include "fifo_cache_mem_state.hpp"


FIFOCacheMemState::FIFOCacheMemState(uint32_t maxLines)
{
	max_lines = maxLines;
}

FIFOCacheMemState::FIFOCacheMemState(const FIFOCacheMemState& copy)
{
	max_lines = copy.max_lines;
	content = copy.content;
}

FIFOCacheMemState::~FIFOCacheMemState()
{
	content.clear();
}

void FIFOCacheMemState::accessAddress(uint32_t addr)
{
	if(!isInState(addr))
	{
		MemEntry tmp;
		tmp.address = addr;

		content.insert(content.begin(), tmp);

		if(content.size() > max_lines)
		{
			content.pop_back();
			assert(content.size() <= max_lines);
		}
	}
	else
	{
		// do nothing.
	}
}

bool FIFOCacheMemState::isInState(uint32_t addr)
{
	for(MemSet::iterator it = content.begin(); it != content.end(); it++)
	{
		if((*it).address == addr)
		{
			return true;
		}
	}
	return false;
}


FIFOCacheMemState& FIFOCacheMemState::operator=(const FIFOCacheMemState& other)
{
	if(this != &other)
	{
		this->max_lines = other.max_lines;
		this->content = other.content;
	}
	return *this;
}

bool FIFOCacheMemState::operator== (const FIFOCacheMemState& other)
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


FIFOCacheMemState::FIFOCacheMemState()
{
}


void FIFOCacheMemState::print(ostringstream *os)
{
	for(MemSet::iterator it = content.begin(); it != content.end(); it++)
	{
		*os << "0x" << hex << it->address << " ";
	}
}

uint32_t FIFOCacheMemState::getAllocatedSize(void)
{
	return ((content.size() * sizeof(MemEntry)) + sizeof(FIFOCacheMemState));
}

uint32_t FIFOCacheMemState::getNumberOfMaintainedReferences(void)
{
	return content.size();
}
