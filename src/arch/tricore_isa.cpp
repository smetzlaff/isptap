/*******************************************************************************
 * ISPTAP - Instruction Scratchpad Timing Analysis Program
 * Copyright (C) 2013 Stefan Metzlaff & Jörg Mische, University of Augsburg, Germany
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
#include "tricore_isa.hpp"
#include "constants.h"


/**
 * Calculates the target address of a jump in disp4 format.
 */
#define CALC_DISP_4_ADD_PC(instr, base_addr) (base_addr + (((instr&0x0f00)>>8)<<1))

/**
 * Sets the relative diplacement in disp4 format of an jump instruction
 */
#define SET_DISP_4_OFFSET(instr, offset) ((instr&(~0x0f00))|(((uint32_t)((int16_t)offset>>1)<<8)&0x0f00))

/**
 * Calculates the target address of a jump in disp4 format used by the LOOP instruction.
 */
#define CALC_DISP_4_LOOP(instr, base_addr) (base_addr + (int32_t(0xffffffe0 | (((instr&0x0f00)>>8)<<1))))

/**
 * Sets the relative diplacement in disp4 format of a LOOP instruction
 */
#define SET_DISP_4_LOOP_OFFSET(instr, offset) ((instr&(~0x0f00))|(((uint32_t)((((int32_t)offset)&(~0xffffffe0))>>1)<<8)&0x0f00))
/**
 * Calculates the target address of a jump in disp8 format.
 */
#define CALC_DISP_8_ADD_PC(instr, base_addr) (base_addr + (int8_t(((instr&0xff00)>>8))<<1))

/**
 * Sets the relative diplacement in disp8 format of an jump instruction
 */
#define SET_DISP_8_OFFSET(instr, offset) ((instr&(~0xff00))|(((uint32_t)(((int8_t)offset)>>1)<<8)&0xff00))

/**
 * Calculates the target address of a jump in disp15 format.
 */
#define CALC_DISP_15_ADD_PC(instr, base_addr) (base_addr + (int16_t(((instr&0x7fff0000)>>16)<<1)))

/**
 * Sets the relative diplacement in disp15 format of an jump instruction
 */
#define SET_DISP_15_OFFSET(instr, offset) ((instr&(~0x7fff0000))|(((uint32_t)(((int16_t)offset)>>1)<<16)&0x7fff0000))

/**
 * Calculates the target address of a jump in disp24 format.
 */
#define CALC_DISP_24_ADD_PC(instr, base_addr) (base_addr + (int32_t(((instr&0x00008000)!=0x8000)?(/*forward jump*/(((((instr&0xffff0000)>>16)|((instr&0x0000ff00)<<8))<<1))):(/*backward jump*/(((((instr&0xffff0000)>>16)|((instr&0x0000ff00)<<8))<<1))|0xff000000))))

/**
 * Sets the relative diplacement in disp24 format of an jump instruction
 */
#define SET_DISP_24_OFFSET(instr, offset) ((offset >= 0)?(/*forward jump*/(instr&(~0xffffff00))|(((((offset>>1)&0xffff)<<16)|(((offset>>1)&0x7f0000)>>8))&0xffff7f00)):(/*backward jump*/((instr&(~0xffffff00))|(((((offset>>1)&0xffff)<<16)|(((offset>>1)&0x7f0000)>>8))&0xffff7f00))|0x8000/*<-sign*/))

/**
 * get the constant 16 as for ADDI and MOVH
 */
#define GET_CONS16(instr) ((uint16_t)((instr&0x0ffff000)>>12))

/**
 * sets the constant 16 as for ADDI and MOVH
 */
#define SET_CONS16(instr, immediate) (((instr&0xf0000fff)|((uint32_t)immediate<<12)))

/**
 * checks if an instruction is the DISP activate instruction
 */
#define IS_DISP_ACTIVATE(instr) ((instr&0x0ffff0ff) == (0x060280cd))

/**
 * checks if an instruction is the DISP function length encoding instruction
 */
#define IS_DISP_INSTRUMENT(instr) ((instr&0xff0000ff) == (0x0e0000cd))

/**
 * obtains the source register from an MTCR instruction opcode
 */
#define GET_MTCR_SRC_REG(a) ((a&0x00000f00)>>8)

/**
 * obtains the target CSFR from an MTCR instruction opcode
 */
#define GET_MTCR_CSFR_REG(a) ((a&0x0ffff000)>>12)


