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
#include "cfgobj.hpp"

LoggerPtr ControlFlowGraphObject::logger(Logger::getLogger("ControlFlowGraphObject"));

ControlFlowGraphObject::ControlFlowGraphObject() : code_size(0)
{
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


	// set up entry node:
	entry = add_vertex(cfg);
	put(nodeTypeNProp, entry, Entry);
	put(startAddrNProp, entry, 0x00000000);
	put(startAddrStringNProp, entry, string("<ENTRY>"));
	put(endAddrNProp, entry, 0x00000000);
	put(callLabelNProp, entry, -1);
	put(bbCodeNProp, entry, "");
	put(bbSizeNProp, entry, 0);
	put(bbInstrsNProp, entry, 0);
	entry_connected = false;

	// set up exit node:
	exit = add_vertex(cfg);
	put(nodeTypeNProp, exit, Exit);
	put(startAddrNProp, exit, 0xffffffff);
	put(startAddrStringNProp, exit, string("<EXIT>"));
	put(endAddrNProp, exit, 0xffffffff);
	put(callLabelNProp, exit, -1);
	put(bbCodeNProp, exit, "");
	put(bbSizeNProp, exit, 0);
	put(bbInstrsNProp, exit, 0);

//	bb_cost_container_onchip = NULL;
//	bb_cost_container_offchip = NULL;

//	conf = Configuration::getInstance();

//	memory_type = (mem_type_t)conf->getUint(CONF_MEMORY_TYPE);

//	carcore_timing_onchip = new CarCoreTiming(ONCHIP);
//	carcore_timing_offchip = new CarCoreTiming(OFFCHIP);

}

ControlFlowGraphObject::~ControlFlowGraphObject()
{
	incomplete_nodes.clear();
	call_targets.clear();
	call_sites.clear();
}

//ControlFlowGraph ControlFlowGraphObject::getCFG(void)
//{
//	return cfg;
//}
//
//CFGVertex ControlFlowGraphObject::getCFGEntry(void)
//{
//	return entry;
//}
//
//CFGVertex ControlFlowGraphObject::getCFGExit(void)
//{
//	return exit;
//}

void ControlFlowGraphObject::addBBNode(uint32_t start_addr, uint32_t end_addr, uint32_t next_addr, string code, uint32_t size, uint32_t instr_count)
{
	CFGVertex u, v;


	u = setSourceBB(start_addr, end_addr, code, size, instr_count);

	v = getTargetBB(next_addr);

	// create connecting edge
	CFGEdge e;
	bool ins_edge;
	tie(e, ins_edge) = add_edge(u, v, cfg);
	if(ins_edge)
	{
		// set properties for edge
		// the costs are set in addBBCostToCfg()
		put(costEProp, e, 0);
		put(costOffChipEProp, e, 0);
		put(costOnChipEProp, e, 0);
		put(memPenaltyEProp, e, 0);

		put(capacitylEProp, e, 0);
		put(capacityhEProp, e, 0);
		put(circEProp, e, -1);
		ostringstream ename;
		ename << "("<< u << ","<< v << ")";
		put(edgeNameEProp, e, ename.str());
		if(start_addr+size == next_addr)
		{
			LOG_DEBUG(logger, "created connecting edge from 0x" << hex << start_addr << " ( " << dec << u << " ) to 0x" << hex << next_addr << " ( " << dec << v << " ) is edge: " << dec << e << " type: forward step");
			put(edgeTypeEProp, e, ForwardStep);
		}
		else if(start_addr+size < next_addr)
		{
			LOG_DEBUG(logger, "created connecting edge from 0x" << hex << start_addr << " ( " << dec << u << " ) to 0x" << hex << next_addr << " ( " << dec << v << " ) is edge: " << dec << e << " type: forward jump");
			put(edgeTypeEProp, e, ForwardJump);

		}
		else if(start_addr+size > next_addr)
		{

			LOG_DEBUG(logger, "created connecting edge from 0x" << hex << start_addr << " ( " << dec << u << " ) to 0x" << hex << next_addr << " ( " << dec << v << " ) is edge: " << dec << e << " type: backward jump");
			put(edgeTypeEProp, e, BackwardJump);
		}
		else
			assert(false); // not possible

	}
}


