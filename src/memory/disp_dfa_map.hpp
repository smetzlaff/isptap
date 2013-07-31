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
#ifndef _DISP_DFA_MAP_HPP_
#define _DISP_DFA_MAP_HPP_

#include "disp_dfa_if.hpp"
#include "fifo_bf_disp_state_maintainer.hpp"
#include "stack_bf_disp_state_maintainer.hpp"
#include "context_stack.hpp"
#include "arch_cfg_factory.hpp"


/*!
 * \brief Data flow analyzer for the dynamic instruction scratchpad (DISP) using a map to hold the abstract DISP states.
 * Analyzes the DISP hit/miss rate of a given memory state graph / control flow graph pair.
 */
class DISPDataFlowAnalyzerMap : public DISPDataFlowAnalyzer_IF {
	public:
		/*!
		 * \brief Constructor for the DISP data flow analyze object.
		 * \param graph_pair A pair of memory state graph and the reference control flow graph, whose DISP behaviour should be analyzed.
		 * \param entry_node A pair of entry nodes for the memory state graph and it's reference control flow graph.
		 * \param exit_node A pair of exit nodes of the memory state graph and it's reference control flow graph.
		 * \param fcgo Pointer to a function call graph object, that is needed to obtain the sizes of the functions.
		 */
		DISPDataFlowAnalyzerMap(CFGMSGPair graph_pair, CFGMSGVPair entry_node, CFGMSGVPair exit_node, FunctionCallGraphObject *fcgo);
		/*!
		 * \brief Constructor for the disp data flow analyze object, with arbitrary DISP memory size.
		 * \param graph_pair A pair of memory state graph and the reference control flow graph, whose DISP behaviour should be analyzed.
		 * \param entry_node A pair of entry nodes for the memory state graph and it's reference control flow graph.
		 * \param exit_node A pair of exit nodes of the memory state graph and it's reference control flow graph.
		 * \param fcgo Pointer to a function call graph object, that is needed to obtain the sizes of the functions.
		 * \param mem_size The memory size of the DISP (if the CONF_MEMORY_SIZE should not be used)
		 */
		DISPDataFlowAnalyzerMap(CFGMSGPair graph_pair, CFGMSGVPair entry_node, CFGMSGVPair exit_node, FunctionCallGraphObject *fcgo, uint32_t mem_size);
		/*!
		 * \brief The default destructor.
		 */
		virtual ~DISPDataFlowAnalyzerMap();
		/*!
		 * \brief Analyzes the DISP for the given parameters (or obtained by global configuration object).
		 */
		void analyzeDISP(void);
		/*!
		 * \brief Counts all Hits, Misses and NC and the number of unique DISP lines to print a log line.
		 */
		void categorizeMemAccesses(void);
		/**
		 * \brief Returns the memory size that is used for the representation of the DISP memory states in the data flow analysis.
		 * For this analysis the concrete states are taken into account.
		 * \returns The memory size that is used for the representation of the DISP memory states in the data flow analysis.
		 */
		uint64_t getUsedMemSize(void);
		/*!
		 * \brief Returns the number of maintained memory references (cache lines or function addresses) that is needed by the memory states used by the data flow analysis.
		 * For this analysis the concrete states are taken into account.
		 * \returns The number of maintained memory references that is needed by the memory states used by the data flow analysis.
		 */
		uint64_t getUsedMemReferences(void);
		/**
		 * \brief Returns the number of DISP memory states needed for the data flow analysis.
		 * For this analysis the concrete states are taken into account.
		 * \returns The number of DISP memory states needed for the data flow analysis.
		 */
		uint64_t getRepresentationStateCount(void);
		/*!
		 * \brief Returns the different memory states that are distinguished by the data flow analysis.
		 * \returns The different memory states that are distinguished by the data flow analysis.
		 */
		uint64_t getMemoryStateCount(void);
	private:
		/*!
		 * \brief Sets the DISP parameters.
		 * This function is called on construction using the parameters of the global configuration object.
		 */
		void setMemParameters(void);
		/*!
		 * \brief Returns the number of non BackwardJump in edges of the given node.
		 * \param v A memory state graph node to obtain the number of in edges.
		 * \returns Returns the number of non BackwardJump in edges of the given node.
		 */
		uint32_t getForwardInEdgeCount(MSGVertex v);
		/*!
		 * \brief Calculates the memory penalty for each entry/exit node in the memory state graph.
		 * This method adds the penalty for DISP misses (into dynamicMemPenaltyEProp in MSG). 
		 * The memory penalty for calls is added to the out edge of the entry node resp. the in edge of the first basic block. For returns the memory penalty is added to the out edge of the last basic block node resp. the in edge of the exit node.
		 * Note: The MSG is inlined and unrolled (such that each entry/exit node has only one predecessor/successor).
		 */
		void calculateMemPenalty(void);
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
		 * \brief Obtains the context stack of a given node.
		 * The context stack holds all active functions for each node.
		 * \param mnode The node for which the context stack will be returned.
		 * \returns The context stack for the given node.
		 */
		ContextStack getContextForNode(MSGVertex mnode);
		/*!
		 * \brief Obtains the context stack of a given node.
		 * The context stack holds all active functions for each node.
		 * \param node The node for which the context stack will be returned.
		 * \returns The context stack for the given node.
		 */
		ContextStack getContextForNode(CFGMSGVPair node);
		/*!
		 * \brief Update the context stack for a given node.
		 * The function updates the context before handling the given node (initial state) to the state of the context after handling the node (initial state of all successors).
		 * If the node is of type Entry the function address of the corresponding function is pushed to the context stack. If the node is of type Exit the top element of the context stack is popped.
		 * \param node The node for which the context will be updated.
		 * \returns The updated context stack, this context is assigned as initial context for every successor (MSG) node.
		 */
		ContextStack updateContextForNode(CFGMSGVPair node);
		/*!
		 * \brief Glues a context stack the its corresponding node.
		 * The context stack represents the initial context, before the node is handled/executed.
		 * The relation is stored in the ctxMap.
		 * \param node The node for which the context will be stored.
		 * \param context The initial context stack of the node to be stored.
		 */
		void addContextForNode(CFGMSGVPair node, ContextStack context);
		/*!
		 * \brief Returns the function address of the Entry node of a function.
		 * The function address is the start address of the first basic block of a function.
		 * The entry node is a meta node which the first basic block of a function as successor.
		 * \param node The Entry node for which the function address will be determined.
		 * \returns The function address for the given entry node. If the node is no Entry node this function violates an assert.
		 */
		uint32_t getFunctionAddress(CFGMSGVPair node);
		/*!
		 * \brief Returns the function address of the Entry node of a function.
		 * The function address is the start address of the first basic block of a function.
		 * The entry node is a meta node which the first basic block of a function as successor.
		 * \param cnode The Entry node for which the function address will be determined.
		 * \returns The function address for the given entry node. If the node is no Entry node this function violates an assert.
		 */
		uint32_t getFunctionAddress(CFGVertex cnode);
		/*!
		 * \brief Calculates the penalty that is added by DISP miss handling and loading an arbitrary function (defined by its size) into the scratchpad.
		 * Notice: If the isFetchMemIndependent(), the function load overlaps with the call/ret handling, thus the penalty is the difference between microcode handling and DISP miss penalty. Otherwise the penalty is added to call/ret handling. 
		 * \param function_size_in_mem The used memory size of an arbitrary function. The size has to be in bytes and has to be a multiple of the memory block size.
		 * \param act Determines if the penalty will be calculated for a call or return.
		 * \return The number of cycles needed to handle a DISP miss and load the given function as additional penalty to the standard call/ret handling. This value is used as memory penalty on DISP miss.
		 */
		uint32_t getDispMissAndLoadPenalty(uint32_t function_size_in_mem, activation_type_t act);
		/*!
		 * \brief Calculates the penalty that is introduced by the hit handling of the DISP controller.
		 * Notice: The hit handling is overlapping with the call/ret handling by the pipeline, thus it is only taken into account if the hit handling takes longer than the call/ret handling. Then the difference of both is the penalty, that is introduced by the DISP. 
		 * \param call True, then the penalty will be calculated for a call, else it will be calculated for a return.
		 * \return The number of cycles needed to handle a DISP hits additional penalty to the standard call/ret handling. This value is used as memory penalty on DISP hit.
		 */
		uint32_t getDispHitPenalty(bool call);
		/*!
		 * \brief The cost to load one block into the scratchpad. In cycles.
		 */
		uint32_t block_load_cost;

