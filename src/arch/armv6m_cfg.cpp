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
#include "armv6m_cfg.hpp"

#define MAX(a, b) (a > b)?(a):(b)

LoggerPtr Armv6mConfig::logger(Logger::getLogger("Armv6mConfig"));

Armv6mConfig *Armv6mConfig::singleton = NULL;

Armv6mConfig *Armv6mConfig::getInstance(void)
{
	return getInstance(false);
}

Armv6mConfig* Armv6mConfig::getInstanceWithHelpMessages(void)
{
	return getInstance(true);
}

Armv6mConfig *Armv6mConfig::getInstance(bool initialise_help_messages)
{
	if(singleton == NULL)
	{
		singleton = new Armv6mConfig(initialise_help_messages);
	}
	return singleton;
}

#define HELP_ARMV6M_ARITHMETIC_OP_LATENCY "The latency of arithmetic instructions (in cycles)."
#define HELP_ARMV6M_ARITHMETIC_OP_LATENCY_WITH_PC "The latency of arithmetic instructions using the PC register (in cycles)."
#define HELP_ARMV6M_LOGIC_OP_LATENCY "The latency of logic instructions (in cycles)."
#define HELP_ARMV6M_MULTIPLY_LATENCY "The latency of multiply instructions (in cycles)."
#define HELP_ARMV6M_MRS_LATENCY "The latency of the \"read special register\" instruction (in cycles)."
#define HELP_ARMV6M_MSR_LATENCY "The latency of the \"write special register\" instruction (in cycles)."
#define HELP_ARMV6M_BC_TAKEN_LATENCY "The branch latency of conditional branches when taken (in cycles)."
#define HELP_ARMV6M_BC_NOTTAKEN_LATENCY "The branch latency of conditional branches when not taken (in cycles)."
#define HELP_ARMV6M_B_LATENCY "The latency of branch instructions (in cycles)."
#define HELP_ARMV6M_BL_LATENCY "The latency of branch and link instructions (in cycles)."
#define HELP_ARMV6M_BX_LATENCY "The latency of branch and exchange instructions (in cycles)."
#define HELP_ARMV6M_BLX_LATENCY "The latency of branch, link and exchange instructions (in cycles)."
#define HELP_ARMV6M_POP_AND_RETURN_EXTRA_LATENCY "The additional latency for the (returning) jump, if the PC is in the register list of pop instructions (in cycles)."
#define HELP_ARMV6M_DSB_LATENCY "The latency of the \"data synchronisation barrier\" instruction (in cycles)."
#define HELP_ARMV6M_DMB_LATENCY "The latency of the \"data memory barrier\" instruction (in cycles)."
#define HELP_ARMV6M_ISB_LATENCY "The latency of the \"instruction synchronisation barrier\" instruction (in cycles)."


