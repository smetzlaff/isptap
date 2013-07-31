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
#ifndef _DISP_INSTRUMENTATOR_HPP_
#define _DISP_INSTRUMENTATOR_HPP_

#include "global.h"
#include "constants.h"
#include "configuration.hpp"
#include "graph_structure.h"
#include "fcgobj.hpp"
#include "cfgmanipulator.hpp"
#include "dumpline_parser.hpp"

#define BIT_ALIGNMENT 3

/**
 * \brief Class for instrumenting each function with the function length encoding (FLE) as first instruction of each function.
 * NOTE: This class alteres only the code, not the data section. (This is possible by adding the unparsed lines of the dump file
 * then the dump has to be created using the -D option (all sections) - but then there is a conlict between virtual and physical addresses.
 * So it is recommended to use the code section of the created raw file and the other sections of the original elf.)
 *
 */
class DISPInstrumentator {
	public:
		/**
		 * \brief Default constructor
		 */
		DISPInstrumentator();
		/**
		 * \brief Default destructor
		 */
		virtual ~DISPInstrumentator();
		/**
		 * \brief Adds a cfg of a function to the D-ISP instrumentator.
		 * \param cfgo The function graph to add.
		 */
		void addFunction(function_graph_t cfgo);
		/**
		 * \brief Adds a a set of function cfgs to the D-ISP instrumentator.
		 * \param vec_cfgo Vector of the function graphs to add.
		 */
		void addFunctions(vector<function_graph_t> vec_cfgo);
		/**
		 * \brief Adds lines from the dump file to the D-ISP instrumentator.
		 * Since the part of the dump file is not parsed no cfg object exist for the code or the lines represent any non code section (data).
		 * \param lines Unparsed lines from the dump file, may contain either code or data.
		 */
		void addUnparsedLines(vector<string> lines);
		/**
		 * \brief Instruments every function (added to the object) with an instruction that encodes it's length.
		 * Also all functions will be aligned to BIT_ALIGNMENT addresses. Due to the addition of instructions and relocating of 
		 * functions (caused by alignment), the addresses of the basic blocks, jump targets and also function addresses (in calls) have to
		 * be adjusted.
		 * NOTE: The code size will be increased due to the instrumentation, therefore the linker script has to changed, sucht that
		 * no other section starts right after the code section.
		 */
		void instrumentFunctions(void);
		/**
		 * \brief Returns all cfgs that where instrumented.
		 * \returns All cfgs that where instrumented.
		 */
		vector<ControlFlowGraph> getInstrumentedFunctions(void);
		/**
		 * \brief Returns the meta data of all instrumented functions.
		 * \returns Returns the meta data of all instrumented functions (address, name, size).
		 * These informations are needed to create a function name table. See FunctionTableCreator.createFunctionTable(vector<addr_name_size_t> functionMetaData).
		 */
		vector<addr_name_size_t> getMetaDataOfInstrumentedFunctions(void);
		/**
		 * \brief Returns the unparsed lines, that may be altered by relocation of functions.
		 * The unparsed lines may contain unparsed code or the data section.
		 * \return The unparsed lines, that may be altered by relocation of functions.
		 */
		vector<string> getUnparsedLines(void) __attribute__ ((deprecated));

	private:
		/**
		 * \brief The uninstrumented functions as cfg with meta informations
		 */
		vector<function_graph_t> original_functions;
		/**
		 * \brief The instrumented functions in CFGManipulator objects.
		 */
		vector<CFGManipulator> instrumented_functions;
		/**
		 * \brief Lines from the dump file, that where not parsed by the CarCoreDumpParser. The unparsed lines may contain unparsed code or the data section.
		 */
		vector<string> unparsed_lines;
		/**
		 * \brief The start address of the first function (lowest address) handled by the object.  
		 */
		uint32_t code_start;
		/**
		 * \brief The end address of the last function (highest address) handled by the object.
		 */
		uint32_t code_end;

		/**
		 * \brief Pointer to object for parsing the lines of the dump files.
		 */
		DumpLineParser *dlp;
		/**
		 * \brief Pointer to the global configuration object.
		 */
		Configuration *conf;
		/**
		 * \brief Pointer to the LOGCXX logger object.
		 */
		static LoggerPtr logger;


};

#endif

