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
#ifndef _CACHE_STATE_MAINTAINER_HPP_
#define _CACHE_STATE_MAINTAINER_HPP_

#include "global.h"
#include "constants.h"
#include "graph_structure.h"

#include "mem_state_maintainer.hpp"

/*!
 * Abstract class for instruction cache data flow analysis wrapping the update() and join() function.
 */
class CacheStateMaintainer : public MemStateMaintainer {
	public:
		/*!
		 * \brief Constructor
		 * \param params The parameters of the cache memory.
		 */
		CacheStateMaintainer(cache_params_t params);
		/*!
		 * \brief Updates the abstract memory state set by a set of cache line addresses that are obtained.
		 * This method updates the must and may memory set. 
		 * \param state The input memory state (containing must and may state).
		 * \param line_addrs A vector of cache line addresses that are obtained (fetched) for execution of one basic block.
		 * \param use_stats If true the updated memory state is considered in the memory usage statistics.
		 * \returns Memory state that takes the changes of the initial state caused by loading the given cache line addresses into account.
		 */
		virtual abs_mem_set_t update(abs_mem_set_t state, vector<uint32_t> line_addrs, bool use_stats)__attribute__((deprecated)) = 0;
		/*!
		 * \brief Updates the abstract memory state set by a set of cache line addresses that are obtained.
		 * This method updates the must and may memory set. 
		 * \param state A pointer to the input memory state (containing must and may state).
		 * \param line_addrs A pointer to the vector of cache line addresses that are obtained (fetched) for execution of one basic block.
		 * \param use_stats If true the updated memory state is considered in the memory usage statistics.
		 * \returns Memory state that takes the changes of the initial state caused by loading the given cache line addresses into account.
		 */
		virtual abs_mem_set_t update(abs_mem_set_t *state, vector<uint32_t> *line_addrs, bool use_stats) = 0;
		/*!
		 * \brief Updates the abstract memory state set by a cache line address that are obtained.
		 * This method updates the must and may memory set. 
		 * \param state A pointer to the input memory state (containing must and may state).
		 * \param line_addr A cache line address that is accessed in the current state.
		 * \param use_stats If true the updated memory state is considered in the memory usage statistics.
		 * \returns Memory state that takes the changes of the initial state caused by accessing the given cache line address into account.
		 */
		virtual abs_mem_set_t update(abs_mem_set_t *state, uint32_t line_addr, bool use_stats) = 0;
		/*!
		 * \brief Joins multiple abstract memory states to one.
		 * This method joins must and may memory set by using different join methods.
		 * \param states Vector of memory states to join.
		 * \returns The memory state resulting after joining the input memory states
		 */
		virtual abs_mem_set_t join(vector<abs_mem_set_t> states)__attribute__((deprecated)) = 0;
		/*!
		 * \brief Joins multiple abstract memory states to one.
		 * This method joins must and may memory set by using different join methods.
		 * \param states A pointer to a vector of memory states to join.
		 * \returns The memory state resulting after joining the input memory states
		 */
		virtual abs_mem_set_t join(vector<abs_mem_set_t> *states) = 0;
	protected:
		/*!
		 * \brief The parameters of the cache memory.
		 */
		cache_params_t cache_parameters;
		/*!
		 * \brief Pointer to the LOGCXX logger object.
		 */
		static LoggerPtr logger;

};

#endif
