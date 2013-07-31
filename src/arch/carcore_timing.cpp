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
#include "carcore_timing.hpp"


LoggerPtr CarCoreTiming::logger(Logger::getLogger("CarCoreTiming"));

#define DEBUG_INSTR 0xa000

#define IWS 3
#define FETCH_WIDTH 64  // bit
#define FETCH_WIDTH_BYTES (FETCH_WIDTH>>3) // byte
#define FETCH_ADDR_OFFSET ((FETCH_WIDTH>>3)-1) 
#define FETCH_ADDR (0xffffffff ^ ((FETCH_WIDTH>>3)-1))
#define TO_FETCH_ADDR(x) (FETCH_ADDR & (x))
#define TO_FETCH_ADDR_OFFSET(x) (FETCH_ADDR_OFFSET & (x))

#define SCHED_INSTR 0xd
#define SCHED_YIELD	0x0440000d
#define SCHED_TIE	0x0780000d
#define SCHED_UNTIE	0x07c0000d


CarCoreTiming::CarCoreTiming(memory_mode_t fetchmode) : fetch_mode(fetchmode)
{
	dlp = TriCoreDumpLineParser::getInstance();
	arch = TricoreISA::getInstance();
	cccfg = CarCoreConfig::getInstance();
	conf = Configuration::getInstance();
	fetch_latency = cccfg->getFetchLatency(fetch_mode==ONCHIP);
}

CarCoreTiming::~CarCoreTiming()
{
	BBAddrInitialIWMap::iterator it;

	for(it = initial_iw_map.begin(); it != initial_iw_map.end(); it++)
	{
		delete(it->second);
	}
	initial_iw_map.clear();
}

