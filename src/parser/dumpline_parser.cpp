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
#include "dumpline_parser.hpp"

LoggerPtr DumpLineParser::logger(Logger::getLogger("DumpLineParser"));

bool DumpLineParser::isCodeLine(string str)
{
//	LOG_DEBUG(logger, "Checking " << str << " with " << re_addr.str());
	return regex_search(str, re_addr);
}

bool DumpLineParser::isLabelLine(string str)
{
	return regex_search(str, re_label);
}

bool DumpLineParser::isIgnoreLabel(string UNUSED_PARAMETER(str))
{
	// do not ignore anything
	return false;
}

bool DumpLineParser::isMemoryHole(string str)
{
	return regex_search(str, re_memory_hole);
}

uint32_t DumpLineParser::getAddrFromCodeLine(string str)
{
	return  strtoul(((str.substr(0, str.find(":"))).c_str()),NULL,16);
}

addr_label_t DumpLineParser::getAddrAndLabelFromLabelLine(string str)
{
	addr_label_t tmp;
	tmp.address = strtoul(((str.substr(0, str.find(" "))).c_str()),NULL,16);
	// get the label and delete surrounding "<???>:"
	tmp.label = regex_replace(str.substr(str.find(" ")+1, (str.find(">") - str.find(" ") + 1)), regex("<|>|:"), "", match_default | format_all);
	return tmp;
}

string DumpLineParser::getInstructionFromCodeLine(string str)
{
	match_flag_type flags = match_default;
	smatch what;
	if(regex_search(str, what, re_opcode, flags))
	{
		string s2 = what[0];
		string opcode = regex_replace(s2, regex("[[:space:]]"), "", match_default | format_all);
		return opcode;
	}
	else
		return "";
}

string DumpLineParser::assembleLabelLine(addr_label_t label)
{
	stringstream result;
	char address[8+1];
	sprintf(address, "%x", label.address);
	result << address <<  " " << "<" << label.label << ">:";

	return result.str();
}