void ControlFlowGraphObject::addBBNode(uint32_t start_addr, uint32_t end_addr, vector<uint32_t> next_addrs, string code, uint32_t size, uint32_t instr_count)
{
	CFGVertex u, v;

	u = setSourceBB(start_addr, end_addr, code, size, instr_count);

	for(uint32_t i = 0; i < next_addrs.size(); i++)
	{
		uint32_t next_addr = next_addrs[i];

		v = getTargetBB(next_addr);

		// create connecting edge
		CFGEdge e;
		bool ins_edge;
		tie(e, ins_edge) = add_edge(u, v, cfg);
		if(ins_edge)
		{
			// set properties for edge
			// the costs are set in addBBCostToCfg()
			put(costEProp, e, 0);
			put(costOffChipEProp, e, 0);
			put(costOnChipEProp, e, 0);
			put(memPenaltyEProp, e, 0);

			put(capacitylEProp, e, 0);
			put(capacityhEProp, e, 0);
			put(circEProp, e, -1);
			ostringstream ename;
			ename << "("<< u << ","<< v << ")";
			put(edgeNameEProp, e, ename.str());
			if(start_addr+size <= next_addr)
			{
				LOG_DEBUG(logger, "created connecting edge from 0x" << hex << start_addr << " ( " << dec << u << " ) to 0x" << hex << next_addr << " ( " << dec << v << " ) is edge: " << dec << e << " type: forward jump");
				put(edgeTypeEProp, e, ForwardJump);

			}
			else if(start_addr+size > next_addr)
			{

				LOG_DEBUG(logger, "created connecting edge from 0x" << hex << start_addr << " ( " << dec << u << " ) to 0x" << hex << next_addr << " ( " << dec << v << " ) is edge: " << dec << e << " type: backward jump");
				put(edgeTypeEProp, e, BackwardJump);
			}
			else
				assert(false); // not possible

		}
	}
}

void ControlFlowGraphObject::addBBNode(uint32_t start_addr, uint32_t end_addr, uint32_t next_addr, uint32_t cond_jump_addr, string code, uint32_t size, uint32_t instr_count)
{
	CFGVertex u, v, w;
	u = setSourceBB(start_addr, end_addr, code, size, instr_count);
	v = getTargetBB(next_addr);
	w = getTargetBB(cond_jump_addr);

	CFGEdge e1, e2;
	bool ins_edge;
	tie(e1, ins_edge) = add_edge(u, v, cfg);
	if(ins_edge)
	{
		// set properties for edge
		// the costs are set in addBBCostToCfg()
		put(costEProp, e1, 0);
		put(costOffChipEProp, e1, 0);
		put(costOnChipEProp, e1, 0);
		put(memPenaltyEProp, e1, 0);

		put(capacitylEProp, e1, 0);
		put(capacityhEProp, e1, 0);
		put(circEProp, e1, -1);
		ostringstream e1name;
		e1name << "("<< u << ","<< v << ")";
		put(edgeNameEProp, e1, e1name.str());
		put(edgeTypeEProp, e1, ForwardStep);

		LOG_DEBUG(logger, "created connecting edge from 0x" << hex << start_addr << " ( " << dec << u << " ) to 0x" << hex << next_addr << " ( " << dec << v << " ) is edge: " << dec << e1 << " type: forward step");
	}
	tie(e2, ins_edge) = add_edge(u, w, cfg);
	if(ins_edge)
	{
		// set properties for edge
		// the costs are set in addBBCostToCfg()
		put(costEProp, e2, 0);
		put(costOffChipEProp, e2, 0);
		put(costOnChipEProp, e2, 0);
		put(memPenaltyEProp, e2, 0);
		
		put(capacitylEProp, e2, 0);
		put(capacityhEProp, e2, 0);
		put(circEProp, e2, -1);
		ostringstream e2name;
		e2name << "("<< u << ","<< w << ")";
		put(edgeNameEProp, e2, e2name.str());
		string edge_type;
		if(end_addr < cond_jump_addr)
		{
			put(edgeTypeEProp, e2, ForwardJump);
			edge_type = "forward jump";
		}
		else
		{
			put(edgeTypeEProp, e2, BackwardJump);
			edge_type = "backward jump";
		}


		LOG_DEBUG(logger, "created connecting edge from 0x" << hex << start_addr << " ( " << dec << u << " ) to 0x" << hex << cond_jump_addr << " ( " << dec << w << " ) is edge: " << dec << e2 << " type: " << edge_type);
	}

}


