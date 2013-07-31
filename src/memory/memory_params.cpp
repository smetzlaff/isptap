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
#include "memory_params.hpp"

LoggerPtr MemoryParameters::logger(Logger::getLogger("MemoryParameters"));

MemoryParameters::MemoryParameters()
{
	conf = Configuration::getInstance();
	arch_cfg = ArchConfigFactory::getInstance()->getArchConfigObject();
}

MemoryParameters::~MemoryParameters()
{
}

mem_type_t MemoryParameters::getMemType(void)
{
	return (mem_type_t)conf->getUint(CONF_MEMORY_TYPE);
}

string MemoryParameters::getMemTypeString(void)
{
	return conf->getString(CONF_MEMORY_TYPE);
}


replacement_policy_t MemoryParameters::getReplacementPolicy(void)
{
//	mem_type_t mtype = getMemType();
//	assert((mtype == ICACHE) || (mtype == DISP));

	return (replacement_policy_t)conf->getUint(CONF_MEMORY_REPLACEMENT_POLICY);
}

cache_params_t MemoryParameters::getCacheParameters(void)
{
	cache_params_t parameters;

	parameters.type = getMemType();
	parameters.rpol = getReplacementPolicy();
	parameters.use_bbs_instead_of_cache_lines = conf->getBool(CONF_MEMORY_CACHE_BBS);

	parameters.line_size = arch_cfg->getCacheLineSize();
	// line size has to be a power of two and has to be larger than 64 bit / 8byte (which is the fetch bandwidth)
	assert(isPowerOfTwo(parameters.line_size) && parameters.line_size >= 8);

	parameters.line_size_bit = log2(parameters.line_size);

	parameters.associativity = arch_cfg->getCacheAssociativity();


	if(conf->getUint(CONF_USE_MEMORY_BUDGET) == 0)
	{
		parameters.size = conf->getUint(CONF_MEMORY_SIZE);
		parameters.line_no = parameters.size / parameters.line_size;
		assert(parameters.size % parameters.line_size == 0);

		if(parameters.associativity != 0)
		{
			parameters.ways = parameters.associativity;
			parameters.sets = parameters.line_no / parameters.associativity;
			// TODO: Add checks
		}
		else
		{
			parameters.ways = parameters.line_no;
			parameters.sets = 1;
		}

	}
	else
	{
		MemoryBudgetCalculator mbc(parameters.type, parameters.rpol, conf->getUint(CONF_MEMORY_BUDGET));
		cache_dimensions_t cd = mbc.calculateCacheParameters();
		parameters.size = cd.cache_memory_size;
		parameters.ways = cd.ways;
		parameters.sets = cd.sets;
		parameters.line_no = cd.line_no;

		LOG_INFO(logger, "Memory budget of " << conf->getUint(CONF_MEMORY_BUDGET) << " bytes results in a " << parameters.size << " byte cache of associativity " << parameters.associativity << " with " << parameters.ways << " ways and " << parameters.sets << " sets. rpol:" << parameters.rpol << ", line_size:" << parameters.line_size << " line_no: " << parameters.line_no <<  " use_bbs_instead_of_lines: " << parameters.use_bbs_instead_of_cache_lines);

		// TODO The CONF_USE_MEMORY_BUDGET option is currently not exhaustively tested.
		assert(false); // remove when tested
	}
	return parameters;
}


cache_params_t MemoryParameters::getCacheParameters(uint32_t size_to_override)
{

	// TODO: Add some checks for size_to_override.

	cache_params_t parameters;

	parameters.type = getMemType();
	parameters.rpol = getReplacementPolicy();
	parameters.use_bbs_instead_of_cache_lines = conf->getBool(CONF_MEMORY_CACHE_BBS);

	parameters.line_size = arch_cfg->getCacheLineSize();
	// line size has to be a power of two and has to be larger than 64 bit / 8byte (which is the fetch bandwidth)
	assert(isPowerOfTwo(parameters.line_size) && parameters.line_size >= 8);

	parameters.line_size_bit = log2(parameters.line_size);

	parameters.associativity = arch_cfg->getCacheAssociativity();

	if(conf->getUint(CONF_USE_MEMORY_BUDGET) == 0)
	{
		parameters.size = size_to_override;
		parameters.line_no = parameters.size / parameters.line_size;
		assert(parameters.size % parameters.line_size == 0);

		if(parameters.associativity != 0)
		{
			parameters.ways = parameters.associativity;
			parameters.sets = parameters.line_no / parameters.associativity;
			// TODO: Add checks
		}
		else
		{
			parameters.ways = parameters.line_no;
			parameters.sets = 1;
		}

	}
	else
	{
		MemoryBudgetCalculator mbc(parameters.type, parameters.rpol, size_to_override);
		cache_dimensions_t cd = mbc.calculateCacheParameters();
		parameters.size = cd.cache_memory_size;
		parameters.ways = cd.ways;
		parameters.sets = cd.sets;
		parameters.line_no = cd.line_no;

		LOG_INFO(logger, "Memory budget of " << size_to_override << " bytes results in a " << parameters.size << " byte cache of associativity " << parameters.associativity << " with " << parameters.ways << " ways and " << parameters.sets << " sets. rpol:" << parameters.rpol << ", line_size:" << parameters.line_size << " line_no: " << parameters.line_no <<  " use_bbs_instead_of_lines: " << parameters.use_bbs_instead_of_cache_lines);

		// TODO The CONF_USE_MEMORY_BUDGET option is currently not exhaustively tested.
		assert(false); // remove when tested
	}
	return parameters;
}

