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
#include "msgtocfg_conv.hpp"


LoggerPtr MsgToCfgConverter::logger(Logger::getLogger("MsgToCfgConverter"));

MsgToCfgConverter::MsgToCfgConverter(CFGMSGPair cfg_msg, CFGMSGVPair entry, CFGMSGVPair exit)
{
	// setup control flow graph and memory state graph and its entry and exit nodes
	cfgmsg = cfg_msg;
	entry_o = entry;
	exit_o = exit;

	// get node property structures for original cfg
	nodeTypeNProp_o = get(nodetype_t(), cfgmsg.first);
	startAddrNProp_o = get(startaddr_t(), cfgmsg.first);
	startAddrStringNProp_o = get(startaddrstring_t(), cfgmsg.first);
	endAddrNProp_o = get(endaddr_t(), cfgmsg.first);
	callLabelNProp_o = get(calllabel_t(), cfgmsg.first);
	bbCodeNProp_o = get(bbcode_t(), cfgmsg.first);
	bbSizeNProp_o = get(bbsize_t(), cfgmsg.first);
	bbInstrsNProp_o = get(bbinstrs_t(), cfgmsg.first);

	// get edge property structures for original cfg
	costEProp_o = get(cost_t(), cfgmsg.first);
	costOnChipEProp_o = get(cost_onchip_t(), cfgmsg.first);
	costOffChipEProp_o = get(cost_offchip_t(), cfgmsg.first);
	memPenaltyEProp_o = get(mem_penalty_t(), cfgmsg.first);
	capacitylEProp_o = get(capacityl_t(), cfgmsg.first);
	capacityhEProp_o = get(capacityh_t(), cfgmsg.first);
	circEProp_o = get(circ_t(), cfgmsg.first);
	sflowEProp_o = get(static_flow_t(), cfgmsg.first);
	edgeNameEProp_o = get(edgename_t(), cfgmsg.first);
	edgeTypeEProp_o = get(edgetype_t(), cfgmsg.first);

	// get node property structures for msg
	memStateNProp_o = get(mem_state_t(), cfgmsg.second);
	mappedCFGVertexNProp_o = get(cfg_vertex_t(), cfgmsg.second);
	contextIDNProp = get(msg_context_id_t(), cfgmsg.second);
	cacheHitsNProp_o = get(mem_cache_hits_t(), cfgmsg.second);
	cacheMissesNProp_o = get(mem_cache_misses_t(), cfgmsg.second);
	cacheNCsNProp_o = get(mem_cache_ncs_t(), cfgmsg.second);

	// get edge property structures for msg
	msgEdgeTypeEProp = get(msg_edgetype_t(), cfgmsg.second);
	dynamicMemPenaltyEProp = get(dynamic_mem_penalty_t(), cfgmsg.second);
	msgCircEprop = get(msg_circ_t(), cfgmsg.second);


	// get node property structures for new cfg
	nodeTypeNProp_n = get(nodetype_t(), cfg);
	startAddrNProp_n = get(startaddr_t(), cfg);
	startAddrStringNProp_n = get(startaddrstring_t(), cfg);
	endAddrNProp_n = get(endaddr_t(), cfg);
	contextIDNProp_n = get(context_id_t(), cfg);
	callLabelNProp_n = get(calllabel_t(), cfg);
	bbCodeNProp_n = get(bbcode_t(), cfg);
	bbSizeNProp_n = get(bbsize_t(), cfg);
	bbInstrsNProp_n = get(bbinstrs_t(), cfg);
	cacheHitsNProp = get(cache_hits_t(), cfg);
	cacheMissesNProp = get(cache_misses_t(), cfg);
	cacheNCsNProp = get(cache_ncs_t(), cfg);

	// get edge property structures for cfg
	costEProp_n = get(cost_t(), cfg);
	costOnChipEProp_n = get(cost_onchip_t(), cfg);
	costOffChipEProp_n = get(cost_offchip_t(), cfg);
	memPenaltyEProp_n = get(mem_penalty_t(), cfg);
	capacitylEProp_n = get(capacityl_t(), cfg);
	capacityhEProp_n = get(capacityh_t(), cfg);
	circEProp_n = get(circ_t(), cfg);
	sflowEProp_n = get(static_flow_t(), cfg);
	edgeNameEProp_n = get(edgename_t(), cfg);
	edgeTypeEProp_n = get(edgetype_t(), cfg);
}

