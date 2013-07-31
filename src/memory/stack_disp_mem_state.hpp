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
#ifndef _STACK_DISP_MEM_STATE_HPP_
#define _STACK_DISP_MEM_STATE_HPP_

#include "disp_mem_state.hpp"

/*!
 * \brief Position of a function in concrete DISP state.
 * Defines the position of a function in the concrete DISP state for the stack-based replacement policy.
 */
typedef struct {
	/// The begin position of a function in the concrete DISP state.
	uint16_t pos_begin; 	
	/// The end position of a function in the concrete DISP state.
	uint16_t pos_end; 
} position_t;



/*!
 * \brief An entry of the concrete DISP state used by STACKDISPMemState.
 */
struct DISPMemEntryStack{
	/*!
	 * The address of a function that is in the concrete DISP state.
	 */
	uint32_t address; 
	/*!
	 * The begin position of a function in the concrete DISP state.
	 */
	uint32_t pos_begin; 
	/*!
	 * The end position of a function in the concrete DISP state.
	 */
	uint32_t pos_end;
};

/*!
 * \brief The concrete DISP state used by STACKDISPMemState.
 */
typedef vector<DISPMemEntryStack> DISPMemSetStack; 

/*!
 * \brief Class for implementation of a concrete DISP state.
 * It is used for a correct STACK DISP content analysis using brute force.
 */
class STACKDISPMemState : public DISPMemState {
	public:
		/*!
		 * \brief Constructor.
		 * \param memSize The size of the DISP memory.
		 * \param blockSize The size of the minimal usable blocks in the scratchpad.
		 * \param maxFuncs The number of function that may be used by the DISP concurrently.
		 * \param ignoreOutsizedFunctions If true functions that are larger than the scratchpad will not be loaded, else if such function should be maintained by the DISP an assertion is violated.
		 * \param fcgo Function call graph object. Used to obtain function properties.
		 */
		STACKDISPMemState(uint32_t memSize, uint32_t blockSize, uint32_t maxFuncs, bool ignoreOutsizedFunctions, FunctionCallGraphObject* fcgo);
		/*!
		 * \brief Copy-constructor.
		 * \param copy The object to copy.
		 */
		STACKDISPMemState(const STACKDISPMemState& copy);
		/*! 
		 * \brief Destructor.
		 */
		virtual ~STACKDISPMemState();
		/*!
		 * \brief Implements the changes to the concrete DISP state if a function is activated.
		 * \param func_addr The address of the function that is activated.
		 * \param act The type of activation: either CALL or RETURN.
		 * \param predecessor The function that caused the activation of the func_addr, either the caller or the callee.
		 */
		void activateFunction(uint32_t func_addr, activation_type_t act, uint32_t predecessor);
		/*!
		 * \brief Determines if a certain function is in the current concrete state of the DISP.
		 * \param func_addr The address of the function that is tested.
		 * \returns True if the function is in the DISP, else false.
		 */
		bool isInState(uint32_t func_addr);
		/*!
		 * \brief Prints the content of the concrete DISP state into a given stream.
		 * \param os The stream to print the concrete DISP state into.
		 */
		void print(ostringstream *os);
		/*!
		 * \brief Prints the content of the concrete DISP state including the positions of each function in the memory into a given stream.
		 * \param os The stream to print the concrete DISP state into.
		 */
		void printPos(ostringstream *os);
		/*!
		 * \brief Copy-operator: copies the content of an object into another.
		 * \param other Object that is the source for the copy by value
		 * \returns A new object containing the same content, but at another address.
		 */
		STACKDISPMemState& operator=(const STACKDISPMemState& other);
		/*!
		 * \brief Compares two concrete DISP memory states.
		 * \param other The object to compare.
		 * \returns True if the concrete DISP states are equal, else false
		 */
		bool operator== (const STACKDISPMemState& other);
		/*!
		 * \brief Normalizes the memory state to the position of the first function in the vector.
		 */
		void normalize(void);
		/*!
		 * \brief Normalizes the memory state to the position of the function with the given address.
		 * \param func_addr The address of the function of which the memory state should be normalized to.
		 */
		void normalize(uint32_t func_addr);
		/*!
		 * \brief Returns the memory size used for the representation of the concrete DISP state.
		 * The size includes the size of the content container and the size of the class.
		 * \returns The memory size used for the representation of the concrete DISP state.
		 */
		uint32_t getAllocatedSize(void);
		/*!
		 * \brief Returns the number of maintained memory references.
		 * \returns The number of maintained memory references.
		 */
		uint32_t getNumberOfMaintainedReferences(void);
	private:
		/*!
		 * \brief Default constructor (declared as private).
		 */
		STACKDISPMemState();
		/*!
		 * \brief Returns the amount of memory that is used by all functions that are currently in the concrete DISP state.
		 * \returns The amount of memory used by all containing functions.
		 */
		uint32_t getUsedSize();
		/*!
		 * \brief Evicts all functions that have an intersection with the given entry.
		 * The entry that causes the eviction will not be evicted.
		 * \param evict The entry which should evict all functions from memory that intersect.
		 * \returns True if at least one function was evicted, else false
		 */
		bool evictFunctionsInRange(DISPMemEntryStack evict);
		 /*!
		 * \brief Checks if two functions intersect eachother.
		 * The function takes any possible alignment of both functions (including wrap around the end of the memory) into account.
		 * \param a The position range of the first function.
		 * \param b The position range of the second function.
		 * \returns True if both funcitons intersect, else false.
		 */
		bool inline checkFunctionIntersection(position_t a, position_t b);
		/*!
		 * \brief Gets the intersection of two functions.
		 * \param a The position range of the first function.
		 * \param b The position range of the second function.
		 * \returns True if both functions intersect and the position range of the intersection, else false.
		 */
		pair<bool, position_t> inline getFunctionIntersection(position_t a, position_t b);
		/*!
		 * \brief Aligns the concrete DISP state with a given address.
		 * The position of the functions is aligned by: pos = pos - offset<br>
		 * To ensure the the positions are in the range of the memory they are normalized to the interval [0,mem_size[.
		 * \param offset The offset with which function in the DISP state is relocated.
		 */
		void alignWithOffset(uint32_t offset);
		/*!
		 * \brief The content of the concrete DISP state.
		 */ 
		DISPMemSetStack content;
		/**
		 * \brief Pointer to the LOGCXX logger object.
		 */
		static LoggerPtr logger;
};
#endif
