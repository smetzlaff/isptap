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
#include "disp_instrumentator.hpp"
#include "dlp_factory.hpp"


bool sort_function_graph_t_by_benefit(function_graph_t a, function_graph_t b){return (a.address < b.address);}
#define sort_by_start_addr(x) sort(x.begin(), x.end(), sort_function_graph_t_by_benefit)

bool sort_strings_by_addr(string a, string b){
	DumpLineParser *dlp = DLPFactory::getInstance()->getDLPObject();
	assert((dlp->isCodeLine(a)) || (dlp->isLabelLine(a))); 
	assert((dlp->isCodeLine(b)) || (dlp->isLabelLine(b))); 
	return ((dlp->isCodeLine(a))?(dlp->getAddrFromCodeLine(a)):(dlp->getAddrAndLabelFromLabelLine(a).address)) < ((dlp->isCodeLine(a))?(dlp->getAddrFromCodeLine(a)):(dlp->getAddrAndLabelFromLabelLine(a).address));
}
#define sort_by_addr(x) sort(x.begin(), x.end(), sort_strings_by_addr)

LoggerPtr DISPInstrumentator::logger(Logger::getLogger("DISPInstrumentator"));

DISPInstrumentator::DISPInstrumentator()
{
	dlp = DLPFactory::getInstance()->getDLPObject();
}

DISPInstrumentator::~DISPInstrumentator()
{
	original_functions.clear();
	instrumented_functions.clear();
	unparsed_lines.clear();
}

