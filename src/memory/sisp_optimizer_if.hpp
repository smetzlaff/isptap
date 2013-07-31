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
#ifndef _BBSISP_OPTIMIZER_IF_HPP_
#define _BBSISP_OPTIMIZER_IF_HPP_


#include "global.h"
#include "constants.h"
#include "graph_structure.h"
#include "lpsolver.hpp"
#include "configuration.hpp"
#include "cfgloophelper.hpp"
#include "arch_cfg.hpp"
#include "isa.hpp"
#include "dumpline_parser.hpp"

#include <boost/regex.hpp>

struct sisp_result_t {
	uint32_t used_size;
	uint32_t estimated_used_size;
	vector <uint32_t> assigned_bbs;
	lp_solution_t solution_type;
	uint64_t estimated_timing;
	uint32_t estimated_jump_penalty;
};

/**
 * \brief Optimization for a static instruction scratchpad, with basic blocks as granularity.
 * Note: the cost for jumping into the scratchpad and jumping back is not considered yet.
 * The optimization depends on the used metric (CONF_USE_RATIO_FILES, CONF_USE_DYNAMIC_INSTRUCTION_COUNT_AS_METRIC, CONF_USE_WCET_PATH_LENGTH_AS_METRIC).
 */
class SISPOptimizer_IF : protected CFGLoopHelper {
	public:
		/**
		 * \brief Constructor.
		 */
		SISPOptimizer_IF(ControlFlowGraph cfgraph, CFGVertex entry, CFGVertex exit);
		/**
		 * \brief Default destructor.
		 */
		virtual ~SISPOptimizer_IF();
		/**
		 * \brief Sets the size of the static basic block based scratchpad in bytes.
		 * \param size Size of the scratchpad in bytes to set.
		 */
		void setSize(uint32_t size);
		/**
		 * \brief Determines the block assignment. The implementation of the function depends on the type of optimisation.
		 */
		virtual void calculateBlockAssignment(void) = 0;
		/** 
		 * \brief Returns all assigned basic blocks for the given scratchpad size and the optimization metric.
		 * Needed for the ILPGenerator to recalculate the WCET if the selected functions are assigned to the static scratchpad.
		 * \returns Vector of addresses of all basic blocks assigned to the static scratchpad.
		 */
		vector <uint32_t> getBlockAssignment(void);
		/**
		 * \brief Returns the part of the scratchpad that is used in bytes.
		 * \returns Size of the used part of the scratchpad in bytes.
		 */
		uint32_t getUsedSispSize(void);
		/**
		 * \brief Returns the used scratchpad size, that was determined by solving the ILP.
		 * \returns The used scratchpad size, that was determines by solvin the ILP.
		 */
		uint32_t getEstimatedUsedSize(void);

		/**
		 * \brief Returns the type of the solution found by lp_solve.
		 * It is used to ease the handling of the different cases that may occur: e.g. unbound values, or subobtimal solutions (caused by stopping the lp_solver if the solving time is exceeded).
		 * \returns The type of the solution found by lp_solve. If the lp_solver was not activated, SolutionNotCalculated is returned.
		 */
		lp_solution_t getSolutionType(void);

		virtual sisp_result_t getResults(void);

	protected:

		vector<lp_result_set> writeAndSolveILPFile(vector<string> ilp_formulation)/* __attribute__((deprecated))*/;
		vector<lp_result_set> writeAndSolveILPFile(string ilp_formulation);

		/**
		 * \brief Returns the first basic block of a function determined by a call/entry point.
		 * \param v Either a CallPoint or a functions Entry node.
		 * \returns The first basic block of the function.
		 */
		CFGVertex getFunctStartBBlock(CFGVertex v);
		/**
		 * \brief Returns the last basic block of a function determined by a return/exit point.
		 * \param v Either a ReturnPoint or a functions Exit node.
		 * \returns The last basic block of the function.
		 */
		CFGVertex getFunctEndBBlock(CFGVertex v);
		/**
		 * \brief Returns the basic block that calls a function determined by a call point.
		 * \param v A CallPoint of the function.
		 * \returns The basic block that calls the function.
		 */
		CFGVertex getFunctCallingBBlock(CFGVertex v);
		/**
		 * \brief Returns the basic block to which it is returned after function execution determined by a return point.
		 * \param v A ReturnPoint of the function.
		 * \returns The basic block to which it is returned after the function.
		 */
		CFGVertex getFunctReturnToBB(CFGVertex v);

		/**
		 * \brief Sets the assignment found by the LpSolver.
		 * \param lpresult Vector with the basic block variable and its assignment value (1 for in scratchpad, else 0)
		 */
		virtual void setAssignment(vector<lp_result_set> lpresult);

		virtual void clear(void)=0;