cycles_t CarCoreTiming::getCycleCountForInstructions(string code, bool precessing_bb_was_executed, uint32_t bb_start_addr, uint32_t bb_end_addr)
{
	uint32_t cycles = 0;
	vector<string> instrs;
	vector<instr_windows_t> iw;
	split(instrs, code, boost::is_any_of("\r"));

	LOG_DEBUG(logger, "Timing for: 0x" << hex << bb_start_addr << " to 0x" << bb_end_addr << " from " << ((fetch_mode==ONCHIP)?("onchip"):("offchip")) << ((precessing_bb_was_executed)?(" without empty iws"):(" with empty iws")));

	// TODO: check the differences with loop and jumps (note: the timing model always assumes that a jump is taken, since this is the worst case)

	
	if(!cccfg->isFetchMemIndependent() || cccfg->isFetchOptBranchAhead() || cccfg->isFetchOptEnoughInstr())
	{
		// isFetchMemIndependent() has to be assumed here, since any memory penalty for fetches is handled by the memory analysis
		// !isFetchOptBranchAhead() has to be assumed, since fetching contextes beyond the current bb are currently not supported
		LOG_ERROR(logger, "Fetch optimizations or shared mem are currently not modeled! " << ((fetch_mode==ONCHIP)?("Onchip"):("Offchip")) << ((!cccfg->isFetchMemIndependent())?("Shared mem is disabled."):("")) << ((cccfg->isFetchOptBranchAhead())?("Branch ahead optimization enabled."):("")) << ((cccfg->isFetchOptEnoughInstr())?("Enough instruction optimization enabled."):("")));
	}


	fillIW(bb_start_addr, !precessing_bb_was_executed, &iw);
	LOG_DEBUG(logger, "Initial IW state is:");
	printIW(&iw);


	// clear non code lines from vector
	vector<string>::iterator it;
	for(it=instrs.begin(); it < instrs.end(); it++)
	{
		if(!dlp->isCodeLine(*it))
		{
			instrs.erase(it);
		}
	}

	pipeline_t last_op_pipe = UNKNOWN_PIPELINE;
	for(uint32_t i=0; i < instrs.size(); i++)
	{
		assert(dlp->isCodeLine(instrs[i]));

		string instruction = dlp->getInstructionFromCodeLine(instrs[i]);
		uint32_t instruction_addr = dlp->getAddrFromCodeLine(instrs[i]);
		uint32_t instruction_uint = arch->convertInstructionToUint(instruction);
		uint32_t instruction_length = arch->getInstructionLength(instruction);
		func_mode_t f = instrset_decode(instruction_uint);
		instr_type_t instruction_type = dlp->getInstructionType(instrs[i]);
		//		assert(f.func != FUNC_UNKNOWN);


		// check if instr is already in IW
		uint32_t instruction_valid_at = getInstructionValidTime(instruction_addr, instruction_length, &iw);
		LOG_TRACE(logger, "Instr: " << instrs[i] << " is valid at: " << instruction_valid_at);
		if(cycles < instruction_valid_at)
		{
			// increase the cycles needed to fetch the instr to iw
			// wait until instruction ready
			cycles = instruction_valid_at;
			// do not allow parallel execution of A and D instructions, since the A instruction is not valid earlier 
			last_op_pipe = UNKNOWN_PIPELINE;
		}
		else if(cycles == instruction_valid_at)
		{
			// do not allow parallel execution of A and D instructions, since the A instruction is not valid earlier 
			last_op_pipe = UNKNOWN_PIPELINE;
		}

		updateIWOnExecution(instruction_addr, instruction_length, &iw, ((last_op_pipe == DATA_PIPELINE)?(cycles-1):(cycles)));

		pipeline_t op_pipe = getPipeline(f.func);
		LOG_DEBUG(logger, "instr: 0x" << hex << instruction_uint << " Pipeline is: " << op_pipe << " instr is: " << instrs[i]);
		if(op_pipe == DATA_PIPELINE)
		{
			// no memory operation is allowed here
			assert(!pd_load(instruction_uint));
			assert(!pd_store(instruction_uint));
			assert(!pd_lms(instruction_uint));
			// no microcode is allowed here
			assert(pd_microcode(instruction_uint)==0);

			LOG_DEBUG(logger, ((fetch_mode==ONCHIP)?("ONC"):("OFC")) << "#" << cycles << "[D]:\t" << instrs[i]);

			if(pd_branch(instruction_uint))
			{
				if(i < instrs.size() -1)
				{
					if(((mem_type_t)conf->getUint(CONF_MEMORY_TYPE) == BBSISP_JP) || ((mem_type_t)conf->getUint(CONF_MEMORY_TYPE) == BBSISP_JP_WCP) || (conf->getBool(CONF_MEMORY_BBSISP_ADD_JUMP_PENALTIES_TO_WCET)))
					{
						// Notice: this is acceptable iff a basic block was altered by the BBSISP assignment to contain additional connecting jumps.
						LOG_DEBUG(logger, "Branch is not the last instruction of the bb:" << instrs[i] << " is instr: " << i << " of " << instrs.size());
					}
					else
					{
						LOG_WARN(logger, "Branch is not the last instruction of the bb:" << instrs[i] << " is instr: " << i << " of " << instrs.size());
						assert((i == instrs.size()-1));
					}
				}
				// TODO: It would be more precise to charge the branch latency on the entrance of the following basic block, IFF it is taken
				if((instruction_type == I_UNCOND_BRANCH) || (instruction_type == I_UNCOND_INDIRECT_BRANCH))
				{
				}
				else if((instruction_type == I_COND_BRANCH) || (instruction_type == I_COND_INDIRECT_BRANCH))
				{
				}
				else
				{
					assert(false);
				}
				cycles += 1 + cccfg->getBranchLatency();
			}
			else
			{
				cycles++;
			}
			last_op_pipe = DATA_PIPELINE;
		}
		else if((op_pipe == ADDRESS_PIPELINE) && (pd_microcode(instruction_uint)==0))
		{
			LOG_DEBUG(logger, ((fetch_mode==ONCHIP)?("ONC"):("OFC")) << "#" << ((last_op_pipe == DATA_PIPELINE)?(cycles-1):(cycles)) << "[A]:\t" << instrs[i]);
			if(pd_load(instruction_uint))
			{
				cycles += 1 + cccfg->getLoadLatency(pd_width(instruction_uint), false);
			}
			else if(pd_store(instruction_uint))
			{
				cycles += 1 + cccfg->getStoreLatency(pd_width(instruction_uint), false);
			}
			else if(pd_lms(instruction_uint))
			{
				cycles += 1 + cccfg->getLMSLatency(pd_width(instruction_uint), false);
			}
			else if(pd_branch(instruction_uint))
			{
				if(i < instrs.size() -1)
				{
					if(((mem_type_t)conf->getUint(CONF_MEMORY_TYPE) == BBSISP_JP) || ((mem_type_t)conf->getUint(CONF_MEMORY_TYPE) == BBSISP_JP_WCP) || (conf->getBool(CONF_MEMORY_BBSISP_ADD_JUMP_PENALTIES_TO_WCET)))
					{
						// Notice: this is acceptable iff a basic block was altered by the BBSISP assignment to contain additional connecting jumps.
						LOG_DEBUG(logger, "Branch is not the last instruction of the bb:" << instrs[i] << " is instr: " << i << " of " << instrs.size());
					}
					else
					{
						LOG_WARN(logger, "Branch is not the last instruction of the bb:" << instrs[i] << " is instr: " << i << " of " << instrs.size());
						assert((i == instrs.size()-1));
					}
				}
				if((instruction_type == I_UNCOND_BRANCH) || (instruction_type == I_UNCOND_INDIRECT_BRANCH))
				{
				}
				else if((instruction_type == I_COND_BRANCH) || (instruction_type == I_COND_INDIRECT_BRANCH))
				{
				}
				else
				{
					assert(false);
				}
				cycles += 1 + cccfg->getBranchLatency();
			}
			else if(instruction_uint == SCHED_INSTR)
			{

				// somehow the scheduling instructions SCHED_YIELD, SCHED_TIE and SCHED_UNTIE are coded in two dump file lines, thus these two lines are merged:

				assert(i+1 < instrs.size());
				assert(dlp->isCodeLine(instrs[i+1]));
				string instruction2 = dlp->getInstructionFromCodeLine(instrs[i+1]);
				uint32_t instruction2_addr = dlp->getAddrFromCodeLine(instrs[i+1]);
				uint32_t instruction2_uint = arch->convertInstructionToUint(instruction2);
				uint32_t instruction2_length = arch->getInstructionLength(instruction2);

				instruction_uint = (instruction2_uint << 16) | instruction_uint;
				string instruction_string;
				switch(instruction_uint)
				{
					// for the latencies of these instructions 1 cycle is taken
					case SCHED_YIELD:
						{
							instruction_string = "yield";
							cycles++;
							break;
						}
					case SCHED_TIE:
						{
							instruction_string = "tie";
							cycles++;
							break;
						}
					case SCHED_UNTIE:
						{
							instruction_string = "untie";
							cycles++;
							break;
						}
					default:
						{
							assert(false);
						}
				}
				LOG_DEBUG(logger, "Found Special instruction " << instruction_string << " : 0x" << hex << instruction_uint);

				// also the IWs have to be updated
				updateIWOnExecution(instruction2_addr, instruction2_length, &iw, ((last_op_pipe == DATA_PIPELINE)?(cycles-1):(cycles)));

				// the next instruction is already handled, so skip it.
				i++;
			}
			else
			{
				cycles++;

				// consider additional latencies for floating point instructions
				switch((alu_func_t)f.func)
				{
					case F_ADD:
					case F_SUB:
					case F_CMP:
						cycles += cccfg->getFPLatency(ARITHMETIC);
						break;
					case F_DIV:
						cycles += cccfg->getFPLatency(DIVIDE);
						break;
					case F_FTOI:
					case F_FTOIZ:
					case F_FTOQ31:
					case F_FTOQ31Z:
					case F_FTOU:
					case F_FTOUZ:
					case F_ITOF:
					case F_Q31TOF:
					case F_UTOF:
						cycles += cccfg->getFPLatency(CONVERSION);
						break;
					case F_MADD:
					case F_MSUB:
						cycles += cccfg->getFPLatency(MULTIPLYACCUMULATE);
						break;
					case F_MUL:
						cycles += cccfg->getFPLatency(MULTIPLY);
						break;
					case F_QSEED:
						cycles += cccfg->getFPLatency(SQRTSEED);
						break;
					case F_UPDFL:
						cycles += cccfg->getFPLatency(UPDATEFLAGS);
						break;
					default:
						break;
				}
			}

			if(last_op_pipe == DATA_PIPELINE)
			{
				// both instructions can be executed in parallel if the previous instruction was in the D pipeline 
				cycles--;
			}
			last_op_pipe = ADDRESS_PIPELINE;

			// no microcode is allowed here
			assert(pd_microcode(instruction_uint)==0);
		}
		else
		{
			LOG_DEBUG(logger, ((fetch_mode==ONCHIP)?("ONC"):("OFC")) << "#" << ((last_op_pipe == DATA_PIPELINE)?(cycles-1):(cycles)) << "[A]:\t" << instrs[i] << " MC_START");
			switch(pd_microcode(instruction_uint))
			{
				case POS_MC_CALL_L:
				case POS_MC_CALL_S:
				case POS_MC_CALLA:
				case POS_MC_CALLI:
					{
						cycles += 1 + cccfg->getCallLatency(fetch_mode==ONCHIP);
						assert((instruction_type == I_CALL) || (instruction_type == I_INDIRECT_CALL));
						break;
					}
				case POS_MC_RET:
					{
						cycles += 1 + cccfg->getReturnLatency(fetch_mode==ONCHIP);
						assert(instruction_type == I_RETURN);
						break;
					}
				case POS_MC_LDMST:
					{
						cycles += 1 + cccfg->getMCLatency(MC_LDMST);
						assert(instruction_type == I_SYNC);
						break;
					}
				case POS_MC_LDMST_ABS:
					{
						cycles += 1 + cccfg->getMCLatency(MC_LDMST_ABS);
						assert(instruction_type == I_SYNC);
						break;
					}
				case POS_MC_STDA_PRE:
					{
						cycles += 1 + cccfg->getMCLatency(MC_STDA_PRE);
						assert(instruction_type == I_STORE);
						break;
					}
				case POS_MC_STDA_POST:
					{
						cycles += 1 + cccfg->getMCLatency(MC_STDA_POST);
						assert(instruction_type == I_STORE);
						break;
					}
				case POS_MC_STDA_NORM:
					{
						cycles += 1 + cccfg->getMCLatency(MC_STDA_NORM);
						assert(instruction_type == I_STORE);
						break;
					}
				case POS_MC_STDA_ABS:
					{
						cycles += 1 + cccfg->getMCLatency(MC_STDA_ABS);
						assert(instruction_type == I_STORE);
						break;
					}
				case POS_MC_STT:
					{
						cycles += 1 + cccfg->getMCLatency(MC_STT);
						assert(instruction_type == I_STORE);
						break;
					}
				case 0: // no microcode
					{
						if(instruction_uint != DEBUG_INSTR) // ignore the debug instruction, since it will never be executed!
						{
							LOG_WARN(logger, "No Microcode type found: " << pd_microcode(instruction_uint) << " Pipeline is: " << op_pipe << " instr is: " << instrs[i] << " instruction: 0x" << hex << instruction_uint);
							assert(false);
						}
						else
						{
							LOG_INFO(logger, "debug instruction found.");
							cycles++;
						}
						break;
					}
				default:
					{
						LOG_WARN(logger, "Unknown Microcode type found: " << pd_microcode(instruction_uint));
						assert(false);
					}
			}

			if(last_op_pipe == DATA_PIPELINE)
			{
				// both instructions can be executed in parallel if the previous instruction was in the D pipeline 
				cycles--;
			}

			last_op_pipe = ADDRESS_PIPELINE;
			LOG_DEBUG(logger, ((fetch_mode==ONCHIP)?("ONC"):("OFC")) << "#" << cycles << "[A]:\t" << instrs[i] << " MC_END");

		}

	}

	if(!instrs.empty()) // check if the instructions are empty (if so no code was in the basic block) 
	{
		LOG_DEBUG(logger, "End IW state is: ");
		printIW(&iw);

		if(!cccfg->isFetchOptBranchAhead())
		{
			// remember the state of the IW after executing the bb, to have an initial state for the subsequent block
			// XXX: Need to update IW here again?
			string instruction_last_for_bb = dlp->getInstructionFromCodeLine(instrs.back());
			uint32_t next_bb_addr = dlp->getAddrFromCodeLine(instrs.back()) + arch->getInstructionLength(instruction_last_for_bb);
			storeIWForSubsequentBB(next_bb_addr, &iw, cycles+1);
		}
		else
		{
			// TODO: BranchAhead Optimization ist not tested & verified yet
			// ignore all fetches after the branch, such that the precessing BB will start with an empty IW, like if isFetchOptEnoughInstr() is enabled.
			string instruction_last_for_bb = dlp->getInstructionFromCodeLine(instrs.back());
			uint32_t next_bb_addr = dlp->getAddrFromCodeLine(instrs.back()) + arch->getInstructionLength(instruction_last_for_bb);
			if(pd_branch(arch->convertInstructionToUint(instruction_last_for_bb)))
			{
				uint32_t addr_last_byte_of_last_instruction = next_bb_addr - 1; // gets the address of the last byte of the branch instruction. The fetch word address of the last byte is needed since instructions may span 2 fetch words
				if(TO_FETCH_ADDR(addr_last_byte_of_last_instruction) == TO_FETCH_ADDR(next_bb_addr)) // compare fetch word address of (the end of) the last instruction of basic block and the fetch word addres of the first instruction of the subsequent block
				{
					// There is another instruction in the same fetch word as the branch. This fetch word is valid for the following basic block, all others are inavalable on beginning of the subsequent basic block.
					if(iw.size() > 1)
					{
						iw.erase(iw.begin()+1,iw.end()); // delete all but the first iw entry
					}
				}
				else
				{
					// The branch was the last instruction in the fetch word, thus no following fetches were made in the BranchAhead optimization, and the next basic block starts with an empty IW 
					iw.clear();
				}
			}
			storeIWForSubsequentBB(next_bb_addr, &iw, cycles+1);
		}
	}

	iw.clear();

	cycles_t cycles_ret;
	cycles_ret.forward_step = cycles;
	cycles_ret.jump = cycles;

	return cycles_ret;
}

