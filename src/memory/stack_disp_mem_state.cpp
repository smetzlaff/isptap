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
#include "stack_disp_mem_state.hpp"

LoggerPtr STACKDISPMemState::logger(Logger::getLogger("STACKDISPMemState"));

STACKDISPMemState::STACKDISPMemState()
{
}

STACKDISPMemState::STACKDISPMemState(uint32_t memSize, uint32_t blockSize, uint32_t maxFuncs, bool ignoreOutsizedFunctions, FunctionCallGraphObject* fcgo)
{
	mem_size = memSize;
	block_size = blockSize;
	max_functions = maxFuncs;
	ignore_outsized_functions = ignoreOutsizedFunctions;
	functions = fcgo;
}

STACKDISPMemState::STACKDISPMemState(const STACKDISPMemState& copy)
{
	mem_size = copy.mem_size;
	block_size = copy.block_size;
	max_functions = copy.max_functions;
	ignore_outsized_functions = copy.ignore_outsized_functions;
	functions = copy.functions;
	content = copy.content;
}

STACKDISPMemState::~STACKDISPMemState()
{
	content.clear();
	functions = NULL;
}

void STACKDISPMemState::activateFunction(uint32_t func_addr, activation_type_t act, uint32_t predecessor)
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
		DISPMemEntryStack tmp;
		tmp.address = func_addr;

		uint32_t size_of_new_function = getFunctionMemSize(func_addr);

		if(act == CALL)
		{

			// insert function
			DISPMemSetStack::iterator it;
			it = content.begin();
			bool insert_pos_found = false;
			if(!content.empty())
			{
				while((it != content.end()) && (!insert_pos_found))
				{
					if((*it).address == predecessor)
					{
						insert_pos_found = true;
						// determine where to insert the function
						tmp.pos_begin = (*it).pos_end;
						tmp.pos_end = (tmp.pos_begin + getFunctionMemSize(func_addr))%mem_size;
					}
					it++;
				}
				// the caller function has to be in the scratchpad, because by definition the active function is in the scratcpad
				assert(insert_pos_found);
			}
			else
			{
				tmp.pos_begin = 0;
				tmp.pos_end = getFunctionMemSize(func_addr);
			}

//			DISPMemSetStack::iterator ins_it=content.insert(it, tmp);
			content.insert(it, tmp);

			LOG_DEBUG(logger, "Added 0x" << hex << tmp.address << dec << " at: [" << tmp.pos_begin << "," << tmp.pos_end << "[" << " activation was CALL previous context was 0x" << hex << predecessor);

			evictFunctionsInRange(tmp);

			assert(content.size() >= 1); // ensure that not everything was deleted
		}
		else if(act == RETURN)
		{

			// insert function
			DISPMemSetStack::reverse_iterator rit_end (content.begin());
			DISPMemSetStack::reverse_iterator rit_start (content.end());
			DISPMemSetStack::reverse_iterator rit;
			rit = rit_start;
			bool insert_pos_found = false;

			// on return the scratchpad cannot be empty, because at least the active function is contained.
			assert(!content.empty());

			while((rit != rit_end) && (!insert_pos_found))
			{
				if((*rit).address == predecessor)
				{
					insert_pos_found = true;
					// determine where to insert the function
					tmp.pos_end = (*rit).pos_begin;
					tmp.pos_begin = ((tmp.pos_end + mem_size) - size_of_new_function)%mem_size;
				}
				rit++;
			}
			// the callee function has to be in the scratchpad, because by definition the active function is in the scratcpad
			assert(insert_pos_found);

			DISPMemSetStack::reverse_iterator ins_rit;
			ins_rit.base() = content.insert(rit.base(), tmp);

			LOG_DEBUG(logger, "Added 0x" << hex << tmp.address << dec << " at: [" << tmp.pos_begin << "," << tmp.pos_end << "[" << " activation was RETURN previous context was 0x" << hex << predecessor);

			evictFunctionsInRange(tmp);

			assert(content.size() >= 1); // ensure that not everything was deleted
		}
		else
		{
			assert(false);
		}

		LOG_DEBUG(logger, "used size is: " << getUsedSize() << " memory size is: " << mem_size << " bytes");

	}
	else
	{
		// do nothing.
	}
}

