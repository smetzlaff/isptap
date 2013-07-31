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
#include "memory_size_stepper.hpp"


MemorySizeStepper::MemorySizeStepper()
{
	conf = Configuration::getInstance();
	cccfg = CarCoreConfig::getInstance();

	assert(conf->getBool(CONF_MEMORY_SIZE_STEPPING) == 1);

	if(conf->getUint(CONF_USE_MEMORY_BUDGET) == 0)
	{
		min_size = conf->getUint(CONF_MEMORY_START_SIZE);
		max_size = conf->getUint(CONF_MEMORY_SIZE);
		step_size = conf->getUint(CONF_MEMORY_STEP_SIZE);
		actual_size = min_size;
	}
	else
	{
		// use other parameters?? or the same??
		assert(false);
	}
}
MemorySizeStepper::~MemorySizeStepper()
{
}

uint32_t MemorySizeStepper::getMemorySize(void)
{
	return mp.getUsableMemorySize(actual_size);
}

cache_params_t MemorySizeStepper::getCacheParameters(void)
{
	return mp.getCacheParameters(actual_size);
}

disp_params_t MemorySizeStepper::getDispParameters(void)
{
	return mp.getDispParameters(actual_size);
}

sisp_params_t MemorySizeStepper::getSispParameters(void)
{
	return mp.getSispParameters(actual_size);
}

nomem_params_t MemorySizeStepper::getNoMemParameters(void)
{
	return mp.getNoMemParameters();
}

bool MemorySizeStepper::isValidSize(void)
{
	if(step_size != 0) // normal mode
	{
		if(actual_size <= max_size)
		{
			return true;
		}
	}
	else // exponential mode
	{
		if(actual_size <= max_size)
		{
			return true;
		}
	}
	return false;
}

void MemorySizeStepper::increaseSize(void)
{
	if(step_size != 0) // normal mode
	{
			actual_size += step_size;
	}
	else // exponential mode
	{
			actual_size <<= 1;
	}
}