cycles_t CarCoreTiming::getCycleCountForInstructionsWithBlockExitSensitivity(string code, bool precessing_bb_was_executed, uint32_t bb_start_addr, uint32_t bb_end_addr)
{
	uint32_t cycles=0;
	cycles_t cycles_ret;
	cycles_ret.forward_step = 0;
	cycles_ret.jump = 0;

	instr_type_t bb_end_instr = I_UNKNOWN;

	vector<string> instrs;
	vector<instr_windows_t> iw;
	split(instrs, code, boost::is_any_of("\r"));

	LOG_DEBUG(logger, "Timing for: 0x" << hex << bb_start_addr << " to 0x" << bb_end_addr << " from " << ((fetch_mode==ONCHIP)?("onchip"):("offchip")) << ((precessing_bb_was_executed)?(" without empty iws"):(" with empty iws")));

	// TODO:
	// - check the differences with loop and jumps (note: the timing model always assumes that a jump is taken, since this is the worst case)

	
	if(!cccfg->isFetchMemIndependent() || cccfg->isFetchOptBranchAhead() || cccfg->isFetchOptEnoughInstr())
	{
		// isFetchMemIndependent() has to be assumed here, since any memory penalty for fetches is handled by the memory analysis
		// !isFetchOptBranchAhead() has to be assumed, since fetching contextes beyond the current bb are currently not supported
		LOG_ERROR(logger, "Fetch optimizations or shared mem are currently not modeled! " << ((fetch_mode==ONCHIP)?("Onchip"):("Offchip")) << ((!cccfg->isFetchMemIndependent())?("Shared mem is disabled."):("")) << ((cccfg->isFetchOptBranchAhead())?("Branch ahead optimization enabled."):("")) << ((cccfg->isFetchOptEnoughInstr())?("Enough instruction optimization enabled."):("")));
	}


	fillIW(bb_start_addr, !precessing_bb_was_executed, &iw);
	LOG_DEBUG(logger, "Initial IW state is:");
	printIW(&iw);


	// clear non code lines from vector
	vector<string>::iterator it;
	for(it=instrs.begin(); it < instrs.end(); it++)
	{
		if(!dlp->isCodeLine(*it))
		{
			instrs.erase(it);
		}
	}

	pipeline_t last_op_pipe = UNKNOWN_PIPELINE;
	for(uint32_t i=0; i < instrs.size(); i++)
	{
		assert(dlp->isCodeLine(instrs[i]));

		string instruction = dlp->getInstructionFromCodeLine(instrs[i]);
		uint32_t instruction_addr = dlp->getAddrFromCodeLine(instrs[i]);
		uint32_t instruction_uint = arch->convertInstructionToUint(instruction);
		uint32_t instruction_length = arch->getInstructionLength(instruction);
		func_mode_t f = instrset_decode(instruction_uint);
		instr_type_t instruction_type = dlp->getInstructionType(instrs[i]);
		//		assert(f.func != FUNC_UNKNOWN);


		// check if instr is already in IW
		uint32_t instruction_valid_at = getInstructionValidTime(instruction_addr, instruction_length, &iw);
		LOG_TRACE(logger, "Instr: " << instrs[i] << " is valid at: " << instruction_valid_at);
		if(cycles < instruction_valid_at)
		{
			// increase the cycles needed to fetch the instr to iw
			// wait until instruction ready
			cycles = instruction_valid_at;
			// do not allow parallel execution of A and D instructions, since the A instruction is not valid earlier 
			last_op_pipe = UNKNOWN_PIPELINE;
		}
		else if(cycles == instruction_valid_at)
		{
			// do not allow parallel execution of A and D instructions, since the A instruction is not valid earlier 
			last_op_pipe = UNKNOWN_PIPELINE;
		}

		updateIWOnExecution(instruction_addr, instruction_length, &iw, ((last_op_pipe == DATA_PIPELINE)?(cycles-1):(cycles)));

		pipeline_t op_pipe = getPipeline(f.func);
		LOG_DEBUG(logger, "instr: 0x" << hex << instruction_uint << " Pipeline is: " << op_pipe << " instr is: " << instrs[i]);
		if(op_pipe == DATA_PIPELINE)
		{
			// no memory operation is allowed here
			assert(!pd_load(instruction_uint));
			assert(!pd_store(instruction_uint));
			assert(!pd_lms(instruction_uint));
			// no microcode is allowed here
			assert(pd_microcode(instruction_uint)==0);

			LOG_DEBUG(logger, ((fetch_mode==ONCHIP)?("ONC"):("OFC")) << "#" << cycles << "[D]:\t" << instrs[i]);

			if(pd_branch(instruction_uint))
			{
				if(i < instrs.size() -1)
				{
					if(((mem_type_t)conf->getUint(CONF_MEMORY_TYPE) == BBSISP_JP) || ((mem_type_t)conf->getUint(CONF_MEMORY_TYPE) == BBSISP_JP_WCP) || (conf->getBool(CONF_MEMORY_BBSISP_ADD_JUMP_PENALTIES_TO_WCET)))
					{
						// Notice: this is acceptable iff a basic block was altered by the BBSISP assignment to contain additional connecting jumps.
						LOG_DEBUG(logger, "Branch is not the last instruction of the bb:" << instrs[i] << " is instr: " << i << " of " << instrs.size());
					}
					else
					{
						LOG_WARN(logger, "Branch is not the last instruction of the bb:" << instrs[i] << " is instr: " << i << " of " << instrs.size());
						assert((i == instrs.size()-1));
					}
				}
				if((instruction_type == I_UNCOND_BRANCH) || (instruction_type == I_UNCOND_INDIRECT_BRANCH))
				{
				}
				else if((instruction_type == I_COND_BRANCH) || (instruction_type == I_COND_INDIRECT_BRANCH))
				{
				}
				else
				{
					assert(false);
				}
				// the branch latency is charged at the end of the basic block
				cycles += 1; // + cccfg->getBranchLatency();
			}
			else
			{
				cycles++;
			}
			last_op_pipe = DATA_PIPELINE;
		}
		else if((op_pipe == ADDRESS_PIPELINE) && (pd_microcode(instruction_uint)==0))
		{
			LOG_DEBUG(logger, ((fetch_mode==ONCHIP)?("ONC"):("OFC")) << "#" << ((last_op_pipe == DATA_PIPELINE)?(cycles-1):(cycles)) << "[A]:\t" << instrs[i]);
			if(pd_load(instruction_uint))
			{
				cycles += 1 + cccfg->getLoadLatency(pd_width(instruction_uint), false);
			}
			else if(pd_store(instruction_uint))
			{
				cycles += 1 + cccfg->getStoreLatency(pd_width(instruction_uint), false);
			}
			else if(pd_lms(instruction_uint))
			{
				cycles += 1 + cccfg->getLMSLatency(pd_width(instruction_uint), false);
			}
			else if(pd_branch(instruction_uint))
			{
				if(i < instrs.size() -1)
				{
					if(((mem_type_t)conf->getUint(CONF_MEMORY_TYPE) == BBSISP_JP) || ((mem_type_t)conf->getUint(CONF_MEMORY_TYPE) == BBSISP_JP_WCP) || (conf->getBool(CONF_MEMORY_BBSISP_ADD_JUMP_PENALTIES_TO_WCET)))
					{
						// Notice: this is acceptable iff a basic block was altered by the BBSISP assignment to contain additional connecting jumps.
						LOG_DEBUG(logger, "Branch is not the last instruction of the bb:" << instrs[i] << " is instr: " << i << " of " << instrs.size());
					}
					else
					{
						LOG_WARN(logger, "Branch is not the last instruction of the bb:" << instrs[i] << " is instr: " << i << " of " << instrs.size());
						assert((i == instrs.size()-1));
					}
				}
				if((instruction_type == I_UNCOND_BRANCH) || (instruction_type == I_UNCOND_INDIRECT_BRANCH))
				{
				}
				else if((instruction_type == I_COND_BRANCH) || (instruction_type == I_COND_INDIRECT_BRANCH))
				{
				}
				else
				{
					assert(false);
				}
				// the branch latency is charged at the end of the basic block
				cycles += 1;// + cccfg->getBranchLatency();
			}
			else if(instruction_uint == SCHED_INSTR)
			{

				// somehow the scheduling instructions SCHED_YIELD, SCHED_TIE and SCHED_UNTIE are coded in two dump file lines, thus these two lines are merged:

				assert(i+1 < instrs.size());
				assert(dlp->isCodeLine(instrs[i+1]));
				string instruction2 = dlp->getInstructionFromCodeLine(instrs[i+1]);
				uint32_t instruction2_addr = dlp->getAddrFromCodeLine(instrs[i+1]);
				uint32_t instruction2_uint = arch->convertInstructionToUint(instruction2);
				uint32_t instruction2_length = arch->getInstructionLength(instruction2);

				instruction_uint = (instruction2_uint << 16) | instruction_uint;
				string instruction_string;
				switch(instruction_uint)
				{
					// for the latencies of these instructions 1 cycle is taken
					case SCHED_YIELD:
						{
							instruction_string = "yield";
							cycles++;
							break;
						}
					case SCHED_TIE:
						{
							instruction_string = "tie";
							cycles++;
							break;
						}
					case SCHED_UNTIE:
						{
							instruction_string = "untie";
							cycles++;
							break;
						}
					default:
						{
							assert(false);
						}
				}
				LOG_DEBUG(logger, "Found Special instruction " << instruction_string << " : 0x" << hex << instruction_uint);

				// also the IWs have to be updated
				updateIWOnExecution(instruction2_addr, instruction2_length, &iw, ((last_op_pipe == DATA_PIPELINE)?(cycles-1):(cycles)));

				// the next instruction is already handled, so skip it.
				i++;
			}
			else
			{
				cycles++;


				// consider additional latencies for floating point instructions
				switch((alu_func_t)f.func)
				{
					case F_ADD:
					case F_SUB:
					case F_CMP:
						cycles += cccfg->getFPLatency(ARITHMETIC);
						break;
					case F_DIV:
						cycles += cccfg->getFPLatency(DIVIDE);
						break;
					case F_FTOI:
					case F_FTOIZ:
					case F_FTOQ31:
					case F_FTOQ31Z:
					case F_FTOU:
					case F_FTOUZ:
					case F_ITOF:
					case F_Q31TOF:
					case F_UTOF:
						cycles += cccfg->getFPLatency(CONVERSION);
						break;
					case F_MADD:
					case F_MSUB:
						cycles += cccfg->getFPLatency(MULTIPLYACCUMULATE);
						break;
					case F_MUL:
						cycles += cccfg->getFPLatency(MULTIPLY);
						break;
					case F_QSEED:
						cycles += cccfg->getFPLatency(SQRTSEED);
						break;
					case F_UPDFL:
						cycles += cccfg->getFPLatency(UPDATEFLAGS);
						break;
					default:
						break;
				}
			}

			if(last_op_pipe == DATA_PIPELINE)
			{
				// both instructions can be executed in parallel if the previous instruction was in the D pipeline 
				cycles--;
			}
			last_op_pipe = ADDRESS_PIPELINE;

			// no microcode is allowed here
			assert(pd_microcode(instruction_uint)==0);
		}
		else
		{
			LOG_DEBUG(logger, ((fetch_mode==ONCHIP)?("ONC"):("OFC")) << "#" << ((last_op_pipe == DATA_PIPELINE)?(cycles-1):(cycles)) << "[A]:\t" << instrs[i] << " MC_START");
			switch(pd_microcode(instruction_uint))
			{
				case POS_MC_CALL_L:
				case POS_MC_CALL_S:
				case POS_MC_CALLA:
				case POS_MC_CALLI:
					{
						cycles += 1 + cccfg->getCallLatency(fetch_mode==ONCHIP);
						assert((instruction_type == I_CALL) || (instruction_type == I_INDIRECT_CALL));
						break;
					}
				case POS_MC_RET:
					{
						cycles += 1 + cccfg->getReturnLatency(fetch_mode==ONCHIP);
						assert(instruction_type == I_RETURN);
						break;
					}
				case POS_MC_LDMST:
					{
						cycles += 1 + cccfg->getMCLatency(MC_LDMST);
						assert(instruction_type == I_SYNC);
						break;
					}
				case POS_MC_LDMST_ABS:
					{
						cycles += 1 + cccfg->getMCLatency(MC_LDMST_ABS);
						assert(instruction_type == I_SYNC);
						break;
					}
				case POS_MC_STDA_PRE:
					{
						cycles += 1 + cccfg->getMCLatency(MC_STDA_PRE);
						assert(instruction_type == I_STORE);
						break;
					}
				case POS_MC_STDA_POST:
					{
						cycles += 1 + cccfg->getMCLatency(MC_STDA_POST);
						assert(instruction_type == I_STORE);
						break;
					}
				case POS_MC_STDA_NORM:
					{
						cycles += 1 + cccfg->getMCLatency(MC_STDA_NORM);
						assert(instruction_type == I_STORE);
						break;
					}
				case POS_MC_STDA_ABS:
					{
						cycles += 1 + cccfg->getMCLatency(MC_STDA_ABS);
						assert(instruction_type == I_STORE);
						break;
					}
				case POS_MC_STT:
					{
						cycles += 1 + cccfg->getMCLatency(MC_STT);
						assert(instruction_type == I_STORE);
						break;
					}
				case 0: // no microcode
					{
						if(instruction_uint != DEBUG_INSTR) // ignore the debug instruction, since it will never be executed!
						{
							LOG_WARN(logger, "No Microcode type found: " << pd_microcode(instruction_uint) << " Pipeline is: " << op_pipe << " instr is: " << instrs[i] << " instruction: 0x" << hex << instruction_uint);
							assert(false);
						}
						else
						{
							LOG_INFO(logger, "debug instruction found.");
							cycles++;
						}
						break;
					}
				default:
					{
						LOG_WARN(logger, "Unknown Microcode type found: " << pd_microcode(instruction_uint));
						assert(false);
					}
			}

			if(last_op_pipe == DATA_PIPELINE)
			{
				// both instructions can be executed in parallel if the previous instruction was in the D pipeline 
				cycles--;
			}

			last_op_pipe = ADDRESS_PIPELINE;
			LOG_DEBUG(logger, ((fetch_mode==ONCHIP)?("ONC"):("OFC")) << "#" << cycles << "[A]:\t" << instrs[i] << " MC_END");

		}

		bb_end_instr = instruction_type;
	}

	if(!instrs.empty()) // check if the instructions are empty (if so no code was in the basic block) 
	{
		LOG_DEBUG(logger, "End IW state is: ");
		printIW(&iw);

		// register the iw state only if the following block can be entered by a forward step, i.e. the last istruction is no unconditional branch:
		if((bb_end_instr != I_UNCOND_BRANCH)&&(bb_end_instr != I_UNCOND_INDIRECT_BRANCH))
		{
			if(!cccfg->isFetchOptBranchAhead())
			{
				// remember the state of the IW after executing the bb, to have an initial state for the subsequent block
				// XXX: Need to update IW here again?
				string instruction_last_for_bb = dlp->getInstructionFromCodeLine(instrs.back());
				uint32_t next_bb_addr = dlp->getAddrFromCodeLine(instrs.back()) + arch->getInstructionLength(instruction_last_for_bb);
				storeIWForSubsequentBB(next_bb_addr, &iw, cycles+1);
			}
			else
			{
				// TODO: BranchAhead Optimization ist not tested & verified yet
				// ignore all fetches after the branch, such that the precessing BB will start with an empty IW, like if isFetchOptEnoughInstr() is enabled.
				string instruction_last_for_bb = dlp->getInstructionFromCodeLine(instrs.back());
				uint32_t next_bb_addr = dlp->getAddrFromCodeLine(instrs.back()) + arch->getInstructionLength(instruction_last_for_bb);
				if(pd_branch(arch->convertInstructionToUint(instruction_last_for_bb)))
				{
					uint32_t addr_last_byte_of_last_instruction = next_bb_addr - 1; // gets the address of the last byte of the branch instruction. The fetch word address of the last byte is needed since instructions may span 2 fetch words
					if(TO_FETCH_ADDR(addr_last_byte_of_last_instruction) == TO_FETCH_ADDR(next_bb_addr)) // compare fetch word address of (the end of) the last instruction of basic block and the fetch word addres of the first instruction of the subsequent block
					{
						// There is another instruction in the same fetch word as the branch. This fetch word is valid for the following basic block, all others are inavalable on beginning of the subsequent basic block.
						if(iw.size() > 1)
						{
							iw.erase(iw.begin()+1,iw.end()); // delete all but the first iw entry
						}
					}
					else
					{
						// The branch was the last instruction in the fetch word, thus no following fetches were made in the BranchAhead optimization, and the next basic block starts with an empty IW 
						iw.clear();
					}
				}
				storeIWForSubsequentBB(next_bb_addr, &iw, cycles+1);
			}
		}
	}

	// charge the jump penalty at the end of the basic block
	// on conditional jump the cost with and without the branch latency is calculated
	switch(bb_end_instr)
	{
		case I_UNCOND_BRANCH:
		case I_UNCOND_INDIRECT_BRANCH:
			{
				cycles_ret.jump = cycles + cccfg->getBranchLatency();
				break;
			}
		case I_CALL:
		case I_INDIRECT_CALL:
			{
				// a call bb is connected to a callsite via forward/backward jump, but the branch latency is already contained in the getCallLatency()
				cycles_ret.jump = cycles;
				break;
			}
		case I_COND_BRANCH:
		case I_COND_INDIRECT_BRANCH:
			{
				// the branch penalty is needed on both paths (due to the pipeline has to determine which is the next instruction and no speculation is used in the CarCore)
				cycles_ret.jump = cycles + cccfg->getBranchLatency();
				cycles_ret.forward_step = cycles + cccfg->getBranchLatency();
				break;
			}
		default:
			{
				cycles_ret.forward_step = cycles;
			}
	}

	iw.clear();
	return cycles_ret;
}

