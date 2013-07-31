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
#include "scfgobj.hpp"

LoggerPtr SuperControlFlowGraphObject::logger(Logger::getLogger("SuperControlFlowGraphObject"));

SuperControlFlowGraphObject::SuperControlFlowGraphObject()
{
	finished = false;

	// get node property structures
	nodeTypeNProp = get(nodetype_t(), scfg);
	startAddrNProp = get(startaddr_t(), scfg);
	startAddrStringNProp = get(startaddrstring_t(), scfg);
	endAddrNProp = get(endaddr_t(), scfg);
	callLabelNProp = get(calllabel_t(), scfg);
	bbCodeNProp = get(bbcode_t(), scfg);
	bbSizeNProp = get(bbsize_t(), scfg);
	bbInstrsNProp = get(bbinstrs_t(), scfg);


	// get edge property structures
	costEProp = get(cost_t(), scfg);
	costOnChipEProp = get(cost_onchip_t(), scfg);
	costOffChipEProp = get(cost_offchip_t(), scfg);
	memPenaltyEProp = get(mem_penalty_t(), scfg);
	capacitylEProp = get(capacityl_t(), scfg);
	capacityhEProp = get(capacityh_t(), scfg);
	circEProp = get(circ_t(), scfg);
	actEProp = get(activation_t(), scfg);
	edgeNameEProp = get(edgename_t(), scfg);
	edgeTypeEProp = get(edgetype_t(), scfg);

}

SuperControlFlowGraphObject::~SuperControlFlowGraphObject()
{
	inserted_functions.clear();
	call_sites.clear();

}

void SuperControlFlowGraphObject::createSuperGraph(FunctionCallGraphObject *fcgo)
{
	ControlFlowGraphObject *cfgo;

	cfgo = fcgo->getCFGOForStartFunction();
	assert(cfgo != NULL);
	inserted_functions.push_back(fcgo->getStartLabel());
	scfg = cfgo->getCFG();

	// find the entry and exit point of the super graph
	cfgVertexIter vp;
	for (vp = vertices(scfg); vp.first != vp.second; ++vp.first)
	{
		CFGVertex v = *vp.first;
		if(get(nodeTypeNProp, v) == Entry)
		{
			entry = v;
			put(startAddrStringNProp, entry, string("<SUPER_ENTRY>"));
		}
		if(get(nodeTypeNProp, v) == Exit)
		{
			exit = v;
			put(startAddrStringNProp, exit, string("<SUPER_EXIT>"));
		}
	}

	// copying the call site nodes from start function (this is working, since the ControlFlowGraph is copied too, so the Vertices of both graphs correspond)
	call_sites = cfgo->getCallSites();


	vector<uint32_t> func_addrs = fcgo->getFunctions();

	for(uint i = 0; i < func_addrs.size(); i++)
	{
		LOG_DEBUG(logger, "Extracting CFG from " << fcgo->getFunctionName(func_addrs[i]));
		cfgo = fcgo->getCFGOForFunction(func_addrs[i]);

		addr_label_t tmp;
		tmp.address = func_addrs[i];
		tmp.label = fcgo->getFunctionName(func_addrs[i]);
		insertCallSite(tmp, cfgo->getCFG());
	}
	finished = true;
	if(call_sites.size() != 0)
	{
		LOG_WARN(logger, "Still " << call_sites.size() << " call sites left.");
		for(uint32_t i = 0; i < call_sites.size(); i++)
			LOG_WARN(logger, call_sites[i].label << " at 0x" << hex << call_sites[i].address);
		LOG_ERROR(logger, "Notice that, recursion cannot be handled correctly.");
	}
	assert(call_sites.size()==0);
}

