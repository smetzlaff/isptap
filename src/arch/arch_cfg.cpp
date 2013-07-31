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
#include "arch_cfg.hpp"

LoggerPtr ArchConfig::logger(Logger::getLogger("ArchConfig"));

#define HELP_ARCH_LOAD_LATENCY_ONCHIP "The latency of loads for on-chip memories (in cycles)."
#define HELP_ARCH_LOAD_LATENCY_OFFCHIP "The latency of loads for off-chip memories (in cycles)."
#define HELP_ARCH_STORE_LATENCY_ONCHIP "The latency of stores for on-chip memories (in cycles)."
#define HELP_ARCH_STORE_LATENCY_OFFCHIP "The latency of stores for off-chip memories (in cycles)."
#define HELP_ARCH_FETCH_LATENCY_ONCHIP "The latency of fetches for on-chip memories (in cycles)."
#define HELP_ARCH_FETCH_LATENCY_OFFCHIP "The latency of fetches for off-chip memories (in cycles)."
#define HELP_ARCH_FETCH_BANDWIDTH "The fetch bandwidth of the instruction memory (in bit)." // in bit
#define HELP_ARCH_JUMP_PENALTY_CA "The CA jump penalty for the BBSISP assignment algorithm (BBSISP_JP and BBSISP_JP_WCP). The penalty (in cycles) is used in the case that two basic blocks that were connected by continuous addressing need to be reconnected when one of the basic blocks is to be moved to the static scratchpad." 
#define HELP_ARCH_JUMP_PENALTY_J4 "The J4 jump penalty for the BBSISP assignment algorithm (BBSISP_JP and BBSISP_JP_WCP). The penalty (in cycles) is used in the case that two basic blocks that were connected by a short jump (disp4) need to be reconnected when one of the basic blocks is to be moved to the static scratchpad." 
#define HELP_ARCH_JUMP_PENALTY_J8 "The J8 jump penalty for the BBSISP assignment algorithm (BBSISP_JP and BBSISP_JP_WCP). The penalty (in cycles) is used in the case that two basic blocks that were connected by a short jump (disp8 [Carcore/ARMv6M] or disp11 [ARMv6M]) need to be reconnected when one of the basic blocks is to be moved to the static scratchpad." 
#define HELP_ARCH_JUMP_PENALTY_J15 "The J15 jump penalty for the BBSISP assignment algorithm (BBSISP_JP and BBSISP_JP_WCP). The penalty (in cycles) is used in the case that two basic blocks that were connected by a jump (disp15) need to be reconnected when one of the basic blocks is to be moved to the static scratchpad." 
#define HELP_ARCH_JUMP_PENALTY_J24 "The J24 jump penalty for the BBSISP assignment algorithm (BBSISP_JP and BBSISP_JP_WCP). The penalty (in cycles) is used in the case that two basic blocks that were connected by a jump (disp24) need to be reconnected when one of the basic blocks is to be moved to the static scratchpad." 
#define HELP_ARCH_JUMP_PENALTY_JI "The JI jump penalty for the BBSISP assignment algorithm (BBSISP_JP and BBSISP_JP_WCP). The penalty (in cycles) is used in the case that two basic blocks that were connected by an indirect jump need to be reconnected when one of the basic blocks is to be moved to the static scratchpad." 
#define HELP_ARCH_JUMP_PENALTY_C8 "The C8 jump penalty for the BBSISP assignment algorithm (BBSISP_JP and BBSISP_JP_WCP). The penalty (in cycles) is used in the case that two basic blocks that were connected by a short call (disp8) need to be reconnected when one of the basic blocks is to be moved to the static scratchpad." 
#define HELP_ARCH_JUMP_PENALTY_C24 "The C24 jump penalty for the BBSISP assignment algorithm (BBSISP_JP and BBSISP_JP_WCP). The penalty (in cycles) is used in the case that two basic blocks that were connected by a call (disp24) need to be reconnected when one of the basic blocks is to be moved to the static scratchpad." 
#define HELP_ARCH_JUMP_PENALTY_CI "The CI jump penalty for the BBSISP assignment algorithm (BBSISP_JP and BBSISP_JP_WCP). The penalty (in cycles) is used in the case that two basic blocks that were connected by an indirect call need to be reconnected when one of the basic blocks is to be moved to the static scratchpad." 
#define HELP_ARCH_SIZE_PENALTY_CA "The CA basic block size penalty for the BBSISP assignment algorithm (BBSISP_JP and BBSISP_JP_WCP). The penalty (in bytes) is used in the case that two basic blocks that were connected by continuous addressing need to be reconnected when one of the basic blocks is to be moved to the static scratchpad."
#define HELP_ARCH_SIZE_PENALTY_J4 "The J4 basic block size penalty for the BBSISP assignment algorithm (BBSISP_JP and BBSISP_JP_WCP). The penalty (in bytes) is used in the case that two basic blocks that were connected by a short jump (disp4) need to be reconnected when one of the basic blocks is to be moved to the static scratchpad." 
#define HELP_ARCH_SIZE_PENALTY_J8 "The J8 basic block size penalty for the BBSISP assignment algorithm (BBSISP_JP and BBSISP_JP_WCP). The penalty (in bytes) is used in the case that two basic blocks that were connected by a short jump (disp8 [Carcore/ARMv6M] or disp11 [ARMv6M]) need to be reconnected when one of the basic blocks is to be moved to the static scratchpad." 
#define HELP_ARCH_SIZE_PENALTY_J15 "The J15 basic block size penalty for the BBSISP assignment algorithm (BBSISP_JP and BBSISP_JP_WCP). The penalty (in bytes) is used in the case that two basic blocks that were connected by a jump (disp15) need to be reconnected when one of the basic blocks is to be moved to the static scratchpad."
#define HELP_ARCH_SIZE_PENALTY_J24 "The J24 basic block size penalty for the BBSISP assignment algorithm (BBSISP_JP and BBSISP_JP_WCP). The penalty (in bytes) is used in the case that two basic blocks that were connected by a jump (disp24) need to be reconnected when one of the basic blocks is to be moved to the static scratchpad."
#define HELP_ARCH_SIZE_PENALTY_JI "The JI basic block size penalty for the BBSISP assignment algorithm (BBSISP_JP and BBSISP_JP_WCP). The penalty (in bytes) is used in the case that two basic blocks that were connected by an indirect jump need to be reconnected when one of the basic blocks is to be moved to the static scratchpad."
#define HELP_ARCH_SIZE_PENALTY_C8 "The C8 basic block size penalty for the BBSISP assignment algorithm (BBSISP_JP and BBSISP_JP_WCP). The penalty (in bytes) is used in the case that two basic blocks that were connected by a short call (disp8) need to be reconnected when one of the basic blocks is to be moved to the static scratchpad."
#define HELP_ARCH_SIZE_PENALTY_C24 "The C24 basic block size penalty for the BBSISP assignment algorithm (BBSISP_JP and BBSISP_JP_WCP). The penalty (in bytes) is used in the case that two basic blocks that were connected by a call (disp24) need to be reconnected when one of the basic blocks is to be moved to the static scratchpad."
#define HELP_ARCH_SIZE_PENALTY_CI "The CI basic block size penalty for the BBSISP assignment algorithm (BBSISP_JP and BBSISP_JP_WCP). The penalty (in bytes) is used in the case that two basic blocks that were connected by an indirect call need to be reconnected when one of the basic blocks is to be moved to the static scratchpad."
#define HELP_ARCH_ADDRESS_WIDTH "The address width the processor (in bit)."
#define HELP_ARCH_DISP_BLOCK_SIZE "The size of the memory blocks in the DISP (in bytes)."
#define HELP_ARCH_DISP_BLOCK_LOAD_LATENCY "The latency for loading one memory block into the DISP (in cycles)."
#define HELP_ARCH_DISP_CONTROLLER_HIT_CYLCES "The number of cycles the DISP controller needs for function hit handling."
#define HELP_ARCH_DISP_CONTROLLER_MISS_CYCLES "The number of cycles the DISP controller needs for function miss handling."
#define HELP_ARCH_DISP_MAX_FUNCTION_SIZE "The maximum size of a function in the DISP (in bytes)."
#define HELP_ARCH_DISP_MAPPING_TABLE_SIZE "The maximum number of entries of the DISP's mapping table."
#define HELP_ARCH_DISP_LOOKUP_WIDTH "The number of mapping table entries the DISP controller looks up in one cycle (i.e. the associativity of the mapping table)"
#define HELP_ARCH_DISP_CONTEXT_STACK_DEPTH "The maximum allowed context stack depth (or maximum call depth) that defines the size of the DISP's context stack memory."
#define HELP_ARCH_ICACHE_LINE_SIZE "The line size of the instruction cache (in bytes). (This parameter will be ignored, if \""CONF_MEMORY_CACHE_BBS"\" set.)"
#define HELP_ARCH_ICACHE_ASSOCIATIVITY "The associativity of the cache. Use '1' for direct-mapped and '0' for fully-associative."
#define HELP_ARCH_ICACHE_MISS_LATENCY "The latency of a cache miss (in cycles)."