MsgToCfgConverter::~MsgToCfgConverter()
{
	msg_cfg_map.clear();
}

ControlFlowGraph MsgToCfgConverter::getCfg(void)
{
	MSGVertex v;
	msgVertexIter vp;

	for (vp = vertices(cfgmsg.second); vp.first != vp.second; ++vp.first)
	{
		v = *vp.first;
		CFGVertex u_orig = get(mappedCFGVertexNProp_o, v);
		CFGVertex u = getNode(make_pair(u_orig, v));

		msgOutEdgeIter ep;
		for(ep = out_edges(v, cfgmsg.second); ep.first != ep.second; ++ep.first) 
		{
			MSGVertex msg_target = target(*ep.first, cfgmsg.second);
			CFGVertex cfg_target = get(mappedCFGVertexNProp_o, msg_target);
			connectNodes(u, make_pair(u_orig, v), make_pair(cfg_target, msg_target));
		}
	}
	return cfg;
}

CFGVertex MsgToCfgConverter::getEntry(void)
{
	return entry_n;
}

CFGVertex MsgToCfgConverter::getExit(void)
{
	return exit_n;
}

CFGVertex MsgToCfgConverter::getNode(CFGMSGVPair orig_node)
{
	CFGVertex node;
	MsgCfgMap::iterator pos;

	pos = msg_cfg_map.find(orig_node.second);

	if(pos != msg_cfg_map.end())
	{
		node = pos->second;
	}
	else
	{
		// create node
		node = add_vertex(cfg);
		put(nodeTypeNProp_n, node, get(nodeTypeNProp_o, orig_node.first));
		put(startAddrNProp_n, node, get(startAddrNProp_o, orig_node.first));
		ostringstream s;
		s << get(startAddrStringNProp_o, orig_node.first) << " MSGID:" << orig_node.second;
		put(startAddrStringNProp_n, node, s.str());
//		put(startAddrStringNProp_n, node, get(startAddrStringNProp_o, orig_node.first));
		put(endAddrNProp_n, node, get(endAddrNProp_o, orig_node.first));
		put(contextIDNProp_n, node, get(contextIDNProp, orig_node.second));
		put(callLabelNProp_n, node, get(callLabelNProp_o, orig_node.first));
		put(bbCodeNProp_n, node, get(bbCodeNProp_o, orig_node.first));
		put(bbSizeNProp_n, node, get(bbSizeNProp_o, orig_node.first));
		put(bbInstrsNProp_n, node, get(bbInstrsNProp_o, orig_node.first));

		put(cacheHitsNProp, node, get(cacheHitsNProp_o, orig_node.second));
		put(cacheMissesNProp, node, get(cacheMissesNProp_o, orig_node.second));
		put(cacheNCsNProp, node, get(cacheNCsNProp_o, orig_node.second));

		MsgCfgMap::iterator ins_pos;
		bool ins_bool;
		tie(ins_pos, ins_bool) = msg_cfg_map.insert(make_pair(orig_node.second, node));
		assert(ins_bool);

		// set entry and exit node for new cfg, for node corresponding to the entry and exit node of the msg
		if(orig_node.second == entry_o.second)
		{
			entry_n = node;
		}
		else if(orig_node.second == exit_o.second)
		{
			exit_n = node;
		}
	}

	return node;
}


