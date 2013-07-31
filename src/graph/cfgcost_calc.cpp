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
#include "cfgcost_calc.hpp"
#include "dlp_factory.hpp"
#include "isa_factory.hpp"
#include "arch_cfg_factory.hpp"

LoggerPtr ControlFlowGraphCostCalculator::logger(Logger::getLogger("ControlFlowGraphCostCalculator"));

ControlFlowGraphCostCalculator::ControlFlowGraphCostCalculator(ControlFlowGraph cfgraph,  CFGVertex cfg_entry, CFGVertex cfg_exit)
{
	dlp = DLPFactory::getInstance()->getDLPObject();
	isa = ISAFactory::getInstance()->getISAObject();
	arch_cfg = ArchConfigFactory::getInstance()->getArchConfigObject();

	cfg = cfgraph;
	entry = cfg_entry;
	exit = cfg_exit;

	// get node property structures
	nodeTypeNProp = get(nodetype_t(), cfg);
	startAddrNProp = get(startaddr_t(), cfg);
	startAddrStringNProp = get(startaddrstring_t(), cfg);
	endAddrNProp = get(endaddr_t(), cfg);
	callLabelNProp = get(calllabel_t(), cfg);
	bbCodeNProp = get(bbcode_t(), cfg);
	bbSizeNProp = get(bbsize_t(), cfg);
	bbInstrsNProp = get(bbinstrs_t(), cfg);


	// get edge property structures
	costEProp = get(cost_t(), cfg);
	costOnChipEProp = get(cost_onchip_t(), cfg);
	costOffChipEProp = get(cost_offchip_t(), cfg);
	memPenaltyEProp = get(mem_penalty_t(), cfg);
	capacitylEProp = get(capacityl_t(), cfg);
	capacityhEProp = get(capacityh_t(), cfg);
	circEProp = get(circ_t(), cfg);
	actEProp = get(activation_t(), cfg);
	edgeNameEProp = get(edgename_t(), cfg);
	edgeTypeEProp = get(edgetype_t(), cfg);



	// build the address vertex map
	cfgVertexIter vp;
	bool inserted;
	AddrVertexMap::iterator pos;
	for (vp = vertices(cfg); vp.first != vp.second; ++vp.first)
	{
		CFGVertex v = *vp.first;

		if(get(nodeTypeNProp, v) == BasicBlock)
		{
			tie(pos, inserted) = avMap.insert(make_pair(get(startAddrNProp, v), v));
			assert(inserted);
		}
	}
}

ControlFlowGraphCostCalculator::~ControlFlowGraphCostCalculator()
{
	dlp = NULL;
	isa = NULL;
	arch_cfg = NULL;
}

void ControlFlowGraphCostCalculator::calculateCost(void)
{
	addBBCostToCfg();
}


void ControlFlowGraphCostCalculator::considerMemoryAssignment(vector<uint32_t> &assigned_blocks, bool addJumpPenalty)
{
	if(addJumpPenalty)
	{
		// First connecting jumps have to be added, then the cost of the basic blocks have to be recalculated.
		addConnectingJumpsForAssignedBlocks(assigned_blocks);
		calculateCost();
	}
	setAssignedBlocks(assigned_blocks);
}

uint32_t ControlFlowGraphCostCalculator::getSizeOfAssignedBlocks(vector<uint32_t> &assigned_blocks)
{
	cfgVertexIter vp;
	CFGVertex v;
	cfgOutEdgeIter epo;
	CFGEdge e;
	uint32_t used_size=0;

	// this method is only usable for static memory types
	assert(IS_STATIC_MEM((mem_type_t)conf->getUint(CONF_MEMORY_TYPE)));

	// search for matching blocks and set the onchip cost for their outedges
	for(uint32_t i = 0; i < assigned_blocks.size(); i++)
	{
		for (vp = vertices(cfg); vp.first != vp.second; ++vp.first)
		{
			v = *vp.first;
			if((get(nodeTypeNProp, v) == BasicBlock) && get(startAddrNProp, v) == assigned_blocks[i])
			{
				used_size += get(bbSizeNProp, v);
			}
		}
	}
	return used_size;
}