ArchConfig::ArchConfig(bool initialise_help_messages)
{
	conf = Configuration::getInstance();

	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_LOAD_LATENCY_ONCHIP, 0, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_LOAD_LATENCY_OFFCHIP, 4, initialise_help_messages);

	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_STORE_LATENCY_ONCHIP, 0, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_STORE_LATENCY_OFFCHIP, 3, initialise_help_messages);
	
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_FETCH_LATENCY_ONCHIP, 0, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_FETCH_LATENCY_OFFCHIP, 3, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_FETCH_BANDWIDTH, 64, initialise_help_messages);

	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_JUMP_PENALTY_CA, 5, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_JUMP_PENALTY_J4, 0, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_JUMP_PENALTY_J8, 0, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_JUMP_PENALTY_J15, 0, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_JUMP_PENALTY_J24, 0, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_JUMP_PENALTY_JI, 0, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_JUMP_PENALTY_C8, 0, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_JUMP_PENALTY_C24, 0, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_JUMP_PENALTY_CI, 0, initialise_help_messages);

	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_SIZE_PENALTY_CA, 4, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_SIZE_PENALTY_J4, 2, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_SIZE_PENALTY_J8, 2, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_SIZE_PENALTY_J15, 0, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_SIZE_PENALTY_J24, 0, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_SIZE_PENALTY_JI, 0, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_SIZE_PENALTY_C8, 2, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_SIZE_PENALTY_C24, 0, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_SIZE_PENALTY_CI, 0, initialise_help_messages);

	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_ADDRESS_WIDTH, 32, initialise_help_messages);

	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_DISP_BLOCK_SIZE,8, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_DISP_BLOCK_LOAD_LATENCY,0, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_DISP_CONTROLLER_HIT_CYLCES,4, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_DISP_CONTROLLER_MISS_CYCLES,4, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_DISP_MAX_FUNCTION_SIZE, 0x400000, initialise_help_messages); // == 2^16;
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_DISP_MAPPING_TABLE_SIZE, 256, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_DISP_LOOKUP_WIDTH, 256, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_DISP_CONTEXT_STACK_DEPTH, 16, initialise_help_messages);

	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_ICACHE_LINE_SIZE,32, initialise_help_messages);
	// the associativity depends on the replacement policy
	if(((mem_type_t)conf->getUint(CONF_MEMORY_TYPE) == ICACHE) && ((replacement_policy_t)conf->getUint(CONF_MEMORY_REPLACEMENT_POLICY))==DIRECT_MAPPED)
	{
		// a direct mapped cache has the associativity of 1
		arch_parameters_map.insert(pair<string, uint32_t>(ARCH_ICACHE_ASSOCIATIVITY, 1)); 
	}
	else
	{
		// non direct-mapped caches are fully associative by default
		arch_parameters_map.insert(pair<string, uint32_t>(ARCH_ICACHE_ASSOCIATIVITY, 0)); 
	}
	ADD_HELP_MESSAGE(ARCH_ICACHE_ASSOCIATIVITY, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARCH_ICACHE_MISS_LATENCY,0, initialise_help_messages);

}

