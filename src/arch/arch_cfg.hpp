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
#ifndef _ARCH_CONFIG_HPP_
#define _ARCH_CONFIG_HPP_

#include "global.h"
#include "configuration.hpp"
#include "isa.hpp"
#include "console_string_printer.hpp"
#include "help_message_types.hpp"


#define ARCH_LOAD_LATENCY_ONCHIP "load_latency_onchip"
#define ARCH_LOAD_LATENCY_OFFCHIP "load_latency_offchip"

#define ARCH_STORE_LATENCY_ONCHIP "store_latency_onchip"
#define ARCH_STORE_LATENCY_OFFCHIP "store_latency_offchip"

#define ARCH_FETCH_LATENCY_ONCHIP "fetch_latency_onchip"
#define ARCH_FETCH_LATENCY_OFFCHIP "fetch_latency_offchip"
#define ARCH_FETCH_BANDWIDTH "fetch_bandwidth" // in bit


#define ARCH_JUMP_PENALTY_CA "jump_penalty_for_continuous_addressing" //cycles penalty for adding a jump to connect a moved block (resp. to return from a moved block)
#define ARCH_JUMP_PENALTY_J4 "jump_penalty_for_jump_disp4" // cycles penalty for replacing a jump (with 4 bit displacement)  by a jump that allows to bridge the address space between the scratchpad and main memory
#define ARCH_JUMP_PENALTY_J8 "jump_penalty_for_jump_disp8" // cycles penalty for replacing a jump (with 8 bit displacement)  by a jump that allows to bridge the address space between the scratchpad and main memory
#define ARCH_JUMP_PENALTY_J15 "jump_penalty_for_jump_disp15" // cycles penalty for replacing a jump (with 15 bit displacement) by a jump that allows to bridge the address space between the scratchpad and main memory
#define ARCH_JUMP_PENALTY_J24 "jump_penalty_for_jump_disp24" // cycles penalty for replacing a jump (with 24 bit displacement) by a jump that allows to bridge the address space between the scratchpad and main memory
#define ARCH_JUMP_PENALTY_JI "jump_penalty_for_jump_ind" // cycles penalty for an indirect jump (full 32 bit are used for target addres). The penalty has to be 0 since every address in the processors address space is reachable.
#define ARCH_JUMP_PENALTY_C8 "jump_penalty_for_call_disp8" // cycles penalty for replacing a call (with 8 bit displacement) by a call that allows to bridge the address space between the scratchpad and main memory
#define ARCH_JUMP_PENALTY_C24 "jump_penalty_for_call_disp24" // cycles penalty for replacing a call (with 24 bit displacement) by a call that allows to bridge the address space between the scratchpad and main memory
#define ARCH_JUMP_PENALTY_CI "jump_penalty_for_call_ind" // cycles penalty for an indirect call (full 32 bit are used for target addres). The penalty has to be 0 since every address in the processors address space is reachable.


#define ARCH_SIZE_PENALTY_CA "size_penalty_for_continuous_addressing" //cycles penalty for adding a size to connect a moved block (resp. to return from a moved block)
#define ARCH_SIZE_PENALTY_J4 "size_penalty_for_jump_disp4" // cycles penalty for replacing a jump (with 4 bit displacement)  by a jump that allows to bridge the address space between the scratchpad and main memory
#define ARCH_SIZE_PENALTY_J8 "size_penalty_for_jump_disp8" // cycles penalty for replacing a jump (with 8 bit displacement)  by a jump that allows to bridge the address space between the scratchpad and main memory
#define ARCH_SIZE_PENALTY_J15 "size_penalty_for_jump_disp15" // cycles penalty for replacing a jump (with 15 bit displacement) by a jump that allows to bridge the address space between the scratchpad and main memory
#define ARCH_SIZE_PENALTY_J24 "size_penalty_for_jump_disp24" // cycles penalty for replacing a jump (with 24 bit displacement) by a jump that allows to bridge the address space between the scratchpad and main memory
#define ARCH_SIZE_PENALTY_JI "size_penalty_for_jump_ind" // cycles penalty for an indirect jump (full 32 bit are used for target addres). The penalty has to be 0 since every address in the processors address space is reachable.
#define ARCH_SIZE_PENALTY_C8 "size_penalty_for_call_disp8" // cycles penalty for replacing a call (with 8 bit displacement) by a call that allows to bridge the address space between the scratchpad and main memory
#define ARCH_SIZE_PENALTY_C24 "size_penalty_for_call_disp24" // cycles penalty for replacing a call (with 24 bit displacement) by a call that allows to bridge the address space between the scratchpad and main memory
#define ARCH_SIZE_PENALTY_CI "size_penalty_for_call_ind" // cycles penalty for an indirect call (full 32 bit are used for target addres). The penalty has to be 0 since every address in the processors address space is reachable.

