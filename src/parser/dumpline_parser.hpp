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
#ifndef _DUMPLINE_PARSER_HPP_
#define _DUMPLINE_PARSER_HPP_

#include "global.h"
#include "constants.h"
#include "configuration.hpp"
#include "graph_structure.h"

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

using namespace boost;

enum instr_type_t {
	I_ARITHMETIC=0, 
	I_UNCOND_BRANCH, 
	I_UNCOND_INDIRECT_BRANCH, 
	I_COND_BRANCH, 
	I_COND_INDIRECT_BRANCH, 
	I_CALL, 
	I_INDIRECT_CALL, 
	I_RETURN, 
	I_LOAD, 
	I_STORE, 
	I_SYNC, 
	I_DEBUG, 
	I_OTHERS,
	I_UNKNOWN
};

class DumpLineParser {
	public:
		/**
		 * \brief Checks if an opcode string represents a branch instruction.
		 * \param str Line of a .dump-file. The line has to be a code line, see isCodeLine()
		 * \returns True, if the line represents a branch instruction, else false.
		 */
		virtual bool isBranchInstr(string str) = 0;
		/**
		 * \brief Checks if an opcode string represents a conditional branch instruction.
		 * \param str Line of a .dump-file. The line has to be a code line, see isCodeLine()
		 * \returns True, if the line represents a conditional branch instruction, else false.
		 */
		virtual bool isCondBranchInstr(string str) = 0;
		/**
		 * \brief Checks if an opcode string represents an indirect branch instruction.
		 * \param str Line of a .dump-file. The line has to be a code line, see isCodeLine()
		 * \returns True, if the line represents an indirect branch instruction, else false.
		 */
		virtual bool isIndirectBranchInstr(string str) = 0;
		/**
		 * \brief Checks if an opcode string represents an indirect call instruction.
		 * \param str Line of a .dump-file. The line has to be a code line, see isCodeLine()
		 * \returns True, if the line represents an indirect call instruction, else false.
		 */
		virtual bool isIndirectCallInstr(string str) = 0;
		/**
		 * \brief Checks if an opcode string represents a return instruction.
		 * \param str Line of a .dump-file. The line has to be a code line, see isCodeLine()
		 * \returns True, if the line represents a return instruction, else false.
		 */
		virtual bool isReturnInstr(string str) = 0;
		/**
		 * \brief Checks if an opcode string represents a debug instruction.
		 * \param str Line of a .dump-file. The line has to be a code line, see isCodeLine()
		 * \returns True, if the line represents a debug instruction, else false.
		 */
		virtual bool isDebugInstr(string str) = 0;
		/**
		 * \brief Checks if an opcode string represents a call instruction.
		 * \param str Line of a .dump-file. The line has to be a code line, see isCodeLine()
		 * \returns True, if the line represents a call instruction, else false.
		 */
		virtual bool isCallInstr(string str) = 0;
		/**
		 * \brief Checks if a line of a .dump-file is a code line.
		 * A code line contains an instruction in the .dump-file format.
		 * \param str Line of a .dump-file.
		 * \returns True, if the line is a code line, else false.
		 */
		bool isCodeLine(string str);
		/**
		 * \brief Checks if a line of a .dump-file is a label line.
		 * A label line contains a jump or function label in the .dump-file format.
		 * \param str Line of a .dump-file.
		 * \returns True, if the line is a label line, else false.
		 */
		bool isLabelLine(string str);
		/**
		 * \brief Checks if label is an ignore label line.
		 * \param str Label, name of the label.
		 * \returns True, if the label is an ignore label line, else false.
		 */
		bool isIgnoreLabel(string str);
		/**
		 *	\brief Checks if a line in the dump file is a memory hole.
		 *	Memory holes are inserted by the linker. In the dump file these holes are marked with "..."
		 * \param str Line of a .dump-file.
		 * \returns True, if the line represents a memory hole, else false.
		 */
		bool isMemoryHole(string str);
		/**
		 * \brief Returns the address of the instruction in a code line.
		 * If a code line is identified by using isCodeLine(), this method returns the address of the instruction that is in this line.
		 * \param str Code line of a dump file.
		 * \returns The address of the instruction in the code line.
		 */
		uint32_t getAddrFromCodeLine(string str);
		/**
		 * \brief Returns address and name of the label in a label line.
		 * If a label line is identified by using isLabelLine(), this method returns the address and the name of the label of this line.
		 * \param str Label line of a dump file.
		 * \returns Structure containing the address and the name of the label.
		 */
		addr_label_t getAddrAndLabelFromLabelLine(string str);
		/**
		 * \brief Returns the opcode of the instruction in the code line.
		 * If a code line is identified by using isCodeLine(), this method returns the opcode of the instruction that is in this line.
		 * To process the opcode in the TricoreISA class, the opcode's byte oder has to be adjusted via TricoreISA::adjustByteOrder(). 
		 * \param str Code line of a dump file.
		 * \returns Hex representation of the instruction as string (byte order is not adjusted).
		 */
		string getInstructionFromCodeLine(string str);
		/**
		 * \brief Returns the human readable comment of the instruction in the code line.
		 * If a code line is identified by using isCodeLine(), this method returns the comment of the instruction that is in this line.
		 * The comment of a code line is the deassembled instructions, giving hints for human reader which instruction is executed and which registers and immediates are used.
		 * \param str Code line of a dump file.
		 * \returns The human readable comment of an instruction.
		 */
		virtual string getCommentFromCodeLine(string str) = 0;
		/**
		 * \brief Assembles a code line from opcode address, opcode and human readable comment.
		 * The assembled code line can be used to alter a basic block, like inserting additional or updating instructions.
		 * \param opcode_address The address of the instruction represented in this code line.
		 * \param opcode_raw The opcode as byte string (wrong byte order)
		 * \param opcode_dump The human readable opcode comment.
		 * \returns A code line assembled by use the parameters. 
		 */
		virtual string assembleCodeLine(uint32_t opcode_address, string opcode_raw, string opcode_dump) = 0;
		/**
		 * \brief Assembles a label line from address and label name.
		 * The assembled label line can be used to create or alter jump targets.
		 * \param label Structure containing the necessary informations needed to create a label line (address and name of the label).
		 * \returns A label line assembled by use of the parameter.
		 */
		string assembleLabelLine(addr_label_t label);
		/**
		 * \brief Updates the address in the human readable comment of call or branch instructions.
		 * This method replaces the first string that matches an address ((HEX_TOKEN"{8}") in the instruction comment with the address given by parameter. The method should be used for branch and call instructions only, since other instructions may have no address in their comment. A lea (load effective address) instruction (or similar instructions) may also be used but then the updated address represents data not an instruction in the code, as for calls and branches, so the caller of the method has to be sure that the address updated is correct.
		 * \param opcode_dump The human readable opcode comment part of a code line (obtained by getCommentFromCodeLine()) of a branch or call instruction.
		 * \param address The new address that replaces the old one.
		 * \returns The updated human readable comment of the code line.
		 */
		virtual string updateAddressInDumpInstruction(string opcode_dump, uint32_t address) = 0;

		virtual instr_type_t getInstructionType(string str) = 0;

	protected:
		/**
		 * \brief Initializes the regular expressions to search for the different instruction typed.
		 */
		virtual void initializeRegExs(void) = 0;
		/**
		 * \brief Test the classification of the different instructions.
		 */
		virtual void testInstrClassification(void) = 0;

		/// regular expression to determining valid code lines (identified by address: "axxxxx:")
		regex re_addr;
		/// regular expression to determine jump/call labels
		regex re_label;
		/// regular expression to find the 16 or 32 bit opcode in dump file
		regex re_opcode;
		/// regular expression to find memory holes inserted by compiler
		regex re_memory_hole;
		/// regular expression to find DISP activate instruction
		regex pattern;

	private:
		/**
		 * \brief Pointer to the LOGCXX logger object.
		 */
		static LoggerPtr logger;
		
};

#endif
