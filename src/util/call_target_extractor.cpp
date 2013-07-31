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
#include "call_target_extractor.hpp"
#include "regex_parser_tokens.h"

LoggerPtr CallTargetExtractor::logger(Logger::getLogger("CallTargetExtractor"));


// regular expression to match the indirect jump line 
regex CallTargetExtractor::re_calli_line("^[[:space:]]*calli");
// regular expression to find the function name in the line
regex CallTargetExtractor::re_src_func_name("\"([[:alnum:]]|_)+\"");
// regular expression to find the source offset in the line
regex CallTargetExtractor::re_src_offset("[[:space:]]from[[:space:]]0x"HEX_TOKEN"+[[:space:]]");
// regular expression to find the target address in the line
regex CallTargetExtractor::re_tgt_address("//[[:space:]]*0x"HEX_TOKEN"+");
// regular expression to find the target function name in the line (it is given as a comment)
regex CallTargetExtractor::re_tgt_func_name("[[:space:]]to[[:space:]]([[:alnum:]]|_)+\\(\\)");


CallTargetExtractor::CallTargetExtractor() : file_read(false)
{
	conf = Configuration::getInstance();
	ff_file_name = conf->getString(CONF_FLOWFACT_FILE);
	use_ff = conf->getBool(CONF_USE_FLOWFACT_FILE);
}

CallTargetExtractor::~CallTargetExtractor()
{
	cmap.clear();
	targets.clear();
	function_table.clear();
}

void CallTargetExtractor::readFile(void)
{
	if(file_read || !use_ff)
		return;

	ifstream ff_in;


	ff_in.open(ff_file_name.c_str());


	if(ff_in.fail())
	{
		LOG_WARN(logger, "File " << ff_file_name <<" cannot be opened");
		return;
	}
	assert(ff_in.is_open());

	while(!ff_in.eof())
	{
		char c[1024];
		ff_in.getline(c, 1024);
		const string str = c;

		if(isCalliLine(str))
		{
			LOG_DEBUG(logger, "Line: " << str);

			uint32_t icall_addr = getICallAddrFromLineWithOffset(str);
			uint32_t tgt_address = getTargetAddrFromLine(str);
			string tgt_name = getTargetNameFromLine(str);

			if(icall_addr == 0x0)
			{
				LOG_WARN(logger, "Could not determine the address of the indirect call in the loop bound.");
				assert(false);
			}

			bool found_in_table = false;
			for(vector<addr_label_t>::iterator it = function_table.begin(); (it != function_table.end() && !found_in_table); it++)
			{
				if(tgt_name.compare(it->label) == 0)
				{
					found_in_table = true;
					if(tgt_address != it->address)
					{
						LOG_WARN(logger, "Function target address from flow facts (0x" << hex << tgt_address << ") does not correspond to address determined during parsing (0x"<< it->address << "). Using parsed address.");
						tgt_address = it->address;
					}
				}
			}

			CallTargetMap::iterator pos;
			bool inserted;

			pos = cmap.find(icall_addr);

			if(pos == cmap.end())
			{
				vector<addr_label_t> v;
				addr_label_t al = { tgt_address, tgt_name};
				v.push_back(al);
				tie(pos, inserted) = cmap.insert(make_pair(icall_addr, v));
				assert(inserted);
				LOG_DEBUG(logger, "Adding call target (0x" << hex << tgt_address << ", " << tgt_name << ") to calling bb (0x" << icall_addr << ")");
			}
			else
			{
				addr_label_t al = { tgt_address, tgt_name};
				pos->second.push_back(al);
				LOG_DEBUG(logger, "Adding another call target (0x" << hex << tgt_address << ", " << tgt_name << ") to calling bb (0x" << icall_addr << ")");
			}

//			addTarget(tgt_address);
		}
	}
	file_read = true;
	ff_in.close();
}

bool CallTargetExtractor::isCalliLine(string line)
{
	return regex_search(line, re_calli_line);
}

string CallTargetExtractor::getTargetNameFromLine(string line)
{
	assert(isCalliLine(line));

	smatch what;
	string tgt_name;

	if(regex_search(line, what, re_tgt_func_name, match_default))
	{
		string str = what[0];
		tgt_name =  regex_replace(str, regex("[[:space:]]|to|\\(|\\)"), "", match_default | format_all);
	}
	else
		assert(false);

	return tgt_name;
}

uint32_t CallTargetExtractor::getSourceOffsetFromLine(string line)
{
	assert(isCalliLine(line));

	smatch what;
	uint32_t src_offset;

	if(regex_search(line, what, re_src_offset, match_default))
	{
		string str = what[0];
		string str2 = regex_replace(str, regex("[[:space:]]|from"), "", match_default | format_all);
		src_offset = strtoul(str2.c_str(),NULL, 16);
		LOG_DEBUG(logger, "found src offset: " << hex << src_offset);
	}
	else
		assert(false);

	return src_offset;
}

uint32_t CallTargetExtractor::getTargetAddrFromLine(string line)
{
	assert(isCalliLine(line));

	smatch what;
	uint32_t tgt_address;

	if(regex_search(line, what, re_tgt_address, match_default))
	{
		string str = what[0];
		string str2 = regex_replace(str, regex("[[:space:]]|/"), "", match_default | format_all);
		tgt_address = strtoul(str2.c_str(),NULL, 16);
		LOG_DEBUG(logger, "found tgt addr: " << hex << tgt_address);
	}
	else
		assert(false);

	return tgt_address;
}

uint32_t CallTargetExtractor::getICallAddrFromLineWithOffset(string line)
{
	smatch what;
	string src_function_name;
	uint32_t src_offset;

	if(regex_search(line, what, re_src_func_name, match_default))
	{
		string str = what[0];
		src_function_name = regex_replace(str, regex("\""), "", match_default | format_all);
//		LOG_DEBUG(logger, "found Function: " << function_name);
	}
	else
		assert(false);

	src_offset = getSourceOffsetFromLine(line);

	for(uint32_t i = 0; i < function_table.size(); i++)
	{
		if(src_function_name.compare(function_table[i].label) == 0)
		{
			return (function_table[i].address + src_offset);
		}
	}
			
	// error
	return 0x0;
}

vector<addr_label_t> CallTargetExtractor::getCallTargetsForIndirectCall(uint32_t bb_address)
{
	vector<addr_label_t> v;

	if(file_read && use_ff)
	{
		CallTargetMap::iterator pos;
		pos = cmap.find(bb_address);
		if(pos != cmap.end())
		{
			return pos->second;
		}
	}
	return v;
}

void CallTargetExtractor::setFunctionTable(vector<addr_label_t> functionTable)
{
	function_table = functionTable;
	readFile();
}

string CallTargetExtractor::getHelpMessage(void)
{
	stringstream s;
	s << "Modelling indirect calls:" << endl;
	s << "-------------------------" << endl;
	s << endl;
	s << " calli \"src_function\" from 0xaa to tgt_function() // cccccccc" << endl;
	s << endl;
	s << "  src_function: the function name in which the indirect jump occurs" << endl;
	s << "  0xaa: address of the indirect jump (relative to function start address)" << endl;
	s << "  tgt_function: a function which is possibly called by this indirect call" << endl;
	s << "  cccccccc: absolute address of the target function (optional, required to check correctness of other addresses)" << endl;
	s << endl;
	s << " Multiple call targets can be modelled by multiple \"calli\" constraints." << endl;

	return s.str();
}

