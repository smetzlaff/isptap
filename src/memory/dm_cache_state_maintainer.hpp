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
#ifndef _DIRECT_MAPPED_CACHE_STATE_MAINTAINER_HPP_
#define _DIRECT_MAPPED_CACHE_STATE_MAINTAINER_HPP_

#include "global.h"
#include "constants.h"
#include "graph_structure.h"

#include "cache_state_maintainer.hpp"


/*!
 * \brief Class for instruction cache data flow analysis implementing the update() and join() function for a direct mapped cache.
 * <br>For a better comparability of the cache and the scratchpads, we implemented the cache such that it supports an arbitrary number of cache lines. We used a modulo operation to select the cache lines. In contrast to that, caches usually allow only a number of cache lines that is a power of two, because the length of the cache tag which is a part of the address defines the number of cache lines. So cache lines can be selected directly by the cache tag part of the address.
 * <br><b>TODO:</b> The PERSIST-Analysis is not implemented!
 */
class DirectMappedCacheStateMaintainer : public CacheStateMaintainer {
	public:
		/*!
		 * \brief Constructor
		 * \param params The parameters of the cache memory.
		 */
		DirectMappedCacheStateMaintainer(cache_params_t params);
		/*!
		 * \brief Updates the abstract memory state set by a set of cache line addresses that are obtained.
		 * This method updates the must, may and persist memory set. 
		 * \param state The input memory state (containing must, may and persist state).
		 * \param line_addrs A vector of cache lines addresses that are obtained (fetched) for execution of one basic block.
		 * \param use_stats If true the updated memory state is considered in the memory usage statistics.
		 * \returns Memory state that takes the changes of the initial state caused by loading the given cache line addresses into account.
		 */
		abs_mem_set_t update(abs_mem_set_t state, vector<uint32_t> line_addrs, bool use_stats) __attribute__((deprecated));
		/*!
		 * \brief Updates the abstract memory state set by a set of cache line addresses that are obtained.
		 * This method updates the must, may and persist memory set. 
		 * \param state A pointer to the input memory state (containing must, may and persist state).
		 * \param line_addrs A pointer to the vector of cache line addresses that are obtained (fetched) for execution of one basic block.
		 * \param use_stats If true the updated memory state is considered in the memory usage statistics.
		 * \returns Memory state that takes the changes of the initial state caused by loading the given cache line addresses into account.
		 */
		abs_mem_set_t update(abs_mem_set_t *state, vector<uint32_t> *line_addrs, bool use_stats);
		/*!
		 * \brief Updates the abstract memory state set by a cache line address that are obtained.
		 * This method updates the must and may memory set. 
		 * \param state A pointer to the input memory state (containing must and may state).
		 * \param line_addr A cache line address that is accessed in the current state.
		 * \param use_stats If true the updated memory state is considered in the memory usage statistics.
		 * \returns Memory state that takes the changes of the initial state caused by accessing the given cache line address into account.
		 */
		abs_mem_set_t update(abs_mem_set_t *state, uint32_t line_addr, bool use_stats);
		/*!
		 * \brief Joins multiple abstract memory states to one.
		 * This method joins must, may and persist memory set by using different join methods.
		 * \param states Vector of memory states to join.
		 * \returns The memory state resulting after joining the input memory states
		 */
		abs_mem_set_t join(vector<abs_mem_set_t> states) __attribute__((deprecated));
		/*!
		 * \brief Joins multiple abstract memory states to one.
		 * This method joins must, may and persist memory set by using different join methods.
		 * \param states A pointer to a vector of memory states to join.
		 * \returns The memory state resulting after joining the input memory states
		 */
		abs_mem_set_t join(vector<abs_mem_set_t> *states);
	private:
		/*!
		 * \brief Adds an cache line address to a memory set using the DirectMapped replacement policy.
		 * By prechecks (isInSet()) it is ensured that the address is not already within the set. 
		 * \param addr The cache line address to be put into the memory set.
		 * \param mem_set The memory set to which the cache line address is added to. The memory set is represented as a vector of abstract memory entries.
		 * \param evict If true lines with the same position are evicted, else multiple lines may have the same position in cache.
		 * \returns If lines were evicted the corresponding addresses are returned, else an empty vector.
		 */
		vector<uint32_t> addAddrToMemSet(uint32_t addr, vector<abs_mem_entry_t> *mem_set, bool evict);
};

#endif
