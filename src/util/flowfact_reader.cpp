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
#include "flowfact_reader.hpp"
#include "regex_parser_tokens.h"

// regular expression to match the loop bound line
regex FlowFactReader::re_lb_line("^[[:space:]]*loop");
// regular expression to match the exact flow constraint line
regex FlowFactReader::re_sfex_line("^[[:space:]]*flow_exact");
// regular expression to match the minimum flow constraint line
regex FlowFactReader::re_sfmi_line("^[[:space:]]*flow_min");
// regular expression to match the maximum flow constraint line
regex FlowFactReader::re_sfma_line("^[[:space:]]*flow_max");
// regular expression to find the block address (usually this is given as a comment after the loop bound) within the line
regex FlowFactReader::re_bb_addr("//[[:space:]]*"HEX_TOKEN"{8}");
// regular expression to find the loop resp. static flow bound within the line 
regex FlowFactReader::re_bound("[[:space:]][0-9]+;");
// regular expression to find the loop resp. static flow bound as a constant declaration within the line 
regex FlowFactReader::re_bound_const("[[:space:]]([[:alnum:]]|_)+;");
// regular expression to find the function name in the line
regex FlowFactReader::re_func_name("\"([[:alnum:]]|_)+\"");
// regular expression to find the offset in the line (notice that the '+' has to be escaped '\\')
regex FlowFactReader::re_func_offset("[[:space:]]\\+[[:space:]]0x"HEX_TOKEN"+[[:space:]]");
// regular expression to find an additional induction basic block information (for the case that the loop head has 2 forward jumps as in edges)
regex FlowFactReader::re_inducting_bb("I:[[:space:]]*0x"HEX_TOKEN"+");
// regular expression to match a flow constant line
regex FlowFactReader::re_const_line("^[[:space:]]*const");
// regular expression to find the flow constant name
regex FlowFactReader::re_const_name("[[:space:]]([[:alnum:]]|_)+:");
// regular expression to find the flow constant value
regex FlowFactReader::re_const_value("[[:space:]][0-9]+;");

LoggerPtr FlowFactReader::logger(Logger::getLogger("FlowFactReader"));


FlowFactReader::FlowFactReader() : file_read(false)
{
	conf = Configuration::getInstance();
}

FlowFactReader::~FlowFactReader()
{
	function_table.clear();
	bb_addr_cap.clear();
	bb_ind.clear();
	bb_sflows.clear();
	const_map.clear();
}

void FlowFactReader::setFile(string filename)
{
	ff_file_name = filename;

	readFile();
}

int32_t FlowFactReader::getLoopBoundCapacity(uint32_t bb_address)
{
	if(file_read)
	{
		BbaddrCapMap::iterator pos;
		pos = bb_addr_cap.find(bb_address);
		if(pos != bb_addr_cap.end())
		{
			return pos->second;
		}
	}

	return -1;
}


static_flow_constraint_t FlowFactReader::getStaticFlowConstraint(uint32_t bb_address)
{
	static_flow_constraint_t tmp;
	tmp.flow_type = UNKNOWN;
	tmp.flow_bound = 0;
	tmp.id = 0;

	if(file_read)
	{
		BbaddrSFlowMap::iterator pos;
		pos = bb_sflows.find(bb_address);
		if(pos != bb_sflows.end())
		{
			return pos->second;
		}
	}

	return tmp;
}


