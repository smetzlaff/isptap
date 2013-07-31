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
#include "memory_budget_calculator.hpp"


LoggerPtr MemoryBudgetCalculator::logger(Logger::getLogger("MemoryBudgetCalculator"));

MemoryBudgetCalculator::MemoryBudgetCalculator(mem_type_t mtype, replacement_policy_t rpol, uint32_t budget) : memory_type(mtype), replacement_policy(rpol), memory_budget(budget)
{
	arch_cfg = ArchConfigFactory::getInstance()->getArchConfigObject();
}

MemoryBudgetCalculator::~MemoryBudgetCalculator()
{
}

cache_dimensions_t MemoryBudgetCalculator::calculateCacheParameters(void)
{
	cache_dimensions_t result;
	assert(memory_type == ICACHE);

	result.line_size = arch_cfg->getCacheLineSize();
	result.associativity = arch_cfg->getCacheAssociativity();
	result.address_width = arch_cfg->getAddressWidth();

	result.line_no = getMaxLineNo(result.line_size, result.associativity, result.address_width);
	if(result.associativity != 0)
	{
		result.ways = result.associativity;
		result.sets = result.line_no / result.associativity;
		// TODO: Add checks.
	}
	else
	{
		result.ways = result.line_no;
		result.sets = 1;
	}

	cache_addr_segments_t c_segs = chopCacheAddr(result.line_size, result.line_no, result.ways, result.address_width);

	result.addr_tag_size = c_segs.tag_size;
	result.addr_index_size = c_segs.index_size;
	result.addr_block_offset_size = c_segs.block_offset_size;

	cache_size_t c_size = calculateCacheSize(result.line_size, result.line_no/result.ways, result.ways, result.address_width);

	result.tag_memory_size = c_size.tag_memory_size;
	result.cache_memory_size = c_size.cache_memory_size;


	LOG_DEBUG(logger, "Calculated: tag: " << result.tag_memory_size << " bytes and cache: " << result.cache_memory_size << " bytes that fits: " << memory_budget << " bytes");
	
	assert(result.tag_memory_size + result.cache_memory_size <= memory_budget);

	return result;
}

uint32_t MemoryBudgetCalculator::getReplacementPolicyBitNumberForCache(uint32_t line_no, uint32_t ways)
{
	uint32_t replacement_policy_bits;

	/*
	 * For the source of the rpol bit numbers refer e.g. to 
	 * Hussein Al-Zoubi, Aleksandar Milenkovic, and Milena Milenkovic. 2004. 
	 * Performance evaluation of cache replacement policies for the SPEC CPU2000 benchmark suite. 
	 * In Proceedings of the 42nd annual Southeast regional conference (ACM-SE 42). ACM, New York, NY, USA, 267-272. 
	 * DOI=10.1145/986537.986601 http://doi.acm.org/10.1145/986537.986601 
	 */
	switch(replacement_policy)
	{
		case DIRECT_MAPPED:
			{
				replacement_policy_bits = 0;
				break;
			}
		case LRU:
			{
				replacement_policy_bits = log2(ways) * line_no * ways;
				break;
			}
		case FIFO:
			{
				replacement_policy_bits = log2(ways) * line_no;
				break;
			}
		default:
			{
				replacement_policy_bits=0;
				assert(false);
			}
	}

	return replacement_policy_bits;	
}

uint32_t MemoryBudgetCalculator::getControlBitNumberForCache(uint32_t line_no, uint32_t ways)
{
	uint32_t control_bits=0;

	control_bits = CACHE_VALID_BIT * line_no * ways;

	return control_bits;
}

cache_addr_segments_t MemoryBudgetCalculator::chopCacheAddr(uint32_t line_size, uint32_t line_no, uint32_t ways, uint32_t address_width)
{
	cache_addr_segments_t result;

	result.block_offset_size = log2(line_size);
	result.index_size = log2(line_size * line_no * ways) - result.block_offset_size - log2(ways);
	result.tag_size = address_width - result.index_size - result.block_offset_size;

	return result;
}

uint32_t MemoryBudgetCalculator::getMaxLineNo(uint32_t line_size, uint32_t associativity, uint32_t address_width)
{
	uint32_t line_no;
	uint32_t ways;
	cache_size_t c_size;

	// direct mapped or set associative case
	if(associativity != 0)
	{
		ways = associativity;
		line_no = 1;
		c_size = calculateCacheSize(line_size, line_no, ways, address_width);

		while(c_size.tag_memory_size + c_size.cache_memory_size <= memory_budget)
		{
			line_no <<= 1;
			// direct mapped or set associative case
			c_size = calculateCacheSize(line_size, line_no, ways, address_width);
		}

		// determine the last valid configuration that fits the budget
		line_no >>= 1;
		c_size = calculateCacheSize(line_size, line_no, ways, address_width);

		assert(line_no > 0);

		if(!isPowerOfTwo(line_no))
		{
			assert(false);
		}
	}
	else 
	{
		// fully associative case
		line_no = 1;
		ways = 1;
		c_size = calculateCacheSize(line_size, line_no, ways, address_width);

		while(c_size.tag_memory_size + c_size.cache_memory_size <= memory_budget)
		{
			ways <<= 1;
			c_size = calculateCacheSize(line_size, line_no, ways, address_width);
		}

		// determine the last valid configuration that fits the budget
		ways >>= 1;
		c_size = calculateCacheSize(line_size, line_no, ways, address_width);

		assert(ways > 0);

		if(!isPowerOfTwo(ways))
		{
			assert(false);
		}
	}

		return c_size.overall_cache_lines;
}

