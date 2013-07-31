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
#ifndef _CONTEXT_STACK_HPP_
#define _CONTEXT_STACK_HPP_

#include "global.h"
#include "constants.h"
#include <sstream>

/**
 * \brief Class for handling and comparing context stacks.
 * A context stack is the context of a node in a function call hierarchy. Using the context stack it is possible to determine if one of the called functions was called from a different calling context. 
 * The context stack is used to search for a path in a cfg and respecting the calling contexts of every node. Such that impossible paths cannot be found. Furthermore it is possible to traverse the code of function in different calling contexts multiple times in path creation.
 * The ContextStack contains of uint32_t-values that represent the addresses of the call (or return) points from where the functions are called.
 */
class ContextStack {
	public:
		/**
		 * \brief Default constructor.
		 */
		ContextStack();
		/**
		 * \brief Copy constructor.
		 * \param copy The original to copy.
		 */
		ContextStack(const ContextStack &copy);
		/**
		 * \brief Default destructor.
		 */
		virtual ~ContextStack();
		/**
		 * \brief Pushes a context onto the stack
		 * \param new_top The context that is pushed onto the stack. This context is the new top value.
		 */
		void push(uint32_t new_top);
		/**
		 * \brief Gets the top value of the context stack or 0 if the stack is empty.
		 * The top value is the currently active context.
		 * \returns The top context of the stack. If empty it returns 0x0
		 */
		uint32_t top(void);
		/**
		 * \brief Gets the (top - position) value of the context stack or 0 if the stack is not enough elements deep.
		 * \param position The position of the stack that is should be returned. The position is defined as distance from the stack top value.
		 * \returns The (top - position) context of the stack. If the stack is not at least position elements deep it returns 0x0.
		 */
		uint32_t top(uint32_t position);
		/**
		 * \brief Returns the depth of the context stack.
		 * The depth represents the number of currently active contexts.
		 * \returns The depth of the context stack. If the depth is 0, the stack is empty.
		 */
		uint32_t depth(void);
		/**
		 * \brief Pops the top context of the stack.
		 * The top context is returned and deleted from stack.
		 * \returns The top element of the stack. If the stack is empty 0x0, is returned.
		 */
		uint32_t pop(void);
		/**
		 * \brief Returns true, if the stack is empty. Otherwise false is returned.
		 * An empty context stack has a depth of 0.
		 * \returns True if the stack is empty. Otherwise false is returned.
		 */
		bool empty(void);
		/**
		 * \brief Compares two context stacks element by element.
		 * \param compare The context stack to compare.
		 * \returns True if both context stacks have the same stack depth and every stack entry is equal. Otherwise false is returned.
		 */
		bool operator== ( const ContextStack& compare );
		/**
		 * \brief Compares two context stacks element by element.
		 * \param compare The context stack to compare.
		 * \returns True if both context stacks have the same stack depth and every stack entry is equal. Otherwise false is returned.
		 */
		bool compare(ContextStack compare);
		/**
		 * \brief Converts the context stack into a string.
		 * The contexts are sorted by age, the top element is at the beginning of the string.
		 * \returns String containing all contexts of the context stack.
		 */
		string toString(void);
		/**
		 * \brief Deletes all elements.
		 */
		void clear(void);
	private: 
		/**
		 * \brief The internal representation of the context stack.
		 * A vector is used since the compare() and the top(uint32_t) function can be implemented more efficient than using the std::stack
		 */
		vector<uint32_t> stack;
};

#endif
