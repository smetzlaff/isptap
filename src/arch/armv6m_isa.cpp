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
#include "armv6m_isa.hpp"
#include "constants.h"

#define ARMV6M_LSL_I 0x00
#define ARMV6M_LSR_I 0x01
#define ARMV6M_ASR_I 0x02
#define ARMV6M_AOS 0x03 
#define ARMV6M_MOV_I 0x04
#define ARMV6M_CMP_I 0x05
#define ARMV6M_ADD_I8 0x06
#define ARMV6M_SUB_I8 0x07
#define ARMV6M_DP_SP 0x08
#define ARMV6M_LDR_L 0x09
#define ARMV6M_L_S_1 0x0a
#define ARMV6M_L_S_2 0x0b
#define ARMV6M_STR_I_T1 0x0c
#define ARMV6M_LDR_I_T1 0x0d
#define ARMV6M_STRB_I 0x0e
#define ARMV6M_LDRB_I 0x0f
#define ARMV6M_STRH_I 0x10
#define ARMV6M_LDRH_I 0x11
#define ARMV6M_STR_I_T2 0x12
#define ARMV6M_LDR_I_T2 0x13
#define ARMV6M_ADR_I 0x14
#define ARMV6M_ADD_SP_I 0x15
#define ARMV6M_MISC_1 0x16
#define ARMV6M_MISC_2 0x17
#define ARMV6M_STM 0x18
#define ARMV6M_LDM 0x19
#define ARMV6M_BC_1 0x1a
#define ARMV6M_BC_2 0x1b
#define ARMV6M_B 0x1c

#define ARMV6M_DP_AND_R 0x0
#define ARMV6M_DP_EOR_R 0x1
#define ARMV6M_DP_LSL_R 0x2
#define ARMV6M_DP_LSR_R 0x3
#define ARMV6M_DP_ASR_R 0x4
#define ARMV6M_DP_ADC_R 0x5
#define ARMV6M_DP_SBC_R 0x6
#define ARMV6M_DP_ROR_R 0x7
#define ARMV6M_DP_TST_R 0x8
#define ARMV6M_DP_RSB_I 0x9
#define ARMV6M_DP_CMP_R 0xa
#define ARMV6M_DP_CMN_R 0xb
#define ARMV6M_DP_ORR_R 0xc
#define ARMV6M_DP_MUL_R 0xd
#define ARMV6M_DP_BIC_R 0xe
#define ARMV6M_DP_MVN_R 0xf
#define ARMV6M_32BIT_INSTR 0x1e


LoggerPtr Armv6mISA::logger(Logger::getLogger("Armv6mISA"));

Armv6mISA *Armv6mISA::singleton = NULL;


Armv6mISA* Armv6mISA::getInstance(void)
{
	if(singleton == NULL)
	{
		singleton = new Armv6mISA;
	}
	return singleton;
}

Armv6mISA::Armv6mISA()
{
}

Armv6mISA::~Armv6mISA()
{
}

jump_target_address_t Armv6mISA::getJumpTargetAddr(string instruction, uint32_t curr_addr)
{
	jump_target_address_t result;
	result.valid = false;
	result.addr = UNKNOWN_ADDR;

	switch(classifyInstruction(instruction))
	{
		case B:
			assert(!is32BitInstruction(instruction));
			result.addr = curr_addr + 4 + getBDisp(instruction);
			LOG_DEBUG(logger, "Found B (IMM11): " << instruction << " target is: 0x" << hex << result.addr);
			result.valid = true;
			break;
		case BC:
			assert(!is32BitInstruction(instruction));
			result.addr = curr_addr + 4 + getBCDisp(instruction);
			LOG_DEBUG(logger, "Found BC (IMM8): " << instruction << " target is: 0x" << hex << result.addr);
			result.valid = true;
			break;
		case BL:
			assert(is32BitInstruction(instruction));
			result.addr = curr_addr + 4 + getBLDisp(instruction);
			LOG_DEBUG(logger, "Found BL (IMM10 & IMM11): " << instruction << " target is: 0x" << hex << result.addr);
			result.valid = true;
			break;
		case BX:
		case BLX:
			LOG_DEBUG(logger, "Indirect target cannot be determined!!");
			result.valid = false;
			break;
		case POP:
			if(isPCinRegisterList(instruction))
			{
				LOG_DEBUG(logger, "Found RET: " << instruction);
				result.valid = false;
				break;
			}
		default:
			LOG_DEBUG(logger, "Found ??: " << instruction);
			assert(false);
	}


	return result;
}

