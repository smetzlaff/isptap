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
#ifndef _CARCORE_TIMING_HPP_
#define _CARCORE_TIMING_HPP_

#include "core_timing.hpp"
#include "tricore_isa.hpp"
#include "tricore_dumpline_parser.hpp"
#include "carcore_cfg.hpp"
#include "tricore_instrset.h"

#include <string>
#include <map>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

enum pipeline_t {
	ADDRESS_PIPELINE=0, // for address and memory instructions
	DATA_PIPELINE,   // for data instructions
	UNKNOWN_PIPELINE // for initialization and if the pipelines are empty, because of fetch latency
};

struct instr_windows_t {
	uint32_t address;
	uint32_t valid_at;
};


typedef map<uint32_t, vector<instr_windows_t>* > BBAddrInitialIWMap;

class CarCoreTiming : public CoreTiming {
	public:
		CarCoreTiming(memory_mode_t fetchmode);
		virtual ~CarCoreTiming();
		cycles_t getCycleCountForInstructions(string code, bool precessing_bb_was_executed, uint32_t bb_start_addr, uint32_t bb_end_addr);
		cycles_t getCycleCountForInstructionsWithBlockExitSensitivity(string code, bool precessing_bb_was_executed, uint32_t bb_start_addr, uint32_t bb_end_addr);
	private:

		// using int as datatype to prevent including the carcore stuff into a header file and this into the whole project
		pipeline_t getPipeline(int function);

		uint32_t getInstructionValidTime(uint32_t instruction_addr, uint32_t instruction_length, vector<instr_windows_t> *iw);
		void fillIW(uint32_t start_addr, bool jump_in_bb, vector<instr_windows_t> *iw);
		void updateIWOnExecution(uint32_t instruction_addr, uint32_t instruction_length, vector<instr_windows_t> *iw) __attribute__ ((deprecated));
		void updateIWOnExecution(uint32_t instruction_addr, uint32_t instruction_length, vector<instr_windows_t> *iw, uint32_t fetch_rq_cycle);

		void storeIWForSubsequentBB(uint32_t next_bb_addr, vector<instr_windows_t> *iw, uint32_t base_cycle);
		vector<instr_windows_t> getInitialIWForBB(uint32_t bb_addr);

		void printIW(vector<instr_windows_t> *iw);

		uint32_t fetch_latency;

		/**
		 * \brief Pointer to object for parsing the lines of the dump files.
		 */
		TriCoreDumpLineParser *dlp;
		/**
		 * \brief Pointer to object for decoding and encoding instructions for the tricore ISA.
		 */
		TricoreISA *arch;
		/**
		 * \brief Pointer to the global architectural configuration object.
		 */
		CarCoreConfig *cccfg;
		/**
		 * \brief Pointer to the global Configuration object.
		 */
		Configuration *conf;
		/**
		 * \brief Pointer to the LOGCXX logger object.
		 */
		static LoggerPtr logger;

		/// Defines if the car core timing should consider on or off-chip fetches
		memory_mode_t fetch_mode;

		BBAddrInitialIWMap initial_iw_map;
};

#endif