LoggerPtr TricoreISA::logger(Logger::getLogger("TricoreISA"));

TricoreISA *TricoreISA::singleton = NULL;


TricoreISA* TricoreISA::getInstance(void)
{
	if(singleton == NULL)
	{
		singleton = new TricoreISA;
	}
	return singleton;
}

TricoreISA::TricoreISA()
{
	instructions["6d"] = CALL_32;
	instructions["5c"] = CALL_16;
	instructions["2d"] = CALL_32_I; // same as JI 32
	instructions["1d"] = J_32;
	instructions["3c"] = J_16;
	instructions["df"] = JEQ_32_BRC; // same as JNE_32_BRC
	instructions["5f"] = JEQ_32_BRR; // same as JNE_32_BRR
	instructions["1e"] = JEQ_16_SBC;
	instructions["3e"] = JEQ_16_SBR;
	instructions["7d"] = JEQA_32_BRR;
	instructions["ff"] = JGE_32_BRC;
	instructions["7f"] = JGE_32_BRR;
	instructions["ce"] = JGEZ_16_SBR;
	instructions["4e"] = JGTZ_16_SBR;
//	instructions["2d"] = JI_32;		// same as CALL_32_I
	instructions["dc"] = JI_16;
	instructions["8e"] = JLEZ_16_SBR;
	instructions["bf"] = JLT_32_BRC;
	instructions["3f"] = JLT_32_BRR;
	instructions["0e"] = JLTZ_16_SBR;
//	instructions["df"] = JNE_32_BRC; // same as JEQ_32_BRC
//	instructions["5f"] = JNE_32_BRR; // same as JEQ_32_BRR
	instructions["5e"] = JNE_16_SBC;
	instructions["7e"] = JNE_16_SBR;
//	instructions["6f"] = JNZT_32_BRN; // same as JZT_32_BRN
	instructions["ae"] = JNZT_16_SBRN;
	instructions["9f"] = JNED_32_BRC;
	instructions["1f"] = JNED_32_BRR;
	instructions["ee"] = JNZ_16_SB;
	instructions["f6"] = JNZ_16_SBR;
//	instructions["bd"] = JNZA_32_BRR; // same as JZA_32_BRR
	instructions["7c"] = JNZA_16_SBR;
	instructions["6e"] = JZ_16_SB;
	instructions["76"] = JZ_16_SBR;
	instructions["bd"] = JZA_32_BRR; // same as JNZA_32_BRR
	instructions["bc"] = JZA_16_SBR;
	instructions["6f"] = JZT_32_BRN; // same as JNZT_32_BRN
	instructions["2e"] = JZT_16_SBRN;
	instructions["fd"] = LOOP_32;
	instructions["fc"] = LOOP_16;
	instructions["00"] = RET;
	instructions["ef"] = TASKING_JZT_32; // this instruction appears in tasking generated code but is not documented in tricore 1 isa manual

	instructions["1b"] = ADDI;
	instructions["7b"] = MOVH;
	instructions["cd"] = MTCR;

	instructions["3b"] = MOV_RLC;
	instructions["0b"] = MOV_RR;
	instructions["da"] = MOV_SC;
	instructions["82"] = MOV_SRC;
}

TricoreISA::~TricoreISA()
{
}