void ControlFlowGraphObject::addBBNode(uint32_t start_addr, uint32_t end_addr, uint32_t next_addr, vector<uint32_t> cond_jump_addrs, string code, uint32_t size, uint32_t instr_count)
{
	CFGVertex u, v, w;
	u = setSourceBB(start_addr, end_addr, code, size, instr_count);
	v = getTargetBB(next_addr);

	CFGEdge e1, e2;
	bool ins_edge;
	tie(e1, ins_edge) = add_edge(u, v, cfg);
	if(ins_edge)
	{
		// set properties for edge
		// the costs are set in addBBCostToCfg()
		put(costEProp, e1, 0);
		put(costOffChipEProp, e1, 0);
		put(costOnChipEProp, e1, 0);
		put(memPenaltyEProp, e1, 0);

		put(capacitylEProp, e1, 0);
		put(capacityhEProp, e1, 0);
		put(circEProp, e1, -1);
		ostringstream e1name;
		e1name << "("<< u << ","<< v << ")";
		put(edgeNameEProp, e1, e1name.str());
		put(edgeTypeEProp, e1, ForwardStep);

		LOG_DEBUG(logger, "created connecting edge from 0x" << hex << start_addr << " ( " << dec << u << " ) to 0x" << hex << next_addr << " ( " << dec << v << " ) is edge: " << dec << e1 << " type: forward step");
	}

	for(uint32_t i=0; i < cond_jump_addrs.size(); i++)
	{
		uint32_t cond_jump_addr = cond_jump_addrs[i];
		w = getTargetBB(cond_jump_addr);

		tie(e2, ins_edge) = add_edge(u, w, cfg);
		if(ins_edge)
		{
			// set properties for edge
			// the costs are set in addBBCostToCfg()
			put(costEProp, e2, 0);
			put(costOffChipEProp, e2, 0);
			put(costOnChipEProp, e2, 0);
			put(memPenaltyEProp, e2, 0);

			put(capacitylEProp, e2, 0);
			put(capacityhEProp, e2, 0);
			put(circEProp, e2, -1);
			ostringstream e2name;
			e2name << "("<< u << ","<< w << ")";
			put(edgeNameEProp, e2, e2name.str());
			string edge_type;
			if(end_addr < cond_jump_addr)
			{
				put(edgeTypeEProp, e2, ForwardJump);
				edge_type = "forward jump";
			}
			else
			{
				put(edgeTypeEProp, e2, BackwardJump);
				edge_type = "backward jump";
			}


			LOG_DEBUG(logger, "created connecting edge from 0x" << hex << start_addr << " ( " << dec << u << " ) to 0x" << hex << cond_jump_addr << " ( " << dec << w << " ) is edge: " << dec << e2 << " type: " << edge_type);
		}
	}

}

