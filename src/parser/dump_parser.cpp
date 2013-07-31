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
#include "dump_parser.hpp"


/**
 * \brief Defines the end of the code section in the dump file.
 */
#define END_OF_CODE "Disassembly of section .rodata:"


LoggerPtr DumpParser::logger(Logger::getLogger("DumpParser"));

DumpParser::DumpParser(uint32_t UNUSED_PARAMETER(core_number))
{
	conf = Configuration::getInstance();
	input_filename =  conf->getString(CONF_DUMP_FILE);

	entry_function.label = conf->getString(CONF_ENTRY_FUNCTION);
	entry_function.address = 0; // the address of the entry function is determined while extractJumpLabels()


	if((analysis_metric_t) conf->getUint(CONF_USE_METRIC) == WCET_RATIO_FILES)
	{
		rfr_onchip = new RatioFileReader();
		rfr_onchip->setFile(conf->getString(CONF_RATIO_FILE_ONCHIP));

		rfr_offchip = new RatioFileReader();
		rfr_offchip->setFile(conf->getString(CONF_RATIO_FILE_OFFCHIP));
	}
	else
	{
		rfr_onchip =NULL;
		rfr_offchip =NULL;
	}
}

DumpParser::~DumpParser(void)
{
	jump_target_vec.clear();
	label_vec.clear();
	blacklist_label.clear();
	for(uint i = 0; i < func_cfgs.size(); i++)
	{
		delete(func_cfgs[i].cfg);
	}
	func_cfgs.clear();

	if((analysis_metric_t) conf->getUint(CONF_USE_METRIC) == WCET_RATIO_FILES)
	{
		delete(rfr_onchip);
		delete(rfr_offchip);
	}

	unparsed_file_content.clear();
	data_section.clear();

	dlp = NULL;
	isa = NULL;
}

bool DumpParser::parse(void)
{
	loadFile();
	extractJumpLabels();

	jte.setFunctionTable(label_vec);
	addIndirectJumpTargets(jte.getAllTargets());

	cte.setFunctionTable(label_vec);

	LOG_DEBUG(logger, " completed reading dump: found " << dec << jump_target_vec.size() << " jump targets and " << label_vec.size() << " labels.");


	if(logger->isDebugEnabled())
	{
		ostringstream debug;
		debug << "jump targets: ";
		for(uint i = 0; i < jump_target_vec.size(); i++)
			debug << hex << "0x" <<  jump_target_vec[i] << " ";
		LOG_DEBUG(logger, debug.str()); 
	}


	if(logger->isDebugEnabled())
	{
		ostringstream debug;
		debug << "labels: ";
		for(uint i = 0; i < label_vec.size(); i++)
			debug << hex << "0x" <<  label_vec[i].address << " ";
		LOG_DEBUG(logger, debug.str()); 
	}

	parseLabels();

	connectFunctionCFGs();

	return true;
}

void DumpParser::parseLabels(void)
{
	vector<addr_label_t> process;
	vector<uint32_t> processed;

	if(entry_function.address == 0)
	{
		LOG_ERROR(logger, "Sorry the entry point: " << entry_function.label << " was not identified in the dump file. Cannot start creation of CFGs");
		assert(false);
	}

	process.push_back(entry_function);

	while(process.size() != 0)
	{
		addr_label_t actual = process.front();

		function_graph_t tmp;
		tmp.address = actual.address;
		tmp.name = actual.label;
		bool label_blacklisted = false;
		bool label_already_processed = false;

		for(vector<addr_label_t>::iterator it = blacklist_label.begin(); it != blacklist_label.end(); it++)
		{
			if(it->address == actual.address)
			{
				assert(it->label.compare(actual.label)==0);
				label_blacklisted = true;
				break;
			}
			else
			{
				// be sure that address and label are the same
				assert(it->label.compare(actual.label)!=0);
			}
		}

		for(vector<uint32_t>::iterator it=processed.begin(); it != processed.end(); it++)
		{
			if(*it == actual.address)
			{
				label_already_processed = true;
			}

		}

		if(!label_blacklisted && !label_already_processed)
		{
			LOG_INFO(logger, "Creating cfg for label " << tmp.name << " at 0x" << hex << tmp.address);
			tmp.cfg = parseCfgForLabel(actual);
			func_cfgs.push_back(tmp);
		}

		process.erase(process.begin());

		if(!label_already_processed)
		{
			processed.push_back(actual.address);

			vector<addr_label_t> call_targets = tmp.cfg->getCallTargets();
			for(vector<addr_label_t>::iterator it = call_targets.begin(); it != call_targets.end(); it++)
			{
				LOG_DEBUG(logger, "Adding called label " << it->label << " at 0x" << hex << it->address);
				process.push_back(*it);
			}
		}
	}
}


