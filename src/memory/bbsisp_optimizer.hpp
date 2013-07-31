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
#ifndef _BBSISP_OPTIMIZER_HPP_
#define _BBSISP_OPTIMIZER_HPP_


#include "sisp_optimizer_if.hpp"

/**
 * \brief Optimization for a static instruction scratchpad, with basic blocks as granularity.
 * Note: the cost for jumping into the scratchpad and jumping back is not considered yet.
 * The optimization depends on the used metric (CONF_USE_RATIO_FILES, CONF_USE_DYNAMIC_INSTRUCTION_COUNT_AS_METRIC, CONF_USE_WCET_PATH_LENGTH_AS_METRIC).
 */
class BBSISPOptimizer : public SISPOptimizer_IF {
	public:
		/**
		 * \brief Constructor.
		 */
		BBSISPOptimizer(ControlFlowGraph cfgraph, CFGVertex entry, CFGVertex exit);
		/**
		 * \brief Default destructor.
		 */
		virtual ~BBSISPOptimizer();
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
		 * \brief The knapsack ILP formulation in lp_solve readable format.
		 */
		vector<string> ilp_knapsack_formulation;
};

#endif
