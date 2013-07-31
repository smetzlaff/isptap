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
#ifndef _CALL_TARGET_EXTRACTOR_HPP_
#define _CALL_TARGET_EXTRACTOR_HPP_


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
 * \brief A map that holds the call targets of an indirect call.
 */
typedef map<uint32_t, vector<addr_label_t> > CallTargetMap;
/**
 * \brief Class parsing the flow facts for the "calli" command that specifies the targets of indirect calls.
 * After parsing it provides the call targets to the CarCoreDumpParser().
 */
class CallTargetExtractor {
	public:
		/**
		 * \brief Default constructor.
		 */
		CallTargetExtractor();
		/**
		 * \brief Default destructor.
		 */
		~CallTargetExtractor();
		/**
		 * \brief Delivers all registered call targets for an indirect call.
		 * \param addr The address indirect call to be looked up.
		 * \returns A vector of the function addresses that can be reached by the indirect call. If the vector is empty no target function is registered.
		 */
		vector<addr_label_t> getCallTargetsForIndirectCall(uint32_t addr);
		/**
		 * \brief Sets the addresses and names of all functions in the current application. Then the parsing of the flow facts file is started.
		 * \param functionTable The vector contains the function names and their addresses of the application.
		 */
		void setFunctionTable(vector<addr_label_t> functionTable);

		static string getHelpMessage(void);
	private:
		/**
		 * \brief Parses the flow fact file for indirect jump targets.
		 * Therfore the "calli" command is parsed:
		 * calli "functionname" from 0x(offset) to 0x(function addr) // target function name
		 */
		void readFile();
		/**
		 * \brief Checks if a line in the flow facts contains the "calli" command and is pretty formated.
		 * \param line The line in the flow facts that is processed.
		 */
		bool isCalliLine(string line);
		/**
		 * \brief Returns the address of the call from the line that is calculated by the function address and the source offset.
		 * \param line The "calli" line in the flow facts that is processed.
		 * \returns The indirect call address.
		 */
		uint32_t getICallAddrFromLineWithOffset(string line);
		/**
		 * \brief Obtains the offset of the indirect call.
		 * The offset is the difference of the function address that contains the indirect call and the address of the indirect call itself.
		 * \param line The "calli" line in the flow facts that is processed.
		 * \returns The source offset of the indirect call relative to the function start address.
		 */
		uint32_t getSourceOffsetFromLine(string line);
		/**
		 * \brief Obtains the target address (function address) of the indirect call.
		 * \param line The "calli" line in the flow facts that is processed.
		 * \returns The target address of the function that is called by the indirect call.
		 */
		uint32_t getTargetAddrFromLine(string line);
		/**
		 * \brief Obtains the target name (function name) of the indirect call.
		 * \param line The "calli" line in the flow facts that is processed.
		 * \returns The target name of the function that is called by the indirect call.
		 */
		string getTargetNameFromLine(string line);

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
		 * \brief The map that contains the addresses of the indirect calls and all their registered functions.
		 */
		CallTargetMap cmap;
		/**
		 * \brief The table contains all function names and addresses of the application.
		 * The table is created by parsing all labels by the CarCoreDumpParser::extractJumpLabels().
		 */
		vector<addr_label_t> function_table;
		/**
		 * \brief All target function addresses and labels of all registered indirect calls.
		 * The vector is free of duplicates.
		 */
		vector<addr_label_t> targets;
		/// regular expression to match the indirect call line 
		static regex re_calli_line;
		/// regular expression to find the source function name in the line
		static regex re_src_func_name;
		/// regular expression to find the source offset in the line
		static regex re_src_offset;
		/// regular expression to find the target address in the line
		static regex re_tgt_address;
		/// regular expression to find the target function name in the line (it is given in the comment)
		static regex re_tgt_func_name;
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