jump_target_address_t TricoreISA::getJumpTargetAddr(string instruction, uint32_t curr_addr)
{
	jump_target_address_t result;
	result.valid = false;
	result.addr = UNKNOWN_ADDR;

	string a_instruction = adjustByteOrder(instruction);
	uint32_t instr = strtoul(a_instruction.c_str(), NULL, 16);

	// getting the op1 field out of instruction
	string op1;
	if(instruction.length() == 4)
		op1 = a_instruction.substr(2,2);
	else
		op1 = a_instruction.substr(6,2);


	LOG_DEBUG(logger, "comparing: " << op1 <<  hex << " 0x" << instr);

	switch(instructions.find(op1)->second)
	{
		case CALL_32:
		case J_32:
			{
				result.addr = CALC_DISP_24_ADD_PC(instr, curr_addr);
				LOG_DEBUG(logger, "Found DISP_24: " << a_instruction << " target is: " << hex << result.addr);
				result.valid = true;
				break;
			}
		case JEQ_32_BRC:
		case JEQ_32_BRR:
		case JEQA_32_BRR:
		case JGE_32_BRC:
		case JGE_32_BRR:
		case JLT_32_BRC:
		case JLT_32_BRR:
		case JNE_32_BRC:
		case JNE_32_BRR:
		case JNED_32_BRC:
		case JNED_32_BRR:
		case JNZA_32_BRR:
		case JNZT_32_BRN:
		case JZA_32_BRR:
		case JZT_32_BRN:
		case LOOP_32:
		case TASKING_JZT_32:
			{
				result.addr = CALC_DISP_15_ADD_PC(instr, curr_addr);
				LOG_DEBUG(logger, "Found DISP_15: " << a_instruction << " target is : " << hex << result.addr);
				result.valid = true;
				break;
			}
		case CALL_16:
		case J_16:
		case JNZ_16_SB:
		case JZ_16_SB:
			{
				result.addr = CALC_DISP_8_ADD_PC(instr, curr_addr);
				LOG_DEBUG(logger, "Found DISP_8: " << a_instruction << " target is : " << hex << result.addr);
				result.valid = true;
				break;
			}
		case JEQ_16_SBC:
		case JEQ_16_SBR:
		case JGEZ_16_SBR:
		case JGTZ_16_SBR:
		case JLEZ_16_SBR:
		case JLTZ_16_SBR:
		case JNE_16_SBC:
		case JNE_16_SBR:
		case JNZ_16_SBR:
		case JNZA_16_SBR:
		case JNZT_16_SBRN:
		case JZ_16_SBR:
		case JZA_16_SBR:
		case JZT_16_SBRN:
			{
				result.addr = CALC_DISP_4_ADD_PC(instr, curr_addr);
				LOG_DEBUG(logger, "Found DISP_4: " << a_instruction << " target is : " << hex << result.addr);
				result.valid = true;
				break;
			}
		case LOOP_16:
			{
				result.addr = CALC_DISP_4_LOOP(instr, curr_addr);
				LOG_DEBUG(logger, "Found DISP_4_LOOP: " << a_instruction << " target is : " << hex << result.addr);
				result.valid = true;
				break;
			}

		case RET:
			{
				if(a_instruction.compare("9000") == 0)
				{
					LOG_DEBUG(logger, "Found RET: " << a_instruction);
				}
				else
				{
					LOG_DEBUG(logger, "Found part of RET" << a_instruction);
				}
				result.valid = false;
				break;
			}
		case CALL_32_I:
		case JI_32:
		case JI_16:
			{
				LOG_DEBUG(logger, "Indirect target cannot be determined!!");
				result.valid = false;
				break;
			}
		default:
			{
				LOG_DEBUG(logger, "Found ??: " << a_instruction);
				assert(false);
			}
	}

	
	return result;

}

displacement_type_t TricoreISA::getDisplacementType(string instruction)
{
	displacement_type_t return_value = UnknownDisplacementType;
	string a_instruction = adjustByteOrder(instruction);
	uint32_t instr = strtoul(a_instruction.c_str(), NULL, 16);

	// getting the op1 field out of instruction
	string op1;
	if(instruction.length() == 4)
		op1 = a_instruction.substr(2,2);
	else
		op1 = a_instruction.substr(6,2);


	LOG_DEBUG(logger, "comparing: " << op1 <<  hex << " 0x" << instr);

	switch(instructions.find(op1)->second)
	{
		case CALL_32:
		case J_32:
			{
				LOG_DEBUG(logger, "Found DISP_24: " << a_instruction);
				return_value = disp24;
				break;
			}
		case JEQ_32_BRC:
		case JEQ_32_BRR:
		case JEQA_32_BRR:
		case JGE_32_BRC:
		case JGE_32_BRR:
		case JLT_32_BRC:
		case JLT_32_BRR:
		case JNE_32_BRC:
		case JNE_32_BRR:
		case JNED_32_BRC:
		case JNED_32_BRR:
		case JNZA_32_BRR:
		case JNZT_32_BRN:
		case JZA_32_BRR:
		case JZT_32_BRN:
		case LOOP_32:
		case TASKING_JZT_32:
			{
				LOG_DEBUG(logger, "Found DISP_15: " << a_instruction);
				return_value = disp15;
				break;
			}
		case CALL_16:
		case J_16:
		case JNZ_16_SB:
		case JZ_16_SB:
			{
				LOG_DEBUG(logger, "Found DISP_8: " << a_instruction);
				return_value = disp8;
				break;
			}
		case JEQ_16_SBC:
		case JEQ_16_SBR:
		case JGEZ_16_SBR:
		case JGTZ_16_SBR:
		case JLEZ_16_SBR:
		case JLTZ_16_SBR:
		case JNE_16_SBC:
		case JNE_16_SBR:
		case JNZ_16_SBR:
		case JNZA_16_SBR:
		case JNZT_16_SBRN:
		case JZ_16_SBR:
		case JZA_16_SBR:
		case JZT_16_SBRN:
			{
				LOG_DEBUG(logger, "Found DISP_4: " << a_instruction);
				return_value = disp4;
				break;
			}
		case LOOP_16:
			{
				LOG_DEBUG(logger, "Found DISP_4_LOOP: " << a_instruction);
				return_value = disp4;
				break;
			}
		case RET:
			{
				if(a_instruction.compare("9000") == 0)
				{
					LOG_DEBUG(logger, "Found RET: " << a_instruction);
				}
				else
				{
					LOG_DEBUG(logger, "Found part of RET" << a_instruction);
				}
				return_value = NoDisplacement;
				break;
			}
		case CALL_32_I:
		case JI_32:
		case JI_16:
			{
				return_value = indirect;
				break;
			}
		default:
			{
				LOG_DEBUG(logger, "Instruction is no control flow changing instruction: " << a_instruction);
				assert(false);
			}
	}
	
	return return_value;

}