void DumpParser::loadFile(void)
{
	LOG_INFO(logger, "opening file: " << input_filename);
	ifstream input_dump;
	input_dump.open(input_filename.c_str());

	assert(input_dump.is_open());

	while(!input_dump.eof()) 
	{ 
		string str;
		getline(input_dump, str);
		unparsed_file_content.push_back(str);
	}
}


bool DumpParser::extractJumpLabels(void)
{
	for(uint32_t dump_line=0; dump_line < unparsed_file_content.size(); dump_line++)
	{ 
		const string str = unparsed_file_content[dump_line];
		LOG_DEBUG(logger, "parsing line: " << str);
		if(str.compare(END_OF_CODE)==0)
		{
			copyDataSections(dump_line);
			return true;
		}
		if(dlp->isCodeLine(str))
		{
			uint32_t curr_addr = dlp->getAddrFromCodeLine(str);
			LOG_DEBUG(logger,  " Code line with address found: 0x" << hex << curr_addr << dec);
			if(dlp->isBranchInstr(str))
			{
				LOG_DEBUG(logger, " end of BB ");
				string instruction = dlp->getInstructionFromCodeLine(str);
				if(instruction.compare("") != 0)
				{
					jump_target_address_t target = isa->getJumpTargetAddr(instruction, curr_addr);
					if(target.valid)
					{
						jump_target_vec.push_back(target.addr);
					}
				}
			}
		}
		else if(dlp->isLabelLine(str))
		{
			addr_label_t tmp = dlp->getAddrAndLabelFromLabelLine(str);
			LOG_DEBUG(logger,  "Label found:  addr: 0x" << hex << tmp.address << " label: " << tmp.label );
			if(tmp.label.compare(entry_function.label) == 0)
			{
				LOG_DEBUG(logger, "Found the address of the analysis entry point.");
				entry_function.address = tmp.address;
			}
			label_vec.push_back(tmp);
		}
	}
	return true;
}

