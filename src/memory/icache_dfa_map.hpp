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
#ifndef _ICACHE_DFA_MAP_HPP_
#define _ICACHE_DFA_MAP_HPP_

#include "icache_dfa_IF.hpp"

#include "fifo_bf_cache_state_maintainer.hpp"

/*!
 * \brief Class to analyse the data flow of a fully associative instruction cache.
 * Analyzes the instruction cache hit/miss rate of a given memory state graph / control flow graph pair.
 */
class ICacheDataFlowAnalyzerMap : public ICacheDataFlowAnalyzer_IF {
	public:
		/*!
		 * \brief Constructor to create the cache analyzer object.
		 * \param graph_pair A pair of memory state graph and the reference control flow graph, whose cache behaviour should be analyzed.
		 * \param entry_node A pair of entry nodes for the memory state graph and it's reference control flow graph.
		 * \param exit_node A pair of exit nodes of the memory state graph and it's reference control flow graph.
		 */
		ICacheDataFlowAnalyzerMap(CFGMSGPair graph_pair, CFGMSGVPair entry_node, CFGMSGVPair exit_node);
		/*!
		 * \brief Constructor to create the cache analyzer object, with arbitrary cache memory size.
		 * \param graph_pair A pair of memory state graph and the reference control flow graph, whose cache behaviour should be analyzed.
		 * \param entry_node A pair of entry nodes for the memory state graph and it's reference control flow graph.
		 * \param exit_node A pair of exit nodes of the memory state graph and it's reference control flow graph.
		 * \param mem_size The memory size of the cache (if the CONF_MEMORY_SIZE should not be used)
		 */
		ICacheDataFlowAnalyzerMap(CFGMSGPair graph_pair, CFGMSGVPair entry_node, CFGMSGVPair exit_node, uint32_t mem_size);

		/*!
		 * \brief The default destructor.
		 */
		virtual ~ICacheDataFlowAnalyzerMap();
		/*!
		 * \brief Analyzes the cache for the given parameters (or obtained by global configuration object).
		 */
		void analyzeCache(void);

		/*!
		 * \brief Counts all Hits, Misses and NC and the number of unique cache lines to print a log line.
		 */
		void categorizeCacheAccesses(void);
		/**
		 * \brief Returns the memory size that is used for the representation of the cache memory states in the data flow analysis.
		 * For this analysis the concrete states are taken into account.
		 * \returns The memory size that is used for the representation of the cache memory states in the data flow analysis.
		 */
		uint64_t getUsedMemSize(void);
		/*!
		 * \brief Returns the number of maintained memory references (cache lines or function addresses) that is needed by the memory states used by the data flow analysis.
		 * For this analysis the concrete states are taken into account.
		 * \returns The number of maintained memory references that is needed by the memory states used by the data flow analysis.
		 */
		uint64_t getUsedMemReferences(void);
		/**
		 * \brief Returns the number of concrete cache memory states needed for the data flow analysis.
		 * For this analysis the concrete states are taken into account.
		 * \returns The number of concrete cache memory states needed for the data flow analysis.
		 */
		uint64_t getRepresentationStateCount(void);
		/*!
		 * \brief Returns the different memory states that are distinguished by the data flow analysis.
		 * \returns The different memory states that are distinguished by the data flow analysis.
		 */
		uint64_t getMemoryStateCount(void);
	private:
		/*!
		 * \brief Sets the cache parameters.
		 * This function is called on construction using the parameters of the global configuration object.
		 */
		void setCacheParameters(void);
		/*!
		 * \brief Checks if the memory state of all predecessors of the node under observation is already known.
		 * \param node The pair of memory state graph node and control flow graph under observation.
		 * \returns False of at least one predecessor node has not an calculated memory state.
		 */
		bool isPredecessorMemStateKnown(CFGMSGVPair node);
		/*!
		 * \brief Joins the memory sates of all predecessors of the node and sets the memory state of the node.
		 * All predecessor memory states are updated to represent the memory state after their execution and then joined. The memory state of the node under observation that is calculated is the state of the memory _before_ the node is executed (initial memory state)
		 * \param node The pair of memory state graph node and control flow graph under observation. Has to have more than one predecessor nodes and all predecessor memory states have to be known.
		 */
		void joinAndSetInitialMemState(CFGMSGVPair node);
		/*!
		 * \brief Sets the initial memory state of the node by taking the initial memory state of the predecessor into account and the memory state changes by executing the predecessor node.
		 * The predecessor memory state is updated to represent the memory state after it's execution. The memory state of the node under observation that is calculated is the state of the memory _before_ the node is executed (initial memory state).
		 * For the entry node (that has no predecessors) a blank memory state is produced.
		 * \param node The pair of memory state graph node and control flow graph under observation. Has to have one predecessor nodes those memory states have to be known.
		 */
		void setInitialMemState(CFGMSGVPair node);
		/*!
		 * \brief Calculates the cache line addresses of a given node and returns it.
		 * If the given node is no basic block, nothing needs to be done. The returned addresses depends on the CONF_MEMORY_CACHE_BBS option. If set only the start address of the corresponding basic block is returned. Else the method calculates all addresses of the cache lines needed to be obtained to fully execute the given basic block node. 
		 * \param node The control flow graph node for which the cache line addresses will be calculated.
		 * \returns The addresses of the cache lines as vector. If the node is no BasicBlock an empty vector is returned.
		 */
		vector<uint32_t> getCacheLinesOfCFGNode(CFGVertex node);
		/*!
		 * \brief Calculates the cache penalty for each node in the memory state graph.
		 * This method adds to each out egde of each basic block a penalty for cache misses (into dynamicMemPenaltyEProp in MSG). 
		 */
		void calculateCachePenalty(void);

		/*!
		 * \brief Returns the number of non BackwardJump in edges of the given node.
		 * \param v A memory state graph node to obtain the number of in edges.
		 * \returns Returns the number of non BackwardJump in edges of the given node.
		 */
		uint32_t getForwardInEdgeCount(MSGVertex v);

		/*!
		 * \brief Returns the cost in cycles for handling a miss for one cache line.
		 * \returns Returns the cost in cycles for handling a miss for one cache line.
		 */
		inline uint32_t getCacheMissCost();

		/*!
		 * \brief Pointer to object hat maintains the abstract cache states depending on it's replacement policy.
		 */
		FIFOBFCacheStateMaintainer *cache_state_maintainer;

		// TODO document graph properties
		property_map<ControlFlowGraph, nodetype_t>::type nodeTypeNProp;
		property_map<ControlFlowGraph, startaddr_t>::type startAddrNProp;
		property_map<ControlFlowGraph, startaddrstring_t>::type startAddrStringNProp;
		property_map<ControlFlowGraph, bbsize_t>::type bbSizeNProp;

		// TODO document graph properties
		property_map<MemoryStateGraph, mem_state_t>::type memStateNProp;
		property_map<MemoryStateGraph, mem_state_valid_t>::type memStateValNProp;
		property_map<MemoryStateGraph, cfg_vertex_t>::type mappedCFGVertexNProp;
		property_map<MemoryStateGraph, msg_edgetype_t>::type msgEdgeTypeEProp;
		property_map<MemoryStateGraph, dynamic_mem_penalty_t>::type dynamicMemPenaltyEProp;

		/**
		 * \brief Pointer to the LOGCXX logger object.
		 */
		static LoggerPtr logger;
};

#endif