ArchConfig::~ArchConfig(void)
{
	arch_parameters_map.clear();
	config_properties.clear();
}

void ArchConfig::readArchConfigFile(string config_file)
{
	ifstream config_file_s;
	LOG_DEBUG(logger, "Opening architecture parameter file: " << config_file << " for arch: " << arch_prefix);
	config_file_s.open(config_file.c_str());

	assert(config_file_s.is_open());

	while(!config_file_s.eof())
	{
		string str;
		getline(config_file_s, str);
		storeArchParameterEntry(str);
	}

	config_file_s.close();
}

void ArchConfig::storeArchParameterEntry(string line)
{
	stringstream s_assignment;
	s_assignment << "^([[:space:]]*)" << arch_prefix << "\\.[[:alnum:]]([[:alnum:]]|\\_|/|\\.)+[[:space:]]*=[[:space:]]*([[:alnum:]]|\\_|/|\\.)*";
	regex re_assignment((s_assignment.str()), regex::icase);

//	LOG_DEBUG(logger, "Inspecting line: " << line) 
	if(regex_search(line, re_assignment))
	{
		stringstream prefix_regex;
		prefix_regex << arch_prefix + "\\.";
		string line_wo_prefix = regex_replace(line, regex(prefix_regex.str(), regex::icase), "", match_default | format_first_only);
		string line_no_spaces = regex_replace(line_wo_prefix, regex("[[:space:]]"), "", match_default | format_all);
		string line_no_comment = regex_replace(line_no_spaces, regex("(#|//)[[:print:]]*"), "", match_default | format_all);

		uint32_t eq_char = line_no_comment.find("=");

		string property = line_no_comment.substr(0,eq_char);
		string value_s = line_no_comment.substr(eq_char+1, line_no_comment.length());
		uint32_t value = strtoul((value_s).c_str(), NULL,10);

		LOG_DEBUG(logger, "Line wo comments is: |" << line_no_comment << "| property detected: |" << property << "| value: |" << value << "|");

		latency_map_t::iterator pos;

		pos = arch_parameters_map.find(property);

		if(pos != arch_parameters_map.end())
		{
			// overwrite the value for this property
			pos->second = value;
		}
		else
		{
			LOG_WARN(logger, "Property: " << property << " is not valid and thus was not initialized!!");
			arch_parameters_map.insert(pair<string, uint32_t>(property, value));
		}

	}
	else
	{
		if(!regex_search(line, regex("^([[:space:]]*)(#|//)*")))
		{
		LOG_ERROR(logger, "Wrong line format for config property assignment: " << line);
		}
		else
		{
			// ignoring blank lines and comments
		}
	}
}

