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
#ifndef _RATIOFILE_READER_HPP_
#define _RATIOFILE_READER_HPP_


#include "global.h"
#include "constants.h"
#include "graph_structure.h"

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <boost/regex.hpp>
#include <map>
#include <string>

using namespace boost;


typedef map<uint32_t, uint32_t> BbaddrCostMap;

class RatioFileReader {
	public:
		RatioFileReader();
		virtual ~RatioFileReader();

		void setFile(string filename);
		uint32_t getBBCost(uint32_t bb_address);


	private:
		void readFile(void);
		bool isBBLine(string line);
		uint32_t getBBAddrFromLine(string line);
		uint32_t getCostFromLine(string line);

		string ratio_file_name;
		bool file_read;

		BbaddrCostMap bb_addr_cost;

		// regular expression to find the block address (usually this is given as a comment after the loop bound) within the line
		static regex re_bbaddr;

		static LoggerPtr logger;
};


#endif

