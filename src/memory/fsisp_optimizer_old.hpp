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
#ifndef _FSISP_OPTIMIZER_HPP_
#define _FSISP_OPTIMIZER_HPP_


#include "global.h"
#include "constants.h"
#include "graph_structure.h"
#include "fcgobj.hpp"
#include "lpsolver.hpp"
#include "configuration.hpp"

/**
 * \brief Structure for all necessary function information.
 * Needed by FSISPOptimizerOLD to store all informations for each function.
 */
struct function_record_t {
	/**
	 * \brief Fucntion ID used for the ILP generation as valiable identifier.
	 */
	uint32_t id;
	/**
	 * \brief Start address of the function.
	 */
	uint32_t address;
	/**
	 * \brief End address of the function.
	 */
	uint32_t end_address;
	/**
	 * \brief Function name.
	 */
	string name;
	/**
	 * \brief Size of the function in bytes.
	 */
	uint32_t size;
	/**
	 * \brief Current cost of the function.
	 * The cost depends on the used optimization metric (CONF_USE_RATIO_FILES, CONF_USE_DYNAMIC_INSTRUCTION_COUNT_AS_METRIC, CONF_USE_WCET_PATH_LENGTH_AS_METRIC) and if the function is located in the scratchpad or not.
	 */
	uint32_t cur_cost;
	/**
	 * \brief Cost of the function, if it is contained in the scratchpad.
	 * The cost depends on the used optimization metric (CONF_USE_RATIO_FILES, CONF_USE_DYNAMIC_INSTRUCTION_COUNT_AS_METRIC, CONF_USE_WCET_PATH_LENGTH_AS_METRIC).
	 */
	uint32_t onchip_cost;
	/**
	 * \brief Cost of the function, if it is not contained in the scratchpad.
	 * The cost depends on the used optimization metric (CONF_USE_RATIO_FILES, CONF_USE_DYNAMIC_INSTRUCTION_COUNT_AS_METRIC, CONF_USE_WCET_PATH_LENGTH_AS_METRIC).
	 */
	uint32_t offchip_cost;
	/**
	 * \brief The benefit of the function, if the function is contained in the scratchpad.
	 * The benefit depends on the used optimization metric (CONF_USE_RATIO_FILES, CONF_USE_DYNAMIC_INSTRUCTION_COUNT_AS_METRIC, CONF_USE_WCET_PATH_LENGTH_AS_METRIC).
	 */
	uint32_t benefit;
	/**
	 * \brief The control flow graph of the function.
	 */
	ControlFlowGraph cfg;
};




/**
 * \brief Optimization for a function based static scratchpad.
 * The optimization depends on the used metric (CONF_USE_RATIO_FILES, CONF_USE_DYNAMIC_INSTRUCTION_COUNT_AS_METRIC, CONF_USE_WCET_PATH_LENGTH_AS_METRIC).
 *
 */
class FSISPOptimizerOLD {
	public:
		/**
		 * \brief Default constructor.
		 */
		FSISPOptimizerOLD();
		/**
		 * \brief Default destructor.
		 */
		virtual ~FSISPOptimizerOLD();
		/**
		 * \brief Sets the size of the static function based scratchpad in bytes.
		 * \param size Size of the scratchpad in bytes to set.
		 */
		void setSize(uint32_t size);
		/**
		 * \brief Sets the functions and the call graph for the optimizer.
		 * \param fcgraph The function call graph of the code.
		 * \param fcgraphs A vector of CFGs of every function, including also meta-data like function name and address.
		 */
		void setFunctions(FunctionCallGraph fcgraph, vector<function_graph_t> fcgraphs);
		/**
		 * \brief Calculates the benefits of every function, if moving them to the scratchpad.
		 * Based on the used metric (CONF_USE_RATIO_FILES, CONF_USE_DYNAMIC_INSTRUCTION_COUNT_AS_METRIC, CONF_USE_WCET_PATH_LENGTH_AS_METRIC) the benefit is calculated in different ways. To consider the activation count of each function (and it's basic blocks)  the ILP solved super CFG is used.
		 * \param scfg Super control flow graph with maximum basic block activation counts, as calculated using ILP. Obtained by ILPGenerator.getILPCFG().
		 */
		void calculateFunctionBenefitsWithSCFGProperties(ControlFlowGraph scfg);
		/**
		 * \brief Formulates knapsack problem, solves it with ILP and sets the assigned functions.
		 */
		void calculateKnapsackFunctionAssignment(void);
		/** 
		 * \brief Returns the assigned functions for the given scratchpad size and the optimization metric.
		 * \returns Vector of addresses and labels of all functions assigned to the static function based scratchpad.
		 */
		vector<addr_label_t> getFunctionAssignment(void);
		/** 
		 * \brief Returns the basic blocks of all assigned functions for the given scratchpad size and the optimization metric.
		 * Needed for the ILPGenerator to recalculate the WCET if the selected functions are assigned to the static scratchpad.
		 * \returns Vector of addresses of all basic blocks of the functions assigned to the static function based scratchpad.
		 */
		vector<uint32_t> getBlockAssignment(void);
		/**
		 * \brief Returns the part of the scratchpad that is used in bytes.
		 * \returns Size of the used part of the scratchpad in bytes.
		 */
		uint32_t getUsedSispSize(void);
	private:
		/**
		 * \brief Write the Knapsack ILP formulation to ilpfile_name.
		 */
		void writeKnapsackILPFile(void);
		/**
		 * \brief Generates the ILP for the Knapsack formulation of the optimization. 
		 * The ILP objective function and its constraints are stored in ilp_knapsack_formulation
		 */
		void generateKnapsackILPFormulation(void);
		/**
		 * \brief Sets the assignment found by the LpSolver.
		 * \param lp_result Vector with the function variable and its assignment value (1 for in scratchpad, else 0)
		 */
		void setAssignment(vector<lp_result_set> lp_result);
		/**
		 * \brief Adds all basic blocks of the given function to the assigned_blocks vector.
		 * \param function_data_id The function of which all basic blocks should be added.
		 */
		void addBBsOfFunctionToAssignedBlocks(uint32_t function_data_id);


		/**
		 * \brief Size of the static instruction scratchpad in bytes.
		 */
		uint32_t sisp_size;

		/**
		 * \brief Size of all assigned functions in the scratchpad in bytes. 
		 */
		uint32_t used_sisp_size;
		/**
		 * \brief The function call graph of the code.
		 */
		FunctionCallGraph fcg;
		/**
		 * \brief All necessary data for each function, see structure for details.
		 */
		vector<function_record_t> function_data;
		/**
		 * \brief Addresses and labels for all functions assigned to the scratchpad.
		 */
		vector<addr_label_t> assigned_functions;
		/**
		 * \brief The addresses of the basic blocks for all functions assigned to the scratchpad;
		 */
		vector<uint32_t> assigned_blocks;
		/**
		 * \brief Name of the file that is generated, when using the LpSolver.
		 * The file contains the knapsack fomulation of the static function scratchpad optimization.
		 */
		string ilpfile_name;
		/**
		 * \brief The knapsack ILP formulation in lp_solve readable format.
		 */
		string ilp_knapsack_formulation;
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