void SuperControlFlowGraphObject::insertCallSite(addr_label_t func, ControlFlowGraph cfg)
{
	// insert complete cfg into scfg (if not already inserted)
	bool already_inserted = false;
	for(uint i = 0; i < inserted_functions.size(); i++)
	{
		if(inserted_functions[i].address == func.address)
		{
			already_inserted = true;
			break;
		}
	}

	if(!already_inserted)
	{
		CFGVertex entry_node, exit_node;
		tie(entry_node, exit_node) = joinCfg(cfg);
		call_entry_map.insert(make_pair(func.address, entry_node));
		call_exit_map.insert(make_pair(func.address, exit_node));

		string function_name = string("<ENTRY: ");
		function_name += func.label;
		function_name += ">";
		put(startAddrStringNProp, entry_node, function_name);
		function_name = string("<EXIT: ");
		function_name += func.label;
		function_name += ">";
		put(startAddrStringNProp, exit_node, function_name);

		inserted_functions.push_back(func);
	}


	vector<uint> call_sites_to_delete;
	for(uint i = 0; i < inserted_functions.size(); i++)
	{
		for(uint j = 0; j < call_sites.size(); j++)
		{
			if(inserted_functions[i].address == call_sites[j].address)
			{
				LOG_DEBUG(logger, "Splitting call site: " << call_sites[j].vertex << " for call to: 0x" << hex << call_sites[j].address);
				splitCallSite(call_sites[j].vertex);
				call_sites_to_delete.push_back(j);
			}
		}
		// kick out replaced call sites
		while(call_sites_to_delete.size())
		{
			LOG_DEBUG(logger, "Deleting splitted call site: " << call_sites[call_sites_to_delete.back()].vertex << " for call to: 0x" << hex << call_sites[call_sites_to_delete.back()].address);
			call_sites.erase(call_sites.begin()+call_sites_to_delete.back());
			call_sites_to_delete.erase(call_sites_to_delete.end()-1);
		}
	}

}