uint32_t ArchConfig::getArchParameter(string parameter)
{
	latency_map_t::iterator pos;
	uint32_t value = numeric_limits<uint32_t>::max();

	pos = arch_parameters_map.find(parameter);

	if(pos != arch_parameters_map.end())
	{
		value = pos->second;
	}
	else
	{
		LOG_WARN(logger, "Parameter: " << parameter << " was not found!");
	}

	return value;
}


uint32_t ArchConfig::getLoadLatency(int32_t UNUSED_PARAMETER(mem_width), bool onchip)
{
	if(onchip)
	{
		return getArchParameter(ARCH_LOAD_LATENCY_ONCHIP);
	}
	else
	{
		return getArchParameter(ARCH_LOAD_LATENCY_OFFCHIP);
	}
}

uint32_t ArchConfig::getStoreLatency(int32_t UNUSED_PARAMETER(mem_width), bool onchip)
{
	if(onchip)
	{
		return getArchParameter(ARCH_STORE_LATENCY_ONCHIP);
	}
	else
	{
		return getArchParameter(ARCH_STORE_LATENCY_OFFCHIP);
	}
}

uint32_t ArchConfig::getFetchLatency(bool onchip)
{
	if(onchip)
	{
		return getArchParameter(ARCH_FETCH_LATENCY_ONCHIP);
	}
	else
	{
		return getArchParameter(ARCH_FETCH_LATENCY_OFFCHIP);
	}
}

