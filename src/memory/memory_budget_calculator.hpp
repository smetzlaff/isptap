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
#ifndef _MEMORY_BUDGET_CALCULATOR_HPP_
#define _MEMORY_BUDGET_CALCULATOR_HPP_

#include "global.h"
#include "constants.h"
#include "configuration.hpp"
#include "arch_cfg_factory.hpp"

#define CACHE_VALID_BIT 1

struct cache_dimensions_t {
	uint32_t line_size;
	uint32_t line_no;
	uint32_t associativity;

	uint32_t sets;
	uint32_t ways;

	uint32_t address_width;
	uint32_t addr_tag_size;
	uint32_t addr_index_size;
	uint32_t addr_block_offset_size;

	uint32_t tag_memory_size;
	uint32_t cache_memory_size;
};

struct disp_dimensions_t {
	uint32_t block_size;
	uint32_t block_no;

	uint32_t address_width;
	uint32_t maximum_function_size;

	uint32_t context_stack_depth;
	uint32_t mapping_table_size;
	uint32_t lookup_width;

	uint32_t mapping_table_memory_size;
	uint32_t lookup_table_memory_size;
	uint32_t context_stack_memory_size;
	uint32_t disp_memory_size;
};

struct sisp_dimensions_t {
	uint32_t sisp_memory_size;
};

struct cache_addr_segments_t {
	uint32_t tag_size;
	uint32_t index_size;
	uint32_t block_offset_size;
};

struct cache_size_t {
	uint32_t overall_cache_lines;
	uint32_t tag_memory_size;
	uint32_t cache_memory_size;
};

struct disp_helper_memory_sizes_t {
	uint32_t mapping_table_memory_size;
	uint32_t lookup_table_memory_size;
	uint32_t context_stack_memory_size;
};


/*!
 * \brief
 */
class MemoryBudgetCalculator {
	public:
		MemoryBudgetCalculator(mem_type_t mtype, replacement_policy_t rpol, uint32_t budget);

		virtual ~MemoryBudgetCalculator();

		cache_dimensions_t calculateCacheParameters(void);

		disp_dimensions_t calculateDispParameters(void);

		sisp_dimensions_t calculateSispParameters(void);

	private:
		disp_helper_memory_sizes_t getHelperMemorySizes(uint32_t mapping_table_size, uint32_t maximum_function_size, uint32_t maximum_context_stack_depth, uint32_t address_width);
		cache_size_t calculateCacheSize(uint32_t line_size, uint32_t line_no_per_way, uint32_t ways, uint32_t address_width);
		cache_addr_segments_t chopCacheAddr(uint32_t line_size, uint32_t line_no, uint32_t associativity, uint32_t address_width);
		uint32_t getReplacementPolicyBitNumberForCache(uint32_t line_no, uint32_t ways);
		uint32_t getControlBitNumberForCache(uint32_t line_no, uint32_t ways);
		uint32_t getMaxLineNo(uint32_t line_size, uint32_t associativity, uint32_t address_width);
		bool isPowerOfTwo(uint32_t number);
		uint32_t log2(uint32_t number);
		uint32_t generateBitStringOfOnes(uint32_t bit_vector_length);
		uint32_t reduceToNextPowerOfTwo(uint32_t number);


		mem_type_t memory_type;
		replacement_policy_t replacement_policy;
		uint32_t memory_budget;


		/**
		 * \brief Pointer to the global architecutre configuration object.
		 */
		ArchConfig *arch_cfg;
		/**
		 * \brief Pointer to the LOGCXX logger object.
		 */
		static LoggerPtr logger;

};

#endif