void ControlFlowGraphObject::addBBCallNode(uint32_t start_addr, uint32_t end_addr, uint32_t next_addr, string code, uint32_t size, uint32_t instr_count, addr_label_t call_target)
{
	CFGVertex u, v, w;
	u = setSourceBB(start_addr, end_addr, code, size, instr_count);
	v = getTargetBB(next_addr);


	// creating callsite node
	w = add_vertex(cfg);
	put(nodeTypeNProp, w, CallSite);
	put(startAddrNProp, w, 0x00000000);
	string function_name = string("<CALLSITE: ");
	function_name += call_target.label;
	function_name += ">";
	put(startAddrStringNProp, w, function_name);
	// remember the basic block that called the function in the callpoint
	put(endAddrNProp, w, start_addr);
	put(callLabelNProp, w, call_target.address);
	put(bbCodeNProp, w, "");
	put(bbSizeNProp, w, 0);
	put(bbInstrsNProp, w, 0);
	addr_label_vec_t tmp;
	tmp.address = call_target.address;
	tmp.label = call_target.label;
	tmp.vertex = w;
	call_sites.push_back(tmp);


	CFGEdge e1, e2;
	bool ins_edge;
	tie(e1, ins_edge) = add_edge(u, w, cfg);
	if(ins_edge)
	{
		// set properties for edge
		// the costs are set in addBBCostToCfg()
		put(costEProp, e1, 0);
		put(costOffChipEProp, e1, 0);
		put(costOnChipEProp, e1, 0);
		put(memPenaltyEProp, e1, 0);
		
		put(capacitylEProp, e1, 0);
		put(capacityhEProp, e1, 0);
		put(circEProp, e1, -1);
		ostringstream e1name;
		e1name << "(" << u << ","<< w << ")";
		put(edgeNameEProp, e1, e1name.str());
		// FIXME: Why a call is always an forward jump? 
		put(edgeTypeEProp, e1, ForwardJump);

		LOG_DEBUG(logger, "created connecting edge from 0x" << hex << start_addr << " ( " << dec << u << " ) to CALLSITE ( " << dec << w << " ) is edge: " << dec << e1 << " type: forward jump");
	}
	tie(e2, ins_edge) = add_edge(w, v, cfg);
	if(ins_edge)
	{
		// set properties for edge
		put(costEProp, e2, 0);
		put(costOffChipEProp, e2, 0);
		put(costOnChipEProp, e2, 0);
		put(memPenaltyEProp, e2, 0);
		put(capacitylEProp, e2, 0);
		put(capacityhEProp, e2, 0);
		put(circEProp, e2, -1);
		ostringstream e2name;
		e2name << "(" << w << ","<< v << ")";
		put(edgeNameEProp, e2, e2name.str());
		put(edgeTypeEProp, e2, Meta);

		LOG_DEBUG(logger, "created connecting edge from CALLSIZE ( " << dec << w << " ) to 0x" << hex << next_addr << " ( " << dec << v << " ) is edge: " << dec << e2 << " type: meta");
	}


	addCallTarget(call_target);
}

void ControlFlowGraphObject::addBBCallNode(uint32_t start_addr, uint32_t end_addr, uint32_t next_addr, string code, uint32_t size, uint32_t instr_count, vector<addr_label_t> call_targets)
{
	CFGVertex u, v;
	u = setSourceBB(start_addr, end_addr, code, size, instr_count);
	v = getTargetBB(next_addr);


	for(uint32_t i = 0; i < call_targets.size(); i++)
	{
		// creating callsite node
		CFGVertex w = add_vertex(cfg);
		put(nodeTypeNProp, w, CallSite);
		put(startAddrNProp, w, 0x00000000);
		string function_name = string("<CALLSITE: ");
		function_name += call_targets[i].label;
		function_name += ">";
		put(startAddrStringNProp, w, function_name);
		// remember the basic block that called the function in the callpoint
		put(endAddrNProp, w, start_addr);
		put(callLabelNProp, w, call_targets[i].address);
		put(bbCodeNProp, w, "");
		put(bbSizeNProp, w, 0);
		put(bbInstrsNProp, w, 0);
		addr_label_vec_t tmp;
		tmp.address = call_targets[i].address;
		tmp.label = call_targets[i].label;
		tmp.vertex = w;
		call_sites.push_back(tmp);


		CFGEdge e1, e2;
		bool ins_edge;
		tie(e1, ins_edge) = add_edge(u, w, cfg);
		if(ins_edge)
		{
			// set properties for edge
			// the costs are set in addBBCostToCfg()
			put(costEProp, e1, 0);
			put(costOffChipEProp, e1, 0);
			put(costOnChipEProp, e1, 0);
			put(memPenaltyEProp, e1, 0);

			put(capacitylEProp, e1, 0);
			put(capacityhEProp, e1, 0);
			put(circEProp, e1, -1);
			ostringstream e1name;
			e1name << "(" << u << ","<< w << ")";
			put(edgeNameEProp, e1, e1name.str());
			// FIXME: Why a call is always an forward jump? 
			put(edgeTypeEProp, e1, ForwardJump);

			LOG_DEBUG(logger, "created connecting edge from 0x" << hex << start_addr << " ( " << dec << u << " ) to CALLSITE ( " << dec << w << " ) is edge: " << dec << e1 << " type: forward jump");
		}
		tie(e2, ins_edge) = add_edge(w, v, cfg);
		if(ins_edge)
		{
			// set properties for edge
			put(costEProp, e2, 0);
			put(costOffChipEProp, e2, 0);
			put(costOnChipEProp, e2, 0);
			put(memPenaltyEProp, e2, 0);
			put(capacitylEProp, e2, 0);
			put(capacityhEProp, e2, 0);
			put(circEProp, e2, -1);
			ostringstream e2name;
			e2name << "(" << w << ","<< v << ")";
			put(edgeNameEProp, e2, e2name.str());
			put(edgeTypeEProp, e2, Meta);

			LOG_DEBUG(logger, "created connecting edge from CALLSIZE ( " << dec << w << " ) to 0x" << hex << next_addr << " ( " << dec << v << " ) is edge: " << dec << e2 << " type: meta");
		}

		addCallTarget(call_targets[i]);
	}
}


