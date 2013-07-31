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
#include "armv6m_core_timing.hpp"

#define MAX(a, b) (a > b)?(a):(b)

LoggerPtr Armv6mCoreTiming::logger(Logger::getLogger("Armv6mCoreTiming"));


Armv6mCoreTiming::Armv6mCoreTiming(memory_mode_t mem_type)
{
	dlp = Armv6mDumpLineParser::getInstance();
	arch = Armv6mISA::getInstance();
	armcfg = Armv6mConfig::getInstance();
	memory_type = mem_type;
}

Armv6mCoreTiming::~Armv6mCoreTiming()
{
}

cycles_t Armv6mCoreTiming::getCycleCountForInstructions(string code, bool precessing_bb_was_executed, uint32_t bb_start_addr, uint32_t bb_end_addr)
{
	uint32_t cycles = 0;
	vector<string> instrs;
	split(instrs, code, boost::is_any_of("\r"));

	LOG_DEBUG(logger, "Timing for: 0x" << hex << bb_start_addr << " to 0x" << bb_end_addr);

	// clear non code lines from vector
	vector<string>::iterator it;
	for(it=instrs.begin(); it < instrs.end(); it++)
	{
		if(!dlp->isCodeLine(*it))
		{
			instrs.erase(it);
		}
	}

	uint32_t fetch_buffer = 0;

	if(precessing_bb_was_executed)
	{
		fetch_buffer = getInitialFetchBufferState(bb_start_addr);
	}

	for(uint32_t i=0; i < instrs.size(); i++)
	{
		assert(dlp->isCodeLine(instrs[i]));
		string instruction = dlp->getInstructionFromCodeLine(instrs[i]);

		cycles += getInstructionsFetchLatency(dlp->getAddrFromCodeLine(instrs[i]), arch->getInstructionLength(instruction), &fetch_buffer);

		switch(arch->classifyInstruction(instruction))
		{
			case MUL_R:
				LOG_TRACE(logger, "MUL_R: +" << 1 + armcfg->getMultLatency() << " cycles");
				cycles += 1 + armcfg->getMultLatency();
				break;
			case MRS_R:
				LOG_TRACE(logger, "MRS_R: +" << 1 + armcfg->getMRSLatency() << " cycles");
				cycles += 1 + armcfg->getMRSLatency();
				break;
			case MSR_R:
				LOG_TRACE(logger, "MSR_R: +" << 1 + armcfg->getMSRLatency() << " cycles");
				cycles += 1 + armcfg->getMSRLatency();
				break;
			case B:
				LOG_TRACE(logger, "B: +" << 1 + armcfg->getUncondBranchLatency() << " cycles");
				cycles += 1 + armcfg->getUncondBranchLatency();
				break;
			case BC:
				{
					uint32_t taken = armcfg->getCondBranchLatency(true);
					uint32_t nottaken = armcfg->getCondBranchLatency(false);
					LOG_TRACE(logger, "BC: + " << 1 + taken << " | " << 1 + nottaken << " cycles");
					cycles += 1 + MAX(taken, nottaken);
				}
				break;
			case BL:
				LOG_TRACE(logger, "BL: +" << 1 + armcfg->getBLLatency() << " cycles");
				cycles += 1 + armcfg->getBLLatency();
				break;
			case BX:
				LOG_TRACE(logger, "BX: +" << 1 + armcfg->getBXLatency() << " cycles");
				cycles += 1 + armcfg->getBXLatency();
				break;
			case BLX:
				LOG_TRACE(logger, "BLX: +" << 1 + armcfg->getBLXLatency() << " cycles");
				cycles += 1 + armcfg->getBLXLatency();
				break;
			case LDR_L:
			case LDRSB_R:
			case LDR_R:
			case LDRH_R:
			case LDRB_R:
			case LDRSH_R:
			case LDR_I:
			case LDRB_I:
			case LDRH_I:
				LOG_TRACE(logger, "LDR_X: +" << 1 + armcfg->getLoadLatency(-1,memory_type == ONCHIP) << " cycles");
				cycles += 1 + armcfg->getLoadLatency(-1,memory_type == ONCHIP);
				break;
			case LDM: // LDM loads multiple registers
				LOG_TRACE(logger, "LDM: +" << 1 + arch->getNumberOfRegistersFromList(instruction) * armcfg->getLoadLatency(-1, memory_type == ONCHIP) << " cycles");
				cycles += 1 + arch->getNumberOfRegistersFromList(instruction) * armcfg->getLoadLatency(-1, memory_type == ONCHIP);
				break;
			case POP: // POP loads multiple registers
				LOG_TRACE(logger, "POP: +" << 1 + arch->getNumberOfRegistersFromList(instruction) * armcfg->getLoadLatency(-1, memory_type == ONCHIP) + ((arch->isPCinRegisterList(instruction))?(armcfg->getPopAndReturnExtraLatency()):(0)) << " cycles");
				cycles += 1 + arch->getNumberOfRegistersFromList(instruction) * armcfg->getLoadLatency(-1, memory_type == ONCHIP);
				if(arch->isPCinRegisterList(instruction))
				{
					cycles += armcfg->getPopAndReturnExtraLatency();
				}
				break;
			case STR_R:
			case STRH_R:
			case STRB_R:
			case STR_I:
			case STRB_I:
			case STRH_I:
				LOG_TRACE(logger, "STR_X: +" << 1 + armcfg->getStoreLatency(-1,memory_type == ONCHIP) << " cycles");
				cycles += 1 + armcfg->getStoreLatency(-1, memory_type==ONCHIP);
				break;
			case STM: // STM stores multiple registers
			case PUSH: // PUSH stores multiple registers
				LOG_TRACE(logger, "STM_X/PUSH: +" <<  1 + arch->getNumberOfRegistersFromList(instruction) * armcfg->getStoreLatency(-1, memory_type==ONCHIP) << " cycles");
				cycles += 1 + arch->getNumberOfRegistersFromList(instruction) * armcfg->getStoreLatency(-1, memory_type==ONCHIP);
				break;
			case DSB:
				LOG_TRACE(logger, "DSB: +" << 1 + armcfg->getDSBLatency() << " cycles");
				cycles += 1 + armcfg->getDSBLatency();
				break;
			case DMB:
				LOG_TRACE(logger, "DMB: +" << 1 + armcfg->getDMBLatency() << " cycles");
				cycles += 1 + armcfg->getDMBLatency();
				break;
			case ISB:
				LOG_TRACE(logger, "ISB: +" << 1 + armcfg->getISBLatency() << " cycles");
				cycles += 1 + armcfg->getISBLatency();
				break;
			default:
				LOG_TRACE(logger, "Arith: +" << 1 + armcfg->getArithmeticOpLatency(false) << " cycles");
				// all other instruction have the latency of arithmetic operations
				cycles += 1 + armcfg->getArithmeticOpLatency(false);
		}


	}

	cycles_t cycles_ret;
	cycles_ret.forward_step = cycles;
	cycles_ret.jump = cycles;

	return cycles_ret;
}

