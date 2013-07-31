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
#include "bf_disp_state_maintainer.hpp"

uint32_t BFDISPStateMaintainer::getFunctionMemSize(uint32_t address)
{
	uint32_t f_size = functions->getFunctionSize(address);

	// no function can be found for this entry
	if(f_size == 0)
	{
		assert(false);
	}

	uint32_t f_size_in_mem = ((f_size/disp_parameters.block_size)*disp_parameters.block_size) + ((f_size % disp_parameters.block_size == 0)?(0):(disp_parameters.block_size));

	return f_size_in_mem;
}

bool BFDISPStateMaintainer::isInMust(uint32_t func_addr, MSGVertex node)
{
	return isInAllSets(func_addr, node);
}

bool BFDISPStateMaintainer::isInMay(uint32_t func_addr, MSGVertex node)
{
	return isInAnySet(func_addr, node);
}
