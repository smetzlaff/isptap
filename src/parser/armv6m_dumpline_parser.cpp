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
#include "armv6m_dumpline_parser.hpp"
#include "regex_parser_tokens.h"

using namespace boost;

LoggerPtr Armv6mDumpLineParser::logger(Logger::getLogger("Armv6mDumpLineParser"));

Armv6mDumpLineParser *Armv6mDumpLineParser::singleton = NULL;

Armv6mDumpLineParser* Armv6mDumpLineParser::getInstance(void)
{
	if(singleton == NULL)
	{
		singleton = new Armv6mDumpLineParser;
	}
	return singleton;
}

Armv6mDumpLineParser::Armv6mDumpLineParser()
{
	isa = Armv6mISA::getInstance();
	initializeRegExs();
	testInstrClassification();
	LOG_DEBUG(logger, "Intialized DLP")
}

Armv6mDumpLineParser::~Armv6mDumpLineParser()
{
}

bool Armv6mDumpLineParser::isBranchInstr(string str)
{
	instr_type_t type = getInstructionType(str);
	return ((type == I_UNCOND_BRANCH)||(type == I_UNCOND_INDIRECT_BRANCH)||(type == I_COND_BRANCH)||(type == I_COND_INDIRECT_BRANCH)||(type == I_CALL)||(type == I_INDIRECT_CALL)||(type == I_RETURN));
}

bool Armv6mDumpLineParser::isReturnInstr(string str)
{
	// Notice to also detect BX jumps to registers POPped from the stack use isReturnInstr(string str, vector<string> bb_code)

	instr_type_t type = getInstructionType(str);
	if(type == I_RETURN)
	{
		return true;
	}
	else if(type == I_UNCOND_INDIRECT_BRANCH)
	{
		return false;
	}
	
	return false;
}

bool Armv6mDumpLineParser::isReturnInstr(string str, vector<string> bb_code)
{
	instr_type_t type = getInstructionType(str);
	if(type == I_RETURN)
	{
		return true;
	}
	else if(type == I_UNCOND_INDIRECT_BRANCH)
	{
		armv6m_registers_t jump_reg = isa->getRm(getInstructionFromCodeLine(str));
		for(vector<string>::iterator it = bb_code.begin(); it != bb_code.end(); it++)
		{
			if(isa->classifyInstruction(*it) == POP)
			{
				vector<armv6m_registers_t> pop_regs = isa->getRegistersFromRegisterList(*it);
				for(vector<armv6m_registers_t>::iterator rit = pop_regs.begin(); rit != pop_regs.end(); rit++)
				{
					if(*rit == jump_reg)
					{
						LOG_DEBUG(logger, "Detected return by BX and a POP for the same register: R" <<  jump_reg << " pop was: " << *it);
						return true;
					}
				}
			}
		}
		return false;
	}
	
	return false;
}

bool Armv6mDumpLineParser::isDebugInstr(string str)
{
	return (getInstructionType(str) == I_DEBUG);
}

bool Armv6mDumpLineParser::isCallInstr(string str)
{
	return (getInstructionType(str) == I_CALL);
}

bool Armv6mDumpLineParser::isCondBranchInstr(string str)
{
	instr_type_t type = getInstructionType(str);
	return ((type == I_COND_BRANCH)||(type == I_COND_INDIRECT_BRANCH));
}

bool Armv6mDumpLineParser::isIndirectBranchInstr(string str)
{
	instr_type_t type = getInstructionType(str);
	assert(type != I_COND_INDIRECT_BRANCH); // conditional indirect branches are not supported!
	return ((type == I_UNCOND_INDIRECT_BRANCH)||(type == I_COND_INDIRECT_BRANCH));
}

bool Armv6mDumpLineParser::isIndirectCallInstr(string str)
{
	return (getInstructionType(str) == I_INDIRECT_CALL);
}


string Armv6mDumpLineParser::getCommentFromCodeLine(string str)
{
	vector<string> strs;
// FIXME: providing everything after the last <TAB> is not enough, since in the comment also tabs are possible
	split(strs, str, boost::is_any_of("\t"));

	return strs.back();
}

string Armv6mDumpLineParser::assembleCodeLine(uint32_t opcode_address, string opcode_raw, string opcode_dump)
{
	stringstream result;
	char address[4+1];
	sprintf(address, "%04x", opcode_address);

	result << address << ":\t";

	if(opcode_raw.size() == 4)
	{
		result << opcode_raw[0] << opcode_raw[1] << opcode_raw[2] << opcode_raw[3];
		result << "    ";
	}
	else if(opcode_raw.size() == 8)
	{
		result << opcode_raw[0] << opcode_raw[1] << opcode_raw[2] << opcode_raw[3];
		result << " " << opcode_raw[4] << opcode_raw[5] << opcode_raw[6] << opcode_raw[7];
	}
	else
	{
		// sorry wrong opcode length
		LOG_DEBUG(logger, "wrong opcode length: " << opcode_raw);
		assert(false);
	}

	result << "\t" << opcode_dump;

	return result.str();
}

