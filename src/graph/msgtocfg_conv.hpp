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
#ifndef _MSGTOCFG_CONV_HPP_
#define _MSGTOCFG_CONV_HPP_

#include "global.h"
#include "graph_structure.h"
#include "fcgobj.hpp"
#include "cfgobj.hpp"
#include "vivug_creator.hpp"

#include <map>

/**
 * \brief Map to combine node in memory state graph and control flow graph.
 */
typedef map<MSGVertex, CFGVertex> MsgCfgMap;

/**
 * \brief Converter class to transfer a memory state graph structure into an control flow graph notation using the properties of the reference control flow graph, except the flow information and the memory penalty that are taken from the memory state graph.
 * Using e.g. basic block addresses, code and flow informations. 
 * This class transforms the memory state graph into a control flow graph using the node and edge properties of the reference control flow graph like start address and cost and also adding the flow information (loop bound) and (cache) memory penalties of the memory state graph to the created graph.
 */
class MsgToCfgConverter{
	public:
		/**
		 * \brief Constructor providing the reference control flow graph and the memory state graph, that will be used for graph transformation.
		 * The created graph has the structure of the memory state graph, but the edge and node properties of the control flow graph, except the flow and memory penalty properties.
		 * \param cfg_msg Pair of input memory state graph and it's reference control flow graph.
		 * \param entry Pair of entry nodes in memory state graph and it's reference control flow graph.
		 * \param exit Pair of exit nodes in memory state graph and it's reference control flow graph.
		 */
		MsgToCfgConverter(CFGMSGPair cfg_msg, CFGMSGVPair entry, CFGMSGVPair exit);
		/**
		 * \brief Default destructor.
		 */
		virtual ~MsgToCfgConverter();
		/**
		 * \brief Creates an control flow graph with the graph structure of the used memory state graph but with the notation of its reference control flow graph.
		 * The properties of both graphs are combined from properties of both graphs: addresses and execution cost are taken from the reference control flow graph and the memory penalties of the memory state graph are used, since a data flow analysis calculated the penalties based on this graph. The flow informations, defined by the loop bounds in the flow facts, of the memory state graph are used, beacause the vivu graph creation (which creates the memory state graph) takes the changes of the flow into account when unrolling the first iteration of the loops
		 * \returns Transformed control flow graph.
		 */
		ControlFlowGraph getCfg(void);

		/*!
		 * \brief Returns the entry node of the created cfg.
		 * \returns The entry node of the created cfg.
		 */
		CFGVertex getEntry();
		/*!
		 * \brief Returns the exit node of the created cfg.
		 * \returns The exit node of the created cfg.
		 */
		CFGVertex getExit();

	private:
		/**
		 * \brief Provides cfg vertex for the new graph. 
		 * If the vertex was created before, it will be returned, otherwise a vertex node is created.
		 * \param orig_node Vertex for that the corresponding node in the transformed control flow graph shall be found or created. As reference vertex a pair of memory state graph
		 * and reference control flow graph is necessary.
		 * \returns The desired vertex (created or found).
		 */
		CFGVertex getNode(CFGMSGVPair orig_node);
		/**
		 * \brief Connects two vertices of the new transformed cfg. Also the edge properties of the created edge are set as in the reference cfg.
		 * \param src Source vertex of the new transformed cfg.
		 * \param orig_src Source vertex in representation of memory state graph vertex and it's reference cfg vertex (to find edge of the original cfg to copy its properties).
		 * \param orig_tgt Target vertex in representation of memory state graph vertex and it's reference cfg vertex (to find/create target vertex in new cfg the the getNode() method is used).
		 */
		void connectNodes(CFGVertex src, CFGMSGVPair orig_src, CFGMSGVPair orig_tgt);

		/**
		 * \brief Input memory state graph and it's reference control flow graph that will be transformed.
		 */
		CFGMSGPair cfgmsg;
		/**
		 * \brief Control flow graph to be created (combining structure of input memory state graph and properties of input control flow graph).
		 */
		ControlFlowGraph cfg;
		/**
		 * \brief Map that maps every vertex of the memory state graph to the newly created control flow graph.
		 * This map is used to check if a vertex in the new cfg is already created or not.
		 * This map can be used because there is a bijective mapping between the vertices of the memory state graph and the vertices of the new transformed control flow graph.#
		 */
		MsgCfgMap msg_cfg_map;

		// TODO documentation needed
		CFGMSGVPair entry_o;
		CFGMSGVPair exit_o;
		CFGVertex entry_n;
		CFGVertex exit_n;


