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
#include "arch_cfg_factory.hpp"

ArchConfigFactory *ArchConfigFactory::singleton = NULL;

ArchConfigFactory* ArchConfigFactory::getInstance(void)
{
	if(singleton == NULL)
	{
		singleton = new ArchConfigFactory;
	}
	return singleton;
}


ArchConfigFactory::ArchConfigFactory()
{
}

ArchConfigFactory::~ArchConfigFactory()
{
}


ArchConfig *ArchConfigFactory::getArchConfigObject(void)
{
	switch((architecture_t)Configuration::getInstance()->getUint(CONF_ARCHITECTURE))
	{
		case CARCORE:
			return CarCoreConfig::getInstance();
		case ARMV6M:
			return Armv6mConfig::getInstance();
		default:
			assert(false);
	}

	return NULL;
}


