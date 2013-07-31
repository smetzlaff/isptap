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
#ifndef _ISA_HPP_
#define _ISA_HPP_

#include <string>
#include "global.h"

/**
 * \brief The different possible types of displacements fields of jumps and calls from the TriCore instruction set.
 */
enum displacement_type_t { indirect = 0, disp4, disp8, disp15, disp11, disp24, NoDisplacement, UnknownDisplacementType};

struct jump_target_address_t {
	bool valid;
	uint32_t addr;
};

/**
 * \brief Interface for methods dealing wit the processors ISA.
 */
class ISA {
	public:
		/**
		 * \brief Calculates the target address for the given instruction and the address of the instruction
		 * \param instruction The instruction as hexadecimal string parsed from dump-file.
		 * \param curr_addr The address of the instruction. Needed for calculation of relative branches.
		 * \returns The target address object.
		 */
		virtual jump_target_address_t getJumpTargetAddr(string instruction, uint32_t curr_addr) = 0;

		/**
		 * \brief Returns the displacement type of a given control flow changing instruction
		 * \param instruction The instruction as hexadecimal string parsed from dump-file.
		 * \returns The type of the displacement, UnknownDisplacementType, if the control flow chaning instruction could not be not classified or on any other error.
		 */
		virtual displacement_type_t getDisplacementType(string instruction) = 0;

		/**
		 * \brief Updates a jump address of an instruction.
		 * \param instruction Original instruction (with old jump target).
		 * \param curr_addr Address of the instruction
		 * \param target_addr The new address for the jump.
		 * \returns The altered instruction with the new jump target.
		 */
		virtual string setJumpTargetAddr(string instruction, uint32_t curr_addr, uint32_t target_addr) = 0;

		/** 
		 * \brief Delivers the length of the given instruction in bytes.
		 * \param instruction The instruction as hexadecimal string parsed from dump-file.
		 * \returns The size of the instruction. On error the returned length is 0.
		 */
		virtual uint32_t getInstructionLength(string instruction) = 0;


//		/**
//		 * \brief Checks if an instruction is the DISP activate instruction
//		 * \param instruction The instruction to check.
//		 * \returns True if the tested instruction is the DISP activate instruction, else false.
//		 */
//		virtual bool isDISPActivate(string instruction) = 0;

		/** 
		 * \brief Decodes the cons16 immediate value from an given instruction.
		 * \param instruction The instruction to extract the immediate value from.
		 * \returns The immediate value of the given instruction.
		 */
		virtual uint16_t getImmediate(string instruction) = 0;

		/**
		 * \brief Sets/Replaces the immediate value of an instruction.
		 * \param instruction The instruction for which the immediate value should be set.
		 * \param immediate The value of the immediate to set.
		 * \returns The new instruction with the updated immediate value.
		 */
		virtual string setImmediate(string instruction, uint16_t immediate) = 0;

		/**
		 * \brief Convert an instruction, given as string into a representation as an unsigned integer.
		 * \param instruction The instruction as string to convert.
		 * \returns The given instruction as unsigned integer value.
		 */
		virtual uint32_t convertInstructionToUint(string instruction) = 0;

		/**
		 * \brief Generates a generic connecting jump instruction.
		 * A connecting jump is necessary to reconnect basic blocks after they were assigned 
		 * to different memories by the BBSISP assignment algorithm.
		 * \returns The generic connecting jump instruction as string.
		 */
		virtual string getConnectingJumpInstruction(void) = 0;

		/**
		 * \brief Returns a comment for a generated connecting jump instruction.
		 * A connecting jump is necessary to reconnect basic blocks after they were assigned 
		 * to different memories by the BBSISP assignment algorithm.
		 * \returns The comment of a connecting jump instruction as string.
		 */
		virtual string getConnectingJumpComment(void) = 0;

	protected:
		virtual ~ISA() {};
};


#endif
