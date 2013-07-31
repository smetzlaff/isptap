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
#ifndef _MEM_STATE_MAINTAINER_HPP_
#define _MEM_STATE_MAINTAINER_HPP_

#include "global.h"
#include "constants.h"
#include "graph_structure.h"
#include "mem_usage_stats.hpp"
#include "memory_params.hpp"

// for setw() and setfill()
#include <iostream>
#include <iomanip>

/*!
 * \brief Maps the age of an abstract memory entry to its address.
 * Used to easily determine the age of an abstract memory entry of a memory set by it's address
 */
typedef map<uint32_t, uint32_t> AddrAgeMap;

/*!
 * \brief Abstract class for memory data flow analysis wrapping basic functions.
 * Implements basic functions to handle abstract memory states like: determining if an address/function is in the abstract memory state, or printing and sorting an abstract memory state.
 */
class MemStateMaintainer : public MemoryUsageStats {
	public:
		/*!
		 * \brief Searches for an address within a given memory set.
		 * \param addr_to_find The address to find in the given set.
		 * \param mem_set Memory set to search for an address. The memory set is represented as a vector of abstract memory entries.
		 * \returns True if the address is in the set, else false.
		 */
		bool isInSet(uint32_t addr_to_find, vector<abs_mem_entry_t> mem_set) __attribute__((deprecated));
		/*!
		 * \brief Searches for an address within a given memory set.
		 * \param addr_to_find The address to find in the given set.
		 * \param mem_set Pointer to memory set to search for an address. The memory set is represented as a vector of abstract memory entries.
		 * \returns True if the address is in the set, else false.
		 */
		bool isInSet(uint32_t addr_to_find, vector<abs_mem_entry_t> *mem_set);
		/*!
		 * \brief Sets a blank memory state for a memory set.
		 * This is needed to clear the mem state before the execution starts. (It can also be used to model a mem flush.)
		 * \param initial_set_must The pointer to the must memory set that is written. The memory set is represented as a vector of addresses.
		 * \param initial_set_may The pointer to the may memory set that is written. The memory set is represented as a vector of addresses.
		 */
		void setBlankMemState(vector<abs_mem_entry_t> *initial_set_must, vector<abs_mem_entry_t> *initial_set_may);
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
		 * \brief Prints a vector of addresses.
		 * \param os The string stream to which the memory state is written to.
		 * \param addrs A vector of addresses to print.
		 */
		void printAddresses(ostringstream *os, vector<uint32_t> addrs) __attribute__((deprecated));
		/*!
		 * \brief Prints a vector of addresses.
		 * \param os The string stream to which the memory state is written to.
		 * \param addrs A pointer to a vector of addresses to print.
		 */
		void printAddresses(ostringstream *os, vector<uint32_t> *addrs);
		/*!
		 * \brief Returns all unique addresses of both input sets in a vector.
		 * Builds a union of the addresses of the abstract memory entries in both sets.
		 * \param set_a An input memory set: a vector of abstract memory entries.
		 * \param set_b An input memory set: a vector of abstract memory entries.
		 * \returns The addresses contained in both input memory set as a vector. Each address is held only once in this vector.
		 */
		vector<uint32_t> getUniqueMemAddrsOfSets(vector<abs_mem_entry_t> set_a, vector<abs_mem_entry_t> set_b) __attribute__((deprecated));
		/*!
		 * \brief Returns all unique addresses of both input sets in a vector.
		 * Builds a union of the addresses of the abstract memory entries in both sets.
		 * \param set_a An input memory set: a pointer to a vector of abstract memory entries.
		 * \param set_b An input memory set: a pointer to a vector of abstract memory entries.
		 * \returns The addresses contained in both input memory set as a vector. Each address is held only once in this vector.
		 */
		vector<uint32_t> getUniqueMemAddrsOfSets(vector<abs_mem_entry_t> *set_a, vector<abs_mem_entry_t> *set_b);
		/*!
		 * \brief Adds addresses of an abstract memory set to a vector of addresses, iff they are not already in this vector.
		 * \param addr_set A pointer to a vector of (unique) addresses. This vector is altered to contain the also addresses of the abstract memory entries that are added.
		 * \param mem_set A memory set consisting of a vector of abstract memory entries, whose addresses should be add to the addr_set.
		 */
		void addUniqueMemAddrsToList(vector<uint32_t> *addr_set, vector<abs_mem_entry_t> mem_set) __attribute__((deprecated));
		/*!
		 * \brief Adds addresses of an abstract memory set to a vector of addresses, iff they are not already in this vector.
		 * \param addr_set A pointer to a vector of (unique) addresses. This vector is altered to contain the also addresses of the abstract memory entries that are added.
		 * \param mem_set A pointer to a memory set consisting of a vector of abstract memory entries, whose addresses should be add to the addr_set.
		 */
		void addUniqueMemAddrsToList(vector<uint32_t> *addr_set, vector<abs_mem_entry_t> *mem_set);
		/*!
		 * \brief Sorts an memory set by the age of the containing abstract memory entries for the LRU and FIFO policy.
		 * As comparison function sort_by_age(abs_mem_entry_t a, abs_mem_entry_t b) is used. 
		 * \param mem_set Pointer to a memory set that is to be sorted by the age of the containing abstract memory entries.
		 */
		void sortMemSetByAge(vector<abs_mem_entry_t> *mem_set);
		/*!
		 * \brief Sorts an memory set by the position of the containing abstract memory entries for the direct mapped policy.
		 * This method simply calls the sortMemSetByAge(vector<abs_mem_entry_t> *mem_set) method, since the position and age are represented in the same field of abs_mem_entry_t.
		 * \param mem_set Pointer to a memory set that is to be sorted by the age of the containing abstract memory entries.
		 */
		void sortMemSetByPosition(vector<abs_mem_entry_t> *mem_set);
		/*!
		 * \brief Adds all members of a memory set to a map containing the addresses and the age in the abstract memory set.
		 * FIXME is the AddrAgeMap really useful?
		 * \param map The address and age map in which the abstract memory entries are added.
		 * \param mem_set The memory set, of which all containing addresses will be added to the address and age map. The memory set is represented as a vector of abstract memory entries.
		 */
		void addAddrAndAgeToMap(AddrAgeMap *map, vector<abs_mem_entry_t> mem_set) __attribute__((deprecated));
		/*!
		 * \brief Adds all members of a memory set to a map containing the addresses and the age in the abstract memory set.
		 * FIXME is the AddrAgeMap really useful?
		 * \param map The address and age map in which the abstract memory entries are added.
		 * \param mem_set A pointer to the memory set, of which all containing addresses will be added to the address and age map. The memory set is represented as a vector of abstract memory entries.
		 */
		void addAddrAndAgeToMap(AddrAgeMap *map, vector<abs_mem_entry_t> *mem_set);


};

#endif
