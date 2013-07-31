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
#include "context_stack.hpp"


ContextStack::ContextStack()
{
}

ContextStack::ContextStack(const ContextStack &copy)
{
	*(this) = copy;
}

ContextStack::~ContextStack()
{
	stack.clear();
}

void ContextStack::push(uint32_t new_top)
{
	stack.push_back(new_top);
}

uint32_t ContextStack::top(void)
{
	return top(0);
}

uint32_t ContextStack::top(uint32_t position)
{
	if(position >= stack.size())
	{
		// stack is to small, returning zero
		return 0x0;
	}
	else
	{
		return stack[stack.size() - 1 - position];
	}
	return 0x0;
}

uint32_t ContextStack::depth(void)
{
	return stack.size();
}

bool ContextStack::empty(void)
{
	if(depth() != 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

uint32_t ContextStack::pop(void)
{
	if(stack.size() != 0)
	{
		uint32_t top = stack.back();
		stack.pop_back();
		return top;
	}
	return 0x0;
}

bool ContextStack::operator== ( const ContextStack& compare)
{
	return this->compare(compare);
}

bool ContextStack::compare(ContextStack compare)
{
	if(this->depth() == compare.depth())
	{
		for(uint32_t i = 0; i < this->depth(); i++)
		{
			if(this->top(i) != compare.top(i))
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}
	return true;
}

string ContextStack::toString(void)
{
	stringstream s;
	// set string to hex
	s << hex << "(";

	for(uint32_t i = 0; i < depth(); i++)
	{
		s << "(0x" << top(i) << ")";
	}
	// set string back to decimal
	s << ")" << dec;

	return s.str();
}

void ContextStack::clear(void)
{
	stack.clear();
}


