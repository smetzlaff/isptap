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
#include "cfgmanipulator.hpp"
#include "dlp_factory.hpp"
#include "isa_factory.hpp"

LoggerPtr CFGManipulator::logger(Logger::getLogger("CFGManipulator"));

CFGManipulator::CFGManipulator()
{
	dlp = DLPFactory::getInstance()->getDLPObject();
	isa = ISAFactory::getInstance()->getISAObject();
}

CFGManipulator::CFGManipulator(const CFGManipulator &copy)
{
	dlp = DLPFactory::getInstance()->getDLPObject();
	isa = ISAFactory::getInstance()->getISAObject();


	// FIXME create copy contructor for all other objects with property_maps
	
	
	// default part of the copy contructor
	*(this) = copy;

	// set all properties
	nodeTypeNProp = get(nodetype_t(), cfg);
	startAddrNProp = get(startaddr_t(), cfg);
	startAddrStringNProp = get(startaddrstring_t(), cfg);
	endAddrNProp = get(endaddr_t(), cfg);
	bbSizeNProp = get(bbsize_t(), cfg);
	bbCodeNProp = get(bbcode_t(), cfg);
	bbInstrsNProp = get(bbinstrs_t(), cfg);
	callLabelNProp = get(calllabel_t(), cfg);
	edgeTypeEProp = get(edgetype_t(), cfg);
}

CFGManipulator::~CFGManipulator()
{
}

//void CFGManipulator::setCFG(ControlFlowGraph set_cfg, uint32_t code_size, CFGVertex cfg_entry, CFGVertex cfg_exit, uint32_t cfg_start_addr, string cfg_name)
void CFGManipulator::setCFG(ControlFlowGraph set_cfg, uint32_t code_size, uint32_t cfg_start_addr, string cfg_name)
{
	cfg = set_cfg;

	// get node property structures
	nodeTypeNProp = get(nodetype_t(), cfg);
	startAddrNProp = get(startaddr_t(), cfg);
	startAddrStringNProp = get(startaddrstring_t(), cfg);
	endAddrNProp = get(endaddr_t(), cfg);
	bbSizeNProp = get(bbsize_t(), cfg);
	bbCodeNProp = get(bbcode_t(), cfg);
	bbInstrsNProp = get(bbinstrs_t(), cfg);
	callLabelNProp = get(calllabel_t(), cfg);

	// get edge properties
	edgeTypeEProp = get(edgetype_t(), cfg);

	size = code_size;
//	entry = cfg_entry;
//	exit = cfg_exit;
	start_addr = cfg_start_addr;
	name = cfg_name;


}


void CFGManipulator::setCFG(function_graph_t *cfgo)
{
	cfg = cfgo->cfg->getCFG();

	// get node property structures
	nodeTypeNProp = get(nodetype_t(), cfg);
	startAddrNProp = get(startaddr_t(), cfg);
	startAddrStringNProp = get(startaddrstring_t(), cfg);
	endAddrNProp = get(endaddr_t(), cfg);
	bbSizeNProp = get(bbsize_t(), cfg);
	bbCodeNProp = get(bbcode_t(), cfg);
	bbInstrsNProp = get(bbinstrs_t(), cfg);
	callLabelNProp = get(calllabel_t(), cfg);

	// get edge properties
	edgeTypeEProp = get(edgetype_t(), cfg);
	
//	entry = cfgo->cfg->getCFGEntry();
//	exit = cfgo->cfg->getCFGExit();
	size = cfgo->cfg->getCodeSize();
	start_addr = cfgo->address;
	name = cfgo->name;

}

ControlFlowGraph CFGManipulator::getCFG(void)
{
	return cfg;
}