int32_t Armv6mISA::getImm11S(string instruction)
{
	uint16_t instr = (uint16_t)strtoul(instruction.c_str(), NULL, 16);
	return ((((int32_t)(instr&0x7ff))<<21)>>21);
}

int32_t Armv6mISA::getImm8S(string instruction)
{
	uint16_t instr = (uint16_t)strtoul(instruction.c_str(), NULL, 16);
	return ((((int32_t)(instr&0xff))<<24)>>24);
}

int32_t Armv6mISA::getBDisp(string instruction)
{
	return 2*getImm11S(instruction);
}

int32_t Armv6mISA::getBCDisp(string instruction)
{
	return 2*getImm8S(instruction);
}

int32_t Armv6mISA::getBLDisp(string instruction)
{
	uint32_t instr = (uint32_t)strtoul(instruction.c_str(), NULL, 16);

	uint32_t s = (instr>>26)&0x1;
	uint32_t j1 = (instr>>13)&0x1;
	uint32_t j2 = (instr>>11)&0x1;
	uint32_t imm10 = (instr>>16)&0x3ff;
	uint32_t imm11 = instr&0x7ff;

	// I1 = NOT(J1 EOR S); I2 = NOT(J2 EOR S); imm32 = SignExtend(S:I1:I2:imm10:imm11:'0', 32);

	int32_t disp = (s << 24) | (((~(j1 ^ s))&1) << 23) | (((~(j2 ^ s))&1) << 22) | (imm10 << 12) | (imm11<<1);
	
	LOG_DEBUG(logger, "s: " << hex << ((s << 24))  << " j1: " <<  ((((~(j1 ^ s))&1) << 23)) << " j2: " <<  ((((~(j2 ^ s))&1) << 22)) << " imm10: " <<  ((imm10 << 12))  << " imm11: " << ((imm11<<1)));
	
//	LOG_DEBUG(logger, "Displacement of before sign extension BL is: " << disp << " " << hex << disp);

	// sign extend
	disp = ((disp<<7)>>7);

//	LOG_DEBUG(logger, "Displacement of BL is: " << disp << " " << hex << disp);
	return disp;
}

displacement_type_t Armv6mISA::getDisplacementType(string instruction)
{
	displacement_type_t return_value = UnknownDisplacementType;
	switch(classifyInstruction(instruction))
	{
		case B:
			assert(!is32BitInstruction(instruction));
			return_value = disp11;
			break;
		case BC:
			assert(!is32BitInstruction(instruction));
			return_value = disp8;
			break;
		case BL:
			assert(is32BitInstruction(instruction));
			return_value = disp24;
			break;
		case BX:
		case BLX:
			return_value = indirect;
			break;
		case POP:
			if(isPCinRegisterList(instruction))
			{
				return_value = indirect;
				break;
			}
		default:
			LOG_ERROR(logger, "Unknown jump instruction: " << instruction);
			assert(false);
	}

	return return_value;
}

string Armv6mISA::setJumpTargetAddr(string UNUSED_PARAMETER(instruction), uint32_t UNUSED_PARAMETER(curr_addr), uint32_t UNUSED_PARAMETER(target_addr))
{
	string result;
	assert(false);
	return result;
}

uint32_t Armv6mISA::getInstructionLength(string instruction)
{
	if(is32BitInstruction(instruction))
	{
//		assert(instruction.length() == 8);
		return 4;
	}

//	if(instruction.length() != 4)
//	{
//		LOG_ERROR(logger, "Something gone wrong: detected 16bit instr but " << instruction.length() << " half bytes detected: " << instruction);
//	}

	// the 16bit instruction is the default case
	return 2;
}


//bool Armv6mISA::isDISPActivate(string instruction)
//{
//	return false;
//}

uint16_t Armv6mISA::getImmediate(string UNUSED_PARAMETER(instruction))
{
	assert(false);
	return 0;
}

string Armv6mISA::setImmediate(string UNUSED_PARAMETER(instruction), uint16_t UNUSED_PARAMETER(immediate))
{
	string result;
	assert(false);
	return result;
}

uint32_t Armv6mISA::convertInstructionToUint(string instruction)
{
	return (uint32_t)strtoul(instruction.c_str(), NULL, 16);
}