cycles_t Armv6mCoreTiming::getCycleCountForInstructionsWithBlockExitSensitivity(string code, bool precessing_bb_was_executed, uint32_t bb_start_addr, uint32_t bb_end_addr)
{
	cycles_t cycles_ret;
	// Notice that only iff a conditional jump occurs in the bb the "forward_step" and "jump" times differ. Therefore, it is safe to always compute both times, even if no "jump" (in case of no jump at the end of the bb) or "forward_step" (in case of an unconditional jump or call) edge may be present.
	cycles_ret.forward_step = 0;
	cycles_ret.jump = 0;
	vector<string> instrs;
	split(instrs, code, boost::is_any_of("\r"));

	LOG_DEBUG(logger, "Timing for: 0x" << hex << bb_start_addr << " to 0x" << bb_end_addr);

	// clear non code lines from vector
	vector<string>::iterator it;
	for(it=instrs.begin(); it < instrs.end(); it++)
	{
		if(!dlp->isCodeLine(*it))
		{
			instrs.erase(it);
		}
	}

	uint32_t fetch_buffer = 0;

	if(precessing_bb_was_executed)
	{
		fetch_buffer = getInitialFetchBufferState(bb_start_addr);
	}

	for(uint32_t i=0; i < instrs.size(); i++)
	{
		assert(dlp->isCodeLine(instrs[i]));
		string instruction = dlp->getInstructionFromCodeLine(instrs[i]);

		uint32_t fetch_latency = getInstructionsFetchLatency(dlp->getAddrFromCodeLine(instrs[i]), arch->getInstructionLength(instruction), &fetch_buffer);
		cycles_ret.forward_step += fetch_latency;
		cycles_ret.jump += fetch_latency;
	
		switch(arch->classifyInstruction(instruction))
		{
			case MUL_R:
				LOG_TRACE(logger, "MUL_R: +" << 1 + armcfg->getMultLatency() << " cycles");
				cycles_ret.forward_step += 1 + armcfg->getMultLatency();
				cycles_ret.jump += 1 + armcfg->getMultLatency();
				break;
			case MRS_R:
				LOG_TRACE(logger, "MRS_R: +" << 1 + armcfg->getMRSLatency() << " cycles");
				cycles_ret.forward_step += 1 + armcfg->getMRSLatency();
				cycles_ret.jump += 1 + armcfg->getMRSLatency();
				break;
			case MSR_R:
				LOG_TRACE(logger, "MSR_R: +" << 1 + armcfg->getMSRLatency() << " cycles");
				cycles_ret.forward_step += 1 + armcfg->getMSRLatency();
				cycles_ret.jump += 1 + armcfg->getMSRLatency();
				break;
			case B:
				LOG_TRACE(logger, "B: +" << 1 + armcfg->getUncondBranchLatency() << " cycles");
				cycles_ret.forward_step += 1 + armcfg->getUncondBranchLatency();
				cycles_ret.jump += 1 + armcfg->getUncondBranchLatency();
				break;
			case BC:
				LOG_TRACE(logger, "BC: + " << 1 + armcfg->getCondBranchLatency(false) << " | " << 1 + armcfg->getCondBranchLatency(true) << " cycles");
				cycles_ret.forward_step += 1 + armcfg->getCondBranchLatency(false);
				cycles_ret.jump += 1 + armcfg->getCondBranchLatency(true);
				break;
			case BL:
				LOG_TRACE(logger, "BL: +" << 1 + armcfg->getBLLatency() << " cycles");
				cycles_ret.forward_step += 1 + armcfg->getBLLatency();
				cycles_ret.jump += 1 + armcfg->getBLLatency();
				break;
			case BX:
				LOG_TRACE(logger, "BX: +" << 1 + armcfg->getBXLatency() << " cycles");
				cycles_ret.forward_step += 1 + armcfg->getBXLatency();
				cycles_ret.jump += 1 + armcfg->getBXLatency();
				break;
			case BLX:
				LOG_TRACE(logger, "BLX: +" << 1 + armcfg->getBLXLatency() << " cycles");
				cycles_ret.forward_step += 1 + armcfg->getBLXLatency();
				cycles_ret.jump += 1 + armcfg->getBLXLatency();
				break;
			case LDR_L:
			case LDRSB_R:
			case LDR_R:
			case LDRH_R:
			case LDRB_R:
			case LDRSH_R:
			case LDR_I:
			case LDRB_I:
			case LDRH_I:
				LOG_TRACE(logger, "LDR_X: +" << 1 + armcfg->getLoadLatency(-1,memory_type == ONCHIP) << " cycles");
				cycles_ret.forward_step += 1 + armcfg->getLoadLatency(-1, memory_type == ONCHIP);
				cycles_ret.jump += 1 + armcfg->getLoadLatency(-1, memory_type == ONCHIP);
				break;
			case LDM: // LDM loads multiple registers
				LOG_TRACE(logger, "LDM: +" << 1 + arch->getNumberOfRegistersFromList(instruction) * armcfg->getLoadLatency(-1, memory_type == ONCHIP) << " cycles");
				cycles_ret.forward_step += 1 + arch->getNumberOfRegistersFromList(instruction) * armcfg->getLoadLatency(-1, memory_type == ONCHIP);
				cycles_ret.jump += 1 + arch->getNumberOfRegistersFromList(instruction) * armcfg->getLoadLatency(-1, memory_type == ONCHIP);
				break;

			case POP: // POP loads multiple registers
				LOG_TRACE(logger, "POP: +" << 1 + arch->getNumberOfRegistersFromList(instruction) * armcfg->getLoadLatency(-1, memory_type == ONCHIP) + ((arch->isPCinRegisterList(instruction))?(armcfg->getPopAndReturnExtraLatency()):(0)) << " cycles");
				cycles_ret.forward_step += 1 + arch->getNumberOfRegistersFromList(instruction) * armcfg->getLoadLatency(-1,memory_type == ONCHIP);
				cycles_ret.jump += 1 + arch->getNumberOfRegistersFromList(instruction) * armcfg->getLoadLatency(-1,memory_type == ONCHIP);
				if(arch->isPCinRegisterList(instruction))
				{
					cycles_ret.forward_step += armcfg->getPopAndReturnExtraLatency();
					cycles_ret.jump += armcfg->getPopAndReturnExtraLatency();
				}
				break;
			case STR_R:
			case STRH_R:
			case STRB_R:
			case STR_I:
			case STRB_I:
			case STRH_I:
				LOG_TRACE(logger, "STR_X: +" << 1 + armcfg->getStoreLatency(-1,memory_type == ONCHIP) << " cycles");
				cycles_ret.forward_step += 1 + armcfg->getStoreLatency(-1, memory_type==ONCHIP);
				cycles_ret.jump += 1 + armcfg->getStoreLatency(-1, memory_type==ONCHIP);
				break;
			case STM: // STM stores multiple registers
			case PUSH: // PUSH stores multiple registers
				LOG_TRACE(logger, "STM/PUSH: +" <<  1 + arch->getNumberOfRegistersFromList(instruction) * armcfg->getStoreLatency(-1, memory_type==ONCHIP) << " cycles");
				cycles_ret.forward_step += 1 + arch->getNumberOfRegistersFromList(instruction) * armcfg->getStoreLatency(-1, memory_type==ONCHIP);
				cycles_ret.jump += 1 + arch->getNumberOfRegistersFromList(instruction) * armcfg->getStoreLatency(-1, memory_type==ONCHIP);
				break;
			case DSB:
				LOG_TRACE(logger, "DSB: +" << 1 + armcfg->getDSBLatency() << " cycles");
				cycles_ret.forward_step += 1 + armcfg->getDSBLatency();
				cycles_ret.jump += 1 + armcfg->getDSBLatency();
				break;
			case DMB:
				LOG_TRACE(logger, "DMB: +" << 1 + armcfg->getDMBLatency() << " cycles");
				cycles_ret.forward_step += 1 + armcfg->getDMBLatency();
				cycles_ret.jump += 1 + armcfg->getDMBLatency();
				break;
			case ISB:
				LOG_TRACE(logger, "ISB: +" << 1 + armcfg->getISBLatency() << " cycles");
				cycles_ret.forward_step += 1 + armcfg->getISBLatency();
				cycles_ret.jump += 1 + armcfg->getISBLatency();
				break;
			default:
				LOG_TRACE(logger, "Arith: +" << 1 + armcfg->getArithmeticOpLatency(false) << " cycles");
				// all other instruction have the latency of arithmetic operations
				cycles_ret.forward_step += 1 + armcfg->getArithmeticOpLatency(false);
				cycles_ret.jump += 1 + armcfg->getArithmeticOpLatency(false);
		}
	}

	return cycles_ret;
}

