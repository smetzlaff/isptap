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
#ifndef _DLPFACTORY_HPP_
#define _DLPFACTORY_HPP_

#include "global.h"
#include "constants.h"
#include "configuration.hpp"
#include "tricore_dumpline_parser.hpp"
#include "armv6m_dumpline_parser.hpp"

class DLPFactory {
	public:
		static DLPFactory* getInstance(void);

		DumpLineParser* getDLPObject(void);
	private:
		DLPFactory();
		virtual ~DLPFactory();
		DLPFactory(const DLPFactory&);                 // Prevent copy-construction
		DLPFactory& operator=(const DLPFactory&);      // Prevent assignment

		static DLPFactory *singleton;
};

#endif
