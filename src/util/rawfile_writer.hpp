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
#ifndef _RAWFILE_WRITER_HPP_
#define _RAWFILE_WRITER_HPP_

#include "global.h"
#include "constants.h"
#include "configuration.hpp"
#include "graph_structure.h"
#include "isa.hpp"
#include "dumpline_parser.hpp"

#include <boost/algorithm/string.hpp>


struct op_raw_t {
	uint32_t address;
	uint32_t size;
	string instr_dump;
	string instr_raw;
};

class RawFileWriter {
	public:
		RawFileWriter();
		virtual ~RawFileWriter();
		void addCfg(ControlFlowGraph cfg);
		void addCfgs(vector<ControlFlowGraph> cfgs);
		void addUnparsedDumpCode(vector<string> dump) __attribute__ ((deprecated));
		/**
		 * This method is not needed, because the data section is imported via the elf file.
		 * The generated raw file contains the code section only. After loading this section into the simulator,
		 * all other sections are imported from the original elf.
		 * TODO Write a merge tool that creates an ELF, that combines the raw code section with the rest of the original elf.
		 */
		void addDataSection(vector<string> data) __attribute__ ((deprecated));
		void generateRaw(void);
		void printCode(void);

	private:
		void openFiles(void);
		void closeFiles(void);
		void writeInstructionAsRaw(string opcode, uint32_t opcode_length);
		void writeDumpLine(string line);
		void bridgeMemoryHole(uint32_t start_addr, uint32_t end_addr);

		list<op_raw_t> code_section;
		vector<op_raw_t> data_section;


		DumpLineParser *dlp;

		ISA *isa;

		ofstream raw_file;
		ofstream dump_file;

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