void CFGManipulator::insertFunctionLengthEncoding(void)
{
	cfgVertexIter vp;
	cfgOutEdgeIter epo;
	CFGVertex v,u;
	CFGEdge e;

	size = size + FLE_INSTRUCTION_LENGTH;

	for(vp = vertices(cfg); vp.first != vp.second; ++vp.first)
	{
		v = *vp.first;
		if(get(nodeTypeNProp, v) == BasicBlock)
		{
			uint32_t bb_start_addr = get(startAddrNProp, v);
			if(bb_start_addr == start_addr)
			{
				// instrument the first bb
				// bb instrumentation
	
				// update the bb size
				uint32_t bb_size = get(bbSizeNProp,v);
				bb_size += FLE_INSTRUCTION_LENGTH;
				put(bbSizeNProp, v, bb_size);

				// update the instruction count
				uint32_t bb_instrs = get(bbInstrsNProp, v);
				bb_instrs++;
				put(bbInstrsNProp, v, bb_instrs);

				// update the bb end address
				uint32_t bb_endaddr = get(endAddrNProp, v);
				bb_endaddr += FLE_INSTRUCTION_LENGTH;
				put(endAddrNProp, v, bb_endaddr);

				// insert FLE (function length encoding) instruction
				string bb_code = get(bbCodeNProp, v);
				bb_code = addFLEInstruction(bb_code, start_addr);

				put(bbCodeNProp, v, bb_code);
			}
			else
			{
				// move all other bbs
				
				// update the bb start address
				bb_start_addr += FLE_INSTRUCTION_LENGTH;
				put(startAddrNProp, v, bb_start_addr);

				// update the bb end address
				uint32_t bb_endaddr = get(endAddrNProp, v);
				bb_endaddr += FLE_INSTRUCTION_LENGTH;
				put(endAddrNProp, v, bb_endaddr);

				string bb_code = get(bbCodeNProp, v);
			 	// relocate the bb
				bb_code = bbRelocate(bb_code, FLE_INSTRUCTION_LENGTH);

				put(bbCodeNProp, v, bb_code);
			}
		}
	}

	// adjust jump targets
	// this should not be necessary, since all jumps are relative and within the function (i.e. within this cfg)
	for(vp = vertices(cfg); vp.first != vp.second; ++vp.first)
	{
		v = *vp.first;
		if(get(nodeTypeNProp, v) == BasicBlock)
		{
			bool jump = false;
			uint32_t bb_target_addr=0x0;
			for(epo = out_edges(v, cfg); epo.first != epo.second; ++epo.first) 
			{
				e = *epo.first;
				edge_type_t et = get(edgeTypeEProp, e);
				if((et == ForwardJump) || (et == BackwardJump))
				{
					u = target(e, cfg);
					if(get(nodeTypeNProp, u) == BasicBlock)
					{
						bb_target_addr = get(startAddrNProp, u);
					}
					else if(get(nodeTypeNProp, u) == CallSite)
					{
						bb_target_addr = get(callLabelNProp, u);
					}
					else
					{
						assert(false);
					}
					jump=true;
					break;
				}
			}

			if(jump)
			{
				string bb_code = get(bbCodeNProp, v);
				bb_code = bbAdjustJumpTarget(bb_code, bb_target_addr);
				put(bbCodeNProp, v, bb_code);
			}
		}
	}
}

void CFGManipulator::relocateCode(uint32_t to_address)
{
	// relocate every bb
	cfgVertexIter vp;
	cfgOutEdgeIter epo;
	CFGVertex v,u;
	CFGEdge e;

	uint32_t offset = to_address - start_addr;
	start_addr = to_address;

	for(vp = vertices(cfg); vp.first != vp.second; ++vp.first)
	{
		v = *vp.first;
		if(get(nodeTypeNProp, v) == BasicBlock)
		{
			// update the bb start address
			uint32_t bb_start_addr = get(startAddrNProp, v);
			bb_start_addr += offset;
			put(startAddrNProp, v, bb_start_addr);

			// update the bb end address
			uint32_t bb_endaddr = get(endAddrNProp, v);
			bb_endaddr += offset;
			put(endAddrNProp, v, bb_endaddr);

			// relocate the bb
			string bb_code = get(bbCodeNProp, v);
			bb_code = bbRelocate(bb_code, offset);
			put(bbCodeNProp, v, bb_code);
		}
	}

	// adjust jump targets
	// this should not be necessary, since all jumps are relative and within the function (i.e. within this cfg)
	for(vp = vertices(cfg); vp.first != vp.second; ++vp.first)
	{
		v = *vp.first;
		if(get(nodeTypeNProp, v) == BasicBlock)
		{
			bool jump = false;
			uint32_t bb_target_addr=0x0;
			for(epo = out_edges(v, cfg); epo.first != epo.second; ++epo.first) 
			{
				e = *epo.first;
				edge_type_t et = get(edgeTypeEProp, e);
				if((et == ForwardJump) || (et == BackwardJump))
				{
					u = target(e, cfg);
					if(get(nodeTypeNProp, u) == BasicBlock)
					{
						bb_target_addr = get(startAddrNProp, u);
					}
					else if(get(nodeTypeNProp, u) == CallSite)
					{
						bb_target_addr = get(callLabelNProp, u);
					}
					else
					{
						assert(false);
					}
					jump=true;
					break;
				}
			}

			if(jump)
			{
				string bb_code = get(bbCodeNProp, v);
				bb_code = bbAdjustJumpTarget(bb_code, bb_target_addr);
				put(bbCodeNProp, v, bb_code);
			}
		}
	}

}

