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
#ifndef _ARMV6M_ISA_HPP_
#define _ARMV6M_ISA_HPP_

#include "isa.hpp"

#include <map>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>


enum armv6m_instruction_t {LSL_I=0, LSR_I, ASR_I, ADD_R, SUB_R, ADD_I3, SUB_I3, MOV_I, CMP_I, ADD_I8, SUB_I8, AND_R, EOR_R, LSL_R, LSR_R, ASR_R, ADC_R, SBC_R, ROR_R, TST_R, RSB_R, CMP_R, CMN_R, ORR_R, MUL_R, BIC_R, MVN_R, MOV_R, BX, BLX, LDR_L, STR_R, STRH_R, STRB_R, LDRSB_R, LDR_R, LDRH_R, LDRB_R, LDRSH_R, STR_I, LDR_I, STRB_I, LDRB_I, STRH_I, LDRH_I, ADR_I, ADD_SP_I, SUB_SP_I, SXTH, SXTB, UXTH, UXTB, PUSH, CPS, REV, REV16, REVSH, POP, BKPT, NOP, YIELD, WFE, WFI, SEV, STM, LDM, BC, UDF, SVC, B, MSR_R, MRS_R, BL, DSB, DMB, ISB, UNKNOWN_ARM_OP};

enum armv6m_registers_t {R0=0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, SP=13, LR=14, PC=15, UNKNOWN_REG=99};

/**
 * \brief Decoding class for control flow instructions for the ARMv6-M instruction set architecture.
 * This class implements the decoding of jump and call instructions to obtain the target addresses. 
 * The instructions to decode have to be in text (.dump) form.
 * The class implements the singleton pattern.
 */
class Armv6mISA : public ISA {
	public:
		/**
		 * \brief Provides pointer to static singleton object.
		 * \returns Pointer to static object.
		 */
		static Armv6mISA* getInstance(void);

		armv6m_instruction_t classifyInstruction(string instruction);

		/**
		 * \brief Calculates the target address for the given instruction and the address of the instruction
		 * \param instruction The instruction as hexadecimal string parsed from dump-file.
		 * \param curr_addr The address of the instruction. Needed for calculation of relative branches.
		 * \returns The target address object.
		 */
		jump_target_address_t getJumpTargetAddr(string instruction, uint32_t curr_addr);

		/**
		 * \brief Returns the displacement type of a given control flow changing instruction
		 * \param instruction The instruction as hexadecimal string parsed from dump-file.
		 * \returns The type of the displacement, UnknownDisplacementType, if the control flow chaning instruction could not be not classified or on any other error.
		 */
		displacement_type_t getDisplacementType(string instruction);

		/**
		 * \brief Updates a jump address of an instruction.
		 * \param instruction Original instruction (with old jump target).
		 * \param curr_addr Address of the instruction
		 * \param target_addr The new address for the jump.
		 * \returns The altered instruction with the new jump target.
		 */
		string setJumpTargetAddr(string instruction, uint32_t curr_addr, uint32_t target_addr);

		/** 
		 * \brief Delivers the length of the given instruction in bytes.
		 * \param instruction The instruction as hexadecimal string parsed from dump-file.
		 * \returns The size of the instruction. On error the returned length is 0.
		 */
		uint32_t getInstructionLength(string instruction);


//		/**
//		 * \brief Checks if an instruction is the DISP activate instruction
//		 * \param instruction The instruction to check.
//		 * \returns True if the tested instruction is the DISP activate instruction, else false.
//		 */
//		bool isDISPActivate(string instruction);

		/** 
		 * \brief Decodes the cons16 immediate value from an given instruction.
		 * \param instruction The instruction to extract the cons16 from.
		 * \returns The immediate value of the given instruction.
		 */
		uint16_t getImmediate(string instruction);

