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
#ifndef _CONTROL_FLOW_GRAPH_IF_HPP_
#define _CONTROL_FLOW_GRAPH_IF_HPP_

#include "global.h"
#include "constants.h"
#include "graph_structure.h"
#include "configuration.hpp"
#include "ratiofile_reader.hpp"
#include "core_timing.hpp"

#include <map>
#include <iostream>
#include <stdio.h>
#include <boost/regex.hpp>

typedef map<uint32_t, CFGVertex> AddrVertexMap;


class ControlFlowGraphObject_IF {
	public:
		ControlFlowGraph getCFG(void);
		CFGVertex getCFGEntry(void);
		CFGVertex getCFGExit(void);

		void setRatioFileReaders(RatioFileReader *rfr_onchip, RatioFileReader *rfr_offchip);
	protected:
		ControlFlowGraphObject_IF();
		virtual ~ControlFlowGraphObject_IF();

		uint32_t getBBSizeFromAddr(uint32_t address);
		uint32_t getInstrCountFromAddr(uint32_t address);

		void addBBCostToCfg(void);

		void calculateCostForBB(CFGVertex v);


		ControlFlowGraph cfg;
		CFGVertex entry, exit;
		AddrVertexMap avMap;


		RatioFileReader *bb_cost_container_onchip, *bb_cost_container_offchip;
		Configuration *conf;
		CoreTiming *core_timing_onchip, *core_timing_offchip;

		/*!
		 * \brief The type of the used memory.
		 * Needed to set the memory penalty (memPenaltyEProp) for static memories. For dynamic memories this is done by the data fow analysis after the cfg creation.
		 */
		mem_type_t memory_type;

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

	private:
		static LoggerPtr logger;

};


#endif