disp_params_t MemoryParameters::getDispParameters(void)
{
	disp_params_t parameters;

	parameters.type = getMemType();
	parameters.rpol = getReplacementPolicy();

	assert(parameters.type == DISP);

	parameters.block_size = arch_cfg->getDISPBlockSize(); 
	parameters.mapping_table_size = arch_cfg->getDISPMappingTableSize();
	parameters.lookup_width = arch_cfg->getDISPLookupWidth();
	parameters.context_stack_depth = arch_cfg->getDISPContextStackDepth();
	parameters.ignore_outsized_functions = conf->getBool(CONF_MEMORY_DISP_IGNORE_OUTSIZED_FUNCTIONS);


	if(conf->getUint(CONF_USE_MEMORY_BUDGET) == 0)
	{
		parameters.size = conf->getUint(CONF_MEMORY_SIZE);
		parameters.block_no = parameters.size / parameters.block_size;
		assert(parameters.size % parameters.block_size == 0);
	}
	else
	{
		MemoryBudgetCalculator mbc(parameters.type, parameters.rpol, conf->getUint(CONF_MEMORY_BUDGET));
		disp_dimensions_t dd = mbc.calculateDispParameters();
		parameters.size = dd.disp_memory_size;
		parameters.block_no = dd.block_no;

		LOG_INFO(logger, "Memory budget of " << conf->getUint(CONF_MEMORY_BUDGET) << " bytes results in a " << parameters.size << " byte DISP. rpol:" << parameters.rpol << ", block_size:" << parameters.block_size << ", block_no: " << parameters.block_no <<  " mapping table size: " << parameters.mapping_table_size << ", lookup width: " << parameters.lookup_width << ", context stack depth: " << parameters.context_stack_depth << ", ignore_outsized_functions: " << parameters.ignore_outsized_functions);

		// TODO The CONF_USE_MEMORY_BUDGET option is currently not exhaustively tested.
		assert(false); // remove when tested
	}
	return parameters;
}


disp_params_t MemoryParameters::getDispParameters(uint32_t size_to_override)
{
	disp_params_t parameters;

	parameters.type = getMemType();
	parameters.rpol = getReplacementPolicy();

	assert(parameters.type == DISP);

	parameters.block_size = arch_cfg->getDISPBlockSize(); 
	parameters.mapping_table_size = arch_cfg->getDISPMappingTableSize();
	parameters.lookup_width = arch_cfg->getDISPLookupWidth();
	parameters.context_stack_depth = arch_cfg->getDISPContextStackDepth();
	parameters.ignore_outsized_functions = conf->getBool(CONF_MEMORY_DISP_IGNORE_OUTSIZED_FUNCTIONS);


	if(conf->getUint(CONF_USE_MEMORY_BUDGET) == 0)
	{
		parameters.size = size_to_override;
		parameters.block_no = parameters.size / parameters.block_size;
		assert(parameters.size % parameters.block_size == 0);
	}
	else
	{
		MemoryBudgetCalculator mbc(parameters.type, parameters.rpol, size_to_override);
		disp_dimensions_t dd = mbc.calculateDispParameters();
		parameters.size = dd.disp_memory_size;
		parameters.block_no = dd.block_no;

		LOG_INFO(logger, "Memory budget of " << size_to_override << " bytes results in a " << parameters.size << " byte DISP. rpol:" << parameters.rpol << ", block_size:" << parameters.block_size << ", block_no: " << parameters.block_no <<  " mapping table size: " << parameters.mapping_table_size << ", lookup width: " << parameters.lookup_width << ", context stack depth: " << parameters.context_stack_depth << ", ignore_outsized_functions: " << parameters.ignore_outsized_functions);

		// TODO The CONF_USE_MEMORY_BUDGET option is currently not exhaustively tested.
		assert(false); // remove when tested
	}
	return parameters;
}