void CFGManipulator::alignCode(uint32_t align_to_bit_addr)
{
	uint32_t alignment = (1 << align_to_bit_addr);
	if((start_addr & (alignment-1)) != 0)
	{
		// the code is not aligned to align_to_bit_addr
		LOG_DEBUG(logger, "Aligning function from: 0x" << hex << start_addr << " to 0x" << ((start_addr & (~(alignment-1))) + alignment));
		relocateCode(((start_addr & (~(alignment-1))) + alignment));
	}
	else
	{
		LOG_DEBUG(logger, "Alignment not needed. function with address 0x" << hex << start_addr << " is already aligned to " << dec << align_to_bit_addr << " bit address");
	}
}

void CFGManipulator::updateCall(addr_label_t update_call_function)
{
	cfgVertexIter vp;
	cfgOutEdgeIter epo;
	CFGVertex v,u;
	CFGEdge e;

	string function_id_string = string("<CALLSITE: ");
	function_id_string += update_call_function.label;
	function_id_string += ">";

	LOG_DEBUG(logger, "checking for call to: " << function_id_string);
	for(vp = vertices(cfg); vp.first != vp.second; ++vp.first)
	{
		v = *vp.first;

		if(get(nodeTypeNProp, v) == BasicBlock)
		{
//			LOG_DEBUG(logger, "Checking bb 0x" << hex << get(startAddrNProp,v));
			for(epo = out_edges(v, cfg); epo.first != epo.second; ++epo.first) 
			{
				e = *epo.first;
				edge_type_t et = get(edgeTypeEProp, e);
				if(et == ForwardJump)
				{
					u = target(e, cfg);
//					LOG_DEBUG(logger, "Checking connection to bb 0x" << hex << get(startAddrNProp,u));

					if((get(nodeTypeNProp, u) == CallSite))
					{
						LOG_DEBUG(logger, "checking call site: " << get(startAddrStringNProp, u));
						if(function_id_string.compare(get(startAddrStringNProp, u)) == 0)
						{
							// update the code of the calling basic block
							string bb_code = get(bbCodeNProp, v);
							bb_code = bbAdjustCallTarget(bb_code, update_call_function);
							put(bbCodeNProp, v, bb_code);


							// update call site node
							put(startAddrStringNProp, u, function_id_string);
							put(callLabelNProp, u, update_call_function.address);
						}
					}
				}
			}
		}
	}
}

bool CFGManipulator::updateDISPActivation(FunctionMappingMap fmMap)
{
	cfgVertexIter vp;
	CFGVertex v;

	for(vp = vertices(cfg); vp.first != vp.second; ++vp.first)
	{
		v = *vp.first;

		if(get(nodeTypeNProp, v) == BasicBlock)
		{
			string bb_code = get(bbCodeNProp, v);
			if(bbContainsDISPActivation(bb_code))
			{
				// update the code of the calling basic block
				bb_code = bbUpdateDISPActivation(bb_code, fmMap);
				put(bbCodeNProp, v, bb_code);
				return true;
			}
		}
	}
	return false;
}

string CFGManipulator::addFLEInstruction(string bbcode, uint32_t address)
{
	// encoding the function size in the mtcr instruction
	// the used mtcr address offset is 0xeXXX
	// this 3 nibbles plus the nibble for the selection of 
	// register (from which usually the value is moved into the 
	// core register) are used for function length encoding in byte
	// the instruction is formated as follows:
	// cd XX XX 0e  -> mtcr $0xeXXX, %dX
	// the 2 bytes marked with X are used for function size encoding
	// NOTE: the core registers starting with 0xexxx cannot be used anymore, by any other application, but the code is still executable when not using the fspad or executing some functions from off-chip mem
	char fle_opcode[8];
	// write opcode in wrong byte order, since the dlp->assembleCodeLine() corrects it.
	sprintf(fle_opcode, "cd%02x%02x0e", (size&0xff), ((size>>8)&0xff));

	char fle_dump[128];
	sprintf(fle_dump, "mtcr special instruction for DISP (function size: %d bytes)", size);

	string opcode_line = dlp->assembleCodeLine(address, string(fle_opcode), string(fle_dump));
	return addInstructionAtBBTop(bbcode, opcode_line, FLE_INSTRUCTION_LENGTH);
}