		// TODO document graph properties
		property_map<ControlFlowGraph, nodetype_t>::type nodeTypeNProp_o;
		property_map<ControlFlowGraph, startaddr_t>::type startAddrNProp_o;
		property_map<ControlFlowGraph, startaddrstring_t>::type startAddrStringNProp_o;
		property_map<ControlFlowGraph, endaddr_t>::type endAddrNProp_o;
		property_map<ControlFlowGraph, calllabel_t>::type callLabelNProp_o;
		property_map<ControlFlowGraph, bbcode_t>::type bbCodeNProp_o;
		property_map<ControlFlowGraph, bbsize_t>::type bbSizeNProp_o;
		property_map<ControlFlowGraph, bbinstrs_t>::type bbInstrsNProp_o;

		property_map<ControlFlowGraph, edgetype_t>::type edgeTypeEProp_o;
		property_map<ControlFlowGraph, cost_t>::type costEProp_o;
		property_map<ControlFlowGraph, cost_onchip_t>::type costOnChipEProp_o;
		property_map<ControlFlowGraph, cost_offchip_t>::type costOffChipEProp_o;
		property_map<ControlFlowGraph, mem_penalty_t>::type memPenaltyEProp_o;
		property_map<ControlFlowGraph, capacityl_t>::type capacitylEProp_o;
		property_map<ControlFlowGraph, capacityh_t>::type capacityhEProp_o;
		property_map<ControlFlowGraph, circ_t>::type circEProp_o;
		property_map<ControlFlowGraph, static_flow_t>::type sflowEProp_o;
		property_map<ControlFlowGraph, edgename_t>::type edgeNameEProp_o;

		property_map<MemoryStateGraph, mem_state_t>::type memStateNProp_o;
		property_map<MemoryStateGraph, msg_context_id_t>::type contextIDNProp;
		property_map<MemoryStateGraph, cfg_vertex_t>::type mappedCFGVertexNProp_o;
		property_map<MemoryStateGraph, msg_edgetype_t>::type msgEdgeTypeEProp;
		property_map<MemoryStateGraph, dynamic_mem_penalty_t>::type dynamicMemPenaltyEProp;
		property_map<MemoryStateGraph, msg_circ_t>::type msgCircEprop;

		property_map<MemoryStateGraph, mem_cache_hits_t>::type cacheHitsNProp_o;
		property_map<MemoryStateGraph, mem_cache_misses_t>::type cacheMissesNProp_o;
		property_map<MemoryStateGraph, mem_cache_ncs_t>::type cacheNCsNProp_o;
		property_map<ControlFlowGraph, cache_hits_t>::type cacheHitsNProp;
		property_map<ControlFlowGraph, cache_misses_t>::type cacheMissesNProp;
		property_map<ControlFlowGraph, cache_ncs_t>::type cacheNCsNProp;

		property_map<ControlFlowGraph, nodetype_t>::type nodeTypeNProp_n;
		property_map<ControlFlowGraph, startaddr_t>::type startAddrNProp_n;
		property_map<ControlFlowGraph, startaddrstring_t>::type startAddrStringNProp_n;
		property_map<ControlFlowGraph, endaddr_t>::type endAddrNProp_n;
		property_map<ControlFlowGraph, context_id_t>::type contextIDNProp_n;
		property_map<ControlFlowGraph, calllabel_t>::type callLabelNProp_n;
		property_map<ControlFlowGraph, bbcode_t>::type bbCodeNProp_n;
		property_map<ControlFlowGraph, bbsize_t>::type bbSizeNProp_n;
		property_map<ControlFlowGraph, bbinstrs_t>::type bbInstrsNProp_n;

		// TODO document graph properties
		property_map<ControlFlowGraph, edgetype_t>::type edgeTypeEProp_n;
		property_map<ControlFlowGraph, cost_t>::type costEProp_n;
		property_map<ControlFlowGraph, cost_onchip_t>::type costOnChipEProp_n;
		property_map<ControlFlowGraph, cost_offchip_t>::type costOffChipEProp_n;
		property_map<ControlFlowGraph, mem_penalty_t>::type memPenaltyEProp_n;
		property_map<ControlFlowGraph, capacityl_t>::type capacitylEProp_n;
		property_map<ControlFlowGraph, capacityh_t>::type capacityhEProp_n;
		property_map<ControlFlowGraph, circ_t>::type circEProp_n;
		property_map<ControlFlowGraph, static_flow_t>::type sflowEProp_n;
		property_map<ControlFlowGraph, edgename_t>::type edgeNameEProp_n;


		/**
		 * \brief Pointer to the LOGCXX logger object.
		 */
		static LoggerPtr logger;
};


#endif