void ControlFlowGraphCostCalculator::setAssignedBlocks(vector<uint32_t> &assigned_blocks)
{
	cfgVertexIter vp;
	CFGVertex v;
	cfgOutEdgeIter epo;
	CFGEdge e;

	// this method is only usable for static memory types
	assert(IS_STATIC_MEM((mem_type_t)conf->getUint(CONF_MEMORY_TYPE)));

	// search for matching blocks and set the onchip cost for their outedges
	for(uint32_t i = 0; i < assigned_blocks.size(); i++)
	{
		for (vp = vertices(cfg); vp.first != vp.second; ++vp.first)
		{
			v = *vp.first;
			if((get(nodeTypeNProp, v) == BasicBlock) && get(startAddrNProp, v) == assigned_blocks[i])
			{
				string str;
				str = "X ";
				str += get(startAddrStringNProp, v);
				put(startAddrStringNProp, v, str);
				for(epo = out_edges(*vp.first, cfg); epo.first != epo.second; ++epo.first) 
				{
					e = *epo.first;
					
					// for assigned blocks the memory penalty is cleared so the blocks execution time is the onchip cost only
					put(memPenaltyEProp, e, 0); 

//					put(costEProp, e, get(costOnChipEProp,e));
				}
			}
		}
	}
}

void ControlFlowGraphCostCalculator::addConnectingJumpsForAssignedBlocks(vector<uint32_t> &assigned_blocks)
{
	cfgVertexIter vp;
	CFGVertex v;
	cfgOutEdgeIter epo;
	CFGEdge e;

	vector<CFGVertex> added_jump;
	vector<CFGVertex> altered_jump;

	// - add jump in bbs
	// - change edge type for inserted jumps
	for (vp = vertices(cfg); vp.first != vp.second; ++vp.first)
	{
		v = *vp.first;
		if((get(nodeTypeNProp, v) == BasicBlock))
		{
			bool v_in_isp = isBlockAssigned(v, assigned_blocks);
			for(epo = out_edges(*vp.first, cfg); epo.first != epo.second; ++epo.first) 
			{
				e = *epo.first;

				CFGVertex w = target(e, cfg);

				if(get(nodeTypeNProp, w) == BasicBlock) 
				{
					bool w_in_isp = isBlockAssigned(w, assigned_blocks);
					if(v_in_isp xor w_in_isp)
					{
						if(get(edgeTypeEProp, e) == ForwardStep)
						{
							// change edge type tp jump (Forward or Backward does not matter in this case)
							put(edgeTypeEProp, e, ForwardJump);
							// add jump instruction to v to reach w
							addConnectingJump(v);
							added_jump.push_back(v);
						}
						else if((get(edgeTypeEProp, e) == ForwardJump) || (get(edgeTypeEProp, e) == BackwardJump))
						{
							if(!isInVertexList(v, altered_jump)) // this prevents altering one block multiple times, wich is possible for indirect jumps, because they have multiple jump targets
							{
								if(isInVertexList(v, added_jump))
								{
									alterConnectingJump(v, true);
								}
								else
								{
									alterConnectingJump(v, false);
								}
								altered_jump.push_back(v);
							}
						}
						else
						{
							// do something?
						}
					}
				}
				else if(get(nodeTypeNProp, w) == CallPoint)
				{
					// TODO: MISSING if a short call has to be extended to a long one: the node after the call point has to be checked, if it is in the scratchpad or not.
					bool bb_in_isp = isBlockAssigned(getFirstBBOfCalledFunction(w), assigned_blocks);
					if(v_in_isp xor bb_in_isp)
					{
						if(!isInVertexList(v, altered_jump)) // this prevents altering one block multiple times, wich is possible for indirect calls, because they have multiple jump targets
						{
							alterConnectingCall(v);
							altered_jump.push_back(v);
						}
					}
				}


				// check if v and target(e) are bbs and both in the isp or mem
			}
		}
	}

}

