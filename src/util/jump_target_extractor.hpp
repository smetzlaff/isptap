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
#ifndef _JUMP_TARGET_EXTRACTOR_HPP_
#define _JUMP_TARGET_EXTRACTOR_HPP_


#include "global.h"
#include "constants.h"
#include "graph_structure.h"
#include "configuration.hpp"

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <boost/regex.hpp>
#include <map>
#include <string>

using namespace boost;
/**
 * \brief A map that holds the jump targets of an indirect jump.
 */
typedef map<uint32_t, vector<uint32_t> > JumpTargetMap;
/**
 * \brief Class parsing the flow facts for the "jind" command that specifies the targets of indirect jumps.
 * After parsing it provides the jump targets to the CarCoreDumpParser().
 */
class JumpTargetExtractor {
	public:
		/**
		 * \brief Default constructor.
		 */
		JumpTargetExtractor();
		/**
		 * \brief Default destructor.
		 */
		virtual ~JumpTargetExtractor();
		/**
		 * \brief Delivers all registered jump targets for a basic block that contains an indirect jump.
		 * \param bb_addr The start address of the basic block for which the targets of the indirect jump are to be looked up.
		 * \returns A vector of the basic block addresses that can be reached by the indirect jump. If the vector is empty no target basic block is registered.
		 */
		vector<uint32_t> getJumpTargetsForIndirectJump(uint32_t bb_addr);
		/**
		 * \brief Returns all registered targets that can be reached by any indirect jump. 
		 * It is needed to correctly split the programm code into basic blocks. 
		 * \returns A vector of all basic block addresses that can be reached by any registered indirect jump. The provided addresses contain no duplicates. If the vector is empty, no registered indirect jump is in the application.
		 */
		vector<uint32_t> getAllTargets(void);
		/**
		 * \brief Sets the addresses and names of all functions in the current application. Then the parsing of the flow facts file is started.
		 * \param functionTable The vector contains the function names and their addresses of the application.
		 */
		void setFunctionTable(vector<addr_label_t> functionTable);

		static string getHelpMessage(void);
	private:
		/**
		 * \brief Parses the flow fact file for indirect jump targets.
		 * Therfore the "jind" command is parsed:
		 * jind "functionname" from 0x(offset) to 0x(offset) // jumping block addr
		 */
		void readFile();
		/**
		 * \brief Checks if a line in the flow facts contains the "jind" command and is pretty formated.
		 * \param line The line in the flow facts that is processed.
		 */
		bool isJindLine(string line);
		/**
		 * \brief Returns the basic block address from the line that is in the comment of the line.
		 * \param line The "jind" line in the flow facts that is processed.
		 * \returns The basic block address of the basic block that contains a indirect jump.
		 */
		uint32_t getBBAddrFromLine(string line);
		/**
		 * \brief Returns the basic block address from the line that is  calculated by the function address and the source offset.
		 * \param line The "jind" line in the flow facts that is processed.
		 * \returns The basic block address of the basic block that contains a indirect jump.
		 */
		uint32_t getBBAddrFromLineWithOffset(string line);
		/**
		 * \brief Obtains the offset of the basic block that contains the indirect jump.
		 * The offset is the difference of the function address that contains the basic block and the address of the basic block itself.
		 * \param line The "jind" line in the flow facts that is processed.
		 * \returns The source offset of the basic block of the indirect jump containing basic block.
		 */
		uint32_t getSourceOffsetFromLine(string line);
		/**
		 * \brief Obtains the offset of the basic block that is targeted by the indirect jump.
		 * The offset is the difference of the function address that contains the basic block and the address of the target basic block itself.
		 * \param line The "jind" line in the flow facts that is processed.
		 * \returns The target offset of the basic block of the indirect jump containing basic block.
		 */
		uint32_t getTargetOffsetFromLine(string line);
		/**
		 * \brief Adds an indirect jump target to the list of all indirect jump targets (targets).
		 * Duplicate entries are not added.
		 * \param tgt The address of a basic block that is a target of an indirect jump.
		 */
		inline void addTarget(uint32_t tgt);

		/**
		 * \brief The file name of the flow facts file, obtained from the Configuration class.
		 */
		string ff_file_name;
		/**
		 * \brief Denotes if the flow facts file is already processed.
		 */
		bool file_read;
		/**
		 * \brief Denotes if there is a flow facts file to read.
		 */
		bool use_ff;
		/**
		 * \brief The map that contains the basic block addresses that contain a indirect jump and all their registered jump targets.
		 */
		JumpTargetMap jmap;
		/**
		 * \brief The table contains all function names and addresses of the application.
		 * The table is created by parsing all labels by the CarCoreDumpParser::extractJumpLabels().
		 */
		vector<addr_label_t> function_table;
		/**
		 * \brief All target basic block addresses of all registered indirect jumps.
		 * The vector is free of duplicates.
		 */
		vector<uint32_t> targets;
		/// regular expression to match the indirect jump line 
		static regex re_ji_line;
		/// regular expression to find the block address (it is given as a comment) within the line
		static regex re_bb_addr;
		/// regular expression to find the function name in the line
		static regex re_func_name;
		/// regular expression to find the source offset in the line
		static regex re_src_offset;
		/// regular expression to find the target offset in the line
		static regex re_tgt_offset;
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