cache_size_t MemoryBudgetCalculator::calculateCacheSize(uint32_t line_size, uint32_t line_no_per_way, uint32_t ways, uint32_t address_width)
{
	cache_size_t result;
	cache_addr_segments_t c_segs = chopCacheAddr(line_size, line_no_per_way, ways, address_width);
	result.overall_cache_lines = line_no_per_way * ways;

	LOG_DEBUG(logger, "Using " << c_segs.tag_size << " bits for TAG, " << getControlBitNumberForCache(line_no_per_way, ways)/result.overall_cache_lines << " bits for control, and " << getReplacementPolicyBitNumberForCache(line_no_per_way, ways)/result.overall_cache_lines << " bits for replacement per cache line.");

	// NOTICE: every tag entry (including control and replacement policy bits) may consume odd number of bits (and not a whole number of bytes) 
#ifndef WHOLE_BYTES_PER_LINE
	uint32_t tag_bits = c_segs.tag_size * result.overall_cache_lines + getControlBitNumberForCache(line_no_per_way, ways) + getReplacementPolicyBitNumberForCache(line_no_per_way, ways);
	uint32_t tag_bytes = (tag_bits / 8) + (((tag_bits % 8) == 0)?(0):(1));
#endif
	result.tag_memory_size = tag_bytes;;
	result.cache_memory_size = line_size * result.overall_cache_lines;
	return result;
}


disp_dimensions_t MemoryBudgetCalculator::calculateDispParameters(void)
{
	disp_dimensions_t result;
	assert(memory_type == DISP);


	result.block_size = arch_cfg->getDISPBlockSize();

	result.address_width = arch_cfg->getAddressWidth();
	result.maximum_function_size = arch_cfg->getDISPMaxFunctionSize();
	result.context_stack_depth = arch_cfg->getDISPContextStackDepth();
	result.mapping_table_size = arch_cfg->getDISPMappingTableSize();
	result.lookup_width = arch_cfg->getDISPLookupWidth();

	disp_helper_memory_sizes_t helper_memories = getHelperMemorySizes(result.mapping_table_size, result.maximum_function_size, result.context_stack_depth, result.address_width);

	result.mapping_table_memory_size = helper_memories.mapping_table_memory_size;
	result.lookup_table_memory_size = helper_memories.lookup_table_memory_size;
	result.context_stack_memory_size = helper_memories.context_stack_memory_size;

	LOG_DEBUG(logger, "Helper memory sizes: MTM: " << helper_memories.mapping_table_memory_size << " LTM: " << helper_memories.lookup_table_memory_size << " CSM: " << helper_memories.context_stack_memory_size << " SUM: " << (helper_memories.mapping_table_memory_size + helper_memories.lookup_table_memory_size + helper_memories.context_stack_memory_size));

	if(result.mapping_table_memory_size + result.lookup_table_memory_size + result.context_stack_memory_size > memory_budget)
	{
		result.block_no = 0;
		result.disp_memory_size = 0;
	}
	else
	{
		result.block_no = (memory_budget - result.mapping_table_memory_size - result.lookup_table_memory_size - result.context_stack_memory_size) / result.block_size;
		result.disp_memory_size = result.block_size * result.block_no;
	}

	return result;
}

disp_helper_memory_sizes_t MemoryBudgetCalculator::getHelperMemorySizes(uint32_t mapping_table_size, uint32_t maximum_function_size, uint32_t context_stack_depth, uint32_t address_width)
{
	disp_helper_memory_sizes_t result;

	uint32_t disp_address_width = log2(maximum_function_size);

	result.mapping_table_memory_size = (address_width + disp_address_width + log2(maximum_function_size)) * mapping_table_size;
	result.lookup_table_memory_size = address_width * mapping_table_size;
	result.context_stack_memory_size = address_width * context_stack_depth;

	return result;
}

sisp_dimensions_t MemoryBudgetCalculator::calculateSispParameters(void)
{
	sisp_dimensions_t result;
	assert((memory_type == BBSISP) || (memory_type == BBSISP_JP) || (memory_type == BBSISP_WCP) || (memory_type == BBSISP_JP_WCP) || (memory_type == FSISP) || (memory_type == FSISP_WCP) || (memory_type == FSISP_OLD));

	result.sisp_memory_size = memory_budget;

	assert(isPowerOfTwo(result.sisp_memory_size));

	return result;
}

bool MemoryBudgetCalculator::isPowerOfTwo(uint32_t number)
{
	assert(number > 0);

	// shift all lower zeros ('0b0')) until a '0b1' appears in the bit string
	while((number & 1) == 0)
	{
		number >>= 1;
	}

//	LOG_DEBUG(logger, "Number: " << number << " is " << ((number==1)?(""):("not ")) << "power of two.");

	// if the '0b1' is the only '0b1' in the original number the number is a power of two
	return(number == 1);
}

uint32_t MemoryBudgetCalculator::log2(uint32_t number)
{
	assert((number > 0) && (isPowerOfTwo(number)));
	uint32_t log2_value=0;


	// count all lower zeros ('0b0')) until a '0b1' appears in the bit string
	while((number & 1) == 0)
	{
		number >>= 1;
		log2_value++;
	}

	return(log2_value);
}

uint32_t MemoryBudgetCalculator::generateBitStringOfOnes(uint32_t bit_vector_length)
{
	uint32_t bit_string=0;
	for(uint32_t i = 0; i<bit_vector_length; i++)
	{
		bit_string = (bit_string << 1) | 1;
	}
	return bit_string;

}

uint32_t MemoryBudgetCalculator::reduceToNextPowerOfTwo(uint32_t number)
{
	uint32_t highest_bit=0;

	// find the highest 1-bit
	for(int i = 0; i < 32; i++)
	{
		if((number & 1) == 1)
		{
			highest_bit = i;
		}
		number >>= 1;
	}

	// return a number in which only the highest 1-bit is set.
	return (1<<highest_bit);
}