void ControlFlowGraphObject::addBBNode(uint32_t start_addr, uint32_t end_addr, string code, uint32_t size, uint32_t instr_count)
{
	CFGVertex u;
	u = setSourceBB(start_addr, end_addr, code, size, instr_count);
	
	// since this function is called for basic block that are completed with return instructions only, connect this node to the exit node
	CFGEdge e1;
	bool ins_edge;
	tie(e1, ins_edge) = add_edge(u, exit, cfg);
	if(ins_edge)
	{
		// set properties for edge
		// the costs are set in addBBCostToCfg()
		put(costEProp, e1, 0);
		put(costOffChipEProp, e1, 0);
		put(costOnChipEProp, e1, 0);
		put(memPenaltyEProp, e1, 0);
		
		put(capacitylEProp, e1, 0);
		put(capacityhEProp, e1, 0);
		put(circEProp, e1, -1);
		ostringstream e1name;
		e1name << "(" << u << ","<< exit << ")";
		put(edgeNameEProp, e1, e1name.str());
		put(edgeTypeEProp, e1, Meta);

		LOG_DEBUG(logger, "created connecting edge from 0x" << hex << start_addr << " ( " << dec << u << " ) to EXIT node" << " ( " << dec << exit << " ) is edge: " << dec << e1 << " type: meta");
	}
}

CFGVertex ControlFlowGraphObject::setSourceBB(uint32_t start_addr, uint32_t end_addr, string code, uint32_t size, uint32_t instr_count)
{
	CFGVertex u;
	bool inserted;
	AddrVertexMap::iterator pos;
	tie(pos, inserted) = avMap.insert(make_pair(start_addr, CFGVertex()));
	if(inserted)
	{
		// this bb is new
		// create vertex
		u = add_vertex(cfg);
		// store vertex in map
		pos->second = u;

		LOG_DEBUG(logger, "creating node for: 0x" << hex << start_addr << " is vertex: " << dec << u);
	}
	else
	{
		// there was already an entry
		// identify corresponding vertex
		u = pos->second;

		LOG_DEBUG(logger, "node already created for: 0x" << hex << start_addr << " is vertex: " << dec << u);

		eraseBBFromIncompleteList(start_addr);
	}

	if((in_degree(u,cfg) == 0) && (!entry_connected))
	{
		// this is the first node of the cfg, connect it to the entry edge
		CFGEdge e1;
		bool ins_edge;
		tie(e1, ins_edge) = add_edge(entry, u, cfg);
		if(ins_edge)
		{
			// set properties for edge
			put(costEProp, e1, 0);
			put(costOffChipEProp, e1, 0);
			put(costOnChipEProp, e1, 0);
			put(memPenaltyEProp, e1, 0);
			put(capacitylEProp, e1, 0);
			put(capacityhEProp, e1, 0);
			put(circEProp, e1, -1);
			ostringstream e1name;
			e1name << "(" << entry << ","<< u << ")";
			put(edgeNameEProp, e1, e1name.str());
			put(edgeTypeEProp, e1, Meta);

			LOG_DEBUG(logger, "created connecting edge from ENTRY  ( " << dec << entry << " ) to 0x" << hex << start_addr << " ( " << dec << u << " ) is edge: " << dec << e1 << " type: meta");
			// set the entry node as connected, because only one ENTRY point is possible
			// if there are more than 1 unconnected blocks the first one parsed (with lower address) is assumed to be the real entry
			// the other node may be dead code
			entry_connected = true;
		}
		// also set the address of the function to the entry node: (to determine the function address by its unique entry point)
		put(callLabelNProp, entry, start_addr); 

	}


	// set properties for vertex
	put(nodeTypeNProp, u, BasicBlock);
	put(startAddrNProp, u, start_addr);
	char tmp[32];
	sprintf(tmp, "0x%x",start_addr);
	put(startAddrStringNProp, u, string(tmp));
	put(endAddrNProp, u, end_addr);
	put(callLabelNProp, u, -1);
	put(bbCodeNProp, u, code);
	put(bbSizeNProp, u, size);
	put(bbInstrsNProp, u, instr_count);
	
	code_size += size;

	return u;
}

