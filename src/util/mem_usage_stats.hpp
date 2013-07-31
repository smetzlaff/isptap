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
#ifndef _MEM_USAGE_STATS_HPP_
#define _MEM_USAGE_STATS_HPP_

#include "global.h"
#include "constants.h"

/*!
 * \brief Small class to allow measuring the complexity of data flow analysis.
 * This is done by registering every memory state object (concrete or abstract) with its memory size that is used to track the content of a dynamic memory.
 */
class MemoryUsageStats {
	public:
		/*!
		 * \brief Returns the number of different memory states that are registered while data flow analysis.
		 * \returns The number of different memory states that are registered while the data flow analysis.
		 */
		uint64_t getMemoryStateCount(void);
		/*!
		 * \brief Returns the number of concrete or abstract memory states used by the data flow analysis.
		 * \returns The number of memory states used by the data flow analysis.
		 */
		uint64_t getRepresentationStateCount(void);
		/*!
		 * \brief Returns the used memory that is needed by the memory states used by the data flow analysis.
		 * \returns The used memory that is needed by the memory states used by the data flow analysis in bytes.
		 */
		uint64_t getUsedSize(void);
		/*!
		 * \brief Returns the number of maintained memory references (cache lines or function addresses) that is needed by the memory states used by the data flow analysis.
		 * \returns The number of maintained memory references that is needed by the memory states used by the data flow analysis.
		 */
		uint64_t getUsedMemReferences(void);
	protected:
		/*!
		 * \brief Constructor. 
		 * Declared as protected to disallow instantiation without inheritance.
		 */
		MemoryUsageStats();
		/*!
		 * \brief Destructor.
		 */
		virtual ~MemoryUsageStats();
		/*!
		 * \brief Increased the memory usage counters on creation of a new memory state in the data flow analysis.
		 * \param number The number of concrete/abstract states for that memory state.
		 * \param size_for_all_objects The size of all allocated concrete/abstract states for that memory state.
		 * \param maintained_mem_references The number of all stored memory references in the concrete/abstract states for that memory state.
		 */
		void allocatedMemoryState(uint32_t number, uint64_t size_for_all_objects, uint64_t maintained_mem_references);
	private:
		/*!
		 * \brief The number of different memory states that are distinguished by data flow analysis.
		 * A memory state may contain multiple concrete or abstract (a must and a may state) states.
		 */
		uint64_t no_memory_states;
		/*!
		 * \brief The number of concrete or abstract memory states that are created by the data flow analysis to track the content of a dynamic memory.
		 */
		uint64_t no_representation_states;
		/*!
		 * \brief The memory size in bytes used by the memory states that are created by the data flow analysis to track the content of a dynamic memory.
		 */
		uint64_t used_mem_size;
		/*!
		 * \brief The number of memory references (cache lines or function addresses) used by the memory states that are created by the data flow analysis to track the content of a dynamic memory.
		 */
		uint64_t used_mem_references;
};
#endif