string Armv6mDumpLineParser::updateAddressInDumpInstruction(string opcode_dump, uint32_t address)
{
	char newAddr[4+1];
	sprintf(newAddr, "%04x", address);
	string result;
	result = regex_replace(opcode_dump, regex(HEX_TOKEN"{4}"), string(newAddr), match_default | format_first_only);
	return result;
}


void Armv6mDumpLineParser::initializeRegExs(void)
{
	// regular expression to determining valid code lines (identified by address: "0000xxxx:")
//	re_addr = regex("^[[:space:]]?("HEX_TOKEN"{8})|("HEX_TOKEN"{4}):");
//	re_addr = regex("("HEX_TOKEN"{8})|("HEX_TOKEN"{4}):");
	re_addr = regex(HEX_TOKEN"{4}:");
	// regular expression to determine jump/call labels
	re_label = regex(HEX_TOKEN"{8}[[:space:]]<[^>]*>:");
	// regular expression to find the 16 or 32 bit opcode in dump file
	re_opcode = regex("[[:space:]]((["HEX_TOKEN"{4}[[:space:]])+)|("HEX_TOKEN"{8}[[:space:]])");
	// regular expression to find memory holes inserted by compiler
	re_memory_hole = regex("^[[:space:]]+[.period.][.period.][.period.][[:space:]]*$");
	// regular expression to delete spaces in the opcode string
	pattern = regex("/ //gi", regex_constants::icase|regex_constants::perl);
}

void Armv6mDumpLineParser::testInstrClassification(void)
{
}

instr_type_t Armv6mDumpLineParser::getInstructionType(string str)
{
	assert(isCodeLine(str));

	switch(isa->classifyInstruction(getInstructionFromCodeLine(str)))
	{
		case STR_R:
		case STRH_R:
		case STRB_R:
		case STR_I:
		case STRB_I:
		case STRH_I:
		case STM: // STM stores multiple registers
		case PUSH: // PUSH stores multiple registers
			LOG_DEBUG(logger, "0x" << str << " is store");
			return I_STORE;
		case LDR_L:
		case LDRSB_R:
		case LDR_R:
		case LDRH_R:
		case LDRB_R:
		case LDRSH_R:
		case LDR_I:
		case LDRB_I:
		case LDRH_I:
		case LDM: // LDM loads multiple registers
			LOG_DEBUG(logger, "0x" << str << " is load");
			return I_LOAD;
		case POP: // POP loads multiple registers
			if(!isa->isPCinRegisterList(getInstructionFromCodeLine(str)))
			{
				LOG_DEBUG(logger, "0x" << str << " is load");
				return I_LOAD;
			}
			else
			{
				// a return typically pops the safed pc from the stack and puts it in the PC register
				LOG_DEBUG(logger, "0x" << str << " is return (POP)");
				return I_RETURN;
			}
		case B:
			LOG_DEBUG(logger, "0x" << str << " is uncond branch");
			return I_UNCOND_BRANCH;
		case BX:
			// TODO some BX instructions may also be used as returns:
			// If the jump is directed to a register which is poped 
			// before and contains the LR (Link Register) value.
			// Or if the target register is the LR.
			// XXX The first case cannot be handled here, a DFA is 
			// needed to solve detect this kind of returns.
			if(isa->getRm(getInstructionFromCodeLine(str)) == LR)
			{
				// Indirect jump to the Link Register content is 
				// handled as a return instruction.
				LOG_DEBUG(logger, "0x" << str << " is return (BX)");
				return I_RETURN;
			}
			else
			{
				LOG_DEBUG(logger, "0x" << str << " is uncond indirect branch");
				return I_UNCOND_INDIRECT_BRANCH;
			}
		case BC:
			LOG_DEBUG(logger, "0x" << str << " is cond branch");
			return I_COND_BRANCH;
		case BL:
			LOG_DEBUG(logger, "0x" << str << " is call");
			return I_CALL;
		case BLX:
			LOG_DEBUG(logger, "0x" << str << " is indirect call");
			return I_INDIRECT_CALL;
		case CPS:
		case BKPT:
		case YIELD:
		case WFE:
		case WFI:
		case SEV:
		case SVC:
		case MSR_R:
		case MRS_R:
			LOG_DEBUG(logger, "0x" << str << " is other");
			return I_OTHERS;
		case DSB:
		case DMB:
		case ISB:
			LOG_DEBUG(logger, "0x" << str << " is sync");
			return I_SYNC;
		case UDF:
			LOG_DEBUG(logger, "0x" << str << " is undefined");
			return I_UNKNOWN;
		default:
			LOG_DEBUG(logger, "0x" << str << " is arithmetic");
			return I_ARITHMETIC;
	}


	return I_UNKNOWN;
}

bool Armv6mDumpLineParser::isWordDirectiveLine(string str)
{
	if(isCodeLine(str) && (str.find(".word") != string::npos))
	{
		return true;
	}
	else
	{
		return false;
	}
}