string TricoreISA::setJumpTargetAddr(string instruction, uint32_t curr_addr, uint32_t target_addr)
{
	string result;
	string a_instruction = adjustByteOrder(instruction);
	uint32_t instr = strtoul(a_instruction.c_str(), NULL, 16);

	// getting the op1 field out of instruction
	string op1;
	if(instruction.length() == 4)
		op1 = a_instruction.substr(2,2);
	else
		op1 = a_instruction.substr(6,2);


	// calculating jump offset
	int32_t offset = target_addr - curr_addr;

	LOG_DEBUG(logger, "comparing: " << op1 <<  hex << " 0x" << instr);

	switch(instructions.find(op1)->second)
	{
		case CALL_32:
		case J_32:
			{
				instr = SET_DISP_24_OFFSET(instr, offset);
				LOG_DEBUG(logger, "Found DISP_24: " << a_instruction << " new instruction with offset " << offset << " is: 0x" << hex << instr);
				break;
			}
		case JEQ_32_BRC:
		case JEQ_32_BRR:
		case JEQA_32_BRR:
		case JGE_32_BRC:
		case JGE_32_BRR:
		case JLT_32_BRC:
		case JLT_32_BRR:
		case JNE_32_BRC:
		case JNE_32_BRR:
		case JNED_32_BRC:
		case JNED_32_BRR:
		case JNZA_32_BRR:
		case JNZT_32_BRN:
		case JZA_32_BRR:
		case JZT_32_BRN:
		case LOOP_32:
		case TASKING_JZT_32:
			{
				instr = SET_DISP_15_OFFSET(instr, offset);
				LOG_DEBUG(logger, "Found DISP_15: " << a_instruction << " new instruction with offset " << offset << " is: 0x" << hex << instr);
				break;
			}
		case CALL_16:
		case J_16:
		case JNZ_16_SB:
		case JZ_16_SB:
			{
				instr = SET_DISP_8_OFFSET(instr, offset);
				LOG_DEBUG(logger, "Found DISP_8: " << a_instruction << " new instruction with offset " << offset << " is: 0x" << hex << instr);
				break;
			}
		case JEQ_16_SBC:
		case JEQ_16_SBR:
		case JGEZ_16_SBR:
		case JGTZ_16_SBR:
		case JLEZ_16_SBR:
		case JLTZ_16_SBR:
		case JNE_16_SBC:
		case JNE_16_SBR:
		case JNZ_16_SBR:
		case JNZA_16_SBR:
		case JNZT_16_SBRN:
		case JZ_16_SBR:
		case JZA_16_SBR:
		case JZT_16_SBRN:
			{
				instr = SET_DISP_4_OFFSET(instr, offset);
				LOG_DEBUG(logger, "Found DISP_4: " << a_instruction << " new instruction with offset " << offset << " is: 0x" << hex << instr);
				break;
			}
		case LOOP_16:
			{
				instr = SET_DISP_4_LOOP_OFFSET(instr, offset);
				LOG_DEBUG(logger, "Found DISP_4_LOOP: " << a_instruction << " new instruction with offset " << offset << " is: 0x" << hex << instr);
				break;
			}

		case RET:
			{
				break;
			}
		case CALL_32_I:
		case JI_32:
		case JI_16:
			{
				LOG_DEBUG(logger, "Indirect target cannot be determined!!");
				break;
			}
		default:
			{
				LOG_DEBUG(logger, "Found ??: " << a_instruction);
				assert(false);
			}
	}

	char jump_instruction[8];
	if(instruction.length() == 4)
	{
		sprintf(jump_instruction, "%04x", instr);
	}
	else
	{
		sprintf(jump_instruction, "%08x", instr);
	}

	return adjustByteOrder(string(jump_instruction));

}