CFGVertex ControlFlowGraphObject::getTargetBB(uint32_t target_addr)
{
	CFGVertex v;
	AddrVertexMap::iterator pos;

	// check if target edge is already created
	pos = avMap.find(target_addr);

	if(pos == avMap.end())
	{
		AddrVertexMap::iterator ins_pos;
		bool ins_bool;
		// bb is not found, create it
		tie(ins_pos, ins_bool) = avMap.insert(make_pair(target_addr, CFGVertex()));
		assert(ins_bool); // has to be inserted, because it wasn't found
		v = add_vertex(cfg);
		if(target_addr == UNKNOWN_ADDR)
		{
			put(nodeTypeNProp, v, UnknownJumpTarget);
		}
		ins_pos->second = v;

		LOG_DEBUG(logger, " target node: 0x" << hex << target_addr <<  " not found. Created it. is vertex: " << dec << v);

		// adding to list of incomplete nodes
		pushTargetBBToIncompleteList(target_addr);
	}
	else
	{
		// get bb identifier for start addr;
		v = pos->second;
		LOG_DEBUG(logger, " target node: 0x" << hex << target_addr << " found. is vertex: " << dec << v);
	}

	return v;

}

void ControlFlowGraphObject::pushTargetBBToIncompleteList(uint32_t addr)
{
	bool found_entry = false;
	if(addr == UNKNOWN_ADDR)
	{
		return;
	}

	for(uint32_t i=0; i < incomplete_nodes.size(); i++)
	{
		if(incomplete_nodes[i] == addr)
		{
			// found incomplete entry
			found_entry = true;
			break;
		}
	}
	if(!found_entry)
	{
		incomplete_nodes.push_back(addr);
		LOG_DEBUG(logger, "added BB 0x" << hex << addr << " to incomplete list.");
	}
	else
	{
		LOG_DEBUG(logger, "BB 0x" << hex << addr << " already in incomplete list.");
	}
}

void ControlFlowGraphObject::eraseBBFromIncompleteList(uint32_t addr)
{
	bool found_entry = false;
	uint32_t entry = 0;
	for(uint32_t i=0; i < incomplete_nodes.size(); i++)
	{
		if(incomplete_nodes[i] == addr)
		{
			// found incomplete entry
			found_entry = true;
			entry = i;
			break;
		}
	}
	if(found_entry)
	{
		incomplete_nodes.erase(incomplete_nodes.begin() + entry);
		LOG_DEBUG(logger, "erased BB 0x" << hex << addr << " from incomplete list.");
	}
	else
	{
		LOG_WARN(logger, "The BB for 0x" << hex << addr << " is not in the incomplete list!");
	}
}

bool ControlFlowGraphObject::isFinished(void)
{
	// all nodes that were added have to be connected
	// also the exit node has to be connected (the return instruction has to be found) (This condition is only needed if a unconditional self loop is in the code, then the parsing is aborted after detection of this loop. Wheras there will be more code inserted by the compiler after this jump. So everything until the return is found is also added to the cfg.)
	if((incomplete_nodes.size() == 0) && (in_degree(exit,cfg) != 0))
	{
		// Notice the cost of the pipeline is added separately to the basic blocks via ControlFlowGraphCostCalculator::calculateCost()
//		addBBCostToCfg();
		return true;
	}
	else
	{
		if(logger->isDebugEnabled())
		{
			ostringstream debug;
			debug << "incomplete list contains: ";

			for(uint32_t i=0; i < incomplete_nodes.size(); i++)
				debug << hex << "0x" <<  incomplete_nodes[i] << " ";

			LOG_DEBUG(logger, debug.str() << " in_degree: exit node: " << in_degree(exit,cfg)); 
		}
		return false;
	}
}