uint32_t ArchConfig::getMissLatency(mem_type_t mem)
{
	uint32_t miss_latency=0;
	switch(mem)
	{
		case ICACHE:
			{
				miss_latency = getArchParameter(ARCH_ICACHE_MISS_LATENCY);
				break;
			}
		case DISP:
			{
				miss_latency = getArchParameter(ARCH_DISP_BLOCK_LOAD_LATENCY);
				break;
			}
		default:
			{
				miss_latency = 0;
			}
	}
	return miss_latency;
}

uint32_t ArchConfig::getMemElementSize(mem_type_t mem)
{
	uint32_t mem_element_size=0;
	switch(mem)
	{
		case ICACHE:
			{
				mem_element_size = getArchParameter(ARCH_ICACHE_LINE_SIZE);
				break;
			}
		case DISP:
			{
				mem_element_size = getArchParameter(ARCH_DISP_BLOCK_SIZE);
				break;
			}
		default:
			{
				mem_element_size = 0;
			}
	}
	return mem_element_size;
}

uint32_t ArchConfig::getDISPCtrlHitCycles(void)
{
	return getArchParameter(ARCH_DISP_CONTROLLER_HIT_CYLCES);
}

uint32_t ArchConfig::getDISPCtrlMissCycles(void)
{
	return getArchParameter(ARCH_DISP_CONTROLLER_MISS_CYCLES);
}

uint32_t ArchConfig::getJumpPenalty(connection_type_t type, displacement_type_t displacement)
{
	uint32_t penalty=0;
	switch(type)
	{
		case ContinuousAdressing:
		{
			assert(displacement == NoDisplacement);
			penalty = getArchParameter(ARCH_JUMP_PENALTY_CA);
			break;
		}
		case Jump:
		{
			switch(displacement)
			{
				case disp4:
					{
						penalty = getArchParameter(ARCH_JUMP_PENALTY_J4);
						break;
					}
				case disp11:
					// FIXME: Using the same penalty for ARMv6M instructions with 11 bit displacement as assumed for Carcore instructions with 8 bit displacement, since both instructions are 16 bit instructions.
				case disp8:
					{
						penalty = getArchParameter(ARCH_JUMP_PENALTY_J8);
						break;
					}
				case disp15:
					{
						penalty = getArchParameter(ARCH_JUMP_PENALTY_J15);
						break;
					}
				case disp24:
					{
						penalty = getArchParameter(ARCH_JUMP_PENALTY_J24);
						break;
					}
				case indirect:
					{
						penalty = getArchParameter(ARCH_JUMP_PENALTY_JI);
						break;
					}
				default:
					assert(false); // No valid jump displacement!

			}
			break;
		}
		case Call:
		{
			switch(displacement)
			{
				case disp8:
					{
						penalty = getArchParameter(ARCH_JUMP_PENALTY_C8);
						break;
					}
				case disp24:
					{
						penalty = getArchParameter(ARCH_JUMP_PENALTY_C24);
						break;
					}
				case indirect:
					{
						penalty = getArchParameter(ARCH_JUMP_PENALTY_CI);
						break;
					}
				default:
					assert(false); // No valid call displacement!
			}
			break;
		}
		case Return:
		{
			penalty = 0; // There is no penalty for returns!
			break;
		}
		default:
		{
			assert(false);
		}
	}

	return penalty;
}