sisp_params_t MemoryParameters::getSispParameters(void)
{
	sisp_params_t parameters;

	parameters.type = getMemType();
	parameters.use_jump_penalties = conf->getBool(CONF_MEMORY_BBSISP_ADD_JUMP_PENALTIES_TO_WCET);

	assert((parameters.type == BBSISP) || (parameters.type == BBSISP_JP) || (parameters.type == BBSISP_WCP) || (parameters.type == BBSISP_JP_WCP) || (parameters.type == FSISP) || (parameters.type == FSISP_WCP) || (parameters.type == FSISP_OLD));

	if(conf->getUint(CONF_USE_MEMORY_BUDGET) == 0)
	{
		parameters.size = conf->getUint(CONF_MEMORY_SIZE);
	}
	else
	{
		MemoryBudgetCalculator mbc(parameters.type, UNKNOWN_RP, conf->getUint(CONF_MEMORY_BUDGET));
		parameters.size = mbc.calculateSispParameters().sisp_memory_size;

		LOG_INFO(logger, "Memory budget of " << conf->getUint(CONF_MEMORY_BUDGET) << " bytes results in a " << parameters.size << " byte SISP (" << parameters.type << ")");

		// TODO The CONF_USE_MEMORY_BUDGET option is currently not exhaustively tested.
		assert(false); // remove when tested
	}

	return parameters;
}

sisp_params_t MemoryParameters::getSispParameters(uint32_t size_to_override)
{
	sisp_params_t parameters;

	parameters.type = getMemType();
	parameters.use_jump_penalties = conf->getBool(CONF_MEMORY_BBSISP_ADD_JUMP_PENALTIES_TO_WCET);

	assert((parameters.type == BBSISP) || (parameters.type == BBSISP_JP) || (parameters.type == BBSISP_WCP) || (parameters.type == BBSISP_JP_WCP) || (parameters.type == FSISP) || (parameters.type == FSISP_WCP) || (parameters.type == FSISP_OLD));

	if(conf->getUint(CONF_USE_MEMORY_BUDGET) == 0)
	{
		parameters.size = size_to_override;
	}
	else
	{
		MemoryBudgetCalculator mbc(parameters.type, UNKNOWN_RP, size_to_override);
		parameters.size = mbc.calculateSispParameters().sisp_memory_size;

		LOG_INFO(logger, "Memory budget of " << size_to_override << " bytes results in a " << parameters.size << " byte SISP (" << parameters.type << ")");

		// TODO The CONF_USE_MEMORY_BUDGET option is currently not exhaustively tested.
		assert(false); // remove when tested
	}

	return parameters;
}


nomem_params_t MemoryParameters::getNoMemParameters(void)
{
	nomem_params_t parameters;

	parameters.type = getMemType();

	assert((parameters.type == NO_MEM) || (parameters.type == VIVU_TEST));

	return parameters;
}

uint32_t MemoryParameters::getUsableMemorySize(void)
{

	switch(getMemType())
	{
		case ICACHE:
			{
				return getCacheParameters().size;
				break;
			}
		case DISP:
			{
				return getDispParameters().size;
				break;
			}
		case BBSISP:
		case BBSISP_JP:
		case BBSISP_WCP:
		case BBSISP_JP_WCP:
		case FSISP:
		case FSISP_WCP:
		case FSISP_OLD:
			{
				return getSispParameters().size;
				break;
			}
		case NO_MEM:
		case VIVU_TEST:
			{
				return 0;
				break;
			}
		default:
			{
				assert(false);
				return 0;
			}
	}
}


uint32_t MemoryParameters::getUsableMemorySize(uint32_t size_to_override)
{

	switch(getMemType())
	{
		case ICACHE:
			{
				return getCacheParameters(size_to_override).size;
				break;
			}
		case DISP:
			{
				return getDispParameters(size_to_override).size;
				break;
			}
		case BBSISP:
		case BBSISP_JP:
		case BBSISP_WCP:
		case BBSISP_JP_WCP:
		case FSISP:
		case FSISP_WCP:
		case FSISP_OLD:
			{
				return getSispParameters(size_to_override).size;
				break;
			}
		case NO_MEM:
		case VIVU_TEST:
			{
				return 0;
				break;
			}
		default:
			{
				assert(false);
				return 0;
			}
	}
}


bool MemoryParameters::isPowerOfTwo(uint32_t number)
{
	assert(number > 0);

	// shift all lower zeros ('0b0')) until a '0b1' appears in the bit string
	while((number & 1) == 0)
	{
		number >>= 1;
	}

	// if the '0b1' is the only '0b1' in the original number the number is a power of two
	return(number == 1);
}

uint32_t MemoryParameters::log2(uint32_t number)
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
