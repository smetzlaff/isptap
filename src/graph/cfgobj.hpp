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
#ifndef _CONTROL_FLOW_GRAPH_OBJECT_HPP_
#define _CONTROL_FLOW_GRAPH_OBJECT_HPP_


#include "cfg_if.hpp"

typedef map<uint32_t, uint32_t> CallCountMap;

class ControlFlowGraphObject : public  ControlFlowGraphObject_IF {
	public:
		ControlFlowGraphObject();
		virtual ~ControlFlowGraphObject();

//		ControlFlowGraph getCFG(void);
//		CFGVertex getCFGEntry(void);
//		CFGVertex getCFGExit(void);

		void addBBNode(uint32_t start_addr, uint32_t end_addr, uint32_t next_addr, string code, uint32_t size, uint32_t instr_count);
		void addBBNode(uint32_t start_addr, uint32_t end_addr, vector<uint32_t> next_addrs, string code, uint32_t size, uint32_t instr_count);
		void addBBNode(uint32_t start_addr, uint32_t end_addr, uint32_t next_addr, uint32_t cond_jump_addr, string code, uint32_t size, uint32_t instr_count);
		void addBBNode(uint32_t start_addr, uint32_t end_addr, uint32_t next_addr, vector<uint32_t> cond_jump_addrs, string code, uint32_t size, uint32_t instr_count);
		void addBBNode(uint32_t start_addr, uint32_t end_addr, string code, uint32_t size, uint32_t instr_count);
		void addBBCallNode(uint32_t start_addr, uint32_t end_addr, uint32_t next_addr, string code, uint32_t size, uint32_t instr_count, addr_label_t call_target);
		void addBBCallNode(uint32_t start_addr, uint32_t end_addr, uint32_t next_addr, string code, uint32_t size, uint32_t instr_count, vector<addr_label_t> call_targets);

		vector<addr_label_t> getCallTargets(void);
		vector<addr_label_vec_t> getCallSites(void);
		uint32_t getCallSiteCount(uint32_t funct_address);
		uint32_t getCodeSize(void);

		bool isFinished(void);

//		void setRatioFileReaders(RatioFileReader *rfr_onchip, RatioFileReader *rfr_offchip);

	private:
		CFGVertex setSourceBB(uint32_t start_addr, uint32_t end_addr, string code, uint32_t size, uint32_t instr_count);
		CFGVertex getTargetBB(uint32_t target_addr);
//		uint32_t getBBSizeFromAddr(uint32_t address);
//		uint32_t getInstrCountFromAddr(uint32_t address);

		void eraseBBFromIncompleteList(uint32_t addr);
		void pushTargetBBToIncompleteList(uint32_t addr);

		void addCallTarget(addr_label_t target);

//		void addBBCostToCfg(void);

//		ControlFlowGraph cfg;
//		CFGVertex entry, exit;
//		AddrVertexMap avMap;
		CallCountMap ccMap;

		uint32_t code_size;

		/*!
		 * \brief Determines if the entry node is connected or not.
		 * The first BB that is added to the CFG without an entry edge is assumed to be the
		 * CFG entry node. For this node the connection to the ENTRY node is established.
		 * All other (later added) nodes without an entry edge are not connected to the ENTRY node,
		 * because eacht CFG can have only one entry node. These BBs may be dead code or connected
		 * later on (e.g. by a back jump). 
		 * Because the parser is proceding the code in forward direction, the first BB without entry edges is the one
		 * with the lowest address, and thus (possibly) the correct function/cfg entry.
		 */
		bool entry_connected;

		/*!
		 * \brief The type of the used memory.
		 * Needed to set the memory penalty (memPenaltyEProp) for static memories. For dynamic memories this is done by the data fow analysis after the cfg creation.
		 */
//		mem_type_t memory_type;

//		property_map<ControlFlowGraph, nodetype_t>::type nodeTypeNProp;
//		property_map<ControlFlowGraph, startaddr_t>::type startAddrNProp;
//		property_map<ControlFlowGraph, startaddrstring_t>::type startAddrStringNProp;
//		property_map<ControlFlowGraph, endaddr_t>::type endAddrNProp;
//		property_map<ControlFlowGraph, calllabel_t>::type callLabelNProp;
//		property_map<ControlFlowGraph, bbcode_t>::type bbCodeNProp;
//		property_map<ControlFlowGraph, bbsize_t>::type bbSizeNProp;
//		property_map<ControlFlowGraph, bbinstrs_t>::type bbInstrsNProp;
//
//		property_map<ControlFlowGraph, edgetype_t>::type edgeTypeEProp;
//		property_map<ControlFlowGraph, cost_t>::type costEProp;
//		property_map<ControlFlowGraph, cost_onchip_t>::type costOnChipEProp;
//		property_map<ControlFlowGraph, cost_offchip_t>::type costOffChipEProp;
//		property_map<ControlFlowGraph, mem_penalty_t>::type memPenaltyEProp;
//		property_map<ControlFlowGraph, capacityl_t>::type capacitylEProp;
//		property_map<ControlFlowGraph, capacityh_t>::type capacityhEProp;
//		property_map<ControlFlowGraph, circ_t>::type circEProp;
//		property_map<ControlFlowGraph, activation_t>::type actEProp;
//		property_map<ControlFlowGraph, edgename_t>::type edgeNameEProp;

		vector<uint32_t> incomplete_nodes;
		vector<addr_label_vec_t> call_sites;
		vector<addr_label_t> call_targets;

//		RatioFileReader *bb_cost_container_onchip, *bb_cost_container_offchip;

//		Configuration *conf;

//		CarCoreTiming *carcore_timing_onchip, *carcore_timing_offchip;

		static LoggerPtr logger;
};

#endif