#define ARCH_ADDRESS_WIDTH "address_width" // in bit

#define ARCH_DISP_BLOCK_SIZE "disp_block_size" // in bytes
#define ARCH_DISP_BLOCK_LOAD_LATENCY "disp_block_load_latency"
#define ARCH_DISP_CONTROLLER_HIT_CYLCES "disp_controller_hit_cycles"
#define ARCH_DISP_CONTROLLER_MISS_CYCLES "disp_controller_miss_cycles"
#define ARCH_DISP_MAX_FUNCTION_SIZE "disp_max_function_size"
#define ARCH_DISP_MAPPING_TABLE_SIZE "disp_mapping_table_size"
#define ARCH_DISP_LOOKUP_WIDTH "disp_lookup_width"
#define ARCH_DISP_CONTEXT_STACK_DEPTH "disp_context_stack_depth"

#define ARCH_ICACHE_LINE_SIZE "icache_line_size" // in bytes
#define ARCH_ICACHE_ASSOCIATIVITY "icache_associativity"
#define ARCH_ICACHE_MISS_LATENCY "icache_miss_latency"

/**
 * \brief The three different scenarios how 2 basic blocks could be connected.
 * Two basic blocks are either connected via: continuous addressing, an (un)conditional jump or by call.
 */
enum connection_type_t { ContinuousAdressing=0, Jump, Call, Return, UnknownConnectionType };

typedef map<const string, uint32_t> latency_map_t;

class ArchConfig 
{
	public:
		uint32_t getLoadLatency(int32_t mem_width, bool onchip);
		uint32_t getStoreLatency(int32_t mem_width, bool onchip);
		uint32_t getFetchLatency(bool onchip);
		uint32_t getMissLatency(mem_type_t mem);
		uint32_t getMemElementSize(mem_type_t mem);
		uint32_t getDISPCtrlHitCycles(void);
		uint32_t getDISPCtrlMissCycles(void);
		uint32_t getJumpPenalty(connection_type_t type, displacement_type_t displacement);
		uint32_t getSizePenalty(connection_type_t type, displacement_type_t displacement);
		uint32_t getAddressWidth(void);
		uint32_t getCacheLineSize(void);
		uint32_t getCacheAssociativity(void);
		uint32_t getDISPBlockSize(void);
		uint32_t getDISPMaxFunctionSize(void);
		uint32_t getDISPContextStackDepth(void);
		uint32_t getDISPMaxFunctionNumber(void);
		uint32_t getDISPMappingTableSize(void);
		uint32_t getDISPLookupWidth(void);
		uint32_t getFetchBandwidth(void);

		virtual uint32_t getCallLatency(bool onchip) = 0;
		virtual uint32_t getReturnLatency(bool onchip) = 0;
		virtual bool isFetchMemIndependent(void) = 0;

		static string getHelpMessageHeader(void);
		static string getBasicHelpMessage(void);
		virtual string getHelpMessage(void)=0;
	protected:
		ArchConfig(bool initialise_help_messages);
		virtual ~ArchConfig(void);
		void readArchConfigFile(string config_file);
		void storeArchParameterEntry(string line);
		uint32_t getArchParameter(string parameter);

		latency_map_t arch_parameters_map;
		string arch_prefix;

		vector<help_message_t> config_properties;

		/**
		 * \brief Pointer to the global configuration object.
		 */
		Configuration *conf;

	private:
		/**
		 * \brief Pointer to the LOGCXX logger object.
		 */
		static LoggerPtr logger;
};
#endif
