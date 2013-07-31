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
#include "mem_usage_stats.hpp"		


MemoryUsageStats::MemoryUsageStats()
{
	no_memory_states = 0;
	no_representation_states = 0;
	used_mem_size = 0;
	used_mem_references = 0;
}

MemoryUsageStats::~MemoryUsageStats()
{
	no_memory_states = 0;
	no_representation_states = 0;
	used_mem_size = 0;
	used_mem_references = 0;
}

uint64_t MemoryUsageStats::getMemoryStateCount(void)
{
	return no_memory_states;
}

uint64_t MemoryUsageStats::getRepresentationStateCount(void)
{
	return no_representation_states;
}

uint64_t MemoryUsageStats::getUsedSize(void)
{
	return used_mem_size;
}

uint64_t MemoryUsageStats::getUsedMemReferences(void)
{
	return used_mem_references;
}

void MemoryUsageStats::allocatedMemoryState(uint32_t number, uint64_t size_for_all_objects, uint64_t maintained_mem_references)
{
	no_memory_states++;
	no_representation_states += number;
	used_mem_size += size_for_all_objects;
	used_mem_references += maintained_mem_references;
}