void FlowFactReader::readFile(void)
{
	if(file_read)
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

		if(isLoopBoundLine(str))
		{
			LOG_DEBUG(logger, "Inspecting loop bound line: " << str);

			uint32_t function_offset_bb_addr = getBBAddrFromFunctionOffset(str);
			uint32_t bb_addr = getBBAddrFromLine(str);
			uint32_t loop_bound = getBoundFromLine(str);

			if(function_offset_bb_addr == 0x0)
			{
				LOG_WARN(logger, "Could not determine the BB address of the loop bound by function name and offset! Using address in comment of line.");
			}
			else if((bb_addr != function_offset_bb_addr))
			{
				LOG_WARN(logger, "Address (0x"<< hex << bb_addr << ") found in comment does not correspond to calculated address from function offset: 0x" << function_offset_bb_addr); 
				bb_addr = function_offset_bb_addr;
			}

			uint32_t inducting_bb;
			if(getInductingBBFromLine(str, &inducting_bb))
			{
				LOG_DEBUG(logger, "Found additional inducting edge information: 0x" << hex << inducting_bb);
				BbaddrIndBbMap::iterator p;
				bool ins;
				p = bb_ind.find(bb_addr);
				assert(p == bb_ind.end());
				tie(p, ins) = bb_ind.insert(make_pair(bb_addr, inducting_bb));
				assert(ins);
			}

//			LOG_DEBUG(logger, "processing line: " << str << " bb_addr: 0x" << hex << bb_addr << " loop_bound:" << dec << loop_bound);

			BbaddrCapMap::iterator pos;
			bool inserted;
			pos = bb_addr_cap.find(bb_addr);
			// address has NOT to be found
			assert(pos == bb_addr_cap.end());

			LOG_DEBUG(logger, "Found new loop bound for: 0x" << hex << bb_addr << " of: " << dec << loop_bound); 

			tie(pos, inserted) = bb_addr_cap.insert(make_pair(bb_addr, loop_bound));

			assert(inserted);
		}
		else if(isStaticFlowConstraintLine(str))
		{
			uint32_t function_offset_bb_addr = getBBAddrFromFunctionOffset(str);
			uint32_t bb_addr = getBBAddrFromLine(str);
			static_flow_constraint_t tmp;
			tmp.flow_bound = getBoundFromLine(str);
			tmp.flow_type = getStaticFlowType(str);


			if(function_offset_bb_addr == 0x0)
			{
				LOG_WARN(logger, "Could not determine the BB address of the loop bound by function name and offset! Using address in comment of line.");
			}
			else if((bb_addr != function_offset_bb_addr))
			{
				LOG_WARN(logger, "Address (0x"<< hex << bb_addr << ") found in comment does not correspond to calculated address from function offset: 0x" << function_offset_bb_addr); 
				bb_addr = function_offset_bb_addr;
			}

			BbaddrSFlowMap::iterator pos;
			bool inserted;
			pos = bb_sflows.find(bb_addr);
			// address has NOT to be found
			assert(pos == bb_sflows.end());

			LOG_DEBUG(logger, "Found new static flow bound for: 0x" << hex << bb_addr << " of: " << dec << tmp.flow_bound << " type: " << tmp.flow_type); 


			tie(pos, inserted) = bb_sflows.insert(make_pair(bb_addr, tmp));

			assert(inserted);
		}
		else if(isConstantDefinitionLine(str))
		{
			LOG_DEBUG(logger, "Inspecting constant definition line: " << str);

			string name = getConstNameFromLine(str);
			uint32_t value = getConstValueFromLine(str);

			LOG_DEBUG(logger, "Found constant " << name << ": " << value); 

			ConstMap::iterator pos;
			bool inserted;
			
			tie(pos, inserted) = const_map.insert(make_pair(name, value));
			
			if(!inserted)
			{
				LOG_WARN(logger, "Constant " << name << " already exists. Value was not overwritten. Old value is: " << pos->second);
			}

		}
	}
	file_read = true;
	ff_in.close();
}

bool FlowFactReader::isLoopBoundLine(string line)
{
	return regex_search(line, re_lb_line);
}

bool FlowFactReader::isStaticFlowConstraintLine(string line)
{
	return (regex_search(line, re_sfex_line) || regex_search(line, re_sfmi_line) || regex_search(line, re_sfma_line));
}

bool FlowFactReader::isConstantDefinitionLine(string line)
{
	return regex_search(line, re_const_line);
}

flowc_type_t FlowFactReader::getStaticFlowType(string line)
{
	assert(isStaticFlowConstraintLine(line));
	flowc_type_t return_val;
	if(regex_search(line, re_sfex_line))
	{
		return_val = EXACT;
	}
	else if(regex_search(line, re_sfmi_line))
	{
		return_val = MIN;
	}
	else if(regex_search(line, re_sfma_line))
	{
		return_val = MAX;
	}
	else
	{
		return_val = UNKNOWN;
		assert(false);
	}
	return return_val;
}