pipeline_t CarCoreTiming::getPipeline(int function)
{
	alu_func_t func = (alu_func_t)function;
	switch(func)
	{
		case A_ADD:
		case A_ADDSC_A:
		case A_ADDSC_AL:
		case A_ADDSC_AT:
		case A_CLCF:
		case A_CPCF:
		case A_CSA:
		case A_INSLD:
		case A_J2:
		case A_J4:
		case A_JI:
		case A_JA:
		case A_LDMST:
		case A_LOOP:
		case A_MFCR:
		case A_POST:
		case A_PRE:
		case A_SETC:
		case A_SRC1:
		case A_STT:
		case A_SUB:
		case A_SWAP:
		case A_XSRC:
		case FUNC_NOP: // the nop is "executed" by the address pipeline
		// Floating Point instructions are assumed to be in the address pipeline:
		case F_ADD:
		case F_CMP:
		case F_DIV:
		case F_FTOI:
		case F_FTOIZ:
		case F_FTOQ31:
		case F_FTOQ31Z:
		case F_FTOU:
		case F_FTOUZ:
		case F_ITOF:
		case F_MADD:
		case F_MSUB:
		case F_MUL:
		case F_Q31TOF:
		case F_QSEED:
		case F_SUB:
		case F_UPDFL:
		case F_UTOF:
			return ADDRESS_PIPELINE;
		case D_ABS_W:
		case D_ABSDIF:
		case D_ADD:
		case D_ADDC:
		case D_ADDX:
		case D_AND:
		case D_ANDC:
		case D_ANDN:
		case D_AORB:
		case D_CADD:
		case D_CADDN:
		case D_CLO:
		case D_CLZ:
		case D_CSUB:
		case D_DEXTR:
		case D_DVADJ:
		case D_DVINIT:
		case D_DVINIT_U:
		case D_DVINIT_B:
		case D_DVINIT_BU:
		case D_DVINIT_H:
		case D_DVINIT_HU:
		case D_DVSTEP:
		case D_DVSTEP_U:
		case D_EQANY_B:
		case D_EQANY_H:
 		case D_EXTR:
		case D_EXTR_U:
		case D_IMASK:
		case D_INSC:
		case D_INSERT:
		case D_J:
		case D_JNED:
		case D_JNEI:
		case D_MADD32:
		case D_MADDU:
		case D_MSUB32:
		case D_MSUBU:
		case D_MUL:
		case D_MUL_U:
		case D_NAND:
		case D_NOR:
		case D_OR:
		case D_ORC:
		case D_ORN:
		case D_RSUB:
		case D_SEL:
		case D_SELN:
		case D_SETC:
		case D_SH:
		case D_SHA:
		case D_SHC:
		case D_SRC1:
		case D_SUB:
		case D_SUBC:
		case D_SUBX:
		case D_XNOR:
		case D_XOR:
		case D_XORC:
		case D_BMERGE:
		case D_BSPLIT:
		case D_CLS:
		case D_CSUBN:
		case D_PACK:
		case D_PARITY:
		case D_UNPACK:
			return DATA_PIPELINE;
		case C_SWAPIN:
		case C_SWAPOUT:
		case C_RELOAD:
		case FUNC_UNKNOWN:
		default:
			return UNKNOWN_PIPELINE;

	}
	return UNKNOWN_PIPELINE;
}