uint32_t ArchConfig::getSizePenalty(connection_type_t type, displacement_type_t displacement)
{
	uint32_t penalty=0;
	switch(type)
	{
		case ContinuousAdressing:
		{
			assert(displacement == NoDisplacement);
			penalty = getArchParameter(ARCH_SIZE_PENALTY_CA);
			break;
		}
		case Jump:
		{
			switch(displacement)
			{
				case disp4:
					{
						penalty = getArchParameter(ARCH_SIZE_PENALTY_J4);
						break;
					}
				case disp11:
					// FIXME: Using the same penalty for ARMv6M instructions with 11 bit displacement as assumed for Carcore instructions with 8 bit displacement, since both instructions are 16 bit instructions.
				case disp8:
					{
						penalty = getArchParameter(ARCH_SIZE_PENALTY_J8);
						break;
					}
				case disp15:
					{
						penalty = getArchParameter(ARCH_SIZE_PENALTY_J15);
						break;
					}
				case disp24:
					{
						penalty = getArchParameter(ARCH_SIZE_PENALTY_J24);
						break;
					}
				case indirect:
					{
						penalty = getArchParameter(ARCH_SIZE_PENALTY_JI);
						assert(penalty == 0); // indirect jumps does not need to be resized, because a register holds the jump target
						break;
					}
				default:
					assert(false); // No valid jump displacement!

			}
			break;
		}
		case Call:
		{
			switch(displacement)
			{
				case disp8:
					{
						penalty = getArchParameter(ARCH_SIZE_PENALTY_C8);
						break;
					}
				case disp24:
					{
						penalty = getArchParameter(ARCH_SIZE_PENALTY_C24);
						break;
					}
				case indirect:
					{
						penalty = getArchParameter(ARCH_SIZE_PENALTY_CI);
						assert(penalty == 0); // indirect calls does not need to be resized, because a register holds the calls target
						break;
					}
				default:
					assert(false); // No valid call displacement!
			}
			break;
		}
		case Return:
		{
			penalty = 0; // There is no penalty for returns!
			break;
		}
		default:
		{
			assert(false);
		}
	}

	return penalty;
}

uint32_t ArchConfig::getAddressWidth(void)
{
	return getArchParameter(ARCH_ADDRESS_WIDTH);
}

uint32_t ArchConfig::getCacheLineSize(void)
{
	return getArchParameter(ARCH_ICACHE_LINE_SIZE);
}

uint32_t ArchConfig::getCacheAssociativity(void)
{
	return getArchParameter(ARCH_ICACHE_ASSOCIATIVITY);
}

uint32_t ArchConfig::getDISPBlockSize(void)
{
	return getArchParameter(ARCH_DISP_BLOCK_SIZE);
}

uint32_t ArchConfig::getDISPMaxFunctionSize(void)
{
	return getArchParameter(ARCH_DISP_MAX_FUNCTION_SIZE);
}

uint32_t ArchConfig::getDISPContextStackDepth(void)
{
	return getArchParameter(ARCH_DISP_CONTEXT_STACK_DEPTH);
}

uint32_t ArchConfig::getDISPMaxFunctionNumber(void)
{
	return getArchParameter(ARCH_DISP_MAPPING_TABLE_SIZE);
}

uint32_t ArchConfig::getDISPMappingTableSize(void)
{
	return getArchParameter(ARCH_DISP_MAPPING_TABLE_SIZE);
}

uint32_t ArchConfig::getDISPLookupWidth(void)
{
	return getArchParameter(ARCH_DISP_LOOKUP_WIDTH);
}

uint32_t ArchConfig::getFetchBandwidth(void)
{
	return getArchParameter(ARCH_FETCH_BANDWIDTH);
}

string ArchConfig::getHelpMessageHeader(void)
{
	ConsoleStringPrinter csp;
	stringstream s;

	s << endl;
	s << "Help for architectural configuration" << endl;
	s << "====================================" << endl << endl;
	s << csp.chopToLineLength("The timing model for the architectural analysis is configured by the architectural property file. It is defined by the options \"use_architecture_config_file\" and \"architecture_config_file\" in the analysis configuration file. If no architectural configuration is provided, default values for the specified architecture are used.") << endl;
	s << endl;

	return s.str();;
}



string ArchConfig::getBasicHelpMessage(void)
{
	ConsoleStringPrinter csp;
	stringstream s;

	s << getHelpMessageHeader();
	s << csp.chopToLineLength("Currently two architectures are supported: the CarCore (\""CONF_ARCHITECTURE" = "CONF_ARCHITECTURE_CARCORE"\"; show help message with --help-carcore) and a model of the ARM Cortex M0 (\""CONF_ARCHITECTURE" = "CONF_ARCHITECTURE_CARCORE"\"; see configuration parameters with --help-armv6m).") << endl;
	s << endl;

	return s.str();;
}