EntryExitNodePair SuperControlFlowGraphObject::joinCfg(ControlFlowGraph cfg)
{
	CFGVertex cfg_entry = CFGVertex();
	CFGVertex cfg_exit = CFGVertex();
	bool exit_found = false;

	property_map<ControlFlowGraph, nodetype_t>::type nodeTypeNProp_ins = get(nodetype_t(), cfg);
	property_map<ControlFlowGraph, startaddr_t>::type startAddrNProp_ins = get(startaddr_t(), cfg);
	property_map<ControlFlowGraph, startaddrstring_t>::type startAddrStringNProp_ins = get(startaddrstring_t(), cfg);
	property_map<ControlFlowGraph, endaddr_t>::type endAddrNProp_ins = get(endaddr_t(), cfg);
	property_map<ControlFlowGraph, calllabel_t>::type callLabelNProp_ins = get(calllabel_t(), cfg);
	property_map<ControlFlowGraph, bbcode_t>::type bbCodeNProp_ins = get(bbcode_t(), cfg);
	property_map<ControlFlowGraph, bbsize_t>::type bbSizeNProp_ins = get(bbsize_t(), cfg);
	property_map<ControlFlowGraph, bbinstrs_t>::type bbInstrsNProp_ins = get(bbinstrs_t(), cfg);

	property_map<ControlFlowGraph, edgetype_t>::type edgeTypeEProp_ins = get(edgetype_t(), cfg);
	property_map<ControlFlowGraph, cost_t>::type costEProp_ins = get(cost_t(), cfg);
	property_map<ControlFlowGraph, cost_onchip_t>::type costOnChipEProp_ins = get(cost_onchip_t(), cfg);
	property_map<ControlFlowGraph, cost_offchip_t>::type costOffChipEProp_ins = get(cost_offchip_t(), cfg);
	property_map<ControlFlowGraph, mem_penalty_t>::type memPenaltyEProp_ins = get(mem_penalty_t(), cfg);
	property_map<ControlFlowGraph, capacityl_t>::type capacitylEProp_ins = get(capacityl_t(), cfg);
	property_map<ControlFlowGraph, capacityh_t>::type capacityhEProp_ins = get(capacityh_t(), cfg);
	property_map<ControlFlowGraph, circ_t>::type circEProp_ins = get(circ_t(), cfg);


	cfgVertexIter vp;
    for (vp = vertices(cfg); vp.first != vp.second; ++vp.first)
	{
		LOG_DEBUG(logger, "Processing bb 0x" << hex << startAddrNProp[*vp.first] << " with call label 0x" << callLabelNProp[*vp.first]);
		CFGVertex src = *vp.first;
		CFGVertex u;

		// only BasicBlocks or Entry nodes can have output edges
		// Exit and CallSite nodes are created completely, when they are found as target nodes of an BasicBlock or Entry node out egde
		if((get(nodeTypeNProp_ins, src) == BasicBlock)|| (get(nodeTypeNProp_ins, src) == Entry))
		{
			u=setSource(get(nodeTypeNProp_ins, src), get(startAddrNProp_ins, src), get(startAddrStringNProp_ins, src), get(endAddrNProp_ins, src), get(bbCodeNProp_ins, src), get(bbSizeNProp_ins, src), get(bbInstrsNProp_ins, src), get(callLabelNProp_ins, src));
			if(get(nodeTypeNProp_ins, src) == Entry)
			{
				cfg_entry = u;

			}


			cfgOutEdgeIter ep;
			for(ep = out_edges(src, cfg); ep.first != ep.second; ++ep.first) 
			{
				CFGEdge e = *ep.first;
				CFGVertex targ = target(e, cfg);
				CFGVertex v;

				switch(nodeTypeNProp_ins[targ])
				{
					case BasicBlock:
						{
							v = getTarget(startAddrNProp_ins[targ]);
							break;
						}
					case CallSite:
						{
							// creating call site node
							v = setSource(get(nodeTypeNProp_ins, targ), get(startAddrNProp_ins, targ), get(startAddrStringNProp_ins, targ), get(endAddrNProp_ins, targ), get(bbCodeNProp_ins, targ), get(bbSizeNProp_ins, targ), get(bbInstrsNProp_ins, targ), get(callLabelNProp_ins, targ));


							// and connecting the one out edge to the next most basic block
							cfgOutEdgeIter cep = out_edges(targ, cfg);

							// use the first edge (since there is only possible this is the right one)
							assert(out_degree(targ, cfg)==1);

							CFGEdge cse = *cep.first;
							CFGVertex w = getTarget(startAddrNProp_ins[target(cse,cfg)]);

							CFGEdge vw;
							bool ins_edge;

							tie(vw, ins_edge) = add_edge(v, w, scfg);
							if(ins_edge)
							{
								put(costEProp, vw, get(costEProp_ins, cse));
								put(costOnChipEProp, vw, get(costOnChipEProp_ins, cse));
								put(costOffChipEProp, vw, get(costOffChipEProp_ins, cse));
								put(memPenaltyEProp, vw, get(memPenaltyEProp_ins, cse));
								put(capacitylEProp, vw, get(capacitylEProp_ins, cse));
								put(capacityhEProp, vw, get(capacityhEProp_ins, cse));
								put(circEProp, vw, get(circEProp_ins, cse));
								ostringstream vwname;
								vwname << "(" << v << ","<< w << ")";
								put(edgeNameEProp, vw, vwname.str());
								put(edgeTypeEProp, vw, Meta);
							}
							break;
						}
					case Entry:
						{
							assert(false);
//							v = setSource(get(nodeTypeNProp_ins, targ), get(startAddrNProp_ins, targ), get(startAddrStringNProp_ins, targ), get(endAddrNProp_ins, targ), get(bbCodeNProp_ins, targ), get(bbSizeNProp_ins, targ), get(callLabelNProp_ins, targ));
//							cfg_entry = v;
							break;
						}
					case Exit:
						{
							if(!exit_found)
							{
								v = setSource(get(nodeTypeNProp_ins, targ), get(startAddrNProp_ins, targ), get(startAddrStringNProp_ins, targ), get(endAddrNProp_ins, targ), get(bbCodeNProp_ins, targ), get(bbSizeNProp_ins, targ), get(bbInstrsNProp_ins, targ), get(callLabelNProp_ins, targ));
								LOG_DEBUG(logger, "created exit node (" << v << ") for edge from node: " << u);
								exit_found = true;
								cfg_exit = v;
							}
							else
							{
								v = cfg_exit;
								LOG_DEBUG(logger, "another edge to the exit node (" << v << ") found. Source node is: " << u);
							}
							break;
						}
					case UnknownJumpTarget:
						{
							LOG_ERROR(logger, "Unknown jump target");
							assert(false);
							break;
						}
					default:
						{
							LOG_ERROR(logger, "Found node type: " << nodeTypeNProp_ins[targ]);
							assert(false);
						}
				}

				CFGEdge uv;
				bool ins_edge;

				tie(uv, ins_edge) = add_edge(u, v, scfg);
				if(ins_edge)
				{
					put(costEProp, uv, get(costEProp_ins, e));
					put(costOnChipEProp, uv, get(costOnChipEProp_ins, e));
					put(costOffChipEProp, uv, get(costOffChipEProp_ins, e));
					put(memPenaltyEProp, uv, get(memPenaltyEProp_ins, e));
					put(capacitylEProp, uv, get(capacitylEProp_ins, e));
					put(capacityhEProp, uv, get(capacityhEProp_ins, e));
					put(circEProp, uv, get(circEProp_ins, e));
					ostringstream uvname;
					uvname << "(" << u << ","<< v << ")";
					put(edgeNameEProp, uv, uvname.str());
					put(edgeTypeEProp, uv, get(edgeTypeEProp_ins, e));
				}
			}
		}
	}
	return make_pair(cfg_entry, cfg_exit);
}

