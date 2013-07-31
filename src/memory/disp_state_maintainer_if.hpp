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
#ifndef _DISP_STATE_MAINTAINER_IF_HPP_
#define _DISP_STATE_MAINTAINER_IF_HPP_

#include "global.h"
#include "constants.h"
#include "graph_structure.h"
#include "fcgobj.hpp"

#include "mem_state_maintainer.hpp"


class DISPStateMaintainer_IF : public MemStateMaintainer {
	public:
		/*!
		 * \brief Updates the memory state set by a function address that is to be executed.
		 * The method updates the must and may set.
		 * \param state The input abstract memory state (containing must and may state).
		 * \param function_addr The address of a function that will be executed next (on call or return).
		 * \param activation_node_type The type of the node that caused the invocation of the update().
		 * \param dummy This parameter is only needed for DISPStateMaintainerStack.
		 * \returns Memory state that takes the activation of the given function into account.
		 */
		virtual abs_mem_set_t update(abs_mem_set_t state, uint32_t function_addr, node_type_t activation_node_type, uint32_t dummy) __attribute__((deprecated)) = 0;
		/*!
		 * \brief Updates the memory state set by a function address that is to be executed.
		 * The method updates the must and may set.
		 * \param state A pointer to the input abstract memory state (containing must and may state).
		 * \param function_addr The address of a function that will be executed next (on call or return).
		 * \param activation_node_type The type of the node that caused the invocation of the update().
		 * \param dummy This parameter is only needed for DISPStateMaintainerStack.
		 * \returns Memory state that takes the activation of the given function into account.
		 */
		virtual abs_mem_set_t update(abs_mem_set_t *state, uint32_t function_addr, node_type_t activation_node_type, uint32_t dummy)= 0 ;
		/*!
		 * \brief Joins multiple abstract memory states to one.
		 * This method joins must and may memory set by using different join methods.
		 * \param states Vector of memory states to join.
		 * \param dummy This parameter is only needed for DISPStateMaintainerStack.
		 * \returns The memory state after joining the input memory states.
		 */
		virtual abs_mem_set_t join(vector<abs_mem_set_t> states, uint32_t dummy) __attribute__((deprecated)) = 0;
		/*!
		 * \brief Joins multiple abstract memory states to one.
		 * This method joins must and may memory set by using different join methods.
		 * \param states Pointer to a vector of memory states to join.
		 * \param dummy This parameter is only needed for DISPStateMaintainerStack.
		 * \returns The memory state after joining the input memory states.
		 */
		virtual abs_mem_set_t join(vector<abs_mem_set_t> *states, uint32_t dummy)= 0 ;
		/*!
		 * \brief Prints a memory set, represented by a vector of abstract memory entries.
		 * \param os The string stream to which the memory state is written to.
		 * \param mem_state A memory state represented as vector of addresses. 
		 */
		virtual void printMemSet(ostringstream *os, vector<abs_mem_entry_t> mem_state) __attribute__((deprecated)) = 0;
		/*!
		 * \brief Prints a memory set, represented by a vector of abstract memory entries.
		 * \param os The string stream to which the memory state is written to.
		 * \param mem_state A pointer to a memory state represented as vector of addresses. 
		 */
		virtual void printMemSet(ostringstream *os, vector<abs_mem_entry_t> *mem_state) = 0;
		/*! 
		 * \brief Returns the size of a function in memory.
		 * Therefore the FunctionCallGraphObject is used. The size of the function in memory is based on the block_size
		 * \param address The address of a function, for which the size is returned.
		 * \returns The size of the function, if no size information for the requested address exists 0 is returned.
		 */
		virtual uint32_t getFunctionMemSize(uint32_t address) = 0;
	protected:
		/*!
		 * \brief Parameters of the DISP.
		 */
		disp_params_t disp_parameters;
		 /*!
		  * \brief Pointer to a function call graph object, that is needed to obtain the sizes of the functions.
		  */
		FunctionCallGraphObject *functions;
};

#endif