uint32_t Armv6mCoreTiming::getInstructionsFetchLatency(uint32_t address, uint32_t length, uint32_t *buffered_bytes)
{
	// TODO: refine fetch time model.
	uint32_t latency = 0;

	if(length == 2)
	{
		if(length < *buffered_bytes)
		{
			*buffered_bytes -= length;
			latency = 0;
		}
		else
		{
			// unaligned address
			// TODO: generalize
			assert(armcfg->getFetchBandwidth() == 32); // currently only fetch widths of 32 bit are supported
			if((address&3) != 0)
			{
				assert((address&1) == 0);
				// Fetch the whole bandwidth bytes but, omit the lower part in the buffer.
				latency = armcfg->getFetchLatency(memory_type == ONCHIP);
				*buffered_bytes += (armcfg->getFetchBandwidth()/8)/2; 
				assert(length <= *buffered_bytes);
				*buffered_bytes -= length;
			}
			else
			{
				latency = armcfg->getFetchLatency(memory_type == ONCHIP);
				*buffered_bytes += armcfg->getFetchBandwidth()/8;
				assert(length <= *buffered_bytes);
				*buffered_bytes -= length;
			}
		}
	}
	else if (length == 4)
	{
		if(length < *buffered_bytes)
		{
			assert(length <= *buffered_bytes);
			*buffered_bytes -= length;
			latency = 0;
		}
		else if((length - 2) < *buffered_bytes)
		{
			// The first half of the instruction is already buffered.
			// TODO: generalize
			assert(armcfg->getFetchBandwidth() == 32); // currently only fetch widths of 32 bit are supported
			// Fetch the 2nd part of the instruction. Also 16bit of the next instruction will be buffered.
			latency = armcfg->getFetchLatency(memory_type == ONCHIP);
			*buffered_bytes += armcfg->getFetchBandwidth()/8;
			assert(length <= *buffered_bytes);
			*buffered_bytes -= length;
		}
		else
		{
			// unaligned address
			// TODO: generalize
			assert(armcfg->getFetchBandwidth() == 32); // currently only fetch widths of 32 bit are supported
			if((address&3) != 0)
			{
				assert((address&1) == 0);
				// Fetch the whole bandwidth bytes but, omit the lower part in the buffer.
				latency = armcfg->getFetchLatency(memory_type == ONCHIP);
				*buffered_bytes += (armcfg->getFetchBandwidth()/8)/2; 
				// Due to the unaligned address, there are two fetches required for the 32 bit instruction.
				// 16Bit of the next instruction will be buffered.
				latency += armcfg->getFetchLatency(memory_type == ONCHIP);
				*buffered_bytes += armcfg->getFetchBandwidth()/8; 
				assert(length <= *buffered_bytes);
				*buffered_bytes -= length;
			}
			else
			{
				latency = armcfg->getFetchLatency(memory_type == ONCHIP);
				*buffered_bytes += armcfg->getFetchBandwidth()/8;
				assert(length <= *buffered_bytes);
				*buffered_bytes -= length;
			}
		}
	}
	else
	{
		assert(false);
	}

	return latency;
}

uint32_t Armv6mCoreTiming::getInitialFetchBufferState(uint32_t address)
{
	// TODO: generalize
	assert(armcfg->getFetchBandwidth() == 32); // currently only fetch widths of 32 bit are supported
	if((address&3) != 0)
	{
		assert((address&1) == 0);
		// The address is not aligned to the fetch word.
		// The fetch buffer contains the upper half of a fetch word that was not consumed by the last basic block.
		return (armcfg->getFetchBandwidth()/8)/2;
	}
	else
	{
		// The address is aligned to the fetch word.
		// Thus the whole fetch word was consumed by any preceding basic block.
		return 0;
	}
}