uint32_t FlowFactReader::getBBAddrFromLine(string line)
{
	assert(isLoopBoundLine(line) || isStaticFlowConstraintLine(line));

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

uint32_t FlowFactReader::getBoundFromLine(string line)
{
	assert(isLoopBoundLine(line) || isStaticFlowConstraintLine(line));

	smatch what;
	uint32_t bound;

	if(regex_search(line, what, re_bound, match_default))
	{
		string str = what[0];
		string str2 =  regex_replace(str, regex("[[:space:]]|;"), "", match_default | format_all);
		bound = strtoul(str2.c_str(),NULL,10);
	}
	else if(regex_search(line, what, re_bound_const, match_default))
	{
		// determining constant name
		string str = what[0];
		string str2 = regex_replace(str, regex("[[:space:]]|;"), "", match_default | format_all);

		// get value for constant name
		ConstMap::iterator pos;
		pos = const_map.find(str2);

		if(pos == const_map.end())
		{
			LOG_ERROR(logger, "Constant " << str2 << " is not defined!");
		}
		assert(pos != const_map.end()); // assert the constant is already defined

		bound = pos->second;

		LOG_DEBUG(logger, "Determined bound from constant: " << str2 << " with value: " << bound);
	}
	else
		assert(false);

	return bound;
}

uint32_t FlowFactReader::getBBAddrFromFunctionOffset(string line)
{
	smatch what;
	string function_name;
	uint32_t function_offset;

	if(regex_search(line, what, re_func_name, match_default))
	{
		string str = what[0];
		function_name = regex_replace(str, regex("\""), "", match_default | format_all);
//		LOG_DEBUG(logger, "found Function: " << function_name);
	}
	else
		assert(false);

	if(regex_search(line, what, re_func_offset, match_default))
	{
		string str = what[0];
		string str2 = regex_replace(str, regex("[[:space:]]|\\+"), "", match_default | format_all);
		function_offset = strtoul(str2.c_str(),NULL, 16);
		LOG_DEBUG(logger, "found function offset: " << hex << function_offset);
	}
	else
		assert(false);

	for(uint32_t i = 0; i < function_table.size(); i++)
	{
		if(function_name.compare(function_table[i].label) == 0)
		{
			return (function_table[i].address + function_offset);
		}
	}
			
	// error
	return 0x0;
}

bool FlowFactReader::getInductingBBFromLine(string line, uint32_t *inducting_bb_abs_addr)
{
	assert(isLoopBoundLine(line));

	smatch what;
	string function_name;
	uint32_t inducting_bb;

	if(regex_search(line, what, re_func_name, match_default))
	{
		string str = what[0];
		function_name = regex_replace(str, regex("\""), "", match_default | format_all);
//		LOG_DEBUG(logger, "found Function: " << function_name);
	}
	else
		assert(false);

	if(regex_search(line, what, re_inducting_bb, match_default))
	{
		string str = what[0];
		if(regex_search(str, what, regex("0x"HEX_TOKEN"+"), match_default))
		{
			string str2 = what[0];
			inducting_bb = strtoul(str2.c_str(),NULL,16);
		}
		else
			assert(false);
	}
	else
	{
		// no additional information found
		return false;
	}

	for(uint32_t i = 0; i < function_table.size(); i++)
	{
		if(function_name.compare(function_table[i].label) == 0)
		{
			*inducting_bb_abs_addr = (function_table[i].address + inducting_bb);
			return true;
		}
	}

	// something might went wrong
	return false;
}

void FlowFactReader::setFunctionTable(vector<addr_label_t> functionTable)
{
	function_table = functionTable;
}

bool FlowFactReader::isLoopDecisionAtTail(uint32_t bb_address)
{
	if(file_read)
	{
		BbaddrIndBbMap::iterator pos;
		pos = bb_ind.find(bb_address);
		if(pos != bb_ind.end())
		{
			return true;
		}
	}

	return false;
}

uint32_t FlowFactReader::getInductingBB(uint32_t bb_address)
{
	if(file_read)
	{
		BbaddrIndBbMap::iterator pos;
		pos = bb_ind.find(bb_address);
		if(pos != bb_ind.end())
		{
			return pos->second;
		}
	}

	return 0x0;
}

string FlowFactReader::getConstNameFromLine(string line)
{
	assert(isConstantDefinitionLine(line));

	smatch what;
	string const_name;

	if(regex_search(line, what, re_const_name, match_default))
	{
		string str = what[0];
		const_name = regex_replace(str, regex("[[:space:]]|:"), "", match_default | format_all);
	}
	else
		assert(false);

	return const_name;
}

uint32_t FlowFactReader::getConstValueFromLine(string line)
{
	assert(isConstantDefinitionLine(line));

	smatch what;
	uint32_t const_value;

	if(regex_search(line, what, re_const_value, match_default))
	{
		string str = what[0];
		string str2 = regex_replace(str, regex("[[:space:]]|;"), "", match_default | format_all);
		const_value = strtoul(str2.c_str(), NULL, 10);
	}
	else
		assert(false);

	return const_value;
}

string FlowFactReader::getHelpMessage(void)
{
	ConsoleStringPrinter csp;
	stringstream s;
	
	s << endl;
	s << "Flow fact file format" << endl;
	s << "=====================" << endl;
	s << csp.chopToLineLength("The flow fact file provides information about the control flow of the program under analysis to the analysis. Therefore, it is possible to propagate loop bounds, infeasable paths and indirect jump/call targets to the program flow analysis. The annotations are created by using relative (or absolute) addresses of the final memory image.") << endl;
	s << endl;
	s << endl << "Loop bounds:" << endl;
	s << "------------" << endl;
	s << " Classic loop bound constraint." << endl;
	s << endl;
	s << " loop \"function\" + 0xaa n; // cccccccc" << endl;
	s << endl;
	s << "  function: function in which the loop occurs" << endl;
	s << "  0xaa: address of the loop injecting jump relative to the function start address" << endl;
	s << "  n: maximum number of loop iterations (0 means the loop is executed at most once)" << endl;
	s << "     Notice: a loop body that is never executed in the worst case has to be modelled by the excact flow constraint." << endl;
	s << "  cccccccc: absolute address of the loop injecting jump (optional, required to check correctness of other addresses)" << endl;
	s << endl;
	s << " A special constraint to model loops is required, if the basic block that contains the loop injecting jump is at the tail of the loop body:" << /* "In this case the loop injecting edge cannot be clearly identified, since the tail basic block of the loop might be left by two back edged. Therefore, the loop injecting edge has to be provided to the tool." <<*/ endl;
	s << endl;
	s << " loop \"function\" + 0xaa I:0xbb n; // cccccccc" << endl;
	s << endl;
	s << "  function: function in which the loop occurs" << endl;
	s << "  0xaa: address of the loop injecting jump relative to the function start address" << endl;
	s << "  n: number of loop iterations (0 means the loop is executed at most once)" << endl;
	s << "     Notice: a loop body that is never executed in the worst case has to be modelled by the excact flow constraint." << endl;
	s << "  0xbb: relative address of the basic block that is the head of the loop body" << endl;
	s << "  cccccccc: absolute address of the loop injecting jump (optional, required to check correctness of other addresses)" << endl;
	s << endl << "Exact flows constraints:" << endl;
	s << "------------------------" << endl;
	s << " Modelling of an exact number of executions of a basic block (and all its successors)." << endl;
	s << endl;
	s << " flow_exact \"function\" + 0xaa n; // cccccccc" << endl;
	s << endl;
	s << "  function: function in which the flow is defined" << endl;
	s << "  0xaa: start address of the basic block relative to the function start address" << endl;
	s << "  n: the number of iterations the selected basic block is executed" << endl;
	s << "  cccccccc: start address of the basic block as absolute address" << endl;
	s << endl << "Minimum flows constraints:" << endl;
	s << "--------------------------" << endl;
	s << " Modelling of a minimum number of executions of a basic block (and all its successors)." << endl;
	s << endl;
	s << " flow_min \"function\" + 0xaa n; // cccccccc" << endl;
	s << endl;
	s << "  function: function in which the flow is defined" << endl;
	s << "  0xaa: start address of the basic block relative to the function start address" << endl;
	s << "  n: the number of iterations the selected basic block is executed at least" << endl;
	s << "  cccccccc: start address of the basic block as absolute address" << endl;
	s << endl << "Maximum flows constraints:" << endl;
	s << "--------------------------" << endl;
	s << " Modelling of a maximum number of executions of a basic block (and all its successors)." << endl;
	s << endl;
	s << " flow_max \"function\" + 0xaa n; // cccccccc" << endl;
	s << endl;
	s << "  function: function in which the flow is defined" << endl;
	s << "  0xaa: start address of the basic block relative to the function start address" << endl;
	s << "  n: the number of iterations the selected basic block is executed at most" << endl;
	s << "  cccccccc: start address of the basic block as absolute address" << endl;
	s << endl << "Constants:" << endl;
	s << "----------" << endl;
	s << " For the flow constraints for the number of iterations n constants can be defined:" << endl;
	s << endl;
	s << " const ITERATIONS: n;" << endl;
	s << endl;
	s << "  ITERATIONS: name of the constant" << endl;
	s << "  n: number that will be assigned to all flow constraints using this constant" << endl;
	return s.str();
}