uint32_t CarCoreTiming::getInstructionValidTime(uint32_t instruction_addr, uint32_t instruction_length, vector<instr_windows_t> *iw)
{
	uint32_t instruction_valid_at = 0;

	uint32_t instruction_start_iw_addr = TO_FETCH_ADDR(instruction_addr);
	uint32_t instruction_end_iw_addr = TO_FETCH_ADDR(instruction_addr + instruction_length - 1);

	assert(iw->front().address == instruction_start_iw_addr);


	if(instruction_start_iw_addr == instruction_end_iw_addr)
	{
		instruction_valid_at = iw->front().valid_at;
	}
	else
	{
		assert(instruction_end_iw_addr == iw->at(1).address); 
		instruction_valid_at = iw->at(1).valid_at;
	}

	return instruction_valid_at;

}


void CarCoreTiming::fillIW(uint32_t start_addr, bool jump_in_bb, vector<instr_windows_t> *iw)
{
	// FIXME: Is the interblock fetch dependence correctly modelled, if a block is a consecutive block but without a nottaken branch?
	if(!jump_in_bb)
	{
		*iw = getInitialIWForBB(start_addr);

//		LOG_DEBUG(logger, "Loaded IW state is: ");
//		printIW(iw);

		if(iw->size() < IWS)
		{
			instr_windows_t last_entry;
			if(!iw->empty())
			{
				last_entry = iw->back();
			}
			else
			{
				last_entry.address = TO_FETCH_ADDR(start_addr);
				last_entry.valid_at = fetch_latency + 1;
				iw->push_back(last_entry);
			}
			instr_windows_t next_entry;
			for(uint32_t i = iw->size(); i < IWS; i++)
			{
				next_entry.address = last_entry.address+0x8;
				next_entry.valid_at = last_entry.valid_at + fetch_latency + 1; // one cycle extra to route the result to pipeline
				iw->push_back(next_entry);

				// prepare next loop iteration
				last_entry.address = next_entry.address;
				last_entry.valid_at = next_entry.valid_at;
			}
		}
	}
	else
	{

		uint32_t iw_addr = TO_FETCH_ADDR(start_addr);

		// on jump into the basic block the ready time of the first block depends on the fetch_latency and the branch penalty, because when branching the new fetch word is requested, while the pipeline ic cleared.
		// TODO is the ready_cycle_on_jump is negative also some following blocks can be already in the instruction window
		int32_t ready_cycle_on_jump = fetch_latency+1;//-cccfg->getBranchLatency();
		if(ready_cycle_on_jump<0)
		{
			ready_cycle_on_jump = 0;
		}


		ready_cycle_on_jump += fetch_latency; // it is possible that one fetch request is on the fly when jumping, which is omitted. Since the memory cannot interrupt inflight requests, it has to wait until the omitted memory request is completed.

		// NOTICE: the ready_cycle_on_jump has to be increased by fetch_latency since it is the latency and not the time needed to fetch.
		// So the first fetchword is ready at: 2x(fetch_latency)+1 == 2x(time_to_fetch_one_word)-1

		uint32_t ready_cycle = ready_cycle_on_jump;

		for(uint32_t i = 0; i < IWS; i++)
		{
			instr_windows_t fetch_rq;
			fetch_rq.address = iw_addr;
			fetch_rq.valid_at = ready_cycle;
			iw->push_back(fetch_rq);

			// address of next fetch word:
			iw_addr += 0x8;
			// ready time of next fetch word:
			ready_cycle += fetch_latency + 1;
		}
	}
	assert(iw->size() == IWS);
}


