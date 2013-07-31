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
#include "jump_target_extractor.hpp"
#include "regex_parser_tokens.h"

LoggerPtr JumpTargetExtractor::logger(Logger::getLogger("JumpTargetExtractor"));


// regular expression to match the indirect jump line 
regex JumpTargetExtractor::re_ji_line("^[[:space:]]*jind");
// regular expression to find the block address (it is given as a comment) within the line
regex JumpTargetExtractor::re_bb_addr("//[[:space:]]*"HEX_TOKEN"{8}");
// regular expression to find the function name in the line
regex JumpTargetExtractor::re_func_name("\"([[:alnum:]]|_)+\"");
// regular expression to find the source offset in the line
regex JumpTargetExtractor::re_src_offset("[[:space:]]from[[:space:]]0x"HEX_TOKEN"+[[:space:]]");
// regular expression to find the target offset in the line
regex JumpTargetExtractor::re_tgt_offset("[[:space:]]to[[:space:]]0x"HEX_TOKEN"+[[:space:]]");


JumpTargetExtractor::JumpTargetExtractor() : file_read(false)
{
	conf = Configuration::getInstance();
	ff_file_name = conf->getString(CONF_FLOWFACT_FILE);
	use_ff = conf->getBool(CONF_USE_FLOWFACT_FILE);
}

JumpTargetExtractor::~JumpTargetExtractor()
{
	jmap.clear();
	targets.clear();
	function_table.clear();
}

void JumpTargetExtractor::readFile(void)
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

		if(isJindLine(str))
		{
			LOG_DEBUG(logger, "Line: " << str);

			uint32_t bb_addr = getBBAddrFromLineWithOffset(str);
			uint32_t bb_addr_abs = getBBAddrFromLine(str);
			uint32_t src_offset = getSourceOffsetFromLine(str);
			uint32_t tgt_offset = getTargetOffsetFromLine(str);

			if(bb_addr == 0x0)
			{
				LOG_WARN(logger, "Could not determine the BB address of the loop bound by function name and offset! Using address in comment of line.");
			}
			else if((bb_addr != bb_addr_abs))
			{
				LOG_WARN(logger, "Address (0x"<< hex << bb_addr_abs << ") found in comment does not correspond to calculated address from function offset: 0x" << bb_addr); 
			}

			JumpTargetMap::iterator pos;
			bool inserted;

			pos = jmap.find(bb_addr);

			if(pos == jmap.end())
			{
				vector<uint32_t> v;
				v.push_back(bb_addr - src_offset + tgt_offset);
				tie(pos, inserted) = jmap.insert(make_pair(bb_addr, v));
				assert(inserted);
				LOG_DEBUG(logger, "Adding jump target (0x" << hex << bb_addr - src_offset + tgt_offset << ") to jumping bb (0x" << bb_addr << ")");
			}
			else
			{
				pos->second.push_back(bb_addr - src_offset + tgt_offset);
				LOG_DEBUG(logger, "Adding another jump target (0x" << hex << bb_addr - src_offset + tgt_offset << ") to jumping bb (0x" << bb_addr << ")");
			}

			addTarget(bb_addr - src_offset + tgt_offset);
		}
	}
	file_read = true;
	ff_in.close();
}

bool JumpTargetExtractor::isJindLine(string line)
{
	return regex_search(line, re_ji_line);
}

uint32_t JumpTargetExtractor::getBBAddrFromLine(string line)
{
	assert(isJindLine(line));

	smatch what;
	uint32_t bb_addr;

	if(regex_search(line, what, re_bb_addr, match_default))
	{
		string str = what[0];
		string str2 =  regex_replace(str, regex("[[:space:]]|/"), "", match_default | format_all);
		bb_addr = strtoul((str2.c_str()),NULL,16);
	}
	else
		assert(false);

	return bb_addr;
}

uint32_t JumpTargetExtractor::getSourceOffsetFromLine(string line)
{
	assert(isJindLine(line));

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

uint32_t JumpTargetExtractor::getTargetOffsetFromLine(string line)
{
	assert(isJindLine(line));

	smatch what;
	uint32_t tgt_offset;

	if(regex_search(line, what, re_tgt_offset, match_default))
	{
		string str = what[0];
		string str2 = regex_replace(str, regex("[[:space:]]|to"), "", match_default | format_all);
		tgt_offset = strtoul(str2.c_str(),NULL, 16);
		LOG_DEBUG(logger, "found tgt offset: " << hex << tgt_offset);
	}
	else
		assert(false);

	return tgt_offset;
}

uint32_t JumpTargetExtractor::getBBAddrFromLineWithOffset(string line)
{
	smatch what;
	string function_name;
	uint32_t src_offset;

	if(regex_search(line, what, re_func_name, match_default))
	{
		string str = what[0];
		function_name = regex_replace(str, regex("\""), "", match_default | format_all);
//		LOG_DEBUG(logger, "found Function: " << function_name);
	}
	else
		assert(false);

	src_offset = getSourceOffsetFromLine(line);

	for(uint32_t i = 0; i < function_table.size(); i++)
	{
		if(function_name.compare(function_table[i].label) == 0)
		{
			return (function_table[i].address + src_offset);
		}
	}
			
	// error
	return 0x0;
}

vector<uint32_t> JumpTargetExtractor::getJumpTargetsForIndirectJump(uint32_t bb_address)
{
	vector<uint32_t> v;

	if(file_read && use_ff)
	{
		JumpTargetMap::iterator pos;
		pos = jmap.find(bb_address);
		if(pos != jmap.end())
		{
			return pos->second;
		}
	}
	return v;
}

void JumpTargetExtractor::setFunctionTable(vector<addr_label_t> functionTable)
{
	function_table = functionTable;
	readFile();
}

vector<uint32_t> JumpTargetExtractor::getAllTargets(void)
{
	return targets;
}

void JumpTargetExtractor::addTarget(uint32_t tgt)
{
	vector<uint32_t>::iterator it;
	for ( it=targets.begin() ; it < targets.end(); it++ )
	{
		if(*it == tgt)
		{
			return;
		}
	}
	targets.push_back(tgt);

}

string JumpTargetExtractor::getHelpMessage(void)
{
	stringstream s;
	s << "Modelling indirect jump targets:" << endl;
	s << "--------------------------------" << endl;
	s << endl;
	s << " jind  \"function\" from 0xaa to 0xbb // cccccccc" << endl;
	s << endl;
	s << "  function: the function name in which the indirect jump occurs" << endl;
	s << "  0xaa: start address of the basic block containing the indirect jump (relative to function start address)" << endl;
	s << "  0xbb: a possible address of the jump target (relative to the function start address)" << endl;
	s << "  cccccccc: absolute address of the basic block containing the indirect jump (optional, required to check correctness of other addresses)" << endl;
	s << endl;
	s << " Multiple jump targets can be modelled by multiple \"jind\" constraints." << endl;

	return s.str();
}