		/**
		 * \brief Sets/Replaces the immediate value of an instruction.
		 * \param instruction The instruction for which the immediate value should be set.
		 * \param immediate The value of the immediate to set.
		 * \returns The new instruction with the updated immediate value.
		 */
		string setImmediate(string instruction, uint16_t immediate);

		/**
		 * \brief Convert an instruction, given as string into a representation as an unsigned integer.
		 * \param instruction The instruction as string to convert.
		 * \returns The given instruction as unsigned integer value.
		 */
		uint32_t convertInstructionToUint(string instruction);


		/**
		 * \brief Determines if the given instruction is a 32bit instruction.
		 * \param instruction The instruction as string to check.
		 * \returns True if the instruction describes a 32bit instruction.
		 */
		bool is32BitInstruction(string instruction);

		/**
		 * \brief Provides the selected registers from the instruction's register list.
		 * Only the following instructions have a register list: POP, PUSH, STM, LDM
		 * \param instruction The instruction to determine the selected registers.
		 * \returns A vector of the registers that are selected by the register list.
		 */
		vector<armv6m_registers_t> getRegistersFromRegisterList(string instruction);
		/**
		 * \brief Provides the number of selected registers from the instruction's register list.
		 * Only the following instructions have a register list: POP, PUSH, STM, LDM
		 * \param instruction The instruction to determine the selected registers.
		 * \returns The number of registers that are selected by the register list.
		 */
		uint32_t getNumberOfRegistersFromList(string instruction);
		/**
		 * \brief Determines if the PC register is in the register list of a POP instruction.
		 * \param instruction The instruction to check if the PC register is in the register list.
		 * \returns True if the PC is in the register list, else false.
		 */
		bool isPCinRegisterList(string instruction);
		/**
		 * \brief Determines if the LR register is in the register list of a PUSH instruction.
		 * \param instruction The instruction to check if the PC register is in the register list.
		 * \returns True if the PC is in the register list, else false.
		 */
		bool isLRinRegisterList(string instruction);
		/**
		 * \brief Determines if an instruction has the register list field.
		 * Only the following instructions have a register list: POP, PUSH, STM, LDM
		 * \param instruction The instruction to check.
		 * \returns True if the instruction has the register list field, else false.
		 */
		bool hasRegisterList(string instruction);
		/**
		 * \brief Returns the register selected by the Rm field of the instruction.
		 * \param instruction The instruction to get the register in the Rm field from.
		 * \returns The register selected by the Rm field.
		 */
		armv6m_registers_t getRm(string instruction);

		/**
		 * \brief Generates a generic connecting jump instruction.
		 * A connecting jump is necessary to reconnect basic blocks after they were assigned 
		 * to different memories by the BBSISP assignment algorithm.
		 * \returns The generic connecting jump instruction as string.
		 */
		string getConnectingJumpInstruction(void);

		/**
		 * \brief Returns a comment for a generated connecting jump instruction.
		 * A connecting jump is necessary to reconnect basic blocks after they were assigned 
		 * to different memories by the BBSISP assignment algorithm.
		 * \returns The comment of a connecting jump instruction as string.
		 */
		string getConnectingJumpComment(void);

	private:
		/**
		 * Default constructor.
		 * The constructor is private, because this class implements the singleton pattern.
		 */
		Armv6mISA();
		/**
		 * Default destructor
		 * The destructor is private, because this class implements the singleton pattern.
		 */
		virtual ~Armv6mISA();
		/**
		 * \brief Copy Constructor to prevent copy-construction.
		 * Is private to implement the singleton pattern.
		 */
		Armv6mISA(const Armv6mISA&);
		/**
		 * \brief Assignment operatior to prevent assignment.
		 * Is private to implement the singleton pattern.
		 */
		Armv6mISA& operator=(const Armv6mISA&);
		/**
		 * \brief Static reference to object. To implement the singleton pattern.
		 */
		static Armv6mISA* singleton;

