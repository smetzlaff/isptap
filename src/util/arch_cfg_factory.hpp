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
#ifndef _ARCH_CFG_FACTORY_HPP_
#define _ARCH_CFG_FACTORY_HPP_

#include "global.h"
#include "constants.h"
#include "configuration.hpp"
#include "carcore_cfg.hpp"
#include "armv6m_cfg.hpp"

class ArchConfigFactory {
	public:
		static ArchConfigFactory* getInstance(void);

		ArchConfig* getArchConfigObject(void);
	private:
		ArchConfigFactory();
		virtual ~ArchConfigFactory();
		ArchConfigFactory(const ArchConfigFactory&);                 // Prevent copy-construction
		ArchConfigFactory& operator=(const ArchConfigFactory&);      // Prevent assignment

		static ArchConfigFactory *singleton;
};

#endif