void ControlFlowGraphObject::addCallTarget(addr_label_t target)
{
	LOG_DEBUG(logger, "Adding label " << target.label << " at 0x" << hex << target.address << " to call target list of function.");
	for(uint i = 0; i < call_targets.size(); i++)
	{
		if(target.address == call_targets[i].address)
		{
			if(target.address == UNKNOWN_ADDR)
			{
				// unknown address: can occure multiple times but one entry is enough
			}
			else
			{
				LOG_DEBUG(logger, "Label " << target.label << " already in call list.");
				assert(target.label.compare(call_targets[i].label) == 0);
			}
			// update number of call sites
			CallCountMap::iterator pos;
			pos = ccMap.find(target.address);
			assert(pos != ccMap.end());
			pos->second = (pos->second+1);
			LOG_DEBUG(logger, "Updating call count  of " << target.label << " to: " <<  (pos->second+1));
			return;
		}
	}
	call_targets.push_back(target);
	ccMap.insert(make_pair(target.address, 1));
}

vector<addr_label_t> ControlFlowGraphObject::getCallTargets(void)
{
	return call_targets;
}

uint32_t ControlFlowGraphObject::getCallSiteCount(uint32_t funct_address)
{
	CallCountMap::iterator pos;
	pos = ccMap.find(funct_address);
	assert(pos != ccMap.end());
	return pos->second;
}

uint32_t ControlFlowGraphObject::getCodeSize(void)
{
	return code_size;
}

vector<addr_label_vec_t> ControlFlowGraphObject::getCallSites(void)
{
	return call_sites;
}

//uint32_t ControlFlowGraphObject::getBBSizeFromAddr(uint32_t address)
//{
//	AddrVertexMap::iterator pos;
//	uint32_t bb_size=0;
//
//	pos = avMap.find(address);
//
//	if(pos != avMap.end())
//	{
//		bb_size = get(bbSizeNProp, pos->second);
//	}
//	else
//	{
//		LOG_WARN(logger, "BB for address 0x" << address << " not found.")
//	}
//	return bb_size;
//}

//void ControlFlowGraphObject::setRatioFileReaders(RatioFileReader *rfr_onchip, RatioFileReader *rfr_offchip)
//{
//	bb_cost_container_onchip = rfr_onchip;
//	bb_cost_container_offchip = rfr_offchip;
//}
//
//uint32_t ControlFlowGraphObject::getInstrCountFromAddr(uint32_t address)
//{
//	AddrVertexMap::iterator pos;
//	uint32_t bb_instr_count=0;
//
//	pos = avMap.find(address);
//
//	if(pos != avMap.end())
//	{
//		bb_instr_count = get(bbInstrsNProp, pos->second);
//	}
//	else
//	{
//		LOG_WARN(logger, "BB for address 0x" << address << " not found.")
//	}
////	LOG_DEBUG(logger, "number of instructions fopr BB 0x" << hex << address << " is: " << dec << bb_instr_count);
//
//	return bb_instr_count;
//}