armv6m_instruction_t Armv6mISA::classifyInstruction(string instruction)
{
	if(getInstructionLength(instruction) == 2)
	{
		assert(!is32BitInstruction(instruction));
		return classifyInstruction((uint16_t)strtoul(instruction.c_str(), NULL, 16));
	}
	else if(getInstructionLength(instruction) == 4)
	{
		assert(is32BitInstruction(instruction));
		return classifyInstruction((uint32_t)strtoul(instruction.c_str(), NULL, 16));
	}
	else
	{
		assert(false);
	}

	LOG_ERROR(logger, "Unknown instruction: 0x" << hex << instruction);
	return UNKNOWN_ARM_OP;
}


armv6m_instruction_t Armv6mISA::classifyInstruction(uint16_t instruction)
{
	LOG_TRACE(logger, "Classifying 16bit instr: 0x" << hex << instruction);

	uint16_t op = (instruction >> 11) & 0x1f;

	switch(op)
	{
		case ARMV6M_LSL_I:
			return LSL_I;
		case ARMV6M_LSR_I:
			return LSR_I;
		case ARMV6M_ASR_I:
			return ASR_I;
		case ARMV6M_AOS:
			/* add or sub */
			switch((instruction >>9)&0x3)
			{
				case 0x0:
					return ADD_R; // encoding T1
				case 0x1:
					return SUB_R;
				case 0x2:
					return ADD_I3;
				case 0x3:
					return SUB_I3;
				default: 
					LOG_ERROR(logger, "Unknown instruction: 0x" << hex << instruction);
					return UNKNOWN_ARM_OP;
			}
		case ARMV6M_MOV_I:
			return MOV_I;
		case ARMV6M_CMP_I:
			return CMP_I;
		case ARMV6M_ADD_I8:
			return ADD_I8;
		case ARMV6M_SUB_I8:
			return SUB_I8;
		case ARMV6M_DP_SP:
			if(((instruction>>10)&0x1) == 0x0)
			{
				// data processing
				switch((instruction>>6)&0xf)
				{
					case ARMV6M_DP_AND_R:
						return AND_R;
					case ARMV6M_DP_EOR_R:
						return EOR_R;
					case ARMV6M_DP_LSL_R:
						return LSL_R;
					case ARMV6M_DP_LSR_R:
						return LSR_R;
					case ARMV6M_DP_ASR_R:
						return ASR_R;
					case ARMV6M_DP_ADC_R:
						return ADC_R;
					case ARMV6M_DP_SBC_R:
						return SBC_R;
					case ARMV6M_DP_ROR_R:
						return ROR_R;
					case ARMV6M_DP_TST_R:
						return TST_R;
					case ARMV6M_DP_RSB_I:
						return RSB_R;
					case ARMV6M_DP_CMP_R:
						return CMP_R; // encoding T1
					case ARMV6M_DP_CMN_R:
						return CMN_R;
					case ARMV6M_DP_ORR_R:
						return ORR_R;
					case ARMV6M_DP_MUL_R:
						return MUL_R;
					case ARMV6M_DP_BIC_R:
						return BIC_R;
					case ARMV6M_DP_MVN_R:
						return MVN_R;
					default: 
						LOG_ERROR(logger, "Unknown instruction: 0x" << hex << instruction);
						return UNKNOWN_ARM_OP;
				}
			}
			else
			{
				// special data instructions and branch and exchange
				switch((instruction>>8)&0x3)
				{
					case 0x0:
						return ADD_R; // encoding T2
					case 0x1:
						if(((instruction>>6)&0x3) != 0)
						{
							return CMP_R; // encoding T2
						}
						else
						{
							LOG_ERROR(logger, "Unknown instruction: 0x" << hex << instruction);
							return UNKNOWN_ARM_OP;
						}
					case 0x2:
						if(isPCinRdOfMov(instruction))
						{
							// A MOV with the PC in the destination register is an indirect jump.
							return BX;
						}
						else
						{
							return MOV_R;
						}
					case 0x3:
						if(((instruction>>7)&0x1) == 0)
						{
							return BX;
						}
						else
						{
							return BLX;
						}
					default: 
						LOG_ERROR(logger, "Unknown instruction: 0x" << hex << instruction);
						return UNKNOWN_ARM_OP;
				}
			}
		case ARMV6M_LDR_L:	
			return LDR_L;
		case ARMV6M_L_S_1:
			switch((instruction>>9)&0x3)
			{
				case 0x0:
					return STR_R;
				case 0x1:
					return STRH_R;
				case 0x2:
					return STRB_R;
				case 0x3:
					return LDRSB_R;
				default: 
					LOG_ERROR(logger, "Unknown instruction: 0x" << hex << instruction);
					return UNKNOWN_ARM_OP;
			}
		case ARMV6M_L_S_2:
			switch((instruction>>9)&0x3)
			{
				case 0x0:
					return LDR_R;
				case 0x1:
					return LDRH_R;
				case 0x2:
					return LDRB_R;
				case 0x3:
					return LDRSH_R;
				default: 
					LOG_ERROR(logger, "Unknown instruction: 0x" << hex << instruction);
					return UNKNOWN_ARM_OP;
			}

		case ARMV6M_STR_I_T1:
			return STR_I; // encoding T1
		case ARMV6M_LDR_I_T1:
			return LDR_I; // encoding T1
		case ARMV6M_STRB_I:
			return STRB_I;
		case ARMV6M_LDRB_I:
			return LDRB_I;
		case ARMV6M_STRH_I:
			return STRH_I;
		case ARMV6M_LDRH_I:
			return LDRH_I;
		case ARMV6M_STR_I_T2:
			return STR_I; // encoding T2
		case ARMV6M_LDR_I_T2:
			return LDR_I; // encoding T2
		case ARMV6M_ADR_I:
			return ADR_I;
		case ARMV6M_ADD_SP_I:
			return ADD_SP_I; // encoding T1
		case ARMV6M_MISC_1:
			if(((instruction>>10)&0x1) == 0)
			{
				switch((instruction>>6)&0xf)
				{
					case 0x0:
					case 0x1:
						return ADD_SP_I; // encoding T2
					case 0x2:
					case 0x3:
						return SUB_SP_I;
					case 0x8:
						return SXTH;
					case 0x9:
						return SXTB;
					case 0xa:
						return UXTH;
					case 0xb:
						return UXTB;
					default:
						LOG_ERROR(logger, "Unknown instruction: 0x" << hex << instruction);
						return UNKNOWN_ARM_OP;
				}
			}
			else
			{
				if(((instruction>>9)&0x1) == 0)
				{
					return PUSH;
				}
				else
				{
					if(((instruction>>5)&0xf) == 3)
					{
						return CPS;
					}
					else
					{
						LOG_ERROR(logger, "Unknown instruction: 0x" << hex << instruction);
						return UNKNOWN_ARM_OP;
					}
				}
			}
		case ARMV6M_MISC_2:
			if(((instruction>>10)&0x1) == 0)
			{
				switch((instruction>>6)&0xf)
				{
					case 0x8:
						return REV;
					case 0x09:
						return REV16;
					case 0xb:
						return REVSH;
					default:
						LOG_ERROR(logger, "Unknown instruction: 0x" << hex << instruction);
						return UNKNOWN_ARM_OP;
				}
			}
			else
			{
				if(((instruction>>9)&0x1) == 0)
				{
					return POP;
				}
				else
				{

					if(((instruction>>9)&0x1) == 0)
					{
						return BKPT;
					}
					else
					{
						switch((instruction>>4)&0xf)
						{
							case 0x0:
								return NOP;
							case 0x1:
								return YIELD;
							case 0x2:
								return WFE;
							case 0x3:
								return WFI;
							case 0x4:
								return SEV;
							default:
								LOG_ERROR(logger, "Unknown instruction: 0x" << hex << instruction);
								return UNKNOWN_ARM_OP;
						}
					}
				}
			}
		case ARMV6M_STM:
			return STM;
		case ARMV6M_LDM:
			return LDM;
		case ARMV6M_BC_1:
		case ARMV6M_BC_2:
			if(((instruction>>9)&0x7) != 0x7)
			{
				return BC;
			}
			else
			{
				if(((instruction>>8)&0x1) == 0x0)
				{
					return UDF;
				}
				else
				{
					return SVC;
				}

			}
		case ARMV6M_B:
			return B;

		case ARMV6M_32BIT_INSTR:
			LOG_ERROR(logger, "Decode error: 0x" << hex << instruction << " is 32Bit instruction, but expecting 16Bit");
			// The instruction is 32bit, cannot be decoded as 16bit instruction.
			assert(false);
			return UNKNOWN_ARM_OP;
		default: 
			LOG_ERROR(logger, "Unknown instruction: 0x" << hex << instruction);
			return UNKNOWN_ARM_OP;
	}
	LOG_ERROR(logger, "Unknown instruction: 0x" << hex << instruction);
	return UNKNOWN_ARM_OP;
}