string CFGManipulator::addInstructionAtBBTop(string bbcode, string new_opcode, uint32_t new_opcode_length)
{
	vector<string> instrs;
	bool first_opcode = false;
	uint32_t first_opcode_id = 0;
	split(instrs, bbcode, boost::is_any_of("\r"));
	for(uint32_t i = 0 ; i < instrs.size(); i++)
	{
		if(instrs[i] != "")
		{
			if(dlp->isCodeLine(instrs[i]))
			{
				if(!first_opcode)
				{
					first_opcode = true;
					first_opcode_id = i;
				}
				// alter address of all instructions 
				uint32_t address = dlp->getAddrFromCodeLine(instrs[i]);
				string instr_raw = dlp->getInstructionFromCodeLine(instrs[i]);
				string instr_dump = dlp->getCommentFromCodeLine(instrs[i]);
				instrs[i] = dlp->assembleCodeLine(address+new_opcode_length, instr_raw, instr_dump);
			}
			else if(dlp->isLabelLine(instrs[i]))
			{
				// do nothing
			}
		}
	}
	assert(first_opcode);
	
	instrs.insert(instrs.begin()+first_opcode_id, new_opcode);


	// join the vector to a string with "\r" at the end of every opcode line
	stringstream str;

	for(uint32_t i = 0 ; i < instrs.size(); i++)
	{
		str << instrs[i] << "\r";
	}

	return str.str();
}

string CFGManipulator::bbRelocate(string bbcode, int32_t offset)
{
	vector<string> instrs;
	split(instrs, bbcode, boost::is_any_of("\r"));
	for(uint32_t i = 0 ; i < instrs.size(); i++)
	{
		if(instrs[i] != "")
		{
			if(dlp->isCodeLine(instrs[i]))
			{
				uint32_t address = dlp->getAddrFromCodeLine(instrs[i]);
				string instr_raw = dlp->getInstructionFromCodeLine(instrs[i]);
				string instr_dump = dlp->getCommentFromCodeLine(instrs[i]);
				instrs[i] = dlp->assembleCodeLine(address+offset, instr_raw, instr_dump);
			}
			else if(dlp->isLabelLine(instrs[i]))
			{
				addr_label_t bb_label = dlp->getAddrAndLabelFromLabelLine(instrs[i]);
				bb_label.address = bb_label.address + offset;
				instrs[i] = dlp->assembleLabelLine(bb_label);
			}
		}
	}
	
	// join the vector to a string with "\r" at the end of every opcode line
	stringstream str;

	for(uint32_t i = 0 ; i < instrs.size(); i++)
	{
		str << instrs[i] << "\r";
	}

	return str.str();
}

string CFGManipulator::bbAdjustJumpTargetRel(string bbcode, int32_t offset)
{
	// DEPRICATED 
	// (do not use)
	vector<string> instrs;
	split(instrs, bbcode, boost::is_any_of("\r"));

	bool finished=false;
	// alter just the last instruction (if it is a jump)
	for(int32_t i = instrs.size()-1; i>=0 && !finished; i--)
	{
		if(dlp->isCodeLine(instrs[i]))
		{
			if(dlp->isBranchInstr(instrs[i]) && !dlp->isCallInstr(instrs[i]))
			{
				uint32_t address = dlp->getAddrFromCodeLine(instrs[i]);
				string instr_raw = dlp->getInstructionFromCodeLine(instrs[i]);
				string instr_dump;
				jump_target_address_t target = isa->getJumpTargetAddr(instr_raw, address);
				if(!target.valid)
				{
					target.addr = target.addr + offset;
					instr_raw = isa->setJumpTargetAddr(instr_raw, address, target.addr);
					instr_dump = dlp->updateAddressInDumpInstruction(dlp->getCommentFromCodeLine(instrs[i]), target.addr);
					instr_dump += "(altered)";
					instrs[i] = dlp->assembleCodeLine(address, instr_raw, instr_dump);
					finished = true;
				}
				else
				{
					LOG_WARN(logger, "Cannot alter indirect jump: " << instr_dump << " at 0x" << hex << address);
				}
			}
			else
			{
				// the last instruction of every basic block that has jumps must be a branch
				assert(false);
			}

		}
	}

	// join the vector to a string with "\r" at the end of every opcode line
	stringstream str;

	for(uint32_t i = 0 ; i < instrs.size(); i++)
	{
		str << instrs[i] << "\r";
	}

	return str.str();
}