CFGVertex SuperControlFlowGraphObject::setSource(node_type_t type, uint32_t start_addr, string start_addr_s, uint32_t end_addr, string code, uint32_t size, uint32_t instr_count, uint32_t call_addr)
{
	CFGVertex u = CFGVertex(); // creating unused vertex to avoid compiler warning

	if(type == BasicBlock)
	{
		bool inserted;
		AddrVertexMap::iterator pos;
		tie(pos, inserted) = avMap.insert(make_pair(start_addr, CFGVertex()));
		if(inserted)
		{
			// this bb is new
			// create vertex
			u = add_vertex(scfg);
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
		}
	}
	else
	{
		u = add_vertex(scfg);
		LOG_DEBUG(logger, "creating special node ("<< type <<") for: 0x" << hex << start_addr << " is vertex: " << dec << u);
	}

	// set properties for vertex
	put(nodeTypeNProp, u, type);
	put(startAddrNProp, u, start_addr);
	put(startAddrStringNProp, u, start_addr_s);
	put(endAddrNProp, u, end_addr);
	put(callLabelNProp, u, call_addr);
	put(bbCodeNProp, u, code);
	put(bbSizeNProp, u, size);
	put(bbInstrsNProp, u, instr_count);
	
	if(type == CallSite)
	{
		if(call_addr != UNKNOWN_ADDR)
		{
			LOG_DEBUG(logger, "creating call site node for 0x" << hex << call_addr);
			addr_label_vec_t tmp;
			tmp.address = call_addr;
			tmp.vertex = u;
			LOG_DEBUG(logger, "Adding call site: " << u << " for call to: 0x" << hex << call_addr);
			call_sites.push_back(tmp);
		}
		else
		{
			LOG_DEBUG(logger, "Found call to 0x0 - possibly indirect call. Omitting callsite");
		}
	}

	return u;
}

CFGVertex SuperControlFlowGraphObject::getTarget(uint32_t target_addr)
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
		v = add_vertex(scfg);
		ins_pos->second = v;

		LOG_DEBUG(logger, "target node: 0x" << hex << target_addr <<  " not found. Created it. is vertex: " << dec << v);
	}
	else
	{
		// get bb identifier for start addr;
		v = pos->second;
		LOG_DEBUG(logger, "target node: 0x" << hex << target_addr << " found. is vertex: " << dec << v);
	}


	return v;
}

