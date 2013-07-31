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
#ifndef _MEMORY_PARAMS_HPP_
#define _MEMORY_PARAMS_HPP_

#include "global.h"
#include "constants.h"
#include "configuration.hpp"
#include "arch_cfg_factory.hpp"
#include "memory_budget_calculator.hpp"

// TODO: Refactor parameters! Use ways for LRU/FIFO to prepare associative cache analysis.
struct cache_params_t {
	mem_type_t type;
	replacement_policy_t rpol;
	uint32_t size;

	/// Size of the cache lines of the modeled cache. The cache line size has to be a power of two.
	uint32_t line_size;
	/// Size of the cache line as bit address position.
	uint32_t line_size_bit;
	/// Maximum number of cache lines of the modeled cache.
	uint32_t line_no;
	uint32_t associativity;

	uint32_t sets;
	uint32_t ways;

	bool use_bbs_instead_of_cache_lines;

};

struct disp_params_t {
	mem_type_t type;
	/// The replacement policy used by the DISP.
	replacement_policy_t rpol;
	uint32_t size;

	uint32_t block_size;
	uint32_t block_no;

	/// The maximal number of functions that the DISP can maintain concurrently.
	uint32_t mapping_table_size;
	uint32_t lookup_width;
	uint32_t context_stack_depth;


	/// Determines if to large functions will be ignored or not.  If true functions that are larger than the scratchpad will not be loaded, else if such function should be maintained by the DISP an assertion is violated.
	bool ignore_outsized_functions;
};

struct sisp_params_t {
	mem_type_t type;
	uint32_t size;
	bool use_jump_penalties;
};


struct nomem_params_t {
	mem_type_t type;
};


/*!
 * \brief
 */
class MemoryParameters {

	public:
		MemoryParameters();
		virtual ~MemoryParameters();

		mem_type_t getMemType(void);
		string getMemTypeString(void);
		replacement_policy_t getReplacementPolicy(void);

		cache_params_t getCacheParameters(void);
		cache_params_t getCacheParameters(uint32_t size_to_override);

		disp_params_t getDispParameters(void);
		disp_params_t getDispParameters(uint32_t size_to_override);

		sisp_params_t getSispParameters(void);
		sisp_params_t getSispParameters(uint32_t size_to_override);

		nomem_params_t getNoMemParameters(void);

		uint32_t getUsableMemorySize(void);
		uint32_t getUsableMemorySize(uint32_t size_to_override);
	private: 
		bool isPowerOfTwo(uint32_t number);
		uint32_t log2(uint32_t number);


		/**
		 * \brief Pointer to the global configuration object.
		 */
		Configuration *conf;
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
