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
#ifndef _DISP_MEM_STATE_HPP_
#define _DISP_MEM_STATE_HPP_

#include "constants.h"
#include "global.h"

#include "configuration.hpp"
#include "fcgobj.hpp"

/*!
 * \brief Class for implementation of a concrete DISP state.
 * It is used for a correct FIFO and STACK DISP content analysis using brute force.
 */
class DISPMemState {
	public:
		/*!
		 * \brief Implements the changes to the concrete DISP state if a function is activated.
		 * \param func_addr The address of the function that is activated.
		 * \param act The type of activation: either CALL or RETURN.
		 * \param predecessor The function that caused the activation of the func_addr, either the caller or the callee.
		 */
		virtual void activateFunction(uint32_t func_addr, activation_type_t act, uint32_t predecessor) = 0;
		/*!
		 * \brief Determines if a certain function is in the current concrete state of the DISP.
		 * \param func_addr The address of the function that is tested.
		 * \returns True if the function is in the DISP, else false.
		 */
		virtual bool isInState(uint32_t func_addr) = 0;
		/*!
		 * \brief Prints the content of the concrete DISP state into a given stream.
		 * \param os The stream to print the concrete DISP state into.
		 */
		virtual void print(ostringstream *os) = 0;
		/*!
		 * \brief Returns the memory size used for the representation of the concrete DISP state.
		 * \returns The memory size used for the representation of the concrete DISP state.
		 */
		virtual uint32_t getAllocatedSize(void) = 0;
	protected:
		/*! 
		 * \brief Returns the size of a function in memory.
		 * Therefore the FunctionCallGraphObject is used. The size of the function in memory is based on the block_size
		 * \param address The address of a function, for which the size is returned.
		 * \returns The size of the function, if no size information for the requested address exists 0 is returned.
		 */
		uint32_t getFunctionMemSize(uint32_t address);
		/*!
		 * \brief Returns the amount of memory that is used by all functions that are currently in the concrete DISP state.
		 * \returns The amount of memory used by all containing functions.
		 */
		virtual uint32_t getUsedSize() = 0;
		/*!
		 * \brief The size of the scratchpad.
		 */
		uint32_t mem_size;
		/*!
		 * \brief The size of the minimal usable blocks in the scratchpad.
		 */
		uint32_t block_size;
		/*!
		 * \brief The maximum number of functions that can be maintained by the DISP.
		 */
		uint32_t max_functions;
		/*!
		 * \brief Determines if to large functions will be ignored or not.  If true functions that are larger than the scratchpad will not be loaded, else if such function should be maintained by the DISP an assertion is violated.
		 */
		bool ignore_outsized_functions;
		/*!
		 * \brief Holds the function call graph, but is just needed to obtain the function sizes.
		 */
		FunctionCallGraphObject *functions;
};
#endif