		uint32_t getUsedSizeIncludingSizePenalties(void);
		uint32_t getSizePenaltyForAssignedBasicBlock(CFGVertex v);
		bool isAssigned(uint32_t addr);
		/**
		 * \brief Obtains the type of the displacement of a jump or call instruction from a given basic block.
		 * \param node A basic block for which the displacement type of a containing jump/call instruction should be determined.
		 * \returns The displacement type of the given node. If the last instruciton of the basic block is no jump/call "NoDisplacement" is returned.
		 */
		displacement_type_t getDisplacementTypeOfBB(CFGVertex node);
		/**
		 * \brief Determins the connection type and the displacement type of a basic block connection given by an edge.
		 * \param e The edge that models the connection of a basic block with a successing node.
		 * \returns The pair of the connection type modelled by the edge e and the type of the displacement of a jump/call instruction in the source node of e. If the source of e does not contain a jump the displacement type is set to "NoDisplacement".
		 */
		pair<connection_type_t, displacement_type_t> getConnectionAndDisplacementType(CFGEdge e);
		/**
		 * \brief Sets variables that is calculated during solving the BB assingment ILP
		 * \param lp_result The variable vector from the lp_solver, that contains global variables e.g. "wentry", "sp" and "jp"
		 */
		virtual void setVariables(vector<lp_result_set> lp_result);

		/**
		 * \brief Size of the static instruction scratchpad in bytes.
		 */
		uint32_t sisp_size;
		/**
		 * \brief Size of all assigned basic blocks in the scratchpad in bytes. 
		 */
		uint32_t used_sisp_size;
		/**
		 * \brief Vector of all assigned basic blocks.
		 */
		vector<uint32_t> assigned_bbaddrs;

		/**
		 * \brief The type of the found solution.
		 * Either  OptimalSolution, SuboptimalSolution, ErrorWhileSolving or SolutionNotCalculated
		 */
		lp_solution_t solution_type;
		/**
		 * \brief The used scratchpad size as deterimined by solving the BB assignment ILP.
		 */
		uint32_t ilp_solution_used_size;

		/**
		 * \brief Property map for the basic block's current cost in the cfg.
		 * Edge property.
		 */
		property_map<ControlFlowGraph, cost_t>::type costEProp;
		/**
		 * \brief Property map for the basic block's on chip cost in the cfg.
		 * Edge property.
		 */
		property_map<ControlFlowGraph, cost_onchip_t>::type costOnChipEProp;
		/**
		 * \brief Property map for the basic block's off chip cost in the cfg.
		 * Edge property.
		 */
		property_map<ControlFlowGraph, cost_offchip_t>::type costOffChipEProp;
		/**
		 * \brief Property map for the basic block's memory penalty  in the cfg
		 * memory penalty: additional cycles needed if the block is not executed from onchip mem, if the IS_STATIC_MEM() this is the difference from costOnChipEProp to costOffChipEProp
		 * Edge property.
		 */
		property_map<ControlFlowGraph, mem_penalty_t>::type memPenaltyEProp;
		/**
		 * \brief Property map for the basic block's activation count in the cfg.
		 * Edge property.
		 */
		property_map<ControlFlowGraph, activation_t>::type actEProp;
		/**
		 * \brief Property map for the basic block's node type in the cfg.
		 * Node property.
		 */
		property_map<ControlFlowGraph, nodetype_t>::type nodeTypeNProp;
		/**
		 * \brief Property map for the basic block's start address in the cfg.
		 * Node property.
		 */
		property_map<ControlFlowGraph, startaddr_t>::type startAddrNProp;
		/**
		 * \brief Property map for the basic block's start address string as string (also used as name for special nodes) in the cfg.
		 * Node property.
		 */
		property_map<ControlFlowGraph, startaddrstring_t>::type startAddrStringNProp;
		/**
		 * \brief Property map for the basic block's size in the cfg.
		 * Node property.
		 */
		property_map<ControlFlowGraph, bbsize_t>::type bbSizeNProp;
		/**
		 * \brief Graph property that contains the code of the basic blocks.
		 */
		property_map<ControlFlowGraph, bbcode_t>::type bbCodeNProp;
		/**
		 * \brief Pointer to object for parsing the lines of the dump files.
		 */
		DumpLineParser *dlp;
		/**
		 * \brief Pointer to object for decoding and encoding instructions for the tricore ISA.
		 */
		ISA *isa;
		/**
		 * \brief Pointer to the global architectural configuration object.
		 */
		ArchConfig *arch_cfg;

		/**
		 * \brief Pointer to the global Configuration object.
		 */
		Configuration *conf;
		/**
		 * \brief Pointer to the LOGCXX logger object.
		 */
		static LoggerPtr logger;
};

#endif
