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
#ifndef _FLOWFACT_READER_HPP_
#define _FLOWFACT_READER_HPP_


#include "global.h"
#include "constants.h"
#include "graph_structure.h"
#include "configuration.hpp"
#include "console_string_printer.hpp"

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <boost/regex.hpp>
#include <map>
#include <string>

using namespace boost;

/**
 * \brief Assigns the loop bounds to registered loop control basic block.
 */
typedef map<uint32_t, uint32_t> BbaddrCapMap;
/**
 * \brief Assigns the inducting basic block of a tail decision loop to registered loop control basic block.
 */
typedef map<uint32_t, uint32_t> BbaddrIndBbMap;

/**
 * \brief The map binds the in edge of an basic block to its static flow constraint.
 * Notice: a static flow constraint can only be applied to basic blocks with one in edge.
 */
typedef map<uint32_t, static_flow_constraint_t> BbaddrSFlowMap;

/**
 * \brief Map to store the names and values of flow fact constants.
 */
typedef map<string, uint32_t> ConstMap;

/**
 * \brief Parses a flow fact file for loop bounds and static flow constraints.
 */
class FlowFactReader {
	public:
		/**
		 * \brief Default constructor.
		 */
		FlowFactReader();
		/**
		 * \brief Default destructor.
		 */
		virtual ~FlowFactReader();
		/**
		 * \brief Sets the flow fact file to parse and then starts reading it.
		 * The function triggers readFile().
		 * \param filename The name of the flow fact file to process.
		 */
		void setFile(string filename);
		/**
		 * \brief Returns the loop bound for a node.
		 * \param bb_address The address of the basic block, for which the loop bound is checked.
		 * \returns The loop bound if it was registered in the flow facts file. Otherwise -1 is returned, indicating that the basic block has no registered loop bound.
		 */
		int32_t getLoopBoundCapacity(uint32_t bb_address);
		/**
		 * \brief Checks if a loop deciding block is at the tail of the loop.
		 * Notice: that the checked basic block has to have a valid loop bound. Otherwise the returning value is senseless.
		 * \param bb_address The address of a basic block with a valid loop bound registered.
		 * \returns True if the loop decision block is at the tail of the loop, then the loop inducting basic block has to be obtained by getInductingBB(). Otherwise false is returned.
		 */
		bool isLoopDecisionAtTail(uint32_t bb_address);
		/**
		 * \brief Obtains the loop inducting basic block for loops with decision at tail.
		 * The function delivers only a valid result, iff the isLoopDecisionAtTail() has returned true for the loop.
		 * \param bb_address The address of a basic block with a valid loop bound registered and indicating a decision tail loop.
		 * \returns The address of the loop inducting basic block.
		 */
		uint32_t getInductingBB(uint32_t bb_address);
		/**
		 * \brief Obtains the static flow constraint for a basic block.
		 * \param bb_address The address of a basic block, for which it is checked if its activation underlys a static flow constraint.
		 * \returns The flow constraint if it was specified in the flow facts file. Otherwise the flow_type UNKNOWN is returned, indicating that no constraint is present for that block.
		 */
		static_flow_constraint_t getStaticFlowConstraint(uint32_t bb_address);

		/**
		 * \brief Sets the addresses and names of all functions in the current application.
		 * \param functionTable The vector contains the function names and their addresses of the application.
		 */
		void setFunctionTable(vector<addr_label_t> functionTable);

		static string getHelpMessage(void);

