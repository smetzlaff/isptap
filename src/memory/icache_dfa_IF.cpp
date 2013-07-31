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
#include "icache_dfa_IF.hpp"

ICacheDataFlowAnalyzer_IF::~ICacheDataFlowAnalyzer_IF()
{
	arch_cfg = NULL;
}

CFGMSGPair ICacheDataFlowAnalyzer_IF::getGraph(void)
{
	return graph;
}

CFGMSGVPair ICacheDataFlowAnalyzer_IF::getEntry(void)
{
	return entry;
}

CFGMSGVPair ICacheDataFlowAnalyzer_IF::getExit(void)
{
	return exit;
}
