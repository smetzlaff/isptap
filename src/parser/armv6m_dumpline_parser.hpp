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
#ifndef _ARMV6M_DUMPLINE_PARSER_HPP_
#define _ARMV6M_DUMPLINE_PARSER_HPP_

#include "dumpline_parser.hpp"
#include "armv6m_isa.hpp"

/**
 * \brief Provides functions to parse lines from the .dump file.
 * This class checks the type of the lines of the .dump file. It provides the opcode or the label/address, if the line type matches.
 * This class implements the singleton pattern.
 */
class Armv6mDumpLineParser : public DumpLineParser {
	public:
		/**
		 * \brief provides pointer to singletn DumpLineParser object.
		 * \returns Pointer to singleton DumpLineParser object.
		 */
		static Armv6mDumpLineParser* getInstance(void);
		/**
		 * \brief Checks if an opcode string represents a branch instruction.
		 * \param str Line of a .dump-file. The line has to be a code line, see isCodeLine()
		 * \returns True, if the line represents a branch instruction, else false.
		 */
		bool isBranchInstr(string str);
		/**
		 * \brief Checks if an opcode string represents a conditional branch instruction.
		 * \param str Line of a .dump-file. The line has to be a code line, see isCodeLine()
		 * \returns True, if the line represents a conditional branch instruction, else false.
		 */
		bool isCondBranchInstr(string str);
		/**
		 * \brief Checks if an opcode string represents an indirect branch instruction.
		 * \param str Line of a .dump-file. The line has to be a code line, see isCodeLine()
		 * \returns True, if the line represents an indirect branch instruction, else false.
		 */
		bool isIndirectBranchInstr(string str);
		/**
		 * \brief Checks if an opcode string represents an indirect call instruction.
		 * \param str Line of a .dump-file. The line has to be a code line, see isCodeLine()
		 * \returns True, if the line represents an indirect call instruction, else false.
		 */
		bool isIndirectCallInstr(string str);
		/**
		 * \brief Checks if an opcode string represents a return instruction.
		 * NOTICE: this implementation cannot detect returns that are coded by a BX 
		 * instruction that jumps to the link register which was present at the stack (the same
		 * register to which the jump is directed was popped from stack before.)
		 * \param str Line of a .dump-file. The line has to be a code line, see isCodeLine()
		 * \returns True, if the line represents a return instruction, else false.
		 */
		bool isReturnInstr(string str);
		/**
		 * \brief Checks if an opcode string represents a return instruction.
		 * NOTICE: the BX/POP return detection is not SAFE!
		 * \param str Line of a .dump-file. The line has to be a code line, see isCodeLine()
		 * \param bb_code The instructions of the basic block before the instruction that is checked.
		 * \returns True, if the line represents a return instruction, else false.
		 */
		bool isReturnInstr(string str, vector<string> bb_code);
		/**
		 * \brief Checks if an opcode string represents a debug instruction.
		 * \param str Line of a .dump-file. The line has to be a code line, see isCodeLine()
		 * \returns True, if the line represents a debug instruction, else false.
		 */
		bool isDebugInstr(string str);
		/**
		 * \brief Checks if an opcode string represents a call instruction.
		 * \param str Line of a .dump-file. The line has to be a code line, see isCodeLine()
		 * \returns True, if the line represents a call instruction, else false.
		 */
		bool isCallInstr(string str);
		/**
		 * \brief Returns the human readable comment of the instruction in the code line.
		 * If a code line is identified by using isCodeLine(), this method returns the comment of the instruction that is in this line.
		 * The comment of a code line is the deassembled instructions, giving hints for human reader which instruction is executed and which registers and immediates are used.
		 * \param str Code line of a dump file.
		 * \returns The human readable comment of an instruction.
		 */
		string getCommentFromCodeLine(string str);
		/**
		 * \brief Assembles a code line from opcode address, opcode and human readable comment.
		 * The assembled code line can be used to alter a basic block, like inserting additional or updating instructions.
		 * \param opcode_address The address of the instruction represented in this code line.
		 * \param opcode_raw The opcode as byte string (wrong byte order)
		 * \param opcode_dump The human readable opcode comment.
		 * \returns A code line assembled by use the parameters. 
		 */
		string assembleCodeLine(uint32_t opcode_address, string opcode_raw, string opcode_dump);
		/**
		 * \brief Updates the address in the human readable comment of call or branch instructions.
		 * This method replaces the first string that matches an address ((HEX_TOKEN"{8}") in the instruction comment with the address given by parameter. The method should be used for branch and call instructions only, since other instructions may have no address in their comment. A lea (load effective address) instruction (or similar instructions) may also be used but then the updated address represents data not an instruction in the code, as for calls and branches, so the caller of the method has to be sure that the address updated is correct.
		 * \param opcode_dump The human readable opcode comment part of a code line (obtained by getCommentFromCodeLine()) of a branch or call instruction.
		 * \param address The new address that replaces the old one.
		 * \returns The updated human readable comment of the code line.
		 */
		string updateAddressInDumpInstruction(string opcode_dump, uint32_t address);

		instr_type_t getInstructionType(string str);

		/**
		 * \brief Detects if a code line is not properly disassembled and contains ".word"
		 * \param str A full line from the dump file to check if it contains the .word directive.
		 * \returns True if the line contains .word, else false
		 */
		bool isWordDirectiveLine(string str);
	protected:
		/**
		 * \brief Initializes the regular expressions to search for the different instruction typed.
		 */
		void initializeRegExs(void);
		/**
		 * \brief Test the classification of the different instructions.
		 */
		void testInstrClassification(void);

	private:
		/**
		 * \brief Default constructor.
		 * The default constructor is private, since the object implements the singleton pattern
		 */
		Armv6mDumpLineParser();
		/**
		 * \brief Default destructor.
		 * The default destructor is private, since the object implements the singleton pattern
		 */
		virtual ~Armv6mDumpLineParser();
		/**
		 * \brief Copy Constructor to prevent copy-construction.
		 * Is private to implement the singleton pattern.
		 */
		Armv6mDumpLineParser(const Armv6mDumpLineParser&);
		/**
		 * \brief Assignment operator to prevent assignment.
		 * Is private to implement the singleton pattern.
		 */
		Armv6mDumpLineParser& operator=(const Armv6mDumpLineParser&);
		/**
		 * \brief Static reference to object. To implement the singleton pattern.
		 */
		static Armv6mDumpLineParser* singleton;
		/**
		 * \brief Pointer to the LOGCXX logger object.
		 */
		static LoggerPtr logger;

		Armv6mISA *isa;
};

#endif
