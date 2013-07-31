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
#include "carcore_cfg.hpp"

LoggerPtr CarCoreConfig::logger(Logger::getLogger("CarCoreConfig"));

CarCoreConfig *CarCoreConfig::singleton = NULL;


CarCoreConfig *CarCoreConfig::getInstance(void)
{
	return getInstance(false);
}

CarCoreConfig* CarCoreConfig::getInstanceWithHelpMessages(void)
{
	return getInstance(true);
}


CarCoreConfig *CarCoreConfig::getInstance(bool initialise_help_messages)
{
	if(singleton == NULL)
	{
		singleton = new CarCoreConfig(initialise_help_messages);
	}
	return singleton;
}


#define HELP_CARCORE_BRANCH_LATENCY "The latency of branches (in cycles)."
#define HELP_CARCORE_CALL_LATENCY "The latency of the call microcode (in cycles)."
#define HELP_CARCORE_RETURN_LATENCY "The latency of the return microcode (in cycles)."

#define HELP_CARCORE_FETCH_INDEPENDENT_IMEM "If set to '1', the instruction and data memory are independent of each other and use a separated memory connection."
#define HELP_CARCORE_FETCH_OPTIMIZATION_BRANCH_AHEAD "If set to '1', the processor stalls the instruction fetch, if a branch instruction is in the instruction window. This optimisation of the CarCore is currently not supported."
#define HELP_CARCORE_FETCH_OPTIMIZATION_ENOUGH_INSTRS "If set to '1', the processor analyses the number of instructions in the instruction window and stops fetching when enough instructions are present.  This optimisation of the CarCore is currently not supported."
#define HELP_FP_ARITHMETIC_LATENCY "The latency of arithmetic floating point instructions (in cycles)."
#define HELP_FP_DIVIDE_LATENCY "The latency of division floating point instructions (in cycles)."
#define HELP_FP_CONVERSION_LATENCY "The latency for conversion floating point instructions (in cycles)."
#define HELP_FP_MULTIPLYACCUMULATE_LATENCY "The latency for multiply-accumulate floating point instructions (in cycles)."
#define HELP_FP_MULTIPLY_LATENCY "The latency for the multiplication floating point instruction (in cycles)."
#define HELP_FP_SQRTSEED_LATENCY "The latency for the square root seed floating point instruction (in cycles)."
#define HELP_FP_UPDATEFLAGS_LATENCY "The latency for the update flags floating point instruction (in cycles)."

CarCoreConfig::CarCoreConfig(bool initialise_help_messages) : ArchConfig(initialise_help_messages)
{
	PREPARE_OPTION_UINT(arch_parameters_map, CARCORE_BRANCH_LATENCY, 3, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, CARCORE_CALL_LATENCY, 57, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, CARCORE_RETURN_LATENCY, 50, initialise_help_messages);

	// FIXME: The instruction memory has to use non shared resources (dmem access cannot interfere fetch)
	PREPARE_OPTION_UINT(arch_parameters_map, CARCORE_FETCH_INDEPENDENT_IMEM, 1, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, CARCORE_FETCH_OPTIMIZATION_BRANCH_AHEAD, 0, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, CARCORE_FETCH_OPTIMIZATION_ENOUGH_INSTRS, 0, initialise_help_messages);

	PREPARE_OPTION_UINT(arch_parameters_map, FP_ARITHMETIC_LATENCY, 1, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, FP_DIVIDE_LATENCY, 5, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, FP_CONVERSION_LATENCY, 1, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, FP_MULTIPLYACCUMULATE_LATENCY, 4, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, FP_MULTIPLY_LATENCY, 3, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, FP_SQRTSEED_LATENCY, 10, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, FP_UPDATEFLAGS_LATENCY, 0, initialise_help_messages);


	arch_prefix = conf->getString(CONF_ARCHITECTURE);

	if(!initialise_help_messages)
	{
		LOG_DEBUG(logger, "Configured for " << arch_prefix << " architecture.");
	}

	if(conf->getBool(CONF_USE_ARCH_CFG_FILE))
	{
		readArchConfigFile(conf->getString(CONF_ARCH_CFG_FILE));
	}

}

CarCoreConfig::~CarCoreConfig()
{
}

uint32_t CarCoreConfig::getLMSLatency(int32_t mem_width, bool onchip)
{
	// FIXME The LMS timing should be refactored.
// the carcore.lms_latency_onchip/offchip property was removed from property files.
	if(onchip)
	{
		assert(false); // there is no on-chip data memory modelled yet.
		return getLoadLatency(mem_width, onchip) + 1 + getStoreLatency(mem_width, onchip) - 1; // the MC needes the time to load, modify and store. Decrement the result by one, because it is a latency
	}
	else
	{
		return getLoadLatency(mem_width, onchip) + 1 + getStoreLatency(mem_width, onchip) - 1; // the MC needes the time to load, modify and store. Decrement the result by one, because it is a latency
	}
}

uint32_t CarCoreConfig::getBranchLatency(void)
{
	return getArchParameter(CARCORE_BRANCH_LATENCY);
}