bool STACKDISPMemState::isInState(uint32_t func_addr)
{
	for(DISPMemSetStack::iterator it = content.begin(); it != content.end(); it++)
	{
		if((*it).address == func_addr)
		{
			return true;
		}
	}
	return false;
}

void STACKDISPMemState::print(ostringstream *os)
{
	for(DISPMemSetStack::iterator it = content.begin(); it != content.end(); it++)
	{
		*os << "0x" << hex << it->address << " ";
	}
	*os << "(" << dec << getUsedSize() << ")";
}

void STACKDISPMemState::printPos(ostringstream *os)
{
	for(DISPMemSetStack::iterator it = content.begin(); it != content.end(); it++)
	{
		*os << "0x" << hex << it->address << "[" << dec << it->pos_begin << "," << it->pos_end << "[" << " ";
	}
	*os << "(" << dec << getUsedSize() << ")";
}



STACKDISPMemState& STACKDISPMemState::operator=(const STACKDISPMemState& other)
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

bool STACKDISPMemState::operator== (const STACKDISPMemState& other)
{
	if(this->content.size() != other.content.size())
	{
		return false;
	}

	for(uint32_t i=0; i < this->content.size(); i++)
	{
		if((this->content[i].address != other.content[i].address) || (this->content[i].pos_begin != other.content[i].pos_begin) || (this->content[i].pos_end != other.content[i].pos_end))
		{
			return false;
		}
	}

	return true;
}

uint32_t STACKDISPMemState::getUsedSize()
{

	uint32_t mem_set_size = 0;
	for(DISPMemSetStack::iterator it = content.begin(); it != content.end(); it++)
	{
		mem_set_size += getFunctionMemSize(it->address);
	}
	return mem_set_size;
}


bool STACKDISPMemState::evictFunctionsInRange(DISPMemEntryStack evict)
{
	DISPMemSetStack::iterator it;
	it = content.begin();
	position_t func_range;
	position_t evict_range;
	evict_range.pos_begin = evict.pos_begin;
	evict_range.pos_end = evict.pos_end;
	uint32_t ignore_func = evict.address;

	bool altered_mem_state = false;

	LOG_DEBUG(logger, "Evict range: [" << evict_range.pos_begin << "," << evict_range.pos_end << "[");

	while(it != content.end())
	{
		if((*it).address != ignore_func)
		{
			func_range.pos_begin = (*it).pos_begin;
			func_range.pos_end = (*it).pos_end;
			if(checkFunctionIntersection(evict_range, func_range))
			{
				LOG_DEBUG(logger, "Function 0x" << hex << (*it).address << dec  << " [" << (*it).pos_begin << "," << (*it).pos_end << "[ is evicted");
				// check if the function is evicted from left or from right
				position_t intersection;
				bool intersection_found;

				tie(intersection_found, intersection) = getFunctionIntersection(evict_range, func_range);
				if((func_range.pos_begin == intersection.pos_begin) || (func_range.pos_end == intersection.pos_end))
				{
					// everything is fine, the function is evicted by overwrite from left or right
				}
				else
				{
					// the eviction pointer does not enter the function from left or right, 
					// to build a safe must set evict it anyway.
					LOG_DEBUG(logger, "func: [" << func_range.pos_begin << ":" << func_range.pos_end << "[ intersect: [" << intersection.pos_begin << ":" << intersection.pos_end << "[");
					assert(false); // this case was not observed until now
				}
				// erase the function on collision
				it = content.erase(it);
				altered_mem_state = true;
			}
			else
			{
				LOG_DEBUG(logger, "Function 0x" << hex << (*it).address << dec  << " [" << (*it).pos_begin << "," << (*it).pos_end << "[ is NOT evicted");
				it++;
			}
		}
		else
		{
			it++;
		}
	}
	return altered_mem_state;
}