		/*!
		 * \brief Maps the function context to all MSG nodes.
		 * Needed to obtain the reactivated function on return.
		 */
		NodeContextMap ctxMap;
		/*!
		 * \brief Holds the function call graph, but is just needed to obtain the function sizes.
		 */
		FunctionCallGraphObject *functions;
		/*!
		 * \brief Pointer to object hat maintains the abstract disp states depending on it's replacement policy.
		 */
		BFDISPStateMaintainer *disp_state_maintainer;

		// TODO document graph properties
		property_map<ControlFlowGraph, nodetype_t>::type nodeTypeNProp;
		property_map<ControlFlowGraph, startaddr_t>::type startAddrNProp;
		property_map<ControlFlowGraph, startaddrstring_t>::type startAddrStringNProp;
		property_map<ControlFlowGraph, bbsize_t>::type bbSizeNProp;

		// TODO document graph properties
		property_map<MemoryStateGraph, mem_state_valid_t>::type memStateValNProp;
		property_map<MemoryStateGraph, cfg_vertex_t>::type mappedCFGVertexNProp;
		property_map<MemoryStateGraph, msg_edgetype_t>::type msgEdgeTypeEProp;
		property_map<MemoryStateGraph, dynamic_mem_penalty_t>::type dynamicMemPenaltyEProp;

		/*!
		 * \brief Pointer to the global architecture configuration object.
		 */
		ArchConfig *arch_cfg;
		/*!
		 * \brief Pointer to the LOGCXX logger object.
		 */
		static LoggerPtr logger;
};

#endif
