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
#ifndef _CONTROL_FLOW_GRAPH_COST_CALCULATOR_HPP_
#define _CONTROL_FLOW_GRAPH_COST_CALCULATOR_HPP_

#include "cfg_if.hpp"
#include "dumpline_parser.hpp"
#include "isa.hpp"
#include "arch_cfg.hpp"


class ControlFlowGraphCostCalculator : public ControlFlowGraphObject_IF {
	public:
		ControlFlowGraphCostCalculator(ControlFlowGraph cfgraph,  CFGVertex cfg_entry, CFGVertex cfg_exit);
		virtual ~ControlFlowGraphCostCalculator();

		void considerMemoryAssignment(vector<uint32_t> &assigned_blocks, bool addJumpPenalty);
		void calculateCost(void);
	private:
		ControlFlowGraphCostCalculator();
		uint32_t getSizeOfAssignedBlocks(vector<uint32_t> &assigned_blocks);
		void setAssignedBlocks(vector<uint32_t> &assigned_blocks);
		void addConnectingJumpsForAssignedBlocks(vector<uint32_t> &assigned_blocks);
		inline bool isBlockAssigned(CFGVertex v, vector<uint32_t> &assigned_blocks);
		inline bool isInVertexList(CFGVertex v, vector<CFGVertex>& vl);
		void addConnectingJump(CFGVertex v);
		void alterConnectingJump(CFGVertex v, bool connective_jump_already_added);
		inline CFGVertex getFirstBBOfCalledFunction(CFGVertex call_point);
		void alterConnectingCall(CFGVertex v);

		/**
		 * \brief Pointer to object for parsing the lines of the dump files.
		 */
		DumpLineParser *dlp;
		/**
		 * \brief Pointer to object for decoding and encoding instructions for the used ISA.
		 */
		ISA *isa;
		/**
		 * \brief Pointer to the global architectural configuration object.
		 */
		ArchConfig *arch_cfg;

		static LoggerPtr logger;
};


#endif //_CONTROL_FLOW_GRAPH_COST_CALCULATOR_HPP_
