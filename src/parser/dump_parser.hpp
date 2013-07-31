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
#ifndef _DUMP_PARSR_HPP_
#define _DUMP_PARSR_HPP_


#include "global.h"
#include "configuration.hpp"
#include "constants.h"

#include "isa.hpp"
#include "dumpline_parser.hpp"
#include "cfgobj.hpp"
#include "fcgobj.hpp"
#include "scfgobj.hpp"
#include "jump_target_extractor.hpp"
#include "call_target_extractor.hpp"


#include <vector>
#include <iostream>
#include <fstream>
#include <stdlib.h>


class DumpParser {
	public:
		DumpParser(uint32_t core_number);
		virtual ~DumpParser();

		bool parse(void);
		vector<addr_label_t> getDetectedLabels(void);
		ControlFlowGraph getCFG(string label);
		function_graph_t *getCFGObject(string label);
		CFGVertex getCFGEntry(string label);
		CFGVertex getCFGExit(string label);
		ControlFlowGraph getCFG(uint32_t addr);
		CFGVertex getCFGEntry(uint32_t addr);
		CFGVertex getCFGExit(uint32_t addr);
		FunctionCallGraph getFCG(void);
		vector<function_graph_t> getCFGForAllFunctions(void);
		vector<ControlFlowGraph> getCfgsConnectedWithStartLabel(void);
		vector<ControlFlowGraph> getCfgsNotConnectedWithStartLabel(void);
		ControlFlowGraph getSCFG(void);
		CFGVertex getSCFGEntry(void);
		CFGVertex getSCFGExit(void);
		FunctionCallGraphObject* getFCGObj(void);
		uint32_t getCodeSize(string label);
		uint32_t getCodeSize(uint32_t addr);
		uint32_t getCodeSize(void);
		uint32_t getFunctionCount(void);
		vector<string> getUnparsedDumpCode(void) __attribute__ ((deprecated));
		/**
		 * This method is not needed, because the data sections are copied via elf file into the memory image
		 */
		vector<string> getDataSection(void);
		addr_label_t getEntryFunction(void);
	protected:
		virtual void loadFile(void);
		bool extractJumpLabels(void);
		virtual void parseLabels(void);
		void addIndirectJumpTargets(vector<uint32_t> indirect_targets);
		virtual ControlFlowGraphObject *parseCfgForLabel(addr_label_t function);
		void connectFunctionCFGs(void);


		/**
		 * This method is not needed, because the data sections are copied via elf file into the memory image
		 */
		void copyDataSections(uint32_t start_line);

		addr_label_t getAddrLabelForAddr(uint32_t address);
		vector<uint32_t> getFunctionIDsCalledByStartLabel(void);
		vector<uint32_t> getFunctionIDsNotCalledByStartLabel(void);


		vector<string> unparsed_file_content;
		vector<string> data_section;


		string input_filename;
		vector<addr_label_t> label_vec;
		vector<addr_label_t> blacklist_label;
		vector<uint32_t> jump_target_vec;
		vector<function_graph_t> func_cfgs;

		addr_label_t entry_function;

		/**
		 * \brief Pointer to object for parsing the lines of the dump files.
		 */
		DumpLineParser *dlp;
		/**
		 * \brief Pointer to object for decoding and encoding instructions for the used ISA.
		 */
		ISA *isa;

		RatioFileReader *rfr_onchip, *rfr_offchip;
		JumpTargetExtractor jte;
		CallTargetExtractor cte;

		FunctionCallGraphObject func_graph;
		SuperControlFlowGraphObject super_graph;

		Configuration *conf;

		static LoggerPtr logger;

};

#endif // _DUMP_PARSR_HPP_