void DISPInstrumentator::addFunction(function_graph_t cfgo)
{
	original_functions.push_back(cfgo);
}
void DISPInstrumentator::addFunctions(vector<function_graph_t> vec_cfgo)
{
	for(uint32_t i = 0; i < vec_cfgo.size(); i++)
	{
		original_functions.push_back(vec_cfgo[i]);
	}
}
void DISPInstrumentator::instrumentFunctions(void)
{
	vector<uint32_t> start_addrs;
	vector<uint32_t> end_addrs;

	// align all functions to 64 bit addresses
	uint32_t bit_alignment = BIT_ALIGNMENT;

	// sort the functions by start address
	sort_by_start_addr(original_functions);

	for(uint32_t i = 0; i < original_functions.size(); i++)
	{
		CFGManipulator cfgm;
		cfgm.setCFG(&(original_functions[i]));
		LOG_DEBUG(logger, "Instrumenting and aligning (to " << bit_alignment << " ): " << original_functions[i].name << " at 0x" << hex << original_functions[i].address);
		// insert into every function the FLE instruction and align the functions to 64 bit addresses ( the lowest 3 bits cannot be used for a function start address)
		cfgm.insertFunctionLengthEncoding();
		if(i==0)
		{
			cfgm.alignCode(bit_alignment);
		}
		else
		{
			if(original_functions[i].address < end_addrs[i-1])
			{
				// if there is a code overlapping (due to alignment changes or insertion of the DISP instrumentation instruction) relocate the code
				uint32_t alignment = (1 << bit_alignment);
				uint32_t new_start_addr = ((end_addrs[i-1] & (~(alignment-1))) + alignment);
				cfgm.relocateCode(new_start_addr);
			}
			else
			{
				// otherwise just align it
				cfgm.alignCode(bit_alignment);
			}
		}
		start_addrs.push_back(cfgm.getStartAddr());
		end_addrs.push_back(cfgm.getEndAddr());
		instrumented_functions.push_back(cfgm);
		LOG_DEBUG(logger, "New address for " << original_functions[i].name << " is 0x" << hex << start_addrs.back() << " to 0x" << end_addrs.back());
	}

	// update function calls, if functions have moved
	for(uint32_t i = 0; i < original_functions.size(); i++)
	{
		if(start_addrs[i] != original_functions[i].address)
		{
			LOG_DEBUG(logger, "Function address changed for " << original_functions[i].name << " from 0x" << hex << original_functions[i].address << " to 0x" << start_addrs[i] );
			addr_label_t new_function_addr;
			new_function_addr.address = start_addrs[i];
			new_function_addr.label = original_functions[i].name;
			for(uint32_t j = 0; j < instrumented_functions.size(); j++)
			{
				instrumented_functions[j].updateCall(new_function_addr);
			}
		}

	}

	// update the DISP activation instruction
	FunctionMappingMap fmMap;
	for(uint32_t i = 0; i < original_functions.size(); i++)
	{
		fmMap.insert(make_pair(original_functions[i].address, start_addrs[i]));
	}
	
	bool finished = false;
	for(uint32_t i = 0; (i < instrumented_functions.size() && !finished); i++)
	{
		LOG_DEBUG(logger, "Trying to update DISP activation in function 0x" << hex << instrumented_functions[i].getStartAddr());
		finished = instrumented_functions[i].updateDISPActivation(fmMap);
	}


	// add unparsed lines!
	code_start = instrumented_functions.front().getStartAddr();
	code_end = instrumented_functions.back().getEndAddr();

	for(uint32_t i = 0; i < unparsed_lines.size(); i++)
	{

		if(dlp->isCodeLine(unparsed_lines[i]))
		{
			uint32_t address = dlp->getAddrFromCodeLine(unparsed_lines[i]);
			if(address < code_start)
			{
				// do nothing this is startup code
			}
			else 
			{
				// relocate everything after the code end

				assert(false); // this is not allowed to happen
				// because call addresses may have changed and jumps have to be uptated

				string instr_raw = dlp->getInstructionFromCodeLine(unparsed_lines[i]);
				string instr_dump = dlp->getCommentFromCodeLine(unparsed_lines[i]);
				// move the code after the code end
				uint32_t new_address = address+(code_end-address);
				unparsed_lines[i] = dlp->assembleCodeLine(new_address, instr_raw, instr_dump);
			}
		}
		else if(dlp->isLabelLine(unparsed_lines[i]))
		{
			addr_label_t label = dlp->getAddrAndLabelFromLabelLine(unparsed_lines[i]);
			if(label.address < code_start)
			{
				// do nothing this is startup code
			}
			else 
			{
				// relocate everything after the code end

				assert(false); // this is not allowed to happen
				// because call addresses may have changed and jumps have to be uptated

				string instr_raw = dlp->getInstructionFromCodeLine(unparsed_lines[i]);
				string instr_dump = dlp->getCommentFromCodeLine(unparsed_lines[i]);
				// move the code after the code end
				addr_label_t new_label;
				new_label.address = label.address+(code_end-label.address);
				new_label.label = label.label;
				unparsed_lines[i] = dlp->assembleLabelLine(new_label);
			}
		}
	}
}

vector<ControlFlowGraph> DISPInstrumentator::getInstrumentedFunctions(void)
{
	vector<ControlFlowGraph> result;

	for(uint32_t i = 0; i < instrumented_functions.size(); i++)
	{
		result.push_back(instrumented_functions[i].getCFG());
	}
	return result;
}


void DISPInstrumentator::addUnparsedLines(vector<string> lines)
{
	for(uint32_t i = 0; i < lines.size(); i++)
	{
		if(dlp->isCodeLine(lines[i]) || dlp->isLabelLine(lines[i]))
		{
			unparsed_lines.push_back(lines[i]);
		}
	}
	sort_by_addr(unparsed_lines);
}

vector<string> DISPInstrumentator::getUnparsedLines(void)
{
	return unparsed_lines;
}

vector<addr_name_size_t> DISPInstrumentator::getMetaDataOfInstrumentedFunctions(void)
{
	vector<addr_name_size_t> result;

	for(uint32_t i = 0; i < instrumented_functions.size(); i++)
	{
		result.push_back(instrumented_functions[i].getFunctionMetaData());
	}
	return result;

}