inline bool ControlFlowGraphCostCalculator::isBlockAssigned(CFGVertex v, vector<uint32_t> &assigned_blocks)
{
	assert(get(nodeTypeNProp, v) == BasicBlock);
	uint32_t addr = get(startAddrNProp, v);

	vector<uint32_t>::iterator it;

	for(it = assigned_blocks.begin(); it < assigned_blocks.end(); it++)
	{
		if(*it == addr)
		{
			return true;
		}
	}
	return false;
}

inline bool ControlFlowGraphCostCalculator::isInVertexList(CFGVertex v, vector<CFGVertex>& vl)
{
	vector<CFGVertex>::iterator it;

	for(it = vl.begin(); it < vl.end(); it++)
	{
		if(*it == v)
		{
			return true;
		}
	}
	return false;
}

void ControlFlowGraphCostCalculator::addConnectingJump(CFGVertex v)
{
	// NOTICE: adding an instruction increases the size of the basic block, thus all subsequent blocks should be relocated!
	// This is not done yet.
//#warning "but it has to because this has an impact to the WCET"

	uint32_t bb_size = get(bbSizeNProp, v);
	bb_size += arch_cfg->getSizePenalty(ContinuousAdressing, NoDisplacement);
	put(bbSizeNProp, v, bb_size);

	string bb_code = get(bbCodeNProp, v);
	vector<string> instrs;
	split(instrs, bb_code, boost::is_any_of("\r"));

	// clear non code lines from vector
	vector<string>::iterator it;
	for(it=instrs.begin(); it < instrs.end(); it++)
	{
		if(!dlp->isCodeLine(*it))
		{
			instrs.erase(it);
		}
	}

	uint32_t bb_endaddr = get(endAddrNProp, v);
	// set the end addr to the address of the inserted jump instruction, that is right after the last instruction
	bb_endaddr += isa->getInstructionLength(dlp->getInstructionFromCodeLine(instrs.back()));
	uint32_t j_code_addr = bb_endaddr;
	put(endAddrNProp, v, bb_endaddr);

	uint32_t bb_instrs = get(bbInstrsNProp, v);
	bb_instrs++;
	put(bbInstrsNProp, v, bb_instrs);

	string j_opcode = isa->getConnectingJumpInstruction();
	string j_dump = isa->getConnectingJumpComment();
	string j_opcode_line = dlp->assembleCodeLine(j_code_addr, j_opcode, j_dump);

	bb_code = bb_code + j_opcode_line + "\r";
	put(bbCodeNProp, v, bb_code);

//	LOG_DEBUG(logger, "Altered Basic Block: 0x" << hex << get(startAddrNProp, v) << " to 0x" << bb_endaddr << " with " << dec <<  bb_instrs << " instructions: " << bb_code);
}


void ControlFlowGraphCostCalculator::alterConnectingJump(CFGVertex v, bool connective_jump_already_added)
{
	// NOTICE: adding an instruction increases the size of the basic block, thus all subsequent blocks should be relocated!
	// This is not done yet.
//#warning "but it has to because this has an impact to the WCET"

	string bb_code = get(bbCodeNProp, v);
	vector<string> instrs;
	split(instrs, bb_code, boost::is_any_of("\r"));

	// clear non code lines from vector
	vector<string>::iterator it;
	for(it=instrs.begin(); it < instrs.end(); it++)
	{
		if(!dlp->isCodeLine(*it))
		{
			instrs.erase(it);
		}
	}

	if(connective_jump_already_added)
	{
		// In this case a connective jump was already added by addConnectingJump() before. Therefore second to last instruction is selected.
		instrs.pop_back();
	}
	string jump_instr = instrs.back();

	uint32_t sizePenalty=0;

	if(dlp->isBranchInstr(jump_instr) && (!dlp->isIndirectBranchInstr(jump_instr)/* indirect jumps does not need to be resized, because a register holds the jump target */) && (!dlp->isReturnInstr(jump_instr)) && (!dlp->isCallInstr(jump_instr)))
	{
		if(isa->getInstructionLength(dlp->getInstructionFromCodeLine(jump_instr)) == 2)
		{
			sizePenalty = 2;
		}
	}
	else if(dlp->isIndirectBranchInstr(jump_instr))
	{
		LOG_DEBUG(logger, "Indirect jumps do not need to be resized.");
	}
	else
	{
		LOG_ERROR(logger, "The last instruction has to be a branch.");
	}

	LOG_DEBUG(logger, "checking instr: " << jump_instr << " from block 0x" << hex << get(startAddrNProp, v) << " penatlty: " << sizePenalty);

	uint32_t bb_size = get(bbSizeNProp, v);
	bb_size += sizePenalty;
	put(bbSizeNProp, v, bb_size);

	uint32_t bb_endaddr = get(endAddrNProp, v);
	// set the end addr to the address of the inserted jump instruction, that is right after the last instruction
	bb_endaddr += sizePenalty;
	put(endAddrNProp, v, bb_endaddr);

	if(sizePenalty == 2)
	{
		stringstream code;
		// chop the last \r and add a comment
		code << bb_code.substr(0, bb_code.length()-1) << " BBSISP_JP: Added size penalty for short jump of " << sizePenalty << "\r";
		put(bbCodeNProp, v, code.str());
	}
}