void SuperControlFlowGraphObject::splitCallSite(CFGVertex split)
{
	AddrVertexMap::iterator pos;
	bool ins_edge;

	if(get(nodeTypeNProp, split) == CallSite)
	{
		// get call target address
		uint32_t call_addr = get(callLabelNProp, split);

		// rename split to callPoint
		put(nodeTypeNProp, split, CallPoint);
		ostringstream call_point_s;
		call_point_s << "Call Point (0x"<<hex << get(endAddrNProp,split) << ")" ;
		put(startAddrStringNProp, split, call_point_s.str());
		// NOTICE: Altered callLabelNProp property to pass-through called function for ilp generation (needed if multiple call targets are given for an indirect call and the different functions need to be distinguished in ILPGenerator::extractFlowConservationForFunctionCalls()).
		// TODO: Check if this change of the property use does affect any other feature. Also check if this property is correctly passed-through all possible graph types during WCET generation (e.g. VIVU)
		// old code: put(callLabelNProp, split, -1);
		put(callLabelNProp, split, call_addr);

		
		// create returnPoint
		CFGVertex return_point = add_vertex(scfg);
		put(nodeTypeNProp, return_point, ReturnPoint);
		put(startAddrNProp, return_point, 0xffffffff);
		ostringstream return_point_s;
		return_point_s << "Return Point (0x" << hex << get(endAddrNProp, split) << ")";
		put(startAddrStringNProp, return_point, return_point_s.str());
		put(endAddrNProp, return_point, get(endAddrNProp, split)); // remember the basic block address that called the function
// NOTICE: Altered callLabelNProp property to pass-through called function for ilp generation (needed if multiple call targets are given for an indirect call and the different functions need to be distinguished in ILPGenerator::extractFlowConservationForFunctionCalls()).
		// TODO: Check if this change of the property use does affect any other feature. Also check if this property is correctly passed-through all possible graph types during WCET generation (e.g. VIVU)
		// old code: put(callLabelNProp, split, -1);
		put(callLabelNProp, return_point, call_addr);
		put(bbCodeNProp, return_point, "");
		put(bbSizeNProp, return_point, 0);
		put(bbInstrsNProp, return_point, 0);

		// delete all outgoing edges from callPoint and create similar in returnPoint
		cfgOutEdgeIter ep;
		for(ep = out_edges(split, scfg); ep.first != ep.second; ++ep.first) 
		{
			CFGEdge e = *ep.first;
			CFGVertex t = target(e, scfg);
			
			CFGEdge et;

			tie(et, ins_edge) = add_edge(return_point, t, scfg);
			assert(ins_edge);
			if(ins_edge)
			{
				put(costEProp, et, 0);
				put(costOnChipEProp, et, 0);
				put(costOffChipEProp, et, 0);
				put(memPenaltyEProp, et, 0);
				put(capacitylEProp, et, 0);
				put(capacityhEProp, et, 0);
				put(circEProp, et, -1);
				ostringstream etname;
				etname << "(" << return_point << ","<< t << ")";
				put(edgeNameEProp, et, etname.str());
				put(edgeTypeEProp, et, Meta);
			}
			remove_edge(e,scfg);
		}


		// create edge from callPoint to entry node of called function
		pos = call_entry_map.find(call_addr);
		assert(pos != call_entry_map.end());
		CFGVertex entry_node = pos->second;
		CFGEdge ec;
		tie(ec, ins_edge) = add_edge(split, entry_node, scfg);
		assert(ins_edge);
		if(ins_edge)
		{
			put(costEProp, ec, 0);
			put(costOnChipEProp, ec, 0);
			put(costOffChipEProp, ec, 0);
			put(memPenaltyEProp, ec, 0);
			put(capacitylEProp, ec, 0);
			put(capacityhEProp, ec, 0);
			put(circEProp, ec, -1);
			ostringstream ecname;
			ecname << "(" << split << ","<< entry_node << ")";
			put(edgeNameEProp, ec, ecname.str());
			put(edgeTypeEProp, ec, Meta);
		}

		// create edge from exit node of called function to returnpoint
		pos = call_exit_map.find(call_addr);
		assert(pos != call_exit_map.end());
		CFGVertex exit_node = pos->second;
		CFGEdge er;
		tie(er, ins_edge) = add_edge(exit_node, return_point, scfg);
		if(ins_edge)
		assert(ins_edge);
		{
			put(costEProp, er, 0);
			put(costOnChipEProp, er, 0);
			put(costOffChipEProp, er, 0);
			put(memPenaltyEProp, er, 0);
			put(capacitylEProp, er, 0);
			put(capacityhEProp, er, 0);
			put(circEProp, er, -1);
			ostringstream ername;
			ername << "(" << exit_node << ","<< return_point << ")";
			put(edgeNameEProp, er, ername.str());
			put(edgeTypeEProp, er, Meta);
		}
	}
	else
	{
		LOG_ERROR(logger, "node is no call site node: " << split << " type: " << get(nodeTypeNProp, split) << " label: " << get(startAddrStringNProp, split));
		assert(false);
	}
}

bool SuperControlFlowGraphObject::isCreated(void)
{
	return finished;
}

ControlFlowGraph SuperControlFlowGraphObject::getSCFG(void)
{
	return scfg;
}

CFGVertex SuperControlFlowGraphObject::getSCFGEntry(void)
{
	return entry;
}

CFGVertex SuperControlFlowGraphObject::getSCFGExit(void)
{
	return exit;
}