ControlFlowGraphObject *DumpParser::parseCfgForLabel(addr_label_t function)
{
	ControlFlowGraphObject *cfgobj = new ControlFlowGraphObject();

	if((analysis_metric_t) conf->getUint(CONF_USE_METRIC) == WCET_RATIO_FILES)
	{
		cfgobj->setRatioFileReaders(rfr_onchip, rfr_offchip);
	}

	bool function_start_found = false;
	uint32_t dump_line = 0;
	for(dump_line=0; (dump_line < unparsed_file_content.size())&&(!function_start_found); dump_line++)
	{	
		const string str = unparsed_file_content[dump_line];
		if(dlp->isLabelLine(str))
		{
			addr_label_t tmp = dlp->getAddrAndLabelFromLabelLine(str);
			if(tmp.address == function.address)
			{
				assert(!function.label.compare(tmp.label));
				function_start_found = true;
			}
		}

	}

	bool in_bb = false;
	string bb_code;
	uint32_t bb_start=0;
	uint32_t bb_size=0;
	uint32_t bb_instrs=0;
	bool finished = false;
	dump_line--;
	uint32_t parsed_dump_lines_begin=dump_line;
	uint32_t parsed_dump_lines_end=dump_line;
	uint32_t next_instr_address=0x0;
	uint32_t mem_hole_start_addr=0x0;
	bool detected_mem_hole=false;

	LOG_DEBUG(logger, "Starting parse for " << function.label << " at " << unparsed_file_content[dump_line]);

	for(/* dump_line: start with value where the label was found */; (dump_line < unparsed_file_content.size())&&(!finished); dump_line++)
	{
		const string str = unparsed_file_content[dump_line];
		parsed_dump_lines_end=dump_line;
//		LOG_DEBUG(logger, str);
		if(dlp->isCodeLine(str))
		{
			LOG_DEBUG(logger, "found instruction: " << str);
			string instruction = dlp->getInstructionFromCodeLine(str);
			uint32_t op_length = isa->getInstructionLength(instruction);
			uint32_t curr_addr = dlp->getAddrFromCodeLine(str);
			next_instr_address = curr_addr+op_length;
			bool is_jump_target = false;
			for(uint i = 0; i < jump_target_vec.size(); i++)
			{
				if(jump_target_vec[i] == curr_addr+op_length)
				{
					is_jump_target = true;
					break;
				}
			}

			if(!in_bb)
			{
				// start of a new bb
				in_bb = true;
				if(detected_mem_hole)
				{
					// special case if a memory hole is at the beginning of a basic block
					bb_start = mem_hole_start_addr;
				}
				else
				{
					bb_start = curr_addr;
				}
			}

			// code to cram memory holes within a function
			if(detected_mem_hole)
			{
				assert((curr_addr - mem_hole_start_addr) % 2 == 0); // be sure
				uint32_t needed_noops_to_fill_mem_hole = (curr_addr  - mem_hole_start_addr) / 2;
				LOG_DEBUG(logger, "Found memory hole in a function, cram it. Need " << needed_noops_to_fill_mem_hole << " noops to brigde: 0x" << hex << curr_addr << " to 0x" << mem_hole_start_addr << ".");
				for(uint32_t i=0; i < needed_noops_to_fill_mem_hole; i++)
				{
					bb_code += dlp->assembleCodeLine(mem_hole_start_addr+2*i, "0000", "nop to cram memory hole");
					bb_code += "\r";
					bb_size += 2;
					bb_instrs++;
				}
				detected_mem_hole=false;
			}

			bb_code += str;
			bb_code += "\r"; /// adding carriage return to ease parsing of code for raw file creation
			bb_size += isa->getInstructionLength(dlp->getInstructionFromCodeLine(str));
			bb_instrs++;
			bool is_branch = dlp->isBranchInstr(str);
//			LOG_DEBUG(logger, "is_branch: " << is_branch << " is_jump_target: " << is_jump_target);
			if(is_branch || is_jump_target)
			{
				// end of bb
				if(is_branch)
				{
					if(dlp->isCondBranchInstr(str))
					{
						LOG_DEBUG(logger, "bb creation on conditional branch: ");
						jump_target_address_t target = isa->getJumpTargetAddr(instruction, curr_addr);
						if(!target.valid)
						{
							LOG_WARN(logger, "In function " << function.label << " (0x" << hex << function.address << ") cannot obtain jump target addr using addr 0x" << hex << UNKNOWN_ADDR << " instead");
							vector<uint32_t> jump_targets = jte.getJumpTargetsForIndirectJump(bb_start);
							if(jump_targets.size() == 0)
							{
								cfgobj->addBBNode(bb_start, curr_addr, curr_addr+op_length, UNKNOWN_ADDR, bb_code, bb_size, bb_instrs);
							}
							else
							{
								for(uint32_t i=0; i < jump_targets.size(); i++)
								{
									LOG_INFO(logger, "Using JTE: found the target: 0x" << hex << jump_targets[i] << " to which a jump from 0x" << bb_start << " is possible");
								}
								cfgobj->addBBNode(bb_start, curr_addr, curr_addr+op_length, jump_targets, bb_code, bb_size, bb_instrs);
							}
						}
						else
							cfgobj->addBBNode(bb_start, curr_addr, curr_addr+op_length, target.addr, bb_code, bb_size, bb_instrs);
					}
					else if(dlp->isReturnInstr(str))
					{
						LOG_DEBUG(logger, "bb creation on return: ");
						cfgobj->addBBNode(bb_start, curr_addr, bb_code, bb_size, bb_instrs);
					}
					else if(dlp->isCallInstr(str))
					{
						addr_label_t tmp;
						//TODO do not connect call targets as normal edges
						LOG_DEBUG(logger, "bb creation on call: ");
						jump_target_address_t target = isa->getJumpTargetAddr(instruction, curr_addr);
						if(!target.valid)
						{
							vector<addr_label_t> call_targets = cte.getCallTargetsForIndirectCall(curr_addr);
							if(call_targets.size() == 0)
							{
								tmp.address = UNKNOWN_ADDR;
								tmp.label = "NOLABEL";
								LOG_WARN(logger, "In function " << function.label << " (0x" << hex << function.address << ") cannot obtain call target addr using addr 0x" << hex << UNKNOWN_ADDR << " instead");
								cfgobj->addBBCallNode(bb_start, curr_addr, curr_addr+op_length, bb_code, bb_size, bb_instrs, tmp);
							}
							else
							{
								for(uint32_t i=0; i < call_targets.size(); i++)
								{
									LOG_INFO(logger, "Using CTE: found the target: 0x" << hex << call_targets[i].address << " to which a call from 0x" << bb_start << " is possible");
								}
								cfgobj->addBBCallNode(bb_start, curr_addr, curr_addr+op_length, bb_code, bb_size, bb_instrs, call_targets);
								assert(false); // this is untested (added code during merge of carcore and armv6m)
							}
						}
						else
						{
							tmp = getAddrLabelForAddr(target.addr);
							cfgobj->addBBCallNode(bb_start, curr_addr, curr_addr+op_length, bb_code, bb_size, bb_instrs, tmp);
						}
					}
					else 
					{
						LOG_DEBUG(logger, "bb creation on unconditional branch: ");
						jump_target_address_t target = isa->getJumpTargetAddr(instruction, curr_addr);
						if(!target.valid)
						{
							LOG_WARN(logger, "In function " << function.label << " (0x" << hex << function.address << ") cannot obtain jump target addr using addr 0x" << hex << UNKNOWN_ADDR <<" instead");
							vector<uint32_t> jump_targets = jte.getJumpTargetsForIndirectJump(bb_start);
							if(jump_targets.size() == 0)
							{
								cfgobj->addBBNode(bb_start, curr_addr, UNKNOWN_ADDR, bb_code, bb_size, bb_instrs);
							}
							else
							{
								for(uint32_t i=0; i < jump_targets.size(); i++)
								{
									LOG_INFO(logger, "Using JTE: found the target: 0x" << hex << jump_targets[i] << " to which a jump from 0x" << bb_start << " is possible");
								}
								cfgobj->addBBNode(bb_start, curr_addr, jump_targets, bb_code, bb_size, bb_instrs);
							}
						}
						else
							cfgobj->addBBNode(bb_start, curr_addr, target.addr, bb_code, bb_size, bb_instrs);
					}
					// check if all basic blocks are created for this function 
					finished = cfgobj->isFinished();
				}
				else
				{
					LOG_DEBUG(logger, "bb creation on jump target found: ");
					cfgobj->addBBNode(bb_start, curr_addr, curr_addr+op_length, bb_code, bb_size, bb_instrs);
				}
				bb_code = "";
				bb_size = 0;
				bb_instrs = 0;
				in_bb = false;
			}
		}
		else if(dlp->isLabelLine(str))
		{
			bb_code += str;
			bb_code += "\r"; /// adding carriage return to ease parsing of code for raw file creation
		}
		else if(dlp->isMemoryHole(str))
		{
			LOG_DEBUG(logger, "Memory hole detected in line: |" << str << "| start address is: 0x" << hex << next_instr_address);
			mem_hole_start_addr = next_instr_address;
			detected_mem_hole = true;
		}
	}

	// delete parsed lines from unparsed_file_content vector
//	LOG_DEBUG(logger, "Deleting lines: " << parsed_dump_lines_begin << "-" << parsed_dump_lines_end+1 << " of unparsed_file_content " << unparsed_file_content.size());
	unparsed_file_content.erase((unparsed_file_content.begin()+parsed_dump_lines_begin), (unparsed_file_content.begin()+parsed_dump_lines_end+1));

	LOG_DEBUG(logger, "Finished creation of cfg for " << function.label << " at 0x" << hex << function.address);

	return cfgobj;
}