string TricoreISA::adjustByteOrder(string instruction)
{
	char return_value[10];
	if(instruction.length() == 4)
	{
		sprintf(return_value, "%c%c%c%c",instruction[2],instruction[3],instruction[0],instruction[1]);
	}
	else if(instruction.length() == 8)
	{
		sprintf(return_value, "%c%c%c%c%c%c%c%c", instruction[6],instruction[7],instruction[4],instruction[5],instruction[2],instruction[3],instruction[0],instruction[1]);
	}
	return string(return_value);
}


uint32_t TricoreISA::getInstructionLength(string instruction)
{
	if(instruction.length() == 4)
	{
		return 2;
	}
	else if(instruction.length() == 8)
	{
		return 4;
	}
	else
		return 0;

}

bool TricoreISA::isDISPActivate(string instruction)
{
	uint16_t return_value=false;
	string a_instruction = adjustByteOrder(instruction);
	uint32_t instr = strtoul(a_instruction.c_str(), NULL, 16);

	// getting the op1 field out of instruction
	string op1;
	if(instruction.length() == 4)
		op1 = a_instruction.substr(2,2);
	else
		op1 = a_instruction.substr(6,2);


//	LOG_DEBUG(logger, "comparing: " << op1 <<  hex << " 0x" << instr);

	switch(instructions.find(op1)->second)
	{
		case MTCR:
		{
//			LOG_DEBUG(logger, "Found MTCR, checking address for " << instruction);
			return_value = IS_DISP_ACTIVATE(instr);
//			LOG_DEBUG(logger, "Compared 0x" << hex << instr << " whith 0x" << (0x060280cd) << " result is: " << return_value)
			break;
		}
		default:
		{
			return_value = false;
		}
	}
	return return_value;
}



uint16_t TricoreISA::getImmediate(string instruction)
{
	uint16_t return_value=0;
	string a_instruction = adjustByteOrder(instruction);
	uint32_t instr = strtoul(a_instruction.c_str(), NULL, 16);

	// getting the op1 field out of instruction
	string op1;
	if(instruction.length() == 4)
		op1 = a_instruction.substr(2,2);
	else
		op1 = a_instruction.substr(6,2);


//	LOG_DEBUG(logger, "comparing: " << op1 <<  hex << " 0x" << instr);

	switch(instructions.find(op1)->second)
	{
		case ADDI:
		case MOVH:
		{
			return_value = GET_CONS16(instr);
			LOG_DEBUG(logger, "Immediate of 0x" << instruction  << " is 0x" << hex << return_value);
			break;
		}
		default:
		{
			LOG_WARN(logger, "found instruction: " << a_instruction << " cannot decode");
			assert(false);
		}
	}
	return return_value;
}


string TricoreISA::setImmediate(string instruction, uint16_t immediate)
{
	string return_instr;
	string a_instruction = adjustByteOrder(instruction);
	uint32_t instr = strtoul(a_instruction.c_str(), NULL, 16);

	// getting the op1 field out of instruction
	string op1;
	if(instruction.length() == 4)
		op1 = a_instruction.substr(2,2);
	else
		op1 = a_instruction.substr(6,2);


	LOG_DEBUG(logger, "comparing: " << op1 <<  hex << " 0x" << instr);

	switch(instructions.find(op1)->second)
	{
		case ADDI:
		case MOVH:
		{
			instr = SET_CONS16(instr, immediate);
			break;
		}
		default:
		{
			LOG_WARN(logger, "found instruction: " << a_instruction << " cannot decode");
			assert(false);
		}
	}

	char new_instruction[8];
	if(instruction.length() == 4)
	{
		sprintf(new_instruction, "%04x", instr);
	}
	else
	{
		sprintf(new_instruction, "%08x", instr);
	}

	string result = adjustByteOrder(string(new_instruction));
	LOG_DEBUG(logger, "Updated immediate: " << result << " original was: " << instruction);
	return result;

}