uint32_t CarCoreConfig::getCallLatency(bool UNUSED_PARAMETER(onchip))
{
    // TODO The call is constructed of several off-chip memory accesses and a number of other instructions that are executed. Use a more modular calculation here.
	return getArchParameter(CARCORE_CALL_LATENCY);
}

uint32_t CarCoreConfig::getReturnLatency(bool UNUSED_PARAMETER(onchip))
{
	// TODO The return is constructed of several off-chip memory accesses and a number of other instructions that are executed. Use a more modular calculation here.
	return getArchParameter(CARCORE_RETURN_LATENCY);
}

uint32_t CarCoreConfig::getMCLatency(alu_func_t micro_code)
{
	uint32_t latency = 0;
	switch(micro_code)
	{
		case MC_LDMST:
			{
				latency = getArchParameter(ARCH_LOAD_LATENCY_OFFCHIP) + 1 + getArchParameter(ARCH_STORE_LATENCY_OFFCHIP) - 1; // the MC needes the time to load, modify and store. Decrement the result by one, because it is a latency
				break;
			}
		case MC_LDMST_ABS:
			{
				latency = getArchParameter(ARCH_LOAD_LATENCY_OFFCHIP) + 1 + getArchParameter(ARCH_STORE_LATENCY_OFFCHIP) - 1; // the MC needes the time to load, modify and store. Decrement the result by one, because it is a latency
				break;
			}
		case MC_STDA_PRE: 
			{
				latency = (getArchParameter(ARCH_LOAD_LATENCY_OFFCHIP)*2) - 1; // the MC needes the time to store 2 values. Decrement the result by one, because it is a latency
				break;
			}
		case MC_STDA_POST:
			{
				latency = (getArchParameter(ARCH_LOAD_LATENCY_OFFCHIP)*2) - 1; // the MC needes the time to store 2 values. Decrement the result by one, because it is a latency
				break;
			}
		case MC_STDA_NORM:
			{
				latency = (getArchParameter(ARCH_LOAD_LATENCY_OFFCHIP)*2) - 1; // the MC needes the time to store 2 values. Decrement the result by one, because it is a latency
				break;
			}
		case MC_STDA_ABS:
			{
				latency = (getArchParameter(ARCH_LOAD_LATENCY_OFFCHIP)*2) - 1; // the MC needes the time to store 2 values. Decrement the result by one, because it is a latency
				break;
			}
		case MC_STT:
			{
				latency = getArchParameter(ARCH_LOAD_LATENCY_OFFCHIP) + 1 + getArchParameter(ARCH_STORE_LATENCY_OFFCHIP) - 1; // the MC needes the time to load the value, set the bit and store the value. Decrement the result by one, because it is a latency
				break;
			}
		default:
			{
				assert(false);
			}
	}
	return latency;
}


bool CarCoreConfig::isFetchMemIndependent(void)
{
	return ((getArchParameter(CARCORE_FETCH_INDEPENDENT_IMEM)==0)?(false):(true));
}

bool CarCoreConfig::isFetchOptBranchAhead(void)
{
	return ((getArchParameter(CARCORE_FETCH_OPTIMIZATION_BRANCH_AHEAD)==0)?(false):(true));
}

bool CarCoreConfig::isFetchOptEnoughInstr(void)
{
	return ((getArchParameter(CARCORE_FETCH_OPTIMIZATION_ENOUGH_INSTRS)==0)?(false):(true));
}


uint32_t CarCoreConfig::getFPLatency(fp_op_latency_t type)
{
	switch(type)
	{
		case ARITHMETIC:
			return getArchParameter(FP_ARITHMETIC_LATENCY);
		case DIVIDE:
			return getArchParameter(FP_DIVIDE_LATENCY);
		case CONVERSION:
			return getArchParameter(FP_CONVERSION_LATENCY);
		case MULTIPLYACCUMULATE:
			return getArchParameter(FP_MULTIPLYACCUMULATE_LATENCY);
		case MULTIPLY:
			return getArchParameter(FP_MULTIPLY_LATENCY);
		case SQRTSEED:
			return getArchParameter(FP_SQRTSEED_LATENCY);
		case UPDATEFLAGS:
			return getArchParameter(FP_UPDATEFLAGS_LATENCY);
	}
	return 0;
}

string CarCoreConfig::getHelpMessage(void)
{
	ConsoleStringPrinter csp;

	stringstream s;
	s << ArchConfig::getHelpMessageHeader() << endl;
	s << "CarCore timing model parameters" << endl;
	s << "-------------------------------" << endl;
	s << csp.chopToLineLength("The architectural configuration file for the CarCore supports the following properties. In the configuration files the prefix \"carcore.\" has to be used for each property definition.") << endl;
	s << endl;

	// print the help messages of the registered properties
	for(vector<help_message_t>::iterator it = config_properties.begin(); it != config_properties.end(); ++it)
	{
		string p = it->property + ": (default:" + ((it->default_value.compare("")!=0)?(it->default_value):("--none--")) + ") " + it->help_message;
		s << csp.chopToLineLength(p, " ") << endl;
	}

	return s.str();
}