void CarCoreTiming::updateIWOnExecution(uint32_t instruction_addr, uint32_t instruction_length, vector<instr_windows_t> *iw)
{
	uint32_t instruction_start_iw_addr = TO_FETCH_ADDR(instruction_addr);
	uint32_t next_instruction_iw_addr = TO_FETCH_ADDR(instruction_addr + instruction_length);

	assert(iw->front().address == instruction_start_iw_addr);

	if(instruction_start_iw_addr != next_instruction_iw_addr)
	{
		assert(instruction_start_iw_addr + 0x8 == next_instruction_iw_addr);

		// kick out first instruction word
		iw->erase(iw->begin());

		if(iw->size() < IWS)
		{
			// fetch another instuction word
			instr_windows_t last_entry = iw->back();
			instr_windows_t next_entry;
			next_entry.address = last_entry.address+0x8;
			next_entry.valid_at = last_entry.valid_at + fetch_latency + 1;
			iw->push_back(next_entry);
		}
	}
}

void CarCoreTiming::updateIWOnExecution(uint32_t instruction_addr, uint32_t instruction_length, vector<instr_windows_t> *iw, uint32_t fetch_rq_cycle)
{
	uint32_t instruction_start_iw_addr = TO_FETCH_ADDR(instruction_addr);
	uint32_t next_instruction_iw_addr = TO_FETCH_ADDR(instruction_addr + instruction_length);

	assert(iw->front().address == instruction_start_iw_addr);

	if(instruction_start_iw_addr != next_instruction_iw_addr)
	{
		assert(instruction_start_iw_addr + 0x8 == next_instruction_iw_addr);

		// kick out first instruction word
		iw->erase(iw->begin());

		if(iw->size() < IWS)
		{
			// fetch another instuction word
			instr_windows_t last_entry = iw->back();
			instr_windows_t next_entry;
			next_entry.address = last_entry.address+0x8;
			if(last_entry.valid_at >= fetch_rq_cycle)
			{
				// if there is a new fetch request before the previous one was finished, start the fetch directly after the completion of the previous fetch request
				next_entry.valid_at = last_entry.valid_at + fetch_latency + 1; // one cycle extra to route the result to pipeline
			}
			else
			{
				// wait until the fetch is requested
				next_entry.valid_at = fetch_rq_cycle + fetch_latency + 1 + 1; // one cycle to forward the fetch rq to memory and one back
			}
			iw->push_back(next_entry);
		}
	}
}