		/**
		 * \brief Determines if the given instruction is a 32bit instruction.
		 * \param instruction The instruction as 32 bit integer to check.
		 * Notice: the parameter is either a full 32 bit instruction or in 
		 * the lower 16 bits of the integer contain a 16 bit instruction 
		 * (and the upper 16 bit are zero).
		 * \returns True if the instruction describes a 32bit instruction.
		 */
		inline bool is32BitInstruction(uint32_t instruction);

		/**
		 * \brief Determines if the given instruction is a 32bitinstruction.
		 * \param instruction The instruction as 16 bit integer to check.
		 * Notice: The parameter has to be either the full 16 bit instruction 
		 * or the bits [31:16] of the 32 bit instruction.
		 * \returns True if the instruction describes a 32bit instruction.
		 */
		inline bool is32BitInstruction(uint16_t instruction);

		/**
		 * \brief Decodes the 16bit Thumb instructions.
		 */
		armv6m_instruction_t classifyInstruction(uint16_t instruction);

		/**
		 * \brief Decodes trhe 32bit Thumb instructions
		 */
		armv6m_instruction_t classifyInstruction(uint32_t instruction);

		/**
		 * \brief Provides the selected registers from the instruction's register list.
		 * Only the following instructions have a register list: POP, PUSH, STM, LDM
		 * \param instruction The instruction to determine the selected registers.
		 * \returns A vector of the registers that are selected by the register list.
		 */
		vector<armv6m_registers_t> getRegistersFromRegisterList(uint16_t instruction);
		/**
		 * \brief Provides the number of selected registers from the instruction's register list.
		 * Only the following instructions have a register list: POP, PUSH, STM, LDM
		 * \param instruction The instruction to determine the selected registers.
		 * \returns The number of registers that are selected by the register list.
		 */
		uint32_t getNumberOfRegistersFromList(uint16_t instruction);
		/**
		 * \brief Determines if the PC register is in the register list of a POP instruction.
		 * \param instruction The instruction to check if the PC register is in the register list.
		 * \returns True if the PC is in the register list, else false.
		 */
		bool isPCinRegisterList(uint16_t instruction);
		/**
		 * \brief Determines if the LR register is in the register list of a PUSH instruction.
		 * \param instruction The instruction to check if the PC register is in the register list.
		 * \returns True if the PC is in the register list, else false.
		 */
		bool isLRinRegisterList(uint16_t instruction);
		/**
		 * \brief Determines if an instruction has the register list field.
		 * Only the following instructions have a register list: POP, PUSH, STM, LDM
		 * \param instruction The instruction to check.
		 * \returns True if the instruction has the register list field, else false.
		 */
		bool hasRegisterList(uint16_t instruction);


		inline int32_t getImm11S(string instruction);
		inline int32_t getImm8S(string instruction);
		inline int32_t getBDisp(string instruction);
		inline int32_t getBCDisp(string instruction);
		inline int32_t getBLDisp(string instruction);


		/**
		 * \brief Returns the register selected by the Rm field of the instruction.
		 * \param instruction The instruction to get the register in the Rm field from.
		 * \returns The register selected by the Rm field.
		 */
		armv6m_registers_t getRm(uint16_t instruction);

		/**
		 * \brief Determines if the PC register is the destination register of a MOV_R instruction.
		 * If so, the MOV_R represents an indirect jump.
		 * \param instruction The instruction to check if the PC register is the destination register.
		 * \returns True if the PC is the destination register, else false.
		 */
		bool isPCinRdOfMov(string instruction);
		/**
		 * \brief Determines if the PC register is the destination register of a MOV_R instruction.
		 * If so, the MOV_R represents an indirect jump.
		 * \param instruction The instruction to check if the PC register is the destination register.
		 * \returns True if the PC is the destination register, else false.
		 */
		bool isPCinRdOfMov(uint16_t instruction);

		/**
		 * \brief Pointer to the LOGCXX logger object.
		 */
		static LoggerPtr logger;
		
};


#endif