inline CFGVertex ControlFlowGraphCostCalculator::getFirstBBOfCalledFunction(CFGVertex call_point)
{
	// get the first bb of the called function
	assert(get(nodeTypeNProp, call_point) == CallPoint);
	assert(out_degree(call_point, cfg) == 1);
	cfgOutEdgeIter eoi = out_edges(call_point, cfg);
	CFGVertex function_entry = target(*eoi.first, cfg);
	assert(get(nodeTypeNProp, function_entry) == Entry);
	assert(out_degree(function_entry, cfg) == 1);
	eoi = out_edges(function_entry, cfg);
	CFGVertex bb = target(*eoi.first, cfg);
	assert(get(nodeTypeNProp, bb) == BasicBlock);
	return bb;
}

void ControlFlowGraphCostCalculator::alterConnectingCall(CFGVertex v)
{
	// NOTICE: adding an instruction increases the size of the basic block, thus all subsequent blocks should be relocated!
	// This is not done yet.
//#warning "but it has to because this has an impact to the WCET"


	string bb_code = get(bbCodeNProp, v);
	vector<string> instrs;
	split(instrs, bb_code, boost::is_any_of("\r"));

	// clear non code lines from vector
	vector<string>::iterator it;
	for(it=instrs.begin(); it < instrs.end(); it++)
	{
		if(!dlp->isCodeLine(*it))
		{
			instrs.erase(it);
		}
	}

	string last_instr = instrs.back();
	uint32_t sizePenalty=0;

	if((dlp->isCallInstr(last_instr)) && (!dlp->isIndirectCallInstr(last_instr)/* indirect calls does not need to be resized, because a register holds the call target */))
	{
		if(isa->getInstructionLength(dlp->getInstructionFromCodeLine(last_instr)) == 2)
		{
			sizePenalty = 2;
		}
	}
	else if(dlp->isIndirectCallInstr(last_instr))
	{
		LOG_DEBUG(logger, "Indirect calls do not need to be resized.");
	}
	else
	{
		LOG_ERROR(logger, "The last instruction has to be a call.");
	}

	LOG_DEBUG(logger, "checking instr: " << last_instr << " from block 0x" << hex << get(startAddrNProp, v)  << " penatlty: " << sizePenalty);

	uint32_t bb_size = get(bbSizeNProp, v);
	bb_size += sizePenalty;
	put(bbSizeNProp, v, bb_size);

	uint32_t bb_endaddr = get(endAddrNProp, v);
	// set the end addr to the address of the inserted jump instruction, that is right after the last instruction
	bb_endaddr += sizePenalty;
	put(endAddrNProp, v, bb_endaddr);

	if(sizePenalty == 2)
	{
		stringstream code;
		// chop the last \r and add a comment
		code << bb_code.substr(0, bb_code.length()-1) << " BBSISP_JP: Added size penalty for short call of " << sizePenalty << "\r";
		put(bbCodeNProp, v, code.str());
	}
}