bool Armv6mISA::is32BitInstruction(string instruction)
{
	return is32BitInstruction((uint32_t)strtoul(instruction.c_str(), NULL, 16));
}

bool Armv6mISA::is32BitInstruction(uint16_t instruction)
{
	switch ((uint8_t)(instruction>>11))
	{
		case 0x1d: // 0b11101
		case 0x1e: // 0b11110
		case 0x1f: // 0b11111
			return true;
		default:
			return false;
	}
}

bool Armv6mISA::is32BitInstruction(uint32_t instruction)
{
	if(instruction > 0xffff)
	{
		instruction >>= 16;
	}
	return is32BitInstruction((uint16_t)(instruction));
}

armv6m_instruction_t Armv6mISA::classifyInstruction(uint32_t instruction)
{
	LOG_TRACE(logger, "Classifying 32bit instr: 0x" << hex << instruction);
	if((((instruction>>27&0x3) == 2) && ((instruction>>15&0x1) == 1)))
	{
		if(((instruction>>12)&0x5) == 0x0)
		{
			if(((instruction>>20)&0x3e) == 0x38)
			{
				return MSR_R;
			}
			else if(((instruction>>20)&0x3f) == 0x3b)
			{
				// misc
				switch((instruction>>4)&0xf)
				{
					case 0x4:
						return DSB;
					case 0x5:
						return DMB;
					case 6:
						return ISB;
					default:
						LOG_ERROR(logger, "Unknown instruction: 0x" << hex << instruction);
						return UNKNOWN_ARM_OP;
				}
			}
			else if(((instruction>>20)&0x3e) == 0x3e)
			{
				return MRS_R;
			}
			else if((((instruction>>12)&0x7) == 0x2) && ((instruction>>20)&0x7f) == 0x7f)
			{
				return UDF;
			}
			else
			{
				LOG_ERROR(logger, "Unknown instruction: 0x" << hex << instruction);
				return UNKNOWN_ARM_OP;
			}
		}
		else if(((instruction>>12)&0x5) == 0x5)
		{
			return BL;
		}
	}
	else
	{
		// undefined opcode
		LOG_ERROR(logger, "Unknown instruction: 0x" << hex << instruction);
		return UNKNOWN_ARM_OP;
	}

	assert(false);
	return UNKNOWN_ARM_OP;
}