void DumpParser::connectFunctionCFGs(void)
{
	if(entry_function.label.compare("") == 0)
	{
		for(uint i = 0; i < func_cfgs.size(); i++)
		{
			func_graph.addFunc(func_cfgs[i]);
		}
	}
	else
	{
		vector<uint32_t> call_list = getFunctionIDsCalledByStartLabel();
		for(uint i = 0; i < call_list.size(); i++)
		{
			func_graph.addFunc(func_cfgs[call_list[i]]);
		}

	}
	func_graph.setStartLabel(entry_function);
}

vector<addr_label_t> DumpParser::getDetectedLabels(void)
{

	// do not return the blacklisted labels, for which no cfg exist

	vector<addr_label_t> result;
	for(uint32_t i=0; i < label_vec.size(); i++)
	{
		bool label_not_blacklisted = true;
		for(uint32_t j=0; j < blacklist_label.size(); j++)
		{
			if(blacklist_label[j].address == label_vec[i].address)
			{
				assert(blacklist_label[j].label.compare(label_vec[i].label)==0);
				label_not_blacklisted = false;
				break;
			}
			else
			{
				// be sure that address and label are the same
				assert(blacklist_label[j].label.compare(label_vec[i].label)!=0);
			}
		}
		if(label_not_blacklisted)
		{
			result.push_back(label_vec[i]);
		}
	}
	return result;
}

