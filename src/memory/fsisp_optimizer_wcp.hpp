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
#ifndef _FSISP_OPTIMIZER2_WCP_HPP_
#define _FSISP_OPTIMIZER2_WCP_HPP_


#include "bbsisp_optimizer_wcp.hpp"
#include "fcgobj.hpp"

/**
 * \brief Optimization for a static instruction scratchpad, with basic blocks as granularity.
 * Note: the cost for jumping into the scratchpad and jumping back is not considered yet.
 * The optimization depends on the used metric (CONF_USE_RATIO_FILES, CONF_USE_DYNAMIC_INSTRUCTION_COUNT_AS_METRIC, CONF_USE_WCET_PATH_LENGTH_AS_METRIC).
 */
class FSISPOptimizerWCP : public BBSISPOptimizerWCP {
	public:
		/**
		 * \brief Constructor.
		 */
		FSISPOptimizerWCP(ControlFlowGraph cfgraph, CFGVertex entry, CFGVertex exit, const vector<function_graph_t> functions);
		/**
		 * \brief Default destructor.
		 */
		virtual ~FSISPOptimizerWCP();

		/**
		 * \brief Gets the label and address of all assigned functions.
		 * \returns The label and address of all assigned functions.
		 */
		vector<addr_label_t> getFunctionAssignment(void);

		/**
		 * \brief Determines the block assignment. The implementation of the function depends on the type of optimisation.
		 */
		virtual void calculateBlockAssignment(void);
	protected:

		virtual void clear(void);

		/**
		 * \brief Writes the binary domains for the basic blocks.
		 * It is needed by the lp_solve to know which variables to assign.
		 */
		virtual void generateBinaryDomain(void);
		/**
		 * \brief Sets the assignment found by the LpSolver.
		 * \param lpresult Vector with the function variable and its assignment value (1 for in scratchpad, else 0)
		 */
		virtual void setFunctionAssignment(vector<lp_result_set> lpresult);
		/**
		 * \brief Sets the WCET estimate and the used BBSISP size that is calculated during solving the BB assingment ILP
		 * \param lp_result The variable vector from the lp_solver, that contains the WCET of the application in the variable "wentry", and "sp" for the BBSISP size
		 */
		void setVariables(vector<lp_result_set> lp_result);

		/**
		 * \brief Generates constraints that assign basic blocks to the function to which they belong.
		 * e.g. a1 = f1; a2 = f1; ...
		 * Thus the formulation of BBSISP can be used for FSISP, because then the function variables are assigned by the ILP.
		 * Furthermore the binary domains for the functions are created here (e.g. bin f1;), therefore the function generateBinaryDomain() is not used here. This is because not all functions in fcgraphs are considered for the analysis (depending on the chosen entry function) and in generateFunctionMembershipConstraints() it is possible to generate only the binary domains of functions for which basic blocks are contained in the scfg.
		 */
		void generateFunctionMembershipConstraints(void);

		/**
		 * \brief The function_graph_t of all functions in the application, containing name, address and the control flow graph.
		 */
		vector<function_graph_t> fcgraphs;
		/**
		 * \brief The address and label of all assigned function.
		 */
		vector<addr_label_t> assigned_functions;

		/**
		 * \brief Contains the constraints that assign the basic blocks to the functions.
		 */
		vector<string> function_membership_constraints;
};

#endif
