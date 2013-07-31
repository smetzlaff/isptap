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
#ifndef _CFGMANIPULATOR_HPP_
#define _CFGMANIPULATOR_HPP_


#include "global.h"
#include "constants.h"
#include "configuration.hpp"
#include "graph_structure.h"
#include "fcgobj.hpp"
#include "dumpline_parser.hpp"
#include "isa.hpp"


#include <boost/algorithm/string.hpp>

#define FLE_INSTRUCTION_LENGTH 4


/**
 * \brief Map that maps the original function address to the updated new address (changed by code alignment or relocation)
 */
typedef map<uint32_t, uint32_t> FunctionMappingMap;

/**
 * \brief Allows code modification of a cfg to e.g. insert a function length encoding instruction or relocate a function.
 * The class can handle CFGs only, no super cfgs (where calls are inlined)
 */
class CFGManipulator {
	public:
		/**
		 * \brief Default constructor
		 */
		CFGManipulator();
		/**
		 * \brief Copy constructor
		 * Default copy contstructor needs to be extended to update the property_maps of the new object.
		 */
		CFGManipulator(const CFGManipulator &copy);
		/**
		 * \brief Default destructor
		 */
		virtual ~CFGManipulator();

//		void setCFG(ControlFlowGraph set_cfg);
//		void setCFG(ControlFlowGraph set_cfg, uint32_t code_size, CFGVertex cfg_entry, CFGVertex cfg_exit, uint32_t cfg_start_addr, string cfg_name);

		/**
		 * \brief Assigns the cfg and some meta data to the object for code modification.
		 * \param set_cfg The cfg that will be altered by this object.
		 * \param code_size The size of the code in the assigned cfg.
		 * \param cfg_start_addr The address of the function that is represented by the cfg (address of first instruction in cfg).
		 * \param cfg_name Name of the function that is represented in the cfg.
		 */
		void setCFG(ControlFlowGraph set_cfg, uint32_t code_size, uint32_t cfg_start_addr, string cfg_name);
		/**
		 * \brief Assigns the cfg and some meta data to the object for code modification.
		 * \param cfgo Pointer to cfg meta data of the function that will be altered by this object.
		 */
		void setCFG(function_graph_t *cfgo);
		/**
		 * \brief Returns the (possibly) altered CFG.
		 * \returns The altered CFG.
		 */
		ControlFlowGraph getCFG(void);
		/**
		 * \brief Inserts the function length encoding instruction at the start of the cfg of the function.
		 * Also updates the addresses and jump targets (if necessary) of following instructions
		 */
		void insertFunctionLengthEncoding(void);
		/**
		 * \brief Relocates the code represented in the cfg to the given address.
		 * \param to_address The address where to code should be located to.
		 */
		void relocateCode(uint32_t to_address);
		/**
		 * \brief Aligns the code represented in the cfg to a given address.
		 * \param align_to_bit_addr Defines to how much LSB zeros the new address have to be aligned.
		 * Parameter 3 means that the code has to be aligned to an address with the 3 least significant address bits have to be zero (in this case a 64bit aligned addressing). 
		 */
		void alignCode(uint32_t align_to_bit_addr);
		/**
		 * \brief Update the call address of a function.
		 * \param function Name and the new (updated) address of a function.
		 */
		void updateCall(addr_label_t function);
		/**
		 * \brief Updates the D-ISP activate instruction, if the activating function is relocated. 
		 * \param fmMap Map that translates old function addresses (used in calls in the current cfg) to updated function addresses (after relocation of other functions).
		 */
		bool updateDISPActivation(FunctionMappingMap fmMap);

		/**
		 * \brief Returns the updated start address of the maintained code.
		 * \returns Updated start address of the code.
		 */
		uint32_t getStartAddr(void);
		/**
		 * \brief Returns the updated end address of the maintained code.
		 * \returns Updated end address of the code.
		 */
		uint32_t getEndAddr(void);
		/**
		 * \brief Returns the updated size of the maintained code.
		 * \returns Updated size of the code.
		 */
		uint32_t getSize(void);
		/**
		 * Returns the meta data of the maintained function.
		 * \returns Meta data: address, name and size of the function.
		 */
		addr_name_size_t getFunctionMetaData(void);



