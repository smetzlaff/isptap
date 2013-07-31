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
#ifndef _LRU_DISP_STATE_MAINTAINER_HPP_
#define _LRU_DISP_STATE_MAINTAINER_HPP_

#include "disp_state_maintainer_if.hpp"

/*!
 * \brief Class for data flow analysis of the dynamic instruction scratchpad implementing join() and update() functions for abstract memory states.
 */
class DISPStateMaintainerLRU : public DISPStateMaintainer_IF {
	public:
		/*!
		 * Constructor
		 * \param params The parameters of the DISP memory.
		 * \param fcgo Pointer to a function call graph object, that is needed to obtain the sizes of the functions.
		 */
		DISPStateMaintainerLRU(disp_params_t params, FunctionCallGraphObject* fcgo);
		/*!
		 * \brief Updates the memory state set by a function address that is to be executed.
		 * The method updates the must and may set.
		 * \param state The input abstract memory state (containing must and may state).
		 * \param function_addr The address of a function that will be executed next (on call or return).
		 * \param activation_node_type The type of the node that caused the invocation of the update().
		 * \param dummy This parameter is only needed for DISPStateMaintainerStack.
		 * \returns Memory state that takes the activation of the given function into account.
		 */
		abs_mem_set_t update(abs_mem_set_t state, uint32_t function_addr, node_type_t activation_node_type, uint32_t dummy) __attribute__((deprecated));
		/*!
		 * \brief Updates the memory state set by a function address that is to be executed.
		 * The method updates the must and may set.
		 * \param state A pointer to the input abstract memory state (containing must and may state).
		 * \param function_addr The address of a function that will be executed next (on call or return).
		 * \param activation_node_type The type of the node that caused the invocation of the update().
		 * \param dummy This parameter is only needed for DISPStateMaintainerStack.
		 * \returns Memory state that takes the activation of the given function into account.
		 */
		abs_mem_set_t update(abs_mem_set_t *state, uint32_t function_addr, node_type_t activation_node_type, uint32_t dummy);
		/*!
		 * \brief Joins multiple abstract memory states to one.
		 * This method joins must and may memory set by using different join methods.
		 * \param states Vector of memory states to join.
		 * \param dummy This parameter is only needed for DISPStateMaintainerStack.
		 * \returns The memory state after joining the input memory states.
		 */
		abs_mem_set_t join(vector<abs_mem_set_t> states, uint32_t dummy) __attribute__((deprecated));
		/*!
		 * \brief Joins multiple abstract memory states to one.
		 * This method joins must and may memory set by using different join methods.
		 * \param states Pointer to a vector of memory states to join.
		 * \param dummy This parameter is only needed for DISPStateMaintainerStack.
		 * \returns The memory state after joining the input memory states.
		 */
		abs_mem_set_t join(vector<abs_mem_set_t> *states, uint32_t dummy);
		/*!
		 * \brief Prints a memory set, represented by a vector of abstract memory entries.
		 * \param os The string stream to which the memory state is written to.
		 * \param mem_state A memory state represented as vector of addresses. 
		 */
		void printMemSet(ostringstream *os, vector<abs_mem_entry_t> mem_state) __attribute__((deprecated));
		/*!
		 * \brief Prints a memory set, represented by a vector of abstract memory entries.
		 * \param os The string stream to which the memory state is written to.
		 * \param mem_state A pointer to a memory state represented as vector of addresses. 
		 */
		void printMemSet(ostringstream *os, vector<abs_mem_entry_t> *mem_state);
		/*! 
		 * \brief Returns the size of a function in memory.
		 * Therefore the FunctionCallGraphObject is used. The size of the function in memory is based on the block_size
		 * \param address The address of a function, for which the size is returned.
		 * \returns The size of the function, if no size information for the requested address exists 0 is returned.
		 */
		uint32_t getFunctionMemSize(uint32_t address);
	private:
		/*!
		 * \brief Adds a function to a memory state using FIFO replacement.
		 * The function is added with the age 0 and the age of all other functions is increased by the size of the added function. The method also checks if the function is to large to add.
		 * \param addr The address of the function to add.
		 * \param mem_set Pointer to the abstract memory state that is altered by adding the given function.
		 */
		void addAddrToMemSet(uint32_t addr, vector<abs_mem_entry_t> *mem_set);
		/*!
		 * \brief Moves the selected function line to the front (age 0).
		 * For all entries with ages less than the selected cache line it's age is incremented.
		 * TODO: untested!
		 * \param f_addr The function address to move to the front. The function address has to be in the abstract memory set.
		 * \param mem_set The memory set where ages of the functions addresses are updated. The memory set is represented as a vector of abstract memory entries.
		 * \param analysis Distinguishes between MUST or MAY analysis.
		 */
		void moveAddrToFront(uint32_t f_addr, vector<abs_mem_entry_t> *mem_set, analysis_type_t analysis);
		/*!
		 * \brief Checks if two functions intersect in an abtract LRU DISP representation.
		 * It is checked if [a_begin,a_end[ overlaps with [b_begin, b_end[
		 * \param a_begin The begin age of the function a.
		 * \param a_end The end age of the function a.
		 * \param b_begin The begin age of the function b.
		 * \param b_end The end age of the function b.
		 * \returns True if the two functions intersect, else false.
		 */
		bool inline checkFunctionIntersection(uint32_t a_begin, uint32_t a_end, uint32_t b_begin, uint32_t b_end);
		/*!
		 * \brief Calculates the new address of function g on activation of f.
		 * The calculation depends on the analysis type: 
		 * For must analysis the maximal age of g is calculated for the given initial maximum ages and sizes of g and f. The calculated age is: max(g_age, f_size + max(f_age - g_size, 0))
		 * For may analysis the minimal age of g is determined for the given intial minimum ages and sizes of g and f. The calculated age is: min(f_age + f_size, g_age + f_size)
		 * A requirement of this function is that both functions intersect (checkFunctionIntersection(f_age, f_age+f_size, g_age, g_age+g_size)). Otherwise the new age of g has to be calculated differently.
		 * \param analysis The type of analysis, determines if either a update of the must or may set is done.
		 * \param f_age The age of the function that is activated on update of the abstract memory state.
		 * \param f_size The size of the function that is activated on update of the abstract memory state.
		 * \param g_age The age of the function for which the updated age after activation of f is calculated.
		 * \param g_size The size of the function for which the updated age after activation of f is calculated.
		 */
		uint32_t getIntersectionalAge(analysis_type_t analysis, uint32_t f_age, uint32_t f_size, uint32_t g_age, uint32_t g_size);
		/**
		 * \brief Pointer to the LOGCXX logger object.
		 */
		static LoggerPtr logger;

};


#endif