vector<armv6m_registers_t> Armv6mISA::getRegistersFromRegisterList(string instruction)
{
	if(instruction.length() == 4)
	{
		assert(!is32BitInstruction(instruction));
		return getRegistersFromRegisterList((uint16_t)strtoul(instruction.c_str(), NULL, 16));
	}
	else
	{
		// wrong instruction length
		// there are only 16bit instructions that support register lists
		assert(false);
	}
	return vector<armv6m_registers_t>();
}

vector<armv6m_registers_t> Armv6mISA::getRegistersFromRegisterList(uint16_t instruction)
{
	vector<armv6m_registers_t> result;

	// only these instruction support register lists
	assert((classifyInstruction(instruction) == POP) || (classifyInstruction(instruction) == PUSH) || (classifyInstruction(instruction) == STM) || (classifyInstruction(instruction) == LDM));

	for(uint32_t i = 0; i < 8; i++)
	{
		if(((instruction>>i)&0x1) == 1)
		{
			result.push_back((armv6m_registers_t)i);
		}
	}

	// for PUSH and POP additionally the LR/PC register can be written/read
	if(((instruction>>8)&0x1) == 1)
	{
		if(classifyInstruction(instruction) == PUSH)
		{
			result.push_back(LR);
		}
		else if(classifyInstruction(instruction) == PUSH)
		{
			result.push_back(PC);
		}
		else
		{
			assert(false);
		}
	}

	return result;
}

uint32_t Armv6mISA::getNumberOfRegistersFromList(string instruction)
{
	if(instruction.length() == 4)
	{
		assert(!is32BitInstruction(instruction));
		return getNumberOfRegistersFromList((uint16_t)strtoul(instruction.c_str(), NULL, 16));
	}
	else
	{
		// wrong instruction length
		// there are only 16bit instructions that support register lists
		assert(false);
	}
	return -1;
}