void MsgToCfgConverter::connectNodes(CFGVertex src, CFGMSGVPair orig_src, CFGMSGVPair orig_tgt)
{
	// TODO DOCUMENTATION WHAT IS DONE HERE: combination of edge properties from original edges and adding the loop bound of the corresponding ones ....


	CFGVertex tgt = getNode(orig_tgt);

	// two edges from source to target, one for the transformed graph and one of the original cfg
	CFGEdge st, st_o;

	bool found_target = false;

	// find the edge in the msg to read the cache penalty and the flow information.
	MSGEdge st_m;
	msgOutEdgeIter ep_m;
	for(ep_m = out_edges(orig_src.second, cfgmsg.second); ep_m.first != ep_m.second; ++ep_m.first) 
	{
		MSGEdge e = *ep_m.first;
		if(target(e, cfgmsg.second) == orig_tgt.second)
		{
			st_m = e;
			found_target = true;
		}
	}

	if(logger->isDebugEnabled() && !found_target)
	{
		LOG_DEBUG(logger, "Sorry cannot find edge (" << orig_src.second << "," <<  orig_tgt.second << ") in msg CFG: Src is: " << get(startAddrStringNProp_o, orig_src.first) << " Tgt is: " << get(startAddrStringNProp_o, orig_tgt.first));
	}
	assert(found_target);

	found_target = false;
	cfgOutEdgeIter ep;

	bool flow_join_edge = false;

	if(get(msgEdgeTypeEProp, st_m) == ForwardStepUnroll)
	{
		// find the edge that leaves the loop body in the origninal cfg
		assert(out_degree(orig_src.first, cfgmsg.first) == 2);

		uint32_t no_of_fw_edges = 0;

		for(ep = out_edges(orig_src.first, cfgmsg.first); ep.first != ep.second; ++ep.first) 
		{
			CFGEdge e = *ep.first;
			if(get(edgeTypeEProp_o, e) != BackwardJump)
			{
				st_o = e;
				found_target = true;
				no_of_fw_edges++;
			}
		}
		assert(no_of_fw_edges == 1); // there can only be one leaving edge at the bottom of a loob body that is no BackwardJump

		if(logger->isDebugEnabled() && !found_target)
		{
			LOG_WARN(logger, "Sorry cannot find leaving edge out of loop body for node " << orig_src.first << " in reference cfg CFG: Src is: " << get(startAddrStringNProp_o, orig_src.first));
		}
	}
	else
	{
		// find edge in original cfg that represents the edge in the memory state graph to copy its properties to the new cfg
		for(ep = out_edges(orig_src.first, cfgmsg.first); ep.first != ep.second; ++ep.first) 
		{
			CFGEdge e = *ep.first;
			if(target(e, cfgmsg.first) == orig_tgt.first)
			{
				st_o = e;
				found_target = true;
			}
		}
		if(!found_target)
		{
			// if no target was found, it is possible that a FlowJoinNode inserted by the VivuGraphCreator. Then no properties of the reference cfg edge can be assigned to the new edge, because there is no edge in the reference cfg. This flow join edge only contains the circulation/loop count.
			if(get(nodeTypeNProp_o, orig_src.first)==FlowJoinNode)
			{
				assert(get(startAddrStringNProp_o, orig_src.first).compare(FLOW_JOIN_NODE)==0);
				flow_join_edge = true;
			}
		}

		if(logger->isDebugEnabled() && !found_target && !flow_join_edge)
		{
			LOG_WARN(logger, "Sorry cannot find edge (" << orig_src.first << "," <<  orig_tgt.first << ") in reference cfg. CFG: Src is: " << get(startAddrStringNProp_o, orig_src.first) << " Tgt is: " << get(startAddrStringNProp_o, orig_tgt.first));
		}

	}

	// either the target was found or a FlowJoinNode out edge was handled
	assert(found_target xor flow_join_edge);


	// add edge
	bool ins_edge;
	tie(st, ins_edge) = add_edge(src, tgt, cfg);
	assert(ins_edge);

	// the edge properties can only be assigned for edges that were found in the reference cfg
	if(found_target)
	{
		// store properties of the edge, except the circ_t() property (which holds the loop bounds)
		put(costEProp_n, st, get(costEProp_o, st_o));
		put(costOnChipEProp_n, st, get(costOnChipEProp_o, st_o));
		put(costOffChipEProp_n, st, get(costOffChipEProp_o, st_o));
		put(capacitylEProp_n, st, get(capacitylEProp_o, st_o));
		put(capacityhEProp_n, st, get(capacityhEProp_o, st_o));
		put(edgeTypeEProp_n, st, get(edgeTypeEProp_o, st_o));
		put(sflowEProp_n, st, get(sflowEProp_o, st_o));
	}

	// store the memory penalty calculated by the cache analysis
	put(memPenaltyEProp_n, st, get(dynamicMemPenaltyEProp, st_m));

	// set the loop bound (circulation / flow value) of the edge using the flow information of the memory state graph (that takes the unrolling into account)
	put(circEProp_n, st, get(msgCircEprop, st_m));

	// add to the edge name the circulation and the cache penalty
	ostringstream s;

	// Distinguish between a normal edge or a flow_join_edge was found.
	if(found_target)
	{
		s <<  get(edgeNameEProp_o, st_o);
	}
	else // flow_join_edge found
	{
		s << "Flow Join Edge ";
	}

	s << " f: " << get(circEProp_n, st) << " c:" << get(memPenaltyEProp_n, st) << " e: " << get(costEProp_n, st);
	put(edgeNameEProp_n, st, s.str());

}


