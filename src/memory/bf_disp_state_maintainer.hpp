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
#ifndef _BF_DISP_STATE_MAINTAINER_HPP_
#define _BF_DISP_STATE_MAINTAINER_HPP_

#include "constants.h"
#include "graph_structure.h"
#include "fcgobj.hpp"
#include "mem_usage_stats.hpp"
#include "memory_params.hpp"

/*!
 * \brief DISP state maintainer for brute force analysis supporting update() and join() for data flow analysis.
 * Brute force means that the abstract memory states are not joined in an abstract domain if they are differ. This causes a larger memory usage during analysis, but the result will be correct since all concrete states are assumed.
 */ 
class BFDISPStateMaintainer : public MemoryUsageStats {
	public:
		 /*!
		 * \brief Updates the abstract DISP state set a function address that is activated.
		 * \param predecessor The predecessor node, from which the function is activated. Thus the DISP state of this node is updated.
		 * \param node The current node, of which the new DISP state is created. Based on the predecessor state and the activated function.
		 * \param function_addr The address of the activated function.
		 * \param activation The type of the activation: either CALL or RETURN
		 * \param previous_function The address of the function that activated the function_addr, i.e. it is either the caller or the callee.
		 */
		virtual void update(MSGVertex predecessor, MSGVertex node, uint32_t function_addr, activation_type_t activation, uint32_t previous_function) = 0;
		/*!
		 * \brief Merges multiple concrete DISP states.
		 * If the concrete states differ they will be transferred in the merged state. So the abstract state is represented by all possible concrete states.
		 * \param predecessors A vector of nodes for which the control flow is merged.
		 * \param node The node in which the predecessor control flows are joined.
		 */
		virtual void join(vector<MSGVertex> *predecessors, MSGVertex node) = 0;
		/*!
		 * \brief Transfers an abstract DISP state from one node to another, without changing it.
		 * When transferring an abstract DISP state the new node is registered to the same address of the abstract DISP state as the original one.
		 * \param predecessor The node of which the abstract DISP state is to be read.
		 * \param node The node to which the same abstract DISP state as its predecessor is assigned.
		 */
		virtual void transfer(MSGVertex predecessor, MSGVertex node) = 0;
		/*!
		 * \brief Prints the abstract DISP state of a given node.
		 * The node has to be processed and an abstract DISP state was created, otherwise an assertion is violated.
		 * \param os The stream in which the abstract DISP state is written into.
		 * \param node The node of which the abstract DISP state is printed.
		 */
		virtual void printMemSet(ostringstream *os, MSGVertex node) = 0;
		/*! 
		 * \brief Returns the size of a function in memory.
		 * Therefore the FunctionCallGraphObject is used. The size of the function in memory is based on the block_size
		 * \param address The address of a function, for which the size is returned.
		 * \returns The size of the function, if no size information for the requested address exists 0 is returned.
		 */
		uint32_t getFunctionMemSize(uint32_t address);
		/*!
		 * \brief Determines if function is definitely in the abstract DISP state of a node.
		 * By definition the function has to be in every concrete state that is contained to the abstract state.
		 * \param func_addr The address of the function to be checked if it is definitely in the abstract DISP state.
		 * \param node The node for which it is checked if the function is definitely in the abstract DISP state.
		 * \returns True if the function is definitely in the abstract DISP state, else false.
		 */
		bool isInMust(uint32_t func_addr, MSGVertex node);
		/*!
		 * \brief Determines if a function might be in the abstract DISP state of a node.
		 * By definition the function has to be in at least one concrete state that is contained to the abstract state.
		 * \param func_addr The address of the function to be checked if it might be in the abstract DISP state.
		 * \param node The node for which it is checked if the function might be in the abstract DISP state.
		 * \returns True if the function might be in the abstract DISP state, else false.
		 */
		bool isInMay(uint32_t func_addr, MSGVertex node);
		/*!
		 * \brief Sets a blank abstract DISP state for a given node.
		 * \param node The node for which a new empty abstract DISP state is to be created.
		 */
		virtual void setBlankMemState(MSGVertex node) = 0;
	protected:
		/*!
		 * \brief Checks if a function is in all possible concrete DISP states of a node.
		 * \param addr The address of the function to be checked if it is in all concrete DISP states.
		 * \param node The node for which it is checked if the function is present in all concrete DISP states.
		 * \returns True if the function is in all concrete DISP states, else false.
		 */
		virtual bool isInAllSets(uint32_t addr, MSGVertex node) = 0;
		/*!
		 * \brief Checks if a function is in any possible concrete DISP state of a node.
		 * \param addr The address of the function to be checked if it is in any concrete DISP state.
		 * \param node The node for which it is checked if the function is present in any concrete DISP state.
		 * \returns True if the function is in any concrete DISP states, else false.
		 */
		virtual bool isInAnySet(uint32_t addr, MSGVertex node) = 0;
		/*!
		 * \brief Checks if a function is none possible concrete DISP state of a node.
		 * \param addr The address of the function to be checked if it is no concrete DISP state.
		 * \param node The node for which it is checked if the function is present in no concrete DISP state.
		 * \returns True if the function is in no concrete DISP state, else false.
		 */
		virtual bool isInNoSet(uint32_t addr, MSGVertex node) = 0;
		/*!
		 * \brief Parameters of the DISP.
		 */
		disp_params_t disp_parameters;
		 /*!
		  * \brief Pointer to a function call graph object, that is needed to obtain the sizes of the functions.
		  */
		FunctionCallGraphObject *functions;
};

#endif