uint32_t TricoreISA::convertInstructionToUint(string instruction)
{
	string a_instruction = adjustByteOrder(instruction);
	return strtoul(a_instruction.c_str(), NULL, 16);
}

bool TricoreISA::isMTCR(string instruction)
{

	string a_instruction = adjustByteOrder(instruction);

	// getting the op1 field out of instruction
	string op1;
	if(instruction.length() == 4)
		op1 = a_instruction.substr(2,2);
	else
		op1 = a_instruction.substr(6,2);


//	LOG_DEBUG(logger, "comparing: " << op1 <<  hex << " 0x" << instr);

	if((instructions.find(op1)->second) == MTCR)
	{
		return true;
	}

	return false;
}

mtcr_decode_t TricoreISA::decodeMTCR(string instruction)
{
	mtcr_decode_t return_value;
	return_value.csfr=0;
	return_value.src_reg=0;

	string a_instruction = adjustByteOrder(instruction);
	uint32_t instr = strtoul(a_instruction.c_str(), NULL, 16);

	// getting the op1 field out of instruction
	string op1;
	if(instruction.length() == 4)
		op1 = a_instruction.substr(2,2);
	else
		op1 = a_instruction.substr(6,2);


//	LOG_DEBUG(logger, "comparing: " << op1 <<  hex << " 0x" << instr);

	if((instructions.find(op1)->second) == MTCR)
	{
		uint16_t csfr_reg = GET_MTCR_CSFR_REG(instr);
		uint8_t src_reg = GET_MTCR_SRC_REG(instr);

		return_value.csfr = csfr_reg;
		return_value.src_reg = src_reg;
	}
	return return_value;
}

bool TricoreISA::isMOVI(string instruction)
{

	string a_instruction = adjustByteOrder(instruction);

	// getting the op1 field out of instruction
	string op1;
	if(instruction.length() == 4)
		op1 = a_instruction.substr(2,2);
	else
		op1 = a_instruction.substr(6,2);


//	LOG_DEBUG(logger, "comparing: " << op1 <<  hex << " 0x" << instr);

	switch(instructions.find(op1)->second)
	{
		case MOV_RLC:
		case MOV_SC:
		case MOV_SRC:
			return true;
		default:
			{
			}
	}
	return false;
}


movi_decode_t TricoreISA::decodeMOVI(string instruction)
{
	movi_decode_t return_value;
	return_value.imm=0;
	return_value.tgt_reg=0;

	string a_instruction = adjustByteOrder(instruction);
	uint32_t instr = strtoul(a_instruction.c_str(), NULL, 16);

	// getting the op1 field out of instruction
	string op1;
	if(instruction.length() == 4)
		op1 = a_instruction.substr(2,2);
	else
		op1 = a_instruction.substr(6,2);


//	LOG_DEBUG(logger, "comparing: " << op1 <<  hex << " 0x" << instr);

	switch(instructions.find(op1)->second)
	{
		case MOV_RLC:
			{
				return_value.tgt_reg = (instr&0xf0000000)>>28;
				return_value.imm = (instr&0x0ffff000)>>12;
				break;
			}
		case MOV_SC:
			{
				return_value.tgt_reg = 15;
				return_value.imm = (instr&0xff00)>>8;
				break;
			}
		case MOV_SRC:
			{
				return_value.tgt_reg = (instr&0x0f00)>>8;
				return_value.imm = (instr&0xf000)>>12;
				break;
			}
		default:
			{
			}
	}

	return return_value;
}


string TricoreISA::getConnectingJumpInstruction()
{
	char j_opcode[8+1];
	uint32_t max_disp24 = 0x00FFFFFF;
	uint32_t j32_opcode = 0x1d;
	// write opcode in wrong byte order, since the dlp->assembleCodeLine() corrects it.
	sprintf(j_opcode, "%02x%02x%04x", j32_opcode, ((max_disp24>>16)&0xff), (max_disp24&0xffff));

	return string(j_opcode);
}

string TricoreISA::getConnectingJumpComment()
{
	return string("j afffffff <XX> BBSISP_JP: inserted jump to preserve control flow");
}


