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
#ifndef _FIFO_CACHE_MEM_STATE_HPP_
#define _FIFO_CACHE_MEM_STATE_HPP_

#include "constants.h"
#include "global.h"

/*!
 * \brief An entry of the concrete cache state used by FIFOCacheMemState.
 */
struct MemEntry{
	/*!
	 * The aligned cache address in the concrete cache state.
	 */
	uint32_t address; 
//	uint16_t position; // a position is not needed for a cache, this can be represented by using the index of the vector
};

/*!
 * \brief The concrete cache state used by FIFOCacheMemState.
 */
typedef vector<MemEntry> MemSet; 


/*!
 * \brief Class for implementation a concrete FIFO cache state.
 * It is used for a correct FIFO cache analysis using brute force.
 */
class FIFOCacheMemState {
	public:
		/*!
		 * \brief Constructor.
		 * \param maxLines Defines the maximal cache lines of the cache
		 */
		FIFOCacheMemState(uint32_t maxLines);
		/*!
		 * \brief Copy-constructor.
		 * \param copy The object to copy.
		 */
		FIFOCacheMemState(const FIFOCacheMemState& copy);
		/*!
		 * \brief Destructor.
		 */
		virtual ~FIFOCacheMemState();
		/*!
		 * \brief Implements the changes on the concrete cache state if a cache line is accessed.
		 * The behaviour depends on the content of the cache. The concrete cache state may be altered after accessing the cache line, e.g. by loading it on a miss.
		 * \param addr The aligned cache address that is accessed.
		 */
		void accessAddress(uint32_t addr);
		/*!
		 * \brief Determines if a certain address is currently in the cache.
		 * \param addr The aligned cache address that is tested.
		 * \returns True if the address is cached, else false.
		 */
		bool isInState(uint32_t addr);
		/*!
		 * \brief Prints the content of the concrete cache state into a given stream.
		 * \param os The stream to print the cache state content into.
		 */
		void print(ostringstream *os);
		/*!
		 * \brief Copy-operator: copies the content of an object into another one.
		 * \param other Object that is the source for the copy by value
		 * \returns A new object with the same content, but at another address.
		 */
		FIFOCacheMemState& operator=(const FIFOCacheMemState& other);
		/*!
		 * \brief Compares two concrete cache memory states.
		 * \param other The object to compare.
		 * \returns True if the concrete cache states are equal, else false
		 */
		bool operator== (const FIFOCacheMemState& other);
		/*!
		 * \brief Returns the memory size used for the representation of the concrete cache state.
		 * The size includes the size of the content container and the size of the class.
		 * \returns The memory size used for the representation of the concrete cache state.
		 */
		uint32_t getAllocatedSize(void);
		/*!
		 * \brief Returns the number of maintained memory references.
		 * \returns The number of maintained memory references.
		 */
		uint32_t getNumberOfMaintainedReferences(void);
	private:
		/*!
		 * \brief Default constructor (declared as private).
		 */
		FIFOCacheMemState();
		/*!
		 * \brief The maximum number of cache lines of the cache represented by this class.
		 */
		uint32_t max_lines;
		/*!
		 * \brief The content of the concrete cache state
		 */
		MemSet content;
};

#endif