Armv6mConfig::Armv6mConfig(bool initialise_help_messages) : ArchConfig(initialise_help_messages)
{
	PREPARE_OPTION_UINT(arch_parameters_map, ARMV6M_ARITHMETIC_OP_LATENCY, 0, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARMV6M_ARITHMETIC_OP_LATENCY_WITH_PC, 2, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARMV6M_LOGIC_OP_LATENCY, 0, initialise_help_messages); // unsused
	PREPARE_OPTION_UINT(arch_parameters_map, ARMV6M_MULTIPLY_LATENCY, 31, initialise_help_messages);

	PREPARE_OPTION_UINT(arch_parameters_map, ARMV6M_MRS_LATENCY, 3, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARMV6M_MSR_LATENCY, 3, initialise_help_messages);

	PREPARE_OPTION_UINT(arch_parameters_map, ARMV6M_BC_TAKEN_LATENCY, 2, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARMV6M_BC_NOTTAKEN_LATENCY, 0, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARMV6M_B_LATENCY, 2, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARMV6M_BL_LATENCY, 3, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARMV6M_BX_LATENCY, 2, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARMV6M_BLX_LATENCY, 2, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARMV6M_POP_AND_RETURN_EXTRA_LATENCY, 3, initialise_help_messages);

	PREPARE_OPTION_UINT(arch_parameters_map, ARMV6M_DSB_LATENCY, 3, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARMV6M_DMB_LATENCY, 3, initialise_help_messages);
	PREPARE_OPTION_UINT(arch_parameters_map, ARMV6M_ISB_LATENCY, 3, initialise_help_messages);


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

Armv6mConfig::~Armv6mConfig()
{
}


uint32_t Armv6mConfig::getArithmeticOpLatency(bool PcAsTarget)
{
	if(PcAsTarget)
	{
		return getArchParameter(ARMV6M_ARITHMETIC_OP_LATENCY_WITH_PC);
	}
	else
	{
		return getArchParameter(ARMV6M_ARITHMETIC_OP_LATENCY);
	}
}

uint32_t Armv6mConfig::getLogicOpLatency(void)
{
	return getArchParameter(ARMV6M_LOGIC_OP_LATENCY);
}

uint32_t Armv6mConfig::getMultLatency(void)
{
	return getArchParameter(ARMV6M_MULTIPLY_LATENCY);
}

uint32_t Armv6mConfig::getMRSLatency(void)
{
	return getArchParameter(ARMV6M_MRS_LATENCY);
}

uint32_t Armv6mConfig::getMSRLatency(void)
{
	return getArchParameter(ARMV6M_MSR_LATENCY);
}

uint32_t Armv6mConfig::getUncondBranchLatency(void)
{
	return getArchParameter(ARMV6M_B_LATENCY);
}

uint32_t Armv6mConfig::getCondBranchLatency(bool taken)
{
	if(taken)
	{
		return getArchParameter(ARMV6M_BC_TAKEN_LATENCY);
	}
	else
	{
		return getArchParameter(ARMV6M_BC_NOTTAKEN_LATENCY);
	}
}

uint32_t Armv6mConfig::getBLLatency(void)
{
	return getArchParameter(ARMV6M_BL_LATENCY);
}

uint32_t Armv6mConfig::getBXLatency(void)
{
	return getArchParameter(ARMV6M_BX_LATENCY);
}

uint32_t Armv6mConfig::getBLXLatency(void)
{
	return getArchParameter(ARMV6M_BLX_LATENCY);
}

uint32_t Armv6mConfig::getPopAndReturnExtraLatency(void)
{
	return getArchParameter(ARMV6M_POP_AND_RETURN_EXTRA_LATENCY);
}

uint32_t Armv6mConfig::getDSBLatency(void)
{
	return getArchParameter(ARMV6M_DSB_LATENCY);
}

uint32_t Armv6mConfig::getDMBLatency(void)
{
	return getArchParameter(ARMV6M_DMB_LATENCY);
}

uint32_t Armv6mConfig::getISBLatency(void)
{
	return getArchParameter(ARMV6M_ISB_LATENCY);
}


uint32_t Armv6mConfig::getCallLatency(bool UNUSED_PARAMETER(onchip))
{
	// in ARMv6M BL (branch and link) or BLX (branch and link and exchange) instructions represent calls
	return MAX(getBLLatency(), getBLXLatency());
}

uint32_t Armv6mConfig::getReturnLatency(bool onchip)
{
	// in ARMv6M BX (branch and exchange) and POP instructions represent returns
	// XXX verify timing: A return could also be a pop with multiple registers in the register list, resulting in a larger instruction execution time than a pop with one register only.
	return MAX(getBXLatency(), getPopAndReturnExtraLatency() + getLoadLatency(-1, onchip) /* the extra latency for fetching the PC from the stack */);
}

bool Armv6mConfig::isFetchMemIndependent(void)
{
	return true;
}


string Armv6mConfig::getHelpMessage(void)
{
	ConsoleStringPrinter csp;

	stringstream s;
	s << ArchConfig::getHelpMessageHeader() << endl;
	s << "ARMv6M timing model parameters" << endl;
	s << "------------------------------" << endl;
	s << csp.chopToLineLength("The architectural configuration file for the ARMv6M supports the following properties. In the configuration files the prefix \"armv6m.\" has to be used for each property definition.") << endl;
	s << endl;

	// print the help messages of the registered properties
	for(vector<help_message_t>::iterator it = config_properties.begin(); it != config_properties.end(); ++it)
	{
		string p = it->property + ": (default:" + ((it->default_value.compare("")!=0)?(it->default_value):("--none--")) + ") " + it->help_message;
		s << csp.chopToLineLength(p, " ") << endl;
	}

	return s.str();
}