ControlFlowGraph DumpParser::getCFG(string label)
{
	ControlFlowGraph g;
	for(uint i = 0; i < func_cfgs.size(); i++)
	{
		if(func_cfgs[i].name.compare(label)==0)
		{
			return func_cfgs[i].cfg->getCFG();
		}
	}
	return g;
}

function_graph_t *DumpParser::getCFGObject(string label)
{
	for(uint i = 0; i < func_cfgs.size(); i++)
	{
		if(func_cfgs[i].name.compare(label)==0)
		{
			return &func_cfgs[i];
		}
	}
	return NULL;
}



CFGVertex DumpParser::getCFGEntry(string label)
{
	for(uint i = 0; i < func_cfgs.size(); i++)
	{
		if(func_cfgs[i].name.compare(label)==0)
		{
			return func_cfgs[i].cfg->getCFGEntry();
		}
	}
	return CFGVertex();
}

CFGVertex DumpParser::getCFGExit(string label)
{
	ControlFlowGraph g;
	for(uint i = 0; i < func_cfgs.size(); i++)
	{
		if(func_cfgs[i].name.compare(label)==0)
		{
			return func_cfgs[i].cfg->getCFGExit();
		}
	}
	return CFGVertex();
}

ControlFlowGraph DumpParser::getCFG(uint32_t addr)
{
	ControlFlowGraph g;
	for(uint i = 0; i < func_cfgs.size(); i++)
	{
		if(func_cfgs[i].address == addr)
		{
			return func_cfgs[i].cfg->getCFG();
		}
	}
	return g;
}