string CFGManipulator::bbAdjustJumpTarget(string bbcode, uint32_t target_addr)
{
	vector<string> instrs;
	split(instrs, bbcode, boost::is_any_of("\r"));

	LOG_DEBUG(logger, "Altering jump of bb at: 0x" << hex << dlp->getAddrFromCodeLine(instrs.front()));
	bool finished=false;
	// alter just the last instruction (if it is a jump)
	for(int32_t i = instrs.size()-1; i>=0 && !finished; i--)
	{
		if(instrs[i].compare("") != 0)
		{
			LOG_DEBUG(logger, "Checking instruction: " << instrs[i]);
			if(dlp->isCodeLine(instrs[i]))
			{
				if(dlp->isBranchInstr(instrs[i]))// && !dlp->isCallInstr(instrs[i]))
				{
					uint32_t address = dlp->getAddrFromCodeLine(instrs[i]);
					string instr_raw = dlp->getInstructionFromCodeLine(instrs[i]);
					string instr_dump = dlp->updateAddressInDumpInstruction(dlp->getCommentFromCodeLine(instrs[i]), target_addr);
					instr_dump += "(altered)";

					instr_raw = isa->setJumpTargetAddr(instr_raw, address, target_addr);
					if(logger->isDebugEnabled())
					{
						jump_target_address_t calculated_target = isa->getJumpTargetAddr(instr_raw, address);
						if((!calculated_target.valid) || (target_addr != calculated_target.addr))
						{
							LOG_ERROR(logger, "Error in bbAdjustJumpTarget(): 0x" << hex << target_addr << " != 0x" << calculated_target.addr);
						}
					}
					LOG_DEBUG(logger, "Altering jump at: 0x" << hex << address << " to: 0x" << target_addr << " diff: " << target_addr-address);

					instrs[i] = dlp->assembleCodeLine(address, instr_raw, instr_dump);
					finished = true;
				}
				else
				{
					// the last instruction of every basic block that has jumps must be a branch
					assert(false);
				}
			}
		}
	}

	// join the vector to a string with "\r" at the end of every opcode line
	stringstream str;

	for(uint32_t i = 0 ; i < instrs.size(); i++)
	{
		str << instrs[i] << "\r";
	}

	return str.str();
}

string CFGManipulator::bbAdjustCallTarget(string bbcode, addr_label_t called_function)
{
	vector<string> instrs;
	split(instrs, bbcode, boost::is_any_of("\r"));


	bool finished=false;
	// alter just the last instruction (if it is a jump)
	for(int32_t i = instrs.size()-1; i>=0 && !finished; i--)
	{
		if(instrs[i].compare("") != 0)
		{
			if(dlp->isCodeLine(instrs[i]))
			{
				if(dlp->isCallInstr(instrs[i]))
				{
					uint32_t address = dlp->getAddrFromCodeLine(instrs[i]);
					string instr_raw = dlp->getInstructionFromCodeLine(instrs[i]);
					string instr_dump = dlp->updateAddressInDumpInstruction(dlp->getCommentFromCodeLine(instrs[i]), called_function.address);
					instr_dump += "(altered)";

					instr_raw = isa->setJumpTargetAddr(instr_raw, address, called_function.address);

					if(logger->isDebugEnabled())
					{
						jump_target_address_t calculated_target = isa->getJumpTargetAddr(instr_raw, address);
						if((!calculated_target.valid) || (called_function.address != calculated_target.addr))
						{
							LOG_ERROR(logger, "Error in bbAdjustJumpTarget(): 0x" << hex << called_function.address << " != 0x" << calculated_target.addr);
						}
					}

					LOG_DEBUG(logger, "Altering call at: 0x" << hex << address << " to: 0x" << called_function.address << " diff: " << called_function.address-address);

					instrs[i] = dlp->assembleCodeLine(address, instr_raw, instr_dump);
					finished = true;
				}
				else
				{
					// the last instruction of every basic block that has a call must be the call
					assert(false);
				}
			}
		}
	}

	// join the vector to a string with "\r" at the end of every opcode line
	stringstream str;

	for(uint32_t i = 0 ; i < instrs.size(); i++)
	{
		str << instrs[i] << "\r";
	}

	return str.str();
}




