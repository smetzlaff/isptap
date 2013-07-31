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
#ifndef _BBSISP_OPTIMIZER_JP_WCP_HPP_
#define _BBSISP_OPTIMIZER_JP_WCP_HPP_

#include "bbsisp_optimizer_wcp.hpp"


typedef map<uint32_t, uint32_t> FunctionMap;

class BBSISPOptimizerJPWCP : public BBSISPOptimizerWCP {
	public:
		BBSISPOptimizerJPWCP(ControlFlowGraph cfgraph, CFGVertex entry, CFGVertex exit);
		virtual ~BBSISPOptimizerJPWCP();
		/**
		 * \brief Determines the block assignment. The implementation of the function depends on the type of optimisation.
		 */
		virtual void calculateBlockAssignment(void);
		/**
		 * \brief Returns the jump penalty that was determined during solving the BBSISP assignment ILP.
		 * \returns The jump penalty that was determined during solving the BBSISP assignmend ILP.
		 */
		uint32_t getEstimatedJumpPenalty(void);

		virtual sisp_result_t getResults(void);
	protected:
		virtual void clear(void);

		virtual string getEdgeCostConstraint(CFGEdge e);
		virtual string getEdgeCostConstraint(CFGEdge e, uint32_t multiplicator);
	private:
		/**
		 * \brief Generates the size constraints for the basic blocks.
		 * It contains the sizes of the basic blocks and the scratchpad size.
		 * Thus the ILP is able to not exceed the scratchpad on assigning basic blocks to it.
		 * Also the size penalties that caused by additional connecting jumps are modelled here.
		 */
		virtual void generateBlockSizeConstraint(void);
		/**
		 * \brief Generates the cost constraints for all basic blocks.
		 * Contains the cost for the basic block for both cases: it is in the scrachpad or not.
		 * Also the jump cost penalties caused by additional connecting jumps are modelled here.
		 */
		virtual void generateBlockCostsConstraints(void);
		/**
		 * \brief Constructs the jump penalty term for the two given basic blocks.
		 * This method is used if both basic blocks are not connected directly, e.g. if a function call or return is in between.
		 * \param src The source node of the connection of the two basic blocks.
		 * \param tgt The target node of the connection of the two basic blocks.
		 * \param node_type The node_type defines the jump scenario, i.e. how the two nodes are connected. E.g. the value "CallPoint" determines that the src basic block calls a function and the tgt basic block is the function's entry block.
		 * \returns The jump penalty term to be embedded in ilp formulation for the src node. The string is empty if there is no jump penalty for the two blocks.
		 */
		string getJumpPenaltyTermForBB(CFGVertex src, CFGVertex tgt, node_type_t node_type);
		/**
		 * \brief Constructs the jump penalty term for a given basic block and an connecting out edge.
		 * This method is used if two basic blocks are directly connected.
		 * \param src The source node of the connection.
		 * \param e The out edge of src that determines the connection to the sucessor basic block.
		 * \returns The jump penalty term to be embedded in ilp formulation for the src node. The string is empty if there is no jump penalty for the two blocks.
		 */
		string getJumpPenaltyTermForBB(CFGVertex src, CFGEdge e);
		string getJumpPenaltyTermForBB(CFGVertex src, CFGEdge e, uint32_t multiplicator);
		/**
		 * \brief Generates an XOR constraint for the assignment variables of the given two blocks.
		 * Models assignment_src XOR assignment_tgt in an ILP formulation.
		 * \param src Basic block that is connected to a successor block tgt.
		 * \param tgt Basic block that is connected to a predeccessor block src.
		 * \returns An ILP formulation of the assignment of either src or tgt to the BBSISP.
		 */
		string getXorConstraint(CFGVertex src, CFGVertex tgt);
		/**
		 * \brief Generates an ANDN constraint for the assignment variables of the given two blocks.
		 * Models assignment_src AND !(assignment_tgt) in an ILP formulation.
		 * \param src Basic block that is connected to a successor block tgt.
		 * \param tgt Basic block that is connected to a predeccessor block src.
		 * \returns An ILP formulation of the assignment of src and not tgt to the BBSISP.
		 */
		string getAndNConstraint(CFGVertex src, CFGVertex tgt);
		/**
		 * \brief Builds the size constraint of a given basic block.
		 * The constraint contains the size of the basic block in memory and all penalties that may arise by any preccessing block that is not assigned to the BBSISP (penalties due to longer or additional jumps). These penalties are subtracted if the according basic blocks are assigned to the BBSISP.
		 * \param v The basic block of which the size including all possible penalties is to be obtained.
		 * \returns An ILP constraint containing the block size of the given basic block, with all possible size penalties. If a basic block cannot suffer any penalties, the basic block size is assigned to the constraint only.
		 */
		string getBBSizeWithPenaltyConstraint(CFGVertex v);

		/**
		 * \brief Small inline function that pushes a string into the binary_constraints vector it the string is not empty.
		 * \param bin_constraint String representing an XOR or an ANDN constraint to be pushed into the binary_constraints vector, but only if it is not empty.
		 */
		void bin_constraint_push_back(string bin_constraint);
		/**
		 * \brief Gets the jump penalty of a call point to a function entry.
		 * \param call_point A CallPoint which is to be connected to the function_entry_node.
		 * \param function_entry_node A Entry node of a function that is connected to the call_point.
		 * \returns A constraints that models the jump penalty if either the calling basic block or the function entry basic block is in the scratchpad.
		 * This functions allows using the generateOptimalILPFormulationForSequentialCode() without changes.
		 */
		virtual string getPenaltyForFunctionEntering(CFGVertex call_point, CFGVertex function_entry_node);
		/**
		 * \brief Gets the jump penalty of a function exit to a return point.
		 * \param return_point A ReturnPoint which is connected to the function_exit_node.
		 * \param function_exit_node A Exit node of a function that is connected to the return_point.
		 * \returns A constraint that models the jump penalty if either the returning basic block of the function or the block to which it is returned is in the scatchpad.
		 * This functions allows using the generateOptimalILPFormulationForSequentialCode() without changes.
		 */
		virtual string getPenaltyForFunctionExit(CFGVertex return_point, CFGVertex function_exit_node);
		/**
		 * \brief Sets variables that is calculated during solving the BB assingment ILP
		 * \param lp_result The variable vector from the lp_solver, that contains global variables e.g. "wentry", "sp" and "jp"
		 */
		virtual void setVariables(vector<lp_result_set> lp_result);
		/**
		 * \brief Vector containing all xor and andn constraints of the ILP
		 */
		vector<string> binary_constraints;
		/**
		 * \brief Vector containing the constraints that form the sum of all jump penalties in the ILP
		 * The jump_penalties vector is only needed to determine the overall jump penalty for debug reasons.
		 */
		vector<string> jump_penalties;
		/**
		 * \brief Contains the calculated jump penalty obtained from the BBSISP assignment ILP.
		 */
		uint32_t ilp_solution_jump_penalty;

};

#endif
