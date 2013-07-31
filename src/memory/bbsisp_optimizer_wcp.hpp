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
#ifndef _BBSISP_OPTIMIZER_WCP_HPP_
#define _BBSISP_OPTIMIZER_WCP_HPP_

#include "memory/sisp_optimizer_if.hpp"

typedef map<uint32_t, uint32_t> FunctionMap;

/**
 * Structure that holds every necessary information about a loop.
 */
struct loop_data_t
{
	/**
	 * The exitNode is the node that is the usual exit point (last node in the loop body) of a loop,
	 */
	CFGVertex exitNode;
	/**
	 * The startNode is the first node of the loop body, it is determined by the loops back edge.
	 * Usually the startNode and the entryNode are the same, but for some tail decision loops like in adpcm:my_sin() a loop body is entered at some node within the loop body (for the first iteration of the loop) - the entryNode. This node is needed to determine while ILP formulation if a loop body is entered or not. Then the second iteration is started - as ussual - with the startNode.
	 */
	CFGVertex startNode;
	/**
	 * The entryNode it the node at which the loop is entered (in the first loop iteration).
	 * Ususally this is the startNode.
	 */
	CFGVertex entryNode;
	/**
	 * The backEdge is the BackwardJump that causes the loop.
	 */
	CFGEdge backEdge;
};

/**
 * Containing the loop_data_t for all loop entryNodes.
 * It is used to identify loop entryNodes and find their loop data while processing the nodes in the cfg.
 */
typedef map<CFGVertex, loop_data_t> LoopDataMap;



class BBSISPOptimizerWCP : public SISPOptimizer_IF {
	public:
		BBSISPOptimizerWCP(ControlFlowGraph cfgraph, CFGVertex entry, CFGVertex exit);
		virtual ~BBSISPOptimizerWCP();
		virtual void calculateBlockAssignment(void);

		/**
		 * \brief Returns the WCET estimate that is calculated during finding the optimal BB assignment.
		 * \returns The WCET estimate that is calculated during finding the optimal BB assignment.
		 */
		uint32_t getEstimatedWCET(void);

		virtual sisp_result_t getResults(void);

	protected:
		virtual void clear(void);

		virtual string getEdgeCostConstraint(CFGEdge e);
		virtual string getEdgeCostConstraint(CFGEdge e, uint32_t multiplicator);
	protected:
		/**
		 * \brief Generates the flow ILP formulation.
		 * Calls therfore the generateOptimalILPFormulationForSequentialCode() method.
		 */
		void generateOptimalILPFormulation(void);
		/**
		 * \brief Generates the size constraints for the basic blocks.
		 * It contains the sizes of the basic blocks and the scratchpad size.
		 * Thus the ILP is able to not exceed the scratchpad on assigning basic blocks to it.
		 */
		virtual void generateBlockSizeConstraint(void);
		/**
		 * \brief Generates the cost constraints for all basic blocks.
		 * Contains the cost for the basic block for both cases: it is in the scrachpad or not.
		 */
		virtual void generateBlockCostsConstraints(void);
		/**
		 * \brief Writes the binary domains for the basic blocks.
		 * It is needed by the lp_solve to know which variables to assign.
		 */
		virtual void generateBinaryDomain(void);
		/**
		 * \brief Builds the WCP-sensitive ILP formulation for a sequential code part, i.e. a loop free control flow, like a loop body or a function.
		 * If the method finds a function or loop within the control flow it is recursively called.
		 * \param start The start node of the control flow to consider.
		 * \param end The end node of the control flow to consider.
		 * \param loop_no The running counter for distinguishing the costs and entry flow for loops (clX & wlX) and functions (cfX & wfXc<addr>).
		 * \param leavingEdges A reference to edges that leave the control flow, except any leaving edges from the end node. The parameter should be used to obtain extra exit edges when transformating a loop free control flow to the ILP form. This is currently not used/implemented.
		 * \returns The updated running counter for loops and functions. It has to be assigned to the callers running counter if recursion is used.
		 */
		uint32_t generateOptimalILPFormulationForSequentialCode(CFGVertex start, CFGVertex end, uint32_t loop_no, vector<CFGEdge>& leavingEdges);
		/**
		 * \brief Builds the structure to identify all loop entry nodes in the application.
		 * Creates the loopMap.
		 * Has to be called before processing the cfg and building the ILP by generateOptimalILPFormulationForSequentialCode().
		 */
		void registerAllLoopHeads(void);
		/**
		 * \brief Empty hook function to allow using the generateOptimalILPFormulationForSequentialCode() by BBSISPOptimizerJPWCP without changes.
		 */
		virtual string getPenaltyForFunctionEntering(CFGVertex call_point, CFGVertex function_entry_node);
		/**
		 * \brief Empty hook function to allow using the generateOptimalILPFormulationForSequentialCode() by BBSISPOptimizerJPWCP without changes.
		 */
		virtual string getPenaltyForFunctionExit(CFGVertex return_point, CFGVertex function_exit_node);
		/**
		 * \brief Sets the WCET estimate and the used BBSISP size that is calculated during solving the BB assingment ILP
		 * \param lp_result The variable vector from the lp_solver, that contains the WCET of the application in the variable "wentry", and "sp" for the BBSISP size
		 */
		virtual void setVariables(vector<lp_result_set> lp_result);
		/**
		 * \brief Containing the loop_data_t for all loop entryNodes.
		 * It is used to identify loop entryNodes and find their loop data while processing the nodes in the cfg.
		 * Is build by registerAllLoopHeads()
		 */
		LoopDataMap loopMap;
		vector<string> cfg_ilps;
		vector<string> block_cost_contraints;
		vector<string> block_size_constraints;
		vector<string> binary_domains;

		/**
		 * \brief The WCET estimate that is calculated in concert with the block assignment.
		 * Notice that this WCET estimate might overestimate the costs of the jump penalties.
		 */
		uint32_t ilp_solution_wcet_estimate;

		FunctionMap functionMap;
};

#endif