uint32_t CFGManipulator::getStartAddr(void)
{
	return start_addr;
}

uint32_t CFGManipulator::getEndAddr(void)
{
	return start_addr + size;
}

uint32_t CFGManipulator::getSize(void)
{
	return size;
}

bool CFGManipulator::bbContainsDISPActivation(string bbcode)
{
	vector<string> instrs;
	split(instrs, bbcode, boost::is_any_of("\r"));


	for(uint32_t i = 0 ; i < instrs.size(); i++)
	{
		if(instrs[i].compare("") != 0)
		{
			if(dlp->isCodeLine(instrs[i]))
			{
//				LOG_DEBUG(logger, "checking DISP activation for " << hex << instrs[i]); 
				if((dynamic_cast<TricoreISA *>(isa))->isDISPActivate(dlp->getInstructionFromCodeLine(instrs[i])))
				{
					return true;
				}
			}
		}
	}
	return false;
}

string CFGManipulator::bbUpdateDISPActivation(string bbcode, FunctionMappingMap fmMap)
{
	vector<string> instrs;
	split(instrs, bbcode, boost::is_any_of("\r"));

	bool finished = false;
	for(int32_t i = instrs.size()-1; (i>=0 && !finished); i--)
	{
		if(instrs[i].compare("") != 0)
		{
			if(dlp->isCodeLine(instrs[i]))
			{
				LOG_DEBUG(logger, "checking DISP activation for " << hex << instrs[i]); 
				if((dynamic_cast<TricoreISA *>(isa))->isDISPActivate(dlp->getInstructionFromCodeLine(instrs[i])))
				{

					// TODO use a push rx, rewrite rx, disp activate using rx and pop rx code sequence instead of register manipulation. But since the code size could change these instructions have to be inserted while relocating code and after that the register rx value has to be set. The object may save the 2 necessary instructions to update the function address without searching

					string opcode = dlp->getInstructionFromCodeLine(instrs[i]);
					LOG_DEBUG(logger, "found DISP activation with reg: " << opcode[3] << " in instr: " << instrs[i]); 
					assert(i > 2);
					
					// XXX This part of the code is very inflexible
					assert(dlp->isCodeLine(instrs[i-2]) && dlp->isCodeLine(instrs[i-1]));

					string op_movh = dlp->getInstructionFromCodeLine(instrs[i-2]);
					string op_addi = dlp->getInstructionFromCodeLine(instrs[i-1]);

					uint32_t function_address = (isa->getImmediate(op_movh)<<16) + isa->getImmediate(op_addi);

					FunctionMappingMap::iterator pos;
					pos = fmMap.find(function_address);
					assert(pos != fmMap.end());
					
					uint32_t new_function_address = pos->second;

					LOG_DEBUG(logger, "Updating call address in movh and addi from 0x" << hex << function_address << " to 0x" << new_function_address);
					op_movh = isa->setImmediate(op_movh, (uint16_t)((new_function_address & 0xffff0000)>>16));
					op_addi = isa->setImmediate(op_addi, (uint16_t)(new_function_address & 0x0000ffff));

					instrs[i-2] = dlp->assembleCodeLine(dlp->getAddrFromCodeLine(instrs[i-2]), op_movh, dlp->getCommentFromCodeLine(instrs[i-2]));
					instrs[i-1] = dlp->assembleCodeLine(dlp->getAddrFromCodeLine(instrs[i-1]), op_addi, dlp->getCommentFromCodeLine(instrs[i-1]));

					finished = true;
					LOG_DEBUG(logger, "Updated DISP activation.");
				}
			}
		}
	}
	// join the vector to a string with "\r" at the end of every opcode line
	stringstream str;

	for(uint32_t i = 0 ; i < instrs.size(); i++)
	{
		str << instrs[i] << "\r";
	}

	return str.str();
}


addr_name_size_t CFGManipulator::getFunctionMetaData(void)
{
	addr_name_size_t result;
	result.address = start_addr;
	result.name = name;
	result.size = size;
	return result;
}