uint32_t Armv6mISA::getNumberOfRegistersFromList(uint16_t instruction)
{
	uint32_t count = 0;

	// only these instruction support register lists
	assert((classifyInstruction(instruction) == POP) || (classifyInstruction(instruction) == PUSH) || (classifyInstruction(instruction) == STM) || (classifyInstruction(instruction) == LDM));

	for(uint32_t i = 0; i < 8; i++)
	{
		if(((instruction>>i)&0x1) == 1)
		{
			count++;;
		}
	}

	if((classifyInstruction(instruction) == POP) || (classifyInstruction(instruction) == PUSH))
	{
		// for PUSH and POP additionally the LR/PC register can be written/read
		if(((instruction>>8)&0x1) == 1)
		{
			count++;;
		}
	}

	return count;
}

bool Armv6mISA::isPCinRegisterList(string instruction)
{
	if(instruction.length() == 4)
	{
		assert(!is32BitInstruction(instruction));
		return isPCinRegisterList((uint16_t)strtoul(instruction.c_str(), NULL, 16));
	}
	else
	{
		// wrong instruction length
		// there are only 16bit instructions that support register lists
		assert(false);
	}
	return false;
}

bool Armv6mISA::isPCinRegisterList(uint16_t instruction)
{
	// only these instruction support register lists
	assert(classifyInstruction(instruction) == POP);

	if(((instruction>>8)&0x1) == 1)
	{
		return true;
	}

	return false;
}

bool Armv6mISA::isLRinRegisterList(string instruction)
{
	if(instruction.length() == 4)
	{
		assert(!is32BitInstruction(instruction));
		return isLRinRegisterList((uint16_t)strtoul(instruction.c_str(), NULL, 16));
	}
	else
	{
		// wrong instruction length
		// there are only 16bit instructions that support register lists
		assert(false);
	}
	return false;
}

bool Armv6mISA::isLRinRegisterList(uint16_t instruction)
{
	// only these instruction support register lists
	assert(classifyInstruction(instruction) == PUSH);

	if(((instruction>>8)&0x1) == 1)
	{
		return true;
	}

	return false;
}

bool Armv6mISA::hasRegisterList(string instruction)
{
	if(instruction.length() == 4)
	{
		assert(!is32BitInstruction(instruction));
		return hasRegisterList((uint16_t)strtoul(instruction.c_str(), NULL, 16));
	}
	else
	{
		// there are only 16bit instructions that support register lists
		return false;
	}

	return false;
}

bool Armv6mISA::hasRegisterList(uint16_t instruction)
{
	switch(classifyInstruction(instruction))
	{
		case POP:
		case PUSH:
		case STM:
		case LDM:
			return true;
		default: 
			return false;
	}
	return false;
}

armv6m_registers_t Armv6mISA::getRm(string instruction)
{
	if(instruction.length() == 4)
	{
		assert(!is32BitInstruction(instruction));
		return getRm((uint16_t)strtoul(instruction.c_str(), NULL, 16));
	}
	else
	{
		assert(false);
		return UNKNOWN_REG;
	}

	assert(false);
	return UNKNOWN_REG;
}

armv6m_registers_t Armv6mISA::getRm(uint16_t instruction)
{
	switch(classifyInstruction(instruction))
	{
		case BX:
			return ((armv6m_registers_t)((instruction>>3)&0xf));
		default:
			// other instructions are not supported
			assert(false);
			return UNKNOWN_REG;
	}

	assert(false);
	return UNKNOWN_REG;
}

string Armv6mISA::getConnectingJumpInstruction()
{
	uint32_t max_disp11 = 0x7FF;
	uint32_t b_opcode = 0xE000;

	stringstream s;
	s << hex << (b_opcode | max_disp11);

	LOG_DEBUG(logger, "Opcode is: " << s.str());

	return string(s.str());
}

string Armv6mISA::getConnectingJumpComment()
{
	return string("b.n 7ff <XX> BBSISP_JP: inserted jump to preserve control flow");
}

bool Armv6mISA::isPCinRdOfMov(string instruction)
{
	if(instruction.length() == 4)
	{
		assert(!is32BitInstruction(instruction));
		return isPCinRdOfMov((uint16_t)strtoul(instruction.c_str(), NULL, 16));
	}
	else
	{
		// wrong instruction length
		// there are only 16bit mov instructions
		assert(false);
	}
	return false;
}

bool Armv6mISA::isPCinRdOfMov(uint16_t instruction)
{
	assert((instruction&0xff00) == 0x4600); // Only the MOV (register) Encoding T1 is supported

	// check if Rd == PC (register identifier for Rd consistis of 4 bit: b7&b2-1)
	if(((instruction)&0x87) == 0x87)
	{
		return true;
	}

	return false;
}