void CarCoreTiming::storeIWForSubsequentBB(uint32_t next_bb_addr, vector<instr_windows_t> *iw, uint32_t base_cycle)
{
	vector<instr_windows_t>::iterator it;
	vector<instr_windows_t> *piw = new vector<instr_windows_t>(*iw);
	it = piw->begin();
	
//	LOG_DEBUG(logger, "Pre stored IW state is: (base_cycle: "<< base_cycle << ")" );
//	printIW(&iw);

	while(it != piw->end())
	{
		if(it->address < TO_FETCH_ADDR(next_bb_addr))
		{
			// delete all iw entries that are before (in sense of address and cycle) the first instruction of the following bb
			it = piw->erase(it);
		}
		else
		{
			if(it->valid_at < base_cycle)
			{
				it->valid_at = 0;
			}
			else
			{
				it->valid_at -= base_cycle;
			}
			it++;
		}
	}

//	LOG_DEBUG(logger, "Stored IW state is:" );
//	printIW(&iw);

	bool ins_bool;
	BBAddrInitialIWMap::iterator pos;
	tie(pos, ins_bool) = initial_iw_map.insert(make_pair(next_bb_addr, piw));

	assert(ins_bool);
}

vector<instr_windows_t> CarCoreTiming::getInitialIWForBB(uint32_t bb_addr)
{

	BBAddrInitialIWMap::iterator pos;
	pos = initial_iw_map.find(bb_addr);

	if(pos == initial_iw_map.end())
	{
		assert(false);
	}

	vector<instr_windows_t> result(*(pos->second));

	// the mapped entry is no longer needed
	delete(pos->second);
	initial_iw_map.erase(pos);

	return result;
}

void CarCoreTiming::printIW(vector<instr_windows_t> *iw)
{
	ostringstream s;
	for(uint32_t i = 0; i < iw->size(); i++)
	{
		s << "0x" << hex << iw->at(i).address << "@" << dec << iw->at(i).valid_at << " ";
	}
	LOG_DEBUG(logger, s.str());
}