	private:
		/**
		 * \brief Adds the function length encoding instruction into a basic block code.
		 * \param bbcode Code of the basic block.
		 * \param address Address of the basic block.
		 * \returns Updated code of the basic block.
		 */
		string addFLEInstruction(string bbcode, uint32_t address);
		/**
		 * \brief Adds an instruction as the first instruction into a basic block code.
		 * \param bbcode The original code of the basic block.
		 * \param new_opcode Opcode to add.
		 * \param new_opcode_length Length of the opcode to add.
		 * \returns Updated code of the basic block.
		 */
		string addInstructionAtBBTop(string bbcode, string new_opcode, uint32_t new_opcode_length);
		/**
		 * \brief Relocated a basic block by an offset.
		 * \param bbcode The original code of the basic block.
		 * \param offset Offset for relocation.
		 * \returns Updated code of the basic block.
		 */
		string bbRelocate(string bbcode, int32_t offset);
		/**
		 * \brief Adjust the jump target of the jump in the basic block.
		 * \param bbcode Code of the basic block.
		 * \param offset Difference of old jump target and new.
		 * \returns Updated code of the basic block.
		 */
		string bbAdjustJumpTargetRel(string bbcode, int32_t offset) __attribute__ ((deprecated));
		/**
		 * \brief Adjust the jump target of the jump in the basic block.
		 * \param bbcode Code of the basic block.
		 * \param target_addr New jump target address.
		 * \returns Updated code of the basic block.
		 */
		string bbAdjustJumpTarget(string bbcode, uint32_t target_addr);
		/**
		 * \brief Adjust the call target in the basic block.
		 * \param bbcode Code of the basic block.
		 * \param called_function The address and name of the call target to update.
		 * \returns Updated code of the basic block.
		 */
		string bbAdjustCallTarget(string bbcode, addr_label_t called_function);
		/**
		 * \brief Updates the D-ISP activation instruction.
		 * \param bbcode Code of the basic block.
		 * \param fmMap Address map that translates the old function address to new function address, after relocation of the functions.
		 * \returns Updated code of the basic block.
		 */
		string bbUpdateDISPActivation(string bbcode, FunctionMappingMap fmMap);
		/**
		 * \brief Checks if the D-ISP activation instruction is in the basic block to check.
		 * \param bbcode Code of the basic block.
		 * \returns True if the D-ISP activation instruction is contained in the basic block, else false.
		 */
		bool bbContainsDISPActivation(string bbcode);

		/**
		 * \brief CFG of the function to alter.
		 */
		ControlFlowGraph cfg;
		/**
		 * \brief Size of the maintained function.
		 */
		uint32_t size;
//		CFGVertex entry;
//		CFGVertex exit;
		/**
		 * \brief Start address of the maintained function.
		 */
		uint32_t start_addr;
		/** 
		 * \brief Name of the maintained function.
		 */
		string name;

		/**
		 * \brief Pointer to object for parsing the lines of the dump files.
		 */
		DumpLineParser *dlp;

		/**
		 * \brief Pointer to object for decoding and encoding instructions for the tricore ISA.
		 */
		ISA *isa;

		/**
		 * \brief Node type node property of maintained cfg.
		 */
		property_map<ControlFlowGraph, nodetype_t>::type nodeTypeNProp;
		/**
		 * \brief Start address node property of maintained cfg.
		 */
		property_map<ControlFlowGraph, startaddr_t>::type startAddrNProp;
		/**
		 * \brief Start address as string node type property of maintained cfg.
		 */
		property_map<ControlFlowGraph, startaddrstring_t>::type startAddrStringNProp;
		/**
		 * \brief End address node property of maintained cfg.
		 */
		property_map<ControlFlowGraph, endaddr_t>::type endAddrNProp;
		/**
		 * \brief Call label node property of maintained cfg.
		 */
		property_map<ControlFlowGraph, calllabel_t>::type callLabelNProp;
		/**
		 * \brief Basic block code node property of maintained cfg.
		 */
		property_map<ControlFlowGraph, bbcode_t>::type bbCodeNProp;
		/**
		 * \brief Basic block size node property of maintained cfg.
		 */
		property_map<ControlFlowGraph, bbsize_t>::type bbSizeNProp;
		/**
		 * \brief Basic block instruction count node property of maintained cfg.
		 */
		property_map<ControlFlowGraph, bbinstrs_t>::type bbInstrsNProp;
		/**
		 * \brief Edge type edge property of maintained cfg.
		 */
		property_map<ControlFlowGraph, edgetype_t>::type edgeTypeEProp;
		/**
		 * \brief Pointer to the global configuration object.
		 */
		Configuration *conf;
		/**
		 * \brief Pointer to the LOGCXX logger object.
		 */
		static LoggerPtr logger;

};

#endif
