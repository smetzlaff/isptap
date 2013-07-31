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
#ifndef _STACK_BF_DISP_STATE_MAINTAINER_HPP_
#define _STACK_BF_DISP_STATE_MAINTAINER_HPP_

#include "bf_disp_state_maintainer.hpp"
#include "stack_disp_mem_state.hpp"

/*!
 * \brief The abstract DISP state used by BFDISPStateMaintainerStack.
 * It is represented as a vector of STACKDISPMemState objects.
 */
typedef vector<STACKDISPMemState>  AbsSTACKDISPMemState;

/*!
 * \brief A map to store the abstract DISP states for the BFDISPStateMaintainerStack for each MSGVertex node in the graph.
 * The abstract DISP state is represented by a pointer to a vector of concrete states.
 */
typedef map<MSGVertex, AbsSTACKDISPMemState*> NodeSTACKDISPStateMap;

/*!
 * \brief DISP state maintainer for brute force analysis supporting update() and join() for data flow analysis for the stack-based replacement policy.
 * Brute force means that the abstract memory states are not joined in an abstract domain if they are differ. This causes a larger memory usage during analysis, but the result will be correct since all concrete states are assumed.
 */ 
class BFDISPStateMaintainerStack : public BFDISPStateMaintainer {
	public:
		/*!
		 * \brief Constructor.
		 * \param params The parameters of the DISP memory.
		 * \param fcgo Function call graph object. Used to obtain function properties.
		 */
		BFDISPStateMaintainerStack(disp_params_t params, FunctionCallGraphObject* fcgo);
		 /*!
		 * \brief Updates the abstract DISP state set a function address that is activated.
		 * \param predecessor The predecessor node, from which the function is activated. Thus the DISP state of this node is updated.
		 * \param node The current node, of which the new DISP state is created. Based on the predecessor state and the activated function.
		 * \param function_addr The address of the activated function.
		 * \param activation The type of the activation: either CALL or RETURN
		 * \param previous_function The address of the function that activated the function_addr, i.e. it is either the caller or the callee.
		 */
		void update(MSGVertex predecessor, MSGVertex node, uint32_t function_addr, activation_type_t activation, uint32_t previous_function);
		/*!
		 * \brief Merges multiple concrete DISP states.
		 * If the concrete states differ they will be transferred in the merged state. So the abstract state is represented by all possible concrete states.
		 * \param predecessors A vector of nodes for which the control flow is merged.
		 * \param node The node in which the predecessor control flows are joined.
		 */
		void join(vector<MSGVertex> *predecessors, MSGVertex node);
		/*!
		 * \brief Transfers an abstract DISP state from one node to another, without changing it.
		 * When transferring an abstract DISP state the new node is registered to the same address of the abstract DISP state as the original one.
		 * \param predecessor The node of which the abstract DISP state is to be read.
		 * \param node The node to which the same abstract DISP state as its predecessor is assigned.
		 */
		void transfer(MSGVertex predecessor, MSGVertex node);
		/*!
		 * \brief Prints the abstract DISP state of a given node.
		 * The node has to be processed and an abstract DISP state was created, otherwise an assertion is violated.
		 * \param os The stream in which the abstract DISP state is written into.
		 * \param node The node of which the abstract DISP state is printed.
		 */
		void printMemSet(ostringstream *os, MSGVertex node);
		/*!
		 * \brief Sets a blank abstract DISP state for a given node.
		 * \param node The node for which a new empty abstract DISP state is to be created.
		 */
		void setBlankMemState(MSGVertex node);
	private:
		/*!
		 * \brief Default constructor.
		 * Made private.
		 */
		BFDISPStateMaintainerStack();
		/*!
		 * \brief Destructor.
		 */ 
		virtual ~BFDISPStateMaintainerStack();
		/*!
		 * \brief Updates an abstract DISP state by activating a function.
		 * Depending on the content of the abstract DISP state it may be altered.
		 * \param state The abstract DISP state that is to be updated by activation of one address.
		 * \param function_addr The address of the function that is activated and updates the abstract DISP state.
		 * \param activation The activation type: either CALL or RETURN.
		 * \param previous_function The address of the function that caused the activation of the function_addr, i.e. it is either the caller of callee.
		 */
		void updateState(AbsSTACKDISPMemState *state, uint32_t function_addr, activation_type_t activation, uint32_t previous_function);
		/*!
		 * \brief Gets the abstract DISP state of a given node.
		 * If the abstract DISP state is not created, an assertion is violated.
		 * All abstract DISP states are stored in the NodeMemSetMap.
		 * \param node The node for which the abstract DISP state is obtained.
		 * \returns The pointer to the abstract DISP state, if it is found. Else an assertion is violated.
		 */
		AbsSTACKDISPMemState *getAbsSTACKDISPMemState(MSGVertex node);
		/*!
		 * \brief Registers an abstract DISP state for a given node in the NodeMemSetMap.
		 * This is necessary to get the abstract DISP state later on using getAbsSTACKDISPMemState().
		 * \param node The node for which the abstract DISP state is registered.
		 * \param state The pointer to the abstract DISP state that is to be registered.
		 * \returns True if the abstract DISP state was registered for that node, else false.
		 */
		bool addAbsMemState(MSGVertex node, AbsSTACKDISPMemState *state);
		/*!
		 * \brief Removes duplicate concrete DISP states from the abstract DISP state.
		 * \param state The pointer to the abstract DISP state of which the duplicates will be deleted.
		 */
		void removeDuplicates(AbsSTACKDISPMemState *state);
		/*!
		 * \brief Prints a given abstract DISP state into a stream.
		 * \param os The stream in which the abstract DISP state is to be printed into.
		 * \param state The abstract DISP state that will be printed.
		 */
		void printMemSet(ostringstream *os, AbsSTACKDISPMemState *state);
		/*!
		 * \brief Checks if a function is in all possible concrete DISP states of a node.
		 * \param addr The address of the function to be checked if it is in all concrete DISP states.
		 * \param node The node for which it is checked if the function is present in all concrete DISP states.
		 * \returns True if the function is in all concrete DISP states, else false.
		 */
		bool isInAllSets(uint32_t addr, MSGVertex node);
		/*!
		 * \brief Checks if a function is in all possible concrete DISP states of a node.
		 * \param addr The address of the function to be checked if it is in all concrete DISP states.
		 * \param state The abstract DISP state for which it is checked if the function is present in all of its concrete DISP states.
		 * \returns True if the function is in all concrete DISP states, else false.
		 */
		bool isInAllSets(uint32_t addr, AbsSTACKDISPMemState *state);
		/*!
		 * \brief Checks if a function is in any possible concrete DISP state of a node.
		 * \param addr The address of the function to be checked if it is in any concrete DISP state.
		 * \param node The node for which it is checked if the function is present in any concrete DISP state.
		 * \returns True if the function is in any concrete DISP states, else false.
		 */
		bool isInAnySet(uint32_t addr, MSGVertex node);
		/*!
		 * \brief Checks if a function is in any possible concrete DISP state of a node.
		 * \param addr The address of the function to be checked if it is in any concrete DISP state.
		 * \param state The abstract DSIP state for which it is checked if the function is present in any of its concrete DISP states.
		 * \returns True if the function is in any concrete DISP states, else false.
		 */
		bool isInAnySet(uint32_t addr, AbsSTACKDISPMemState *state);
		/*!
		 * \brief Checks if a function is none possible concrete DISP state of a node.
		 * \param addr The address of the function to be checked if it is no concrete DISP state.
		 * \param node The node for which it is checked if the function is present in no concrete DISP state.
		 * \returns True if the function is in no concrete DISP state, else false.
		 */
		bool isInNoSet(uint32_t addr, MSGVertex node);
		/*!
		 * \brief Checks if a function is none possible concrete DISP state of a node.
		 * \param addr The address of the function to be checked if it is no concrete DISP state.
		 * \param state The abstract DISP state for which it is checked if the function is present in no of its concrete DISP states.
		 * \returns True if the function is in no concrete DISP state, else false.
		 */
		bool isInNoSet(uint32_t addr, AbsSTACKDISPMemState *state);
		/*!
		 * \brief The map that contains for each processed node the corresponding abstract DISP state.
		 */
		NodeSTACKDISPStateMap state_map;
		/*!
		 * \brief Pointer to the LOGCXX logger object.
		 */
		static LoggerPtr logger;
};

#endif