	private:
		/**
		 * \brief Parses the flow fact file for loop bounds and static flow constraints
		 * The following commands are parsed: "loop", "flow_exact", "flow_max" and "flow_min"
		 */
		void readFile(void);
		/**
		 * \brief Checks if a line in the flow facts contains the "loop" command and is pretty formated.
		 * \param line The line in the flow facts that is processed.
		 */
		bool isLoopBoundLine(string line);
		/**
		 * \brief Checks if a line in the flow facts contains one of the "flow_*" commands and is pretty formated.
		 * \param line The line in the flow facts that is processed.
		 */
		bool isStaticFlowConstraintLine(string line);
		/**
		 * \brief Checks if a line in the flow facts contains the "const" command and is pretty formated.
		 * \param line The line in the flow facts that is processed.
		 */
		bool isConstantDefinitionLine(string line);
		/**
		 * \brief Returns the basic block address from the line that is in the comment of the line.
		 * \param line The "loop" or "flow_*" line in the flow facts that is processed.
		 * \returns The basic block address of the basic block that marks a loop (loop control block).
		 */
		uint32_t getBBAddrFromLine(string line);
		/**
		 * \brief Returns the loop or flow bound from the line.
		 * \param line The "loop" or "flow_*" line in the flow facts that is processed.
		 * \returns The loop or flow bound.
		 */
		uint32_t getBoundFromLine(string line);
		/**
		 * \brief Returns the basic block address from the line that is  calculated by the function address and the source offset.
		 * \param line The "loop" or "flow_*" line in the flow facts that is processed.
		 * \returns The basic block address of the basic block that marks a loop(loop control block).
		 */
		uint32_t getBBAddrFromFunctionOffset(string line);
		/**
		 * \brief Obtains the inducting basic block for a decision tail loop.
		 * NEEDED FOR DIRTY WORKAROUND IF THE LOOP EDGE OF A LOOP IS NO BACKWARD EDGE.
		 * \param line The "loop" or "flow_*" line in the flow facts that is processed.
		 * \param inducting_bb_abs_addr The pointer where to write the basic block address of a loop inducting basic block, iff the loop is a tail decision loop.
		 * \returns True if the loop is a tail decision loop, otherwise false.
		 */
		bool getInductingBBFromLine(string line, uint32_t *inducting_bb_abs_addr);
		/**
		 * \brief Gets the type of the static flow constraint.
		 * It is either EXACT, MAX or MIN.
		 * \param line The "flow_*" line in the flow facts that is processed.
		 * \returns The type of the static flow constraint. On any error UNKNOWN is returned.
		 */
		flowc_type_t getStaticFlowType(string line);
		/**
		 * \brief Returns the name of a flow fact constant for a given flow fact constant definition line.
		 * \param line The "const" line in the flow facts that is processed.
		 * \returns The name of the flow fact constant.
		 */
		string getConstNameFromLine(string line);
		/**
		 * \brief Returns the value of a flow fact constant for a given flow fact constant definition line.
		 * \param line The "const" line in the flow facts that is processed.
		 * \returns The value of the flow fact constant.
		 */
		uint32_t getConstValueFromLine(string line);
		/**
		 * \brief The file name of the flow facts file, obtained from the Configuration class.
		 */
		string ff_file_name;
		/**
		 * \brief Denotes if the flow facts file is already processed.
		 */
		bool file_read;

		/**
		 * \brief Assigns the inducting basic block of a tail decision loop to registered loop control basic block.
		 */
		BbaddrCapMap bb_addr_cap;
		/**
		 * \brief Assigns the loop bounds to registered loop control basic block.
		 */
		BbaddrIndBbMap bb_ind;
		/**
		 * \brief The map contains the static flow constraints extracted from the flow fact file.
		 */
		BbaddrSFlowMap bb_sflows;
		/**
		 * \brief The map contains the constants for the flow facts set in the flow fact file.
		 */
		ConstMap const_map;

		/**
		 * \brief The table contains all function names and addresses of the application.
		 * The table is created by parsing all labels by the CarCoreDumpParser::extractJumpLabels().
		 */
		vector<addr_label_t> function_table;
		// regular expression to match the loop bound line
		static regex re_lb_line;
		/// regular expression to match the exact flow constraint line
		static regex re_sfex_line;
		/// regular expression to match the minimum flow constraint line
		static regex re_sfmi_line;
		/// regular expression to match the maximum flow constraint line
		static regex re_sfma_line;
		/// regular expression to find the block address (usually this is given as a comment after the loop bound) within the line
		static regex re_bb_addr;
		/// regular expression to find the loop resp. static flow bound within the line 
		static regex re_bound;
		/// regular expression to find the loop resp. static flow bound as a constant declaration within the line 
		static regex re_bound_const;
		/// regular expression to find the function name in the line
		static regex re_func_name;
		/// regular expression to find the offset in the line
		static regex re_func_offset;
		// regular expression to find an additional induction basic block information (for the case that the loop head has 2 forward jumps as in edges)
		/// NEEDED  FOR DIRTY WORKAROUND IF THE LOOP EDGE OF A LOOP IS NO BACKWARD EDGE
		static regex re_inducting_bb;
		/// regular expression to match a flow constant line
		static regex re_const_line;
		/// regular expression to find the flow constant name
		static regex re_const_name;
		/// regular expression to find the flow constant value
		static regex re_const_value;
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