CFGVertex DumpParser::getCFGEntry(uint32_t addr)
{
	for(uint i = 0; i < func_cfgs.size(); i++)
	{
		if(func_cfgs[i].address == addr)
		{
			return func_cfgs[i].cfg->getCFGEntry();
		}
	}
	return CFGVertex();
}


CFGVertex DumpParser::getCFGExit(uint32_t addr)
{
	for(uint i = 0; i < func_cfgs.size(); i++)
	{
		if(func_cfgs[i].address == addr)
		{
			return func_cfgs[i].cfg->getCFGExit();
		}
	}
	return CFGVertex();
}

addr_label_t DumpParser::getAddrLabelForAddr(uint32_t address)
{
	for(uint i = 0; i < label_vec.size(); i++)
	{
		if(address == label_vec[i].address)
		{

			if(!dlp->isIgnoreLabel(label_vec[i].label))
			{
				return label_vec[i];
			}
			else
			{
				LOG_INFO(logger, "Label detected that sould be ignored. Ignoring it.");
				break;
			}
		}
	}
	addr_label_t tmp;
	tmp.address = UNKNOWN_ADDR;
	tmp.label = "LABELNOTFOUND";
	LOG_WARN(logger, "Label for address 0x" << hex << address << " not found in label vector or ignored.");
	return tmp;
}


FunctionCallGraph DumpParser::getFCG(void)
{
	return func_graph.getFCG();
}


uint32_t DumpParser::getCodeSize(string label)
{
	for(uint i = 0; i < func_cfgs.size(); i++)
	{
		if(func_cfgs[i].name.compare(label)==0)
		{
			return func_cfgs[i].cfg->getCodeSize();
		}
	}
	return 0;
}

uint32_t DumpParser::getCodeSize(uint32_t addr)
{
	for(uint i = 0; i < func_cfgs.size(); i++)
	{
		if(func_cfgs[i].address == addr)
		{
			return func_cfgs[i].cfg->getCodeSize();
		}
	}
	return 0;
}
uint32_t DumpParser::getCodeSize(void)
{
	return func_graph.getCodeSize();
}

uint32_t DumpParser::getFunctionCount(void)
{
	return func_graph.getFunctionCount();
}

vector<uint32_t> DumpParser::getFunctionIDsCalledByStartLabel(void)
{
	vector<uint32_t> process, finished;

	// get start label
	for(uint i = 0; i < func_cfgs.size(); i++)
	{
		if(func_cfgs[i].name.compare(entry_function.label)==0)
		{
			process.push_back(i);
		}
	}

	// implementing BFS
	uint32_t process_size = process.size();
	while(process_size != 0)
	{
		uint32_t actual = process.front();

		LOG_DEBUG(logger, "Checking callees of " << func_cfgs[actual].name);

		vector<addr_label_t> targets = func_cfgs[actual].cfg->getCallTargets();
		for(uint i = 0; i < targets.size(); i++)
		{
			for(uint j = 0; j < func_cfgs.size(); j++)
			{
				if(targets[i].address == func_cfgs[j].address)
				{
					bool already_in_process_list = false;
					bool already_in_finished_list = false;
					// check if the target is already in process list
					for(uint k = 0; k < process.size(); k++)
					{
						if(j == process[k])
						{
							already_in_process_list = true;
							break;
						}
					}
					// check if the is already finished
					for(uint k = 0; k < finished.size(); k++)
					{
						if(j == finished[k])
						{
							already_in_finished_list = true;
							break;
						}
					}

					// don't forget to check if the target ist the actual element
					if((!already_in_process_list) && (!already_in_finished_list) && (actual != j))
					{
						LOG_DEBUG(logger, "Adding " << targets[i].label  << " to process list");
						process.push_back(j);
						process_size++;
					}
					break;
				}
			}
		}

		finished.push_back(actual);
		process.erase(process.begin());
		process_size--;
	}

	return finished;
}

