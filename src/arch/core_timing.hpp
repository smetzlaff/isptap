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
#ifndef _CORE_TIMING_HPP_
#define _CORE_TIMING_HPP_

#include "global.h"
#include "configuration.hpp"
#include "constants.h"

/**
 * The cycles it takes to execute a given basic block depends on how the block is left.
 * The jump penalty is charged depending on the edge.
 */
struct cycles_t {
	uint32_t forward_step;
	uint32_t jump;
};

enum memory_mode_t {
	ONCHIP=0,
	OFFCHIP
};

class CoreTiming {
	public:
		virtual ~CoreTiming() {};
		virtual cycles_t getCycleCountForInstructions(string code, bool precessing_bb_was_executed, uint32_t bb_start_addr, uint32_t bb_end_addr) = 0;
		virtual cycles_t getCycleCountForInstructionsWithBlockExitSensitivity(string code, bool precessing_bb_was_executed, uint32_t bb_start_addr, uint32_t bb_end_addr) = 0;
};

#endif
