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
#include "ratiofile_reader.hpp"
#include "regex_parser_tokens.h"

// regular expression to find the block address (usually this is given as a comment after the loop bound) within the line
regex RatioFileReader::re_bbaddr("a"HEX_TOKEN"{7}");


LoggerPtr RatioFileReader::logger(Logger::getLogger("RatioFileReader"));


RatioFileReader::RatioFileReader() : file_read(false)
{
}

RatioFileReader::~RatioFileReader()
{
}

void RatioFileReader::setFile(string filename)
{
	ratio_file_name = filename;

	readFile();
}

uint32_t RatioFileReader::getBBCost(uint32_t bb_address)
{
	if(file_read)
	{

		BbaddrCostMap::iterator pos;
		pos = bb_addr_cost.find(bb_address);
		if(pos != bb_addr_cost.end())
		{
			return pos->second;
		}
	}

	return 0;
}


void RatioFileReader::readFile(void)
{
	if(file_read)
		return;

	ifstream rf_in;

	rf_in.open(ratio_file_name.c_str());

	if(rf_in.fail())
	{
		LOG_WARN(logger, "File " << ratio_file_name <<" cannot be opened");
		return;
	}

	assert(rf_in.is_open());

	while(!rf_in.eof())
	{
		char c[1024];
		rf_in.getline(c, 1024);
		const string str = c;

		if(isBBLine(str))
		{
			uint32_t bb_addr = getBBAddrFromLine(str);
			uint32_t cost = getCostFromLine(str);

			BbaddrCostMap::iterator pos;
			bool inserted;
			pos = bb_addr_cost.find(bb_addr);
			// address has NOT to be found
			if(pos == bb_addr_cost.end())
			{
				// basic block cost for that block is not already stored, storing it
				LOG_DEBUG(logger, "Found new basic block cost for: 0x" << hex << bb_addr << " of: " << dec << cost); 
				tie(pos, inserted) = bb_addr_cost.insert(make_pair(bb_addr, cost));
				assert(inserted);
			}
			else
			{
				// basic block cost for that block is already stored, comparing the value to the stored one
				pos = bb_addr_cost.find(bb_addr);
				assert((pos != bb_addr_cost.end()) && (cost == pos->second));
			}
		}
	}
	file_read = true;
	rf_in.close();
}

bool RatioFileReader::isBBLine(string line)
{
	return regex_search(line, re_bbaddr);
}

uint32_t RatioFileReader::getBBAddrFromLine(string line)
{
	assert(isBBLine(line));

	smatch what;
	uint32_t bb_addr;

	if(regex_search(line, what, re_bbaddr, match_default))
	{
		string str = what[0];
		bb_addr = strtoul(str.c_str(),NULL,16);
	}
	else
		assert(false);

	return bb_addr;
}

uint32_t RatioFileReader::getCostFromLine(string line)
{
	assert(isBBLine(line));

	smatch what;
	uint32_t cost;

	uint32_t pos1 = line.find("\t", 0);
	uint32_t pos2 = line.find("\t", pos1+1);
	uint32_t pos3 = line.find("\t", pos2+1);
	uint32_t pos4 = line.find("\t", pos3+1);

	string cost_s = line.substr(pos3+1, pos4-pos3+1);
	cost = strtoul(cost_s.c_str(), NULL, 10);

//	LOG_DEBUG(logger, "found costs for line: " << line << " costs: " << cost << " ( " << pos3+1 << " ; " << pos4-pos3+1 << " ) " );

	return cost;
}


