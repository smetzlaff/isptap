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
#ifndef _TRICORE_ISA_HPP_
#define _TRICORE_ISA_HPP_

#include "isa.hpp"

#include <map>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>


/**
 * \brief Enum for all supported control flow changing instructions of the Tricore ISA
 * This enum is used for distinguish the different instructions and calculating of the target address.
 */
enum tricore_instruction_t { CALL_32=0, CALL_16, CALL_32_I, J_32, J_16, JEQ_32_BRC, JEQ_32_BRR, JEQ_16_SBC, JEQ_16_SBR, JEQA_32_BRR, JGE_32_BRC, JGE_32_BRR, JGEZ_16_SBR, JGTZ_16_SBR, JI_32, JI_16, JLEZ_16_SBR, JLT_32_BRC, JLT_32_BRR, JLTZ_16_SBR, JNE_32_BRC, JNE_32_BRR, JNE_16_SBC, JNE_16_SBR, JNED_32_BRC, JNED_32_BRR, JNZ_16_SB, JNZ_16_SBR, JNZA_32_BRR, JNZA_16_SBR, JNZT_32_BRN, JNZT_16_SBRN, JZ_16_SB, JZ_16_SBR, JZA_32_BRR, JZA_16_SBR, JZT_32_BRN, JZT_16_SBRN, LOOP_32, LOOP_16, RET, TASKING_JZT_32, ADDI, MOVH, MTCR, MOV_RLC, MOV_RR, MOV_SC, MOV_SRC };

///**
// * \brief The different possible types of displacements fields of jumps and calls from the TriCore instruction set.
// */
//enum displacement_type_t { indirect = 0, disp4, disp8, disp15, disp24, NoDisplacement, UnknownDisplacementType};

/**
 * \brief Structure to maintain the target CSFR and the source register of a MTCR instruction.
 */
struct mtcr_decode_t {
	uint16_t csfr;
	uint8_t src_reg;
};

/** 
 * \brief Structure to maintain the target register and the immediate value of an mov instruction with immediate value.
 */
struct movi_decode_t {
	uint16_t imm;
	uint8_t tgt_reg;
};


/**
 * \brief Decoding class for control flow instructions for the Tricore instruction set architecture.
 * This class implements the decoding of jump and call instructions to obtain the target addresses. 
 * The instructions to decode have to be in text (.dump) form.
 * The class implements the singleton pattern.
 */
class TricoreISA : public ISA {
	public:

		/**
		 * \brief Provides pointer to static singleton object.
		 * \returns Pointer to static object.
		 */
		static TricoreISA* getInstance(void);

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
		 * Note: Works only if it is possible just to change the address. Doesn't work if the original jump cannot reach the target (e.g. if a 4 bit DISP was used but the new address is unreachable with a 4 bit DISP)
		 */
		string setJumpTargetAddr(string instruction, uint32_t curr_addr, uint32_t target_addr);
		/** 
		 * \brief Delivers the length of the given instruction in bytes.
		 * \param instruction The instruction as hexadecimal string parsed from dump-file.
		 * \returns The size of the instruction, either 2 bytes or 4 bytes. On error the returned length is 0.
		 */
		uint32_t getInstructionLength(string instruction);


		/**
		 * \brief Checks if an instruction is the DISP activate instruction
		 * \param instruction The instruction to check.
		 * \returns True if the tested instruction is the DISP activate instruction, else false.
		 */
		bool isDISPActivate(string instruction);

		/** 
		 * \brief Decodes the cons16 immediate value from an given instruction.
		 * \param instruction The instruction to extract the cons16 from.
		 * \returns The cons16 immediate value of the given instruction.
		 */
		uint16_t getImmediate(string instruction);

		/**
		 * \brief Sets/Replaces the cons16 immediate value of an instruction.
		 * \param instruction The instruction for which the cons16 immediate value should be set.
		 * \param immediate The value of the cons16 immediate to set.
		 * \returns The new instruction with the updated cons16 immediate value.
		 */
		string setImmediate(string instruction, uint16_t immediate);

		/**
		 * \brief Convert an instruction, given as string into a representation as an unsigned integer.
		 * \param instruction The instruction as string to convert.
		 * \returns The given instruction as unsigned integer value.
		 */
		uint32_t convertInstructionToUint(string instruction);

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

		bool isMTCR(string instruction);
		mtcr_decode_t decodeMTCR(string instruction);
		bool isMOVI(string instruction);
		movi_decode_t decodeMOVI(string instruction);

	private:
		/**
		 * Default constructor.
		 * The constructor is private, because this class implements the singleton pattern.
		 */
		TricoreISA();
		/**
		 * Default destructor
		 * The destructor is private, because this class implements the singleton pattern.
		 */
		virtual ~TricoreISA();
		/**
		 * \brief Copy Constructor to prevent copy-construction.
		 * Is private to implement the singleton pattern.
		 */
		TricoreISA(const TricoreISA&);
		/**
		 * \brief Assignment operatior to prevent assignment.
		 * Is private to implement the singleton pattern.
		 */
		TricoreISA& operator=(const TricoreISA&);
		/**
		 * \brief Static reference to object. To implement the singleton pattern.
		 */
		static TricoreISA* singleton;
		/**
		 * \brief Adjustes the byte order of the instruction parameter.
		 * Needed because the byte order of the dump file is flipped in the .dump file.
		 * \param instruction The instruction as hexadecimal string parsed from dump-file.
		 * \returns The corrected byte ordering.
		 */
		string adjustByteOrder(string instruction);
		/**
		 * \brief A map that holds for each instruction identifier (op1 field in Tricore ISA manual) the coresponding enum for decoding.
		 */
		map<string, tricore_instruction_t> instructions;
		/**
		 * \brief Pointer to the LOGCXX logger object.
		 */
		static LoggerPtr logger;
};


#endif