bool inline STACKDISPMemState::checkFunctionIntersection(position_t a, position_t b)
{
	bool intersect = false;
	bool wrap_a = false;
	bool wrap_b = false;

	if(a.pos_begin > a.pos_end)
	{
		wrap_a = true;
	}

	if(b.pos_begin > b.pos_end)
	{
		wrap_b = true;
	}

	if((a.pos_begin >= b.pos_begin) && ( (a.pos_begin < ((wrap_b)?(b.pos_end+mem_size):(b.pos_end))) || ((wrap_a) && (a.pos_end > b.pos_begin)) ))
	{
		intersect = true;
	}
	else if((b.pos_begin >= a.pos_begin) && ( (b.pos_begin < ((wrap_a)?(a.pos_end+mem_size):(a.pos_end))) || ((wrap_b) && (b.pos_end > a.pos_begin)) ))
	{
		intersect = true;
	}

	return intersect;
}

pair<bool, position_t> inline STACKDISPMemState::getFunctionIntersection(position_t a, position_t b)
{
	position_t result;
	result.pos_begin = 0;
	result.pos_end = 0;
	bool intersection_found = false;

	bool wrap_a = false;
	bool wrap_b = false;



	if(checkFunctionIntersection(a,b))
	{
		intersection_found = true;
		if(a.pos_begin > a.pos_end)
		{
			wrap_a = true;
		}

		if(b.pos_begin > b.pos_end)
		{
			wrap_b = true;
		}

		if(!wrap_a && !wrap_b)
		{
			//
			// a: -----aaaa-
			// b: ---bbb----
			//
			result.pos_begin = (a.pos_begin > b.pos_begin)?(a.pos_begin):(b.pos_begin);
			result.pos_end = (a.pos_end < b.pos_end)?(a.pos_end):(b.pos_end);
		}
		else if(wrap_a && !wrap_b)
		{
			if(a.pos_begin > b.pos_end)
			{
				//
				// a: a------aaa
				// b: bbb-------
				//
				result.pos_begin = b.pos_begin;
				result.pos_end = (a.pos_end < b.pos_end)?(a.pos_end):(b.pos_end);
			}
			else
			{
				//
				// a: a------aaa
				// b: ------bb--
				//
				result.pos_begin = (a.pos_begin > b.pos_begin)?(a.pos_begin):(b.pos_begin);
				result.pos_end = b.pos_end;
			}
		}
		else if(!wrap_a && wrap_b)
		{
			if(b.pos_begin > a.pos_end)
			{
				//
				// a: -aa-------
				// b: bb-----bbb
				//
				result.pos_begin = a.pos_begin;
				result.pos_end = (b.pos_end < a.pos_end)?(b.pos_end):(a.pos_end);
			}
			else
			{
				//
				// a: ------aa--
				// b: b------bbb
				//
				result.pos_begin = (b.pos_begin > a.pos_begin)?(b.pos_begin):(a.pos_begin);
				result.pos_end = a.pos_end;
			}
		}
		else // wrap_a && wrap_b
		{
			//
			// a: aaa-----aa
			// b: bb-----bbb
			//
			result.pos_begin = (a.pos_begin > b.pos_begin)?(a.pos_begin):(b.pos_begin);
			result.pos_end = (a.pos_end < b.pos_end)?(a.pos_end):(b.pos_end);
		}
	}

	return make_pair(intersection_found, result);

}


void STACKDISPMemState::normalize(void)
{
	alignWithOffset(content.begin()->pos_begin);
}


void STACKDISPMemState::normalize(uint32_t func_addr)
{
	uint32_t offset = 0;
	for(DISPMemSetStack::iterator it = content.begin(); it != content.end(); it++)
	{
		if(it->address == func_addr)
		{
			offset = it->pos_begin;
		}
	}
	alignWithOffset(offset);
}

void STACKDISPMemState::alignWithOffset(uint32_t offset)
{
	if(offset == 0)
	{
		return;
	}

	ostringstream os;

	os << "Aligning functions by " << offset << " ";
	printPos(&os);
	os << " to ";

	for(DISPMemSetStack::iterator it = content.begin(); it != content.end(); it++)
	{
		it->pos_begin = (it->pos_begin + mem_size - offset) % mem_size;
		it->pos_end = (it->pos_end + mem_size - offset) % mem_size;
	}

	printPos(&os);
	LOG_DEBUG(logger, os.str());

}

uint32_t STACKDISPMemState::getAllocatedSize(void)
{
	return ((content.size() * sizeof(DISPMemEntryStack)) + sizeof(STACKDISPMemState));
}

uint32_t STACKDISPMemState::getNumberOfMaintainedReferences(void)
{
	return content.size();
}
