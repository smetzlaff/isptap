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
#ifndef _FIFO_BF_CACHE_STATE_MAINTAINER_HPP_
#define _FIFO_BF_CACHE_STATE_MAINTAINER_HPP_

#include "constants.h"
#include "graph_structure.h"
#include "fifo_cache_mem_state.hpp"
#include "memory_params.hpp"


#include "mem_usage_stats.hpp"

/*!
 * \brief The abstract cache state used by FIFOBFCacheStateMaintainer.
 * It is represented as a vector of FIFOCacheMemState objects.
 */
typedef vector<FIFOCacheMemState>  AbsMemSet;

/*!
 * \brief A map to store the abstract cache states for the FIFOBFCacheStateMaintainer for each MSGVertex node in the graph.
 */
typedef map<MSGVertex, AbsMemSet*> NodeMemSetMap;

/*!
 * \brief Cache state maintainer for brute force FIFO analysis supporting update() and join() for data flow analysis.
 * Since no other solution is found, every possible concrete state is maintained in the abstract state.
 */
class FIFOBFCacheStateMaintainer : public MemoryUsageStats {
	public:
		/*!
		 * \brief Constructor.
		 * \param params The parameters of the cache memory.
		 */
		FIFOBFCacheStateMaintainer(cache_params_t params);
		/*!
		 * \brief Default destructor.
		 */
		virtual ~FIFOBFCacheStateMaintainer();
		 /*!
		 * \brief Updates the abstract cache state set by a set of cache line addresses that are accessed.
		 * By design every state that is assigned to a node represents the inital state of the node, before it is executed. So to get the inital state of a node the state of the precessing node has to be updated by the cache addresses that are accessed by the precessing node.
		 * \param predecessor The predecessor node, from which the cache state is updated.
		 * \param node The current node, of which the new cache state is created.
		 * \param line_addrs The accessed cache addresses that update the state of the predecessor to the correct state of the curent node.
		 */
		void update(MSGVertex predecessor, MSGVertex node, vector<uint32_t> *line_addrs);
		/*!
		 * \brief Joins multiple abstract cache states to one.
		 * \param predecessors A vector of nodes for which the control flow is merged.
		 * \param predecessor_cache_lines A vector of cache lines for each predecessor to update the cache state before they are merged.
		 * \param node The node in which the predecessor control flows are joined.
		 */
		void join(vector<MSGVertex> *predecessors, vector< vector< uint32_t > > *predecessor_cache_lines, MSGVertex node);
		/*!
		 * \brief Transfers an abstract cache state from one node to another, without changing it.
		 * When transferring an abstract cache state the new node is registered to the same address of the abstract cache state as the original one.
		 * \param predecessor The node of which the abstract cache state is to be read.
		 * \param node The node to which the same abstract cache state as its predecessor is assigned.
		 */
		void transfer(MSGVertex predecessor, MSGVertex node);
		/*!
		 * \brief Prints the abstract cache state of a given node.
		 * The node has to be processed and a abstract cache state is created, otherwise an assertion is violated.
		 * \param os The stream in which the abstract cache state is written into.
		 * \param node The node of which the abstract cache state is printed.
		 */
		void printMemSet(ostringstream *os, MSGVertex node);
		/*!
		 * \brief Determines if an address is definitely in the abstract cache state of a node.
		 * \param prev_addrs Vector of addresses that are accessed after the initial state of the node. (Thus the initial state is altered.)
		 * \param addr The address to be checked if it is definitely in the abstract cache state.
		 * \param node The node for which it is checked if the address is definitely in the abstract cache state.
		 * \returns True if the address is definitely in the abstract cache state, else false.
		 */
		bool isInMust(vector<uint32_t> *prev_addrs, uint32_t addr, MSGVertex node);
		/*!
		 * \brief Determines if an address is definitely in the abstract cache state of a node.
		 * \param addr The address to be checked if it is definitely in the abstract cache state.
		 * \param node The node for which it is checked if the address is definitely in the abstract cache state.
		 * \returns True if the address is definitely in the abstract cache state, else false.
		 */
		bool isInMust(uint32_t addr, MSGVertex node);
		/*!
		 * \brief Determines if an address might be in the abstract cache state of a node.
		 * \param prev_addrs Vector of addresses that are accessed after the initial state of the node. (Thus the initial state is altered.)
		 * \param addr The address to be checked if it might be in the abstract cache state.
		 * \param node The node for which it is checked if the address might be in the abstract cache state.
		 * \returns True if the address might be in the abstract cache state, else false.
		 */
		bool isInMay(vector<uint32_t> *prev_addrs, uint32_t addr, MSGVertex node);
		/*!
		 * \brief Determines if an address might be in the abstract cache state of a node.
		 * \param addr The address to be checked if it might be in the abstract cache state.
		 * \param node The node for which it is checked if the address might be in the abstract cache state.
		 * \returns True if the address might be in the abstract cache state, else false.
		 */
		bool isInMay(uint32_t addr, MSGVertex node);
		/*!
		 * \brief Sets a blank abstract cache state for a given node.
		 * \param node The node for which a new empty abstract cache state is to be created.
		 */
		void setBlankMemState(MSGVertex node);
	private:
		/*!
		 * \brief Default constructor.
		 * Made private.
		 */
		FIFOBFCacheStateMaintainer();
		/*!
		 * \brief Gets the abstract cache state of a given node.
		 * If the abstract cache state is not created an assertion is violated.
		 * All abstract cache states are stored in the NodeMemSetMap.
		 * \param node The node for which the abstract cache state is obtained.
		 * \returns The pointer to the abstract cache state, if it is found. Else an assertion is violated.
		 */
		AbsMemSet *getAbsMemState(MSGVertex node);
		/*!
		 * \brief Registers an abstract cache state for a given node in the NodeMemSetMap.
		 * This is necessary to get the abstract cache state later on using getAbsMemState().
		 * \param node The node for which the abstract cache state is registered.
		 * \param state The pointer to the abstract cache state that is to be registered.
		 * \returns True if the abstract cache state was registered for that node, else false.
		 */
		bool addAbsMemState(MSGVertex node, AbsMemSet *state);
		/*!
		 * \brief Removes duplicate concrete cache states from the abstract cache state.
		 * \param state The pointer to the abstract cache state of which the duplicates will be deleted.
		 */
		void removeDuplicates(AbsMemSet *state);
		/*!
		 * \brief Updates an abstract cache state by accessing an aligned cache address.
		 * Depending on the content of the abstract cache state it may be altered.
		 * \param state The abstract cache state that is to be updated by activation of one address.
		 * \param line_addr The aligned cache address that is activated on the current abstract cache state.
		 */
		void updateState(AbsMemSet *state, uint32_t line_addr);
		/*!
		 * \brief Prints a given abstract cache state into a stream.
		 * \param os The stream in which the abstract cache state is to be printed into.
		 * \param state The abstract cache state that will be printed.
		 */
		void printMemSet(ostringstream *os, AbsMemSet *state);
		/*!
		 * \brief Checks if an address is in all possible concrete cache states of a node.
		 * \param prev_addrs Vector of addresses that are accessed after the initial state of the node. (Thus the initial state is altered.)
		 * \param addr The address to be checked if it is in all concrete cache states.
		 * \param node The node for which it is checked if the address is present in all concrete cache states.
		 * \returns True if the address is in all concrete cache states, else false.
		 */
		bool isInAllSets(vector<uint32_t> *prev_addrs, uint32_t addr, MSGVertex node);
		/*!
		 * \brief Checks if an address is in all possible concrete cache states of a node.
		 * \param addr The address to be checked if it is in all concrete cache states.
		 * \param node The node for which it is checked if the address is present in all concrete cache states.
		 * \returns True if the address is in all concrete cache states, else false.
		 */
		bool isInAllSets(uint32_t addr, MSGVertex node);
		/*!
		 * \brief Checks if an address is in all possible concrete cache states.
		 * \param addr The address to be checked if it is in all concrete cache states.
		 * \param state The abstract cache state for which it is checked if the address is present in all of its concrete cache states.
		 * \returns True if the address is in all concrete cache states, else false.
		 */
		bool isInAllSets(uint32_t addr, AbsMemSet *state);
		/*!
		 * \brief Checks if an address is in any possible concrete cache state of a node.
		 * \param prev_addrs Vector of addresses that are accessed after the initial state of the node. (Thus the initial state is altered.)
		 * \param addr The address to be checked if it is in any concrete cache state.
		 * \param node The node for which it is checked if the address is present in any concrete cache state.
		 * \returns True if the address is in any concrete cache states, else false.
		 */
		bool isInAnySet(vector<uint32_t> *prev_addrs, uint32_t addr, MSGVertex node);
		/*!
		 * \brief Checks if an address is in any possible concrete cache state of a node.
		 * \param addr The address to be checked if it is in any concrete cache state.
		 * \param node The node for which it is checked if the address is present in any concrete cache state.
		 * \returns True if the address is in any concrete cache states, else false.
		 */
		bool isInAnySet(uint32_t addr, MSGVertex node);
		/*!
		 * \brief Checks if an address is in any possible concrete cache state of a node.
		 * \param addr The address to be checked if it is in any concrete cache state.
		 * \param state The abstract cache state for which it is checked if the address is present in any of its concrete cache states.
		 * \returns True if the address is in any concrete cache states, else false.
		 */
		bool isInAnySet(uint32_t addr, AbsMemSet *state);
		/*!
		 * \brief Checks if an address is no possible concrete cache state of a node.
		 * \param prev_addrs Vector of addresses that are accessed after the initial state of the node. (Thus the initial state is altered.)
		 * \param addr The address to be checked if it is no concrete cache state.
		 * \param node The node for which it is checked if the address is present in no concrete cache state.
		 * \returns True if the address is in no concrete cache state, else false.
		 */
		bool isInNoSet(vector<uint32_t> *prev_addrs, uint32_t addr, MSGVertex node);
		/*!
		 * \brief Checks if an address is no possible concrete cache state of a node.
		 * \param addr The address to be checked if it is no concrete cache state.
		 * \param node The node for which it is checked if the address is present in no concrete cache state.
		 * \returns True if the address is in no concrete cache state, else false.
		 */
		bool isInNoSet(uint32_t addr, MSGVertex node);
		/*!
		 * \brief Checks if an address is no possible concrete cache state of a node.
		 * \param addr The address to be checked if it is no concrete cache state.
		 * \param state The abstract cache state for which it is checked if the address is present in no of its concrete cache states.
		 * \returns True if the address is in no concrete cache state, else false.
		 */
		bool isInNoSet(uint32_t addr, AbsMemSet *state);
		/*!
		 * \brief The parameters of the cache memory.
		 */
		cache_params_t cache_parameters;
		/*!
		 * \brief The map that contains for each processed node the corresponding abstract cache state.
		 */
		NodeMemSetMap state_map;
		/*!
		 * \brief Pointer to the LOGCXX logger object.
		 */
		static LoggerPtr logger;
};

#endif
