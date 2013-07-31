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
#include "armv6m_dump_parser.hpp"
#include "armv6m_dumpline_parser.hpp"
#include "armv6m_isa.hpp"

Armv6mDumpParser::Armv6mDumpParser(uint32_t core_number) : DumpParser(core_number)
{
	core_id = core_number;

	dlp = Armv6mDumpLineParser::getInstance();
	isa = Armv6mISA::getInstance();
}

Armv6mDumpParser::~Armv6mDumpParser() 
{
//	dlp = NULL;
//	isa = NULL;
}


void Armv6mDumpParser::loadFile(void)
{
	LOG_INFO(logger, "opening file: " << input_filename);
	ifstream input_dump;
	input_dump.open(input_filename.c_str());

	assert(input_dump.is_open());

	while(!input_dump.eof()) 
	{ 
		string str;
		getline(input_dump, str);

		if(dlp->isLabelLine(str) || dlp->isCodeLine(str))
		{
			unparsed_file_content.push_back(str);
		}
	}
}

uint32_t Armv6mDumpParser::getCoreID(void)
{
	return core_id;
}

ControlFlowGraphObject *Armv6mDumpParser::parseCfgForLabel(addr_label_t function)
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
	vector<string> bb_code_instructions;
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

	Armv6mDumpLineParser *armdlp = dynamic_cast<Armv6mDumpLineParser *>(dlp);
	assert(armdlp!=NULL);

	LOG_DEBUG(logger, "Starting parse for " << function.label << " at " << unparsed_file_content[dump_line] << " #dumplines: " << unparsed_file_content.size());

	for(/* dump_line: start with value where the label was found */; (dump_line < unparsed_file_content.size())&&(!finished); dump_line++)
	{
		const string str = unparsed_file_content[dump_line];
		parsed_dump_lines_end=dump_line;
		LOG_DEBUG(logger, dump_line << "/" << unparsed_file_content.size() << " " << str);
//		if(str.compare("") == 0)
//		{
//			LOG_DEBUG(logger, "Empty line");
//		}
//		else 

		if(armdlp->isWordDirectiveLine(str))
		{
			// If a .word directive is found, continue parsing until a jump target is identified.
			// Then omit the currently parsed bb. This is sufficient, because the .word directives
			// will never be executed. They contain data and the compiler ensures that the pc will
			// not fetch from there. Any dangling code that may reach the data section is considered
			// as dead code (e.g. a single 16bit nop instruction in front of a data section of 32bit 
			// .word directives is used for the alignment of the data).
			uint32_t op_length = 4;
			uint32_t curr_addr = dlp->getAddrFromCodeLine(str);

			LOG_DEBUG(logger, "found .word directive: " << str << "address: " << hex << curr_addr << " length: " << op_length);

			bool is_jump_target = false;
			for(uint i = 0; i < jump_target_vec.size(); i++)
			{
				if(jump_target_vec[i] == curr_addr+op_length)
				{
					is_jump_target = true;
					break;
				}
			}

//			LOG_DEBUG(logger, "next addr is jump tgt: " << is_jump_target << " am in bb: " << in_bb);
			if(is_jump_target && in_bb)
			{
				LOG_DEBUG(logger, "clearing dead code basic block, that is interrupted by a data section (.word directives)");
				bb_code = "";
				bb_code_instructions.clear();
				bb_size = 0;
				bb_instrs = 0;
				in_bb = false;
			}
		}
		else if(dlp->isCodeLine(str))
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
			bb_code_instructions.push_back(dlp->getInstructionFromCodeLine(str));
			bb_size += isa->getInstructionLength(dlp->getInstructionFromCodeLine(str));
			bb_instrs++;
			bool is_branch = dlp->isBranchInstr(str);
			LOG_DEBUG(logger, hex << curr_addr << ": is_branch: " << is_branch << " is_jump_target: " << is_jump_target);
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
					else if(armdlp->isReturnInstr(str, bb_code_instructions))
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
							tmp.address = UNKNOWN_ADDR;
							tmp.label = "NOLABEL";
							LOG_WARN(logger, "In function " << function.label << " (0x" << hex << function.address << ") cannot obtain call target addr using addr 0x" << hex << UNKNOWN_ADDR << " instead");

							vector<uint32_t> jump_targets = jte.getJumpTargetsForIndirectJump(bb_start);
							if(jump_targets.size() == 0)
							{
								cfgobj->addBBCallNode(bb_start, curr_addr, curr_addr+op_length, bb_code, bb_size, bb_instrs, tmp);
							}
							else
							{
								// Implementation missing :)
								assert(false);
							}
						}
						else
						{
							tmp = getAddrLabelForAddr(target.addr);

							// FIXME: Dirty workaround for the case if a BL instruction is not used as call and jumps into a function body.
							// So if the call target is no valid function address, the branch is considered as unconditional jump.
							if(tmp.address == UNKNOWN_ADDR)
							{
								LOG_INFO(logger, "Interpreting BL as jump and not as call, since target address is no valid function address. Proceeding with unconditional jump...");
								goto uncond_branch;
							}

							cfgobj->addBBCallNode(bb_start, curr_addr, curr_addr+op_length, bb_code, bb_size, bb_instrs, tmp);
						}
					}
					else 
					{
uncond_branch:
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
				bb_code_instructions.clear();
				bb_size = 0;
				bb_instrs = 0;
				in_bb = false;
			}
		}
		else if(dlp->isLabelLine(str))
		{
//			LOG_DEBUG(logger, "Is Labelline");
			bb_code += str;
			bb_code += "\r"; /// adding carriage return to ease parsing of code for raw file creation
		}
		else if(dlp->isMemoryHole(str))
		{
			LOG_DEBUG(logger, "Memory hole detected in line: |" << str << "| start address is: 0x" << hex << next_instr_address);
			mem_hole_start_addr = next_instr_address;
			detected_mem_hole = true;
		}
//		else
//		{
//			LOG_DEBUG(logger, "Unknown line, do nothing");
//		}
	}

	// delete parsed lines from unparsed_file_content vector
//	LOG_DEBUG(logger, "Deleting lines: " << parsed_dump_lines_begin << "-" << parsed_dump_lines_end+1 << " of unparsed_file_content " << unparsed_file_content.size());
	unparsed_file_content.erase((unparsed_file_content.begin()+parsed_dump_lines_begin), (unparsed_file_content.begin()+parsed_dump_lines_end+1));

	LOG_DEBUG(logger, "Finished creation of cfg for " << function.label << " at 0x" << hex << function.address << " (#dumplines: " << dec << unparsed_file_content.size() << ")");
	if(!finished)
	{
		LOG_ERROR(logger, "Parsing was NOT finished!");
	}
	return cfgobj;
}