//void ControlFlowGraphObject::addBBCostToCfg(void)
//{
//
////	assuming an edge u -> v the cost for executing u is assigned to that edge!
//
//	cfgVertexIter vp;
//	for (vp = vertices(cfg); vp.first != vp.second; ++vp.first)
//	{
//		CFGVertex v = *vp.first;
//
//		if(get(nodeTypeNProp, v) == BasicBlock)
//		{
//			uint32_t cost_onchip = 0;
//			uint32_t cost_offchip = 0;
//
//			switch((analysis_metric_t) conf->getUint(CONF_USE_METRIC))
//			{
//				case WCET_RATIO_FILES:
//					{
//						// get cost from ratio file (create otawa ratio file reader)
//						cost_offchip = bb_cost_container_offchip->getBBCost(get(startAddrNProp, v));
//						if(cost_offchip == 0)
//						{
//							LOG_INFO(logger, "Cannot obtain offchip cost for block address 0x" << hex << get(startAddrNProp, v));
//						}
//
//						cost_onchip = bb_cost_container_onchip->getBBCost(get(startAddrNProp, v));
//						if(cost_onchip == 0)
//						{
//							LOG_INFO(logger, "Cannot obtain onchip cost for block address 0x" << hex << get(startAddrNProp, v));
//						}
//
//						break;
//
//					}
//				case WCET:
//					{
//						if(conf->getBool(CONF_USE_ARCH_CFG_FILE))
//						{
//							cfgInEdgeIter epi;
//							bool bb_entered_by_forward_step_only = true;
//							for(epi = in_edges(v, cfg); epi.first != epi.second; ++epi.first) 
//							{
//								CFGEdge e = *epi.first;
//								if(get(edgeTypeEProp, e) != ForwardStep)
//								{
//									bb_entered_by_forward_step_only = false;
//								}
//							}
//
//							LOG_DEBUG(logger, "Calculating cycle cost for bb: " << get(startAddrStringNProp, v) << " with id: " << v);
//
//							// get cost for basic block, if the code is in the onchip mem:
//							cost_onchip = carcore_timing_onchip->getCycleCountForInstructions(get(bbCodeNProp, v), bb_entered_by_forward_step_only, get(startAddrNProp, v), get(endAddrNProp,v));
//
//							// get cost for basic block, if the code is in the offchip mem:
//							cost_offchip = carcore_timing_offchip->getCycleCountForInstructions(get(bbCodeNProp, v), bb_entered_by_forward_step_only, get(startAddrNProp, v), get(endAddrNProp,v));
//						}
//						else
//						{
//							// cannot use timing model, it is not activated by user.
//							cost_offchip = -1;
//							cost_onchip = -1;
//							assert(false);
//						}
//						break;
//					}
//				case MDIC:
//					{
//						cost_offchip = getInstrCountFromAddr(get(startAddrNProp, v));
//						cost_onchip = cost_offchip;
//						if(cost_offchip == 0)
//						{
//							LOG_INFO(logger, "Cannot obtain instruction count for block address 0x" << hex << get(startAddrNProp, v));
//						}
//						break;
//					}
//				case MPL:
//					{
//						cost_offchip = getBBSizeFromAddr(get(startAddrNProp, v));
//						cost_onchip = cost_offchip;
//						break;
//					}
//				default:
//					{
//						cost_offchip = getBBSizeFromAddr(get(startAddrNProp, v));
//						cost_onchip = cost_offchip;
//						LOG_ERROR(logger, "Illegal analysis metric.");
//						assert(false);
//					}
//			}
//
//
//			cfgOutEdgeIter epo;
//			for(epo = out_edges(v, cfg); epo.first != epo.second; ++epo.first) 
//			{
//				CFGEdge e = *epo.first;
//
//				// compare values to the ones in the ratio file:
//				LOG_DEBUG(logger, "Cost for bb: 0x" << hex << get(startAddrNProp, v) << " - 0x" << get(endAddrNProp, v) << dec << " Onchip: " << cost_onchip << "\tOffchip: " <<  cost_offchip << "\tMemPenalty: " << cost_offchip-cost_onchip);
//
//				// set the costs:
//				put(costOnChipEProp, e, cost_onchip);
//				put(costOffChipEProp, e, cost_offchip);
//				if((memory_type == NO_MEM) || (memory_type == VIVU_TEST))
//				{
//					// the cost of the block is the cost for off-chip memory without any further memory penalty
//					put(costEProp, e, cost_offchip);
//					put(memPenaltyEProp, e, 0);
//				}
//				else if(IS_STATIC_MEM(memory_type))
//				{
//					// the cost of the block is the cost for on-chip memory and the difference between off- and on-chip memory as memory penalty
//					put(costEProp, e, cost_onchip);
//					put(memPenaltyEProp, e, cost_offchip-cost_onchip);
//				}
//				else
//				{
//					// the cost of the block is the cost for on-chip memory. The memory penalty is set to 0 because it is calculated via DFA later.
//					put(costEProp, e, cost_onchip);
//					put(memPenaltyEProp, e, 0);
//				}
//			}
//			
//		}
//	}
//
//}
