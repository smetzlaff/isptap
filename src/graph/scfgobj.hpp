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
#ifndef _SUPERCONTROLFLOWGRAPH_OBJECT_HPP_
#define _SUPERCONTROLFLOWGRAPH_OBJECT_HPP_

#include "global.h"
#include "graph_structure.h"
#include "fcgobj.hpp"
#include "cfgobj.hpp"

#include <map>

typedef map<uint32_t, CFGVertex> AddrVertexMap;

typedef pair<CFGVertex, CFGVertex> EntryExitNodePair;

class SuperControlFlowGraphObject {
	public:
		SuperControlFlowGraphObject();
		virtual ~SuperControlFlowGraphObject();

		void createSuperGraph(FunctionCallGraphObject *fcgo);
		bool isCreated(void);
		ControlFlowGraph getSCFG(void);
		CFGVertex getSCFGEntry(void);
		CFGVertex getSCFGExit(void);

	private:

		void addCfg(uint32_t func_addr, ControlFlowGraph cfg);
		void insertCallSite(addr_label_t func_addr, ControlFlowGraph cfg);
		EntryExitNodePair joinCfg(ControlFlowGraph cfg);
		void splitCallSite(CFGVertex split);
		CFGVertex setSource(node_type_t type, uint32_t start_addr, string start_addr_s, uint32_t end_addr, string code, uint32_t size, uint32_t instr_count, uint32_t call_addr);
		CFGVertex getTarget(uint32_t target_addr);

		AddrVertexMap call_entry_map;
		AddrVertexMap call_exit_map;
		AddrVertexMap avMap;

		ControlFlowGraph scfg;
		CFGVertex entry, exit;

		bool finished;

		property_map<ControlFlowGraph, nodetype_t>::type nodeTypeNProp;
		property_map<ControlFlowGraph, startaddr_t>::type startAddrNProp;
		property_map<ControlFlowGraph, startaddrstring_t>::type startAddrStringNProp;
		property_map<ControlFlowGraph, endaddr_t>::type endAddrNProp;
		property_map<ControlFlowGraph, calllabel_t>::type callLabelNProp;
		property_map<ControlFlowGraph, bbcode_t>::type bbCodeNProp;
		property_map<ControlFlowGraph, bbsize_t>::type bbSizeNProp;
		property_map<ControlFlowGraph, bbinstrs_t>::type bbInstrsNProp;

		property_map<ControlFlowGraph, edgetype_t>::type edgeTypeEProp;
		property_map<ControlFlowGraph, cost_t>::type costEProp;
		property_map<ControlFlowGraph, cost_onchip_t>::type costOnChipEProp;
		property_map<ControlFlowGraph, cost_offchip_t>::type costOffChipEProp;
		property_map<ControlFlowGraph, mem_penalty_t>::type memPenaltyEProp;
		property_map<ControlFlowGraph, capacityl_t>::type capacitylEProp;
		property_map<ControlFlowGraph, capacityh_t>::type capacityhEProp;
		property_map<ControlFlowGraph, circ_t>::type circEProp;
		property_map<ControlFlowGraph, activation_t>::type actEProp;
		property_map<ControlFlowGraph, edgename_t>::type edgeNameEProp;

		vector<addr_label_t> inserted_functions;
		vector<addr_label_vec_t> call_sites;

		static LoggerPtr logger;
};

#endif