ControlFlowGraph DumpParser::getSCFG(void)
{
	if(!super_graph.isCreated())
	{
		super_graph.createSuperGraph(&func_graph);
	}
	return super_graph.getSCFG();

}

CFGVertex DumpParser::getSCFGEntry(void)
{
	if(super_graph.isCreated())
		return super_graph.getSCFGEntry();
	else
		return CFGVertex();
}

CFGVertex DumpParser::getSCFGExit(void)
{
	if(super_graph.isCreated())
		return super_graph.getSCFGExit();
	else
		return CFGVertex();
}

vector<function_graph_t> DumpParser::getCFGForAllFunctions(void)
{
	return func_cfgs;
}

FunctionCallGraphObject *DumpParser::getFCGObj(void)
{
	return &func_graph;
}

vector<uint32_t> DumpParser::getFunctionIDsNotCalledByStartLabel(void)
{
	vector<uint32_t> labels_called_by_start_label = getFunctionIDsCalledByStartLabel();

	vector<uint32_t> labels_not_connected_with_start_label;

	for(uint32_t i = 0; i < func_cfgs.size(); i++)
	{
		bool found = false;
		for(uint32_t j=0; (j < labels_called_by_start_label.size())&&(!found); j++)
		{
			if(labels_called_by_start_label[j] == i)
			{
//				LOG_DEBUG(logger, "Found " << func_cfgs[i].name) << " will not add to not connected list");
				found = true;
			}
		}
		if(!found && (func_cfgs[i].name.compare(entry_function.label)!=0))
		{
//			LOG_DEBUG(logger, "Added " << func_cfgs[i].name << " to not connected to start label list");
			labels_not_connected_with_start_label.push_back(func_cfgs[i].address);
		}
	}

	return labels_not_connected_with_start_label;
}

vector<ControlFlowGraph> DumpParser::getCfgsNotConnectedWithStartLabel(void)
{
	vector<uint32_t> not_connected_labels = getFunctionIDsNotCalledByStartLabel();

	vector<ControlFlowGraph> not_connected_cfgs;

	for(uint32_t i=0; i<not_connected_labels.size(); i++)
	{
		not_connected_cfgs.push_back(getCFG(not_connected_labels[i]));
	}
	return not_connected_cfgs;
}

vector<ControlFlowGraph> DumpParser::getCfgsConnectedWithStartLabel(void)
{
	vector<uint32_t> connected_labels = getFunctionIDsCalledByStartLabel();

	vector<ControlFlowGraph> connected_cfgs;

	for(uint32_t i=0; i<connected_labels.size(); i++)
	{
		connected_cfgs.push_back(getCFG(connected_labels[i]));
	}
	return connected_cfgs;
}

vector<string> DumpParser::getUnparsedDumpCode(void)
{
	return unparsed_file_content;
}

void DumpParser::copyDataSections(uint32_t start_line)
{
	// copy data section
	for(uint32_t line = start_line; line < unparsed_file_content.size(); line++)
	{
		data_section.push_back(unparsed_file_content[line]);
	}

	// delete the lines from code vector
	unparsed_file_content.erase((unparsed_file_content.begin()+start_line), unparsed_file_content.end());
}

vector<string> DumpParser::getDataSection(void)
{
	return data_section;
}

void DumpParser::addIndirectJumpTargets(vector<uint32_t> indirect_targets)
{
	vector<uint32_t>::iterator it;

	for(uint32_t i=0; i < indirect_targets.size(); i++)
	{
		bool stop = false;
		for(it=jump_target_vec.begin(); it < jump_target_vec.end() && !stop; it++)
		{
			if(*it == indirect_targets[i])
			{
				stop = true;
			}
		}
		if(!stop)
		{
			jump_target_vec.push_back(indirect_targets[i]);
		}
	}
}

addr_label_t DumpParser::getEntryFunction(void)
{
	return entry_function;
}

