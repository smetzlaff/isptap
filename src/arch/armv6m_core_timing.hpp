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
#ifndef _ARMV6M_CORE_TIMING_HPP_
#define _ARMV6M_CORE_TIMING_HPP_

#include "core_timing.hpp"
#include "armv6m_isa.hpp"
#include "armv6m_dumpline_parser.hpp"
#include "armv6m_cfg.hpp"

class Armv6mCoreTiming : public CoreTiming {
	public:
		Armv6mCoreTiming(memory_mode_t mem_type);
		virtual ~Armv6mCoreTiming();
		cycles_t getCycleCountForInstructions(string code, bool precessing_bb_was_executed, uint32_t bb_start_addr, uint32_t bb_end_addr);
		cycles_t getCycleCountForInstructionsWithBlockExitSensitivity(string code, bool precessing_bb_was_executed, uint32_t bb_start_addr, uint32_t bb_end_addr);

	private:
		memory_mode_t memory_type;

		/**
		 * \brief Calculates the instruction fetch latency for a given instruction and fetch buffer content.
		 * \param address Address of the instruction to fetch.
		 * \param length Length of the instruction to fetch.
		 * \param buffered_bytes The fill state of the instruction buffer.
		 * \returns The instruction fetch latency for a given instruction and fetch buffer content.
		 */
		uint32_t getInstructionsFetchLatency(uint32_t address, uint32_t length, uint32_t *buffered_bytes);

		/**
		 * \brief Returns the number of bytes that the fetch buffer contains on entering a basic block by continuous addressing.
		 * \param address The start address of the basic block for which the initial fetch buffer state is determined.
		 * \returns The number of bytes the fetch buffer initially contains on entering the basic block by continuous addressing.
		 */
		uint32_t getInitialFetchBufferState(uint32_t address);

		/**
		 * \brief Pointer to object for parsing the lines of the dump files.
		 */
		Armv6mDumpLineParser *dlp;
		/**
		 * \brief Pointer to object for decoding and encoding instructions for the ARMv6M ISA.
		 */
		Armv6mISA *arch;
		/**
		 * \brief Pointer to the global architectural configuration object.
		 */
		Armv6mConfig *armcfg;

		/**
		 * \brief Pointer to the LOGCXX logger object.
		 */
		static LoggerPtr logger;
};

#endif
