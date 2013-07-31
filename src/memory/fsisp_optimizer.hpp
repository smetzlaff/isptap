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
#ifndef _FSISP_OPTIMIZER2_HPP_
#define _FSISP_OPTIMIZER2_HPP_


#include "sisp_optimizer_if.hpp"
#include "fcgobj.hpp"

/**
 * \brief Optimization for a static instruction scratchpad, with basic blocks as granularity.
 * Note: the cost for jumping into the scratchpad and jumping back is not considered yet.
 * The optimization depends on the used metric (CONF_USE_RATIO_FILES, CONF_USE_DYNAMIC_INSTRUCTION_COUNT_AS_METRIC, CONF_USE_WCET_PATH_LENGTH_AS_METRIC).
 */
class FSISPOptimizer : public SISPOptimizer_IF {
	public:
		/**
		 * \brief Constructor.
		 */
		FSISPOptimizer(ControlFlowGraph cfgraph, CFGVertex entry, CFGVertex exit, const vector<function_graph_t> functions);
		/**
		 * \brief Default destructor.
		 */
		virtual ~FSISPOptimizer();

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
		 * \brief Generates the ILP for the Knapsack formulation of the optimization. 
		 * The ILP objective function and its constraints are stored in ilp_knapsack_formulation
		 */
		virtual void generateKnapsackILPFormulation(void);
		/**
		 * \brief Sets the assignment found by the LpSolver.
		 * \param lpresult Vector with the function variable and its assignment value (1 for in scratchpad, else 0)
		 */
		virtual void setFunctionAssignment(vector<lp_result_set> lpresult);

		/**
		 * \brief Generates constraints that assign basic blocks to the function to which they belong.
		 * e.g. a1 = f1; a2 = f1; ...
		 * Thus the formulation of BBSISP can be used for FSISP, because then the function variables are assigned by the ILP.
		 * \param set_binary_domains A stringstream into which the binary domains for the functions (e.g. bin f1;) are written.
		 */
		void generateFunctionMembershipConstraints(stringstream &set_binary_domains, stringstream &function_constraints);

		/**
		 * \brief The function_graph_t of all functions in the application, containing name, address and the control flow graph.
		 */
		vector<function_graph_t> fcgraphs;
		/**
		 * \brief The address and label of all assigned function.
		 */
		vector<addr_label_t> assigned_functions;

		/**
		 * \brief The knapsack ILP formulation in lp_solve readable format.
		 */
		vector<string> ilp_knapsack_formulation;
};

#endif
