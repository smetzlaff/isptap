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
#include "vivug_creator.hpp"

LoggerPtr VivuGraphCreator::logger(Logger::getLogger("VivuGraphCreator"));


#ifdef VIVUG_CREATOR_USES_CFG_LOOP_HELPER
VivuGraphCreator::VivuGraphCreator(ControlFlowGraph cfgraph, CFGVertex entry, CFGVertex exit) : CFGLoopHelper(cfgraph, entry, exit) 
#else
VivuGraphCreator::VivuGraphCreator(ControlFlowGraph cfgraph, CFGVertex entry, CFGVertex exit)
#endif
{
#ifndef VIVUG_CREATOR_USES_CFG_LOOP_HELPER
	// setup control flow graph
	cfg = cfgraph;
	cfg_entry = entry;
	cfg_exit = exit;
#endif

	// get node property structures for cfg
#ifndef VIVUG_CREATOR_USES_CFG_LOOP_HELPER
	nodeTypeNProp = get(nodetype_t(), cfg);
	startAddrNProp = get(startaddr_t(), cfg);
	startAddrStringNProp = get(startaddrstring_t(), cfg);
	endAddrNProp = get(endaddr_t(), cfg);
#endif
	callLabelNProp = get(calllabel_t(), cfg);
	bbCodeNProp = get(bbcode_t(), cfg);
	bbSizeNProp = get(bbsize_t(), cfg);
	bbInstrsNProp = get(bbinstrs_t(), cfg);

	// get edge property structures for cfg
#ifndef VIVUG_CREATOR_USES_CFG_LOOP_HELPER
	edgeTypeEProp = get(edgetype_t(), cfg);
	circEProp = get(circ_t(), cfg);
#endif
	costEProp = get(cost_t(), cfg);
	costOnChipEProp = get(cost_onchip_t(), cfg);
	costOffChipEProp = get(cost_offchip_t(), cfg);
	capacitylEProp = get(capacityl_t(), cfg);
	capacityhEProp = get(capacityh_t(), cfg);
	actEProp = get(activation_t(), cfg);
	edgeNameEProp = get(edgename_t(), cfg);

	// get node property structures for msg
	memStateNProp = get(mem_state_t(), msg);
	memStateValNProp = get(mem_state_valid_t(), msg);
	mappedCFGVertexNProp = get(cfg_vertex_t(), msg);
	contextIDNProp = get(msg_context_id_t(), msg);

	// get edge property structures for msg
	msgEdgeTypeEProp = get(msg_edgetype_t(), msg);
	msgCircEprop = get(msg_circ_t(), msg);

	conf = Configuration::getInstance();
}

VivuGraphCreator::~VivuGraphCreator()
{
	contextMap.clear();
}

CFGMSGPair VivuGraphCreator::createVivuGraph(void)
{
	assert(get(nodeTypeNProp, cfg_entry)== Entry);
	msg_entry = add_vertex(msg);
	put(mappedCFGVertexNProp, msg_entry, cfg_entry);
	put(memStateValNProp, msg_entry, false);

	assert(get(nodeTypeNProp, cfg_exit)== Exit);
	msg_exit = add_vertex(msg);
	put(mappedCFGVertexNProp, msg_exit, cfg_exit);
	put(memStateValNProp, msg_exit, false);

#ifdef VIVUG_CREATOR_USES_CFG_LOOP_HELPER
	LOG_INFO(logger, "The VivuGraphCreator uses CFGLoopHelpers implementation of finding injecting edges for loops and constructing paths.");
#endif

	MSGVertex out = createVivuGraphForSequentialCode(make_pair(cfg_entry, msg_entry), cfg_exit);
	assert(msg_exit == out);


	/*
	 * do not create inducting edge!
	bool ins_edge;
	MSGEdge et;

	tie(et, ins_edge) = add_edge(msg_exit, msg_entry, msg);
	put(msgEdgeTypeEProp, et, InductingBackEdge);
	assert(ins_edge);
	*/

	return make_pair(cfg, msg);
}

MSGVertex VivuGraphCreator::createVivuGraphForSequentialCode(CFGMSGVPair start, CFGVertex end)
{
	vector<CFGMSGVPair> processing;
	vector<CFGVertex> processed;
	vector<CFGVertex> progressed_call_contexts;
	vector<loop_nodes_t> loop_exits;
	processing.push_back(start);
	MSGVertex msg_end = start.second; // temporally initialize this variable
	cfgOutEdgeIter ep;
	CfgMsgMap cfg_msg_map;

	cfg_msg_map.insert(start);

	LOG_DEBUG(logger, "Processing from: ( " << start.first  << ", " << start.second <<  ") " << "[" << get(startAddrStringNProp, start.first) << "]" << " to " << end << " [" << get(startAddrStringNProp, end) << "]" << ".");


	while(processing.size() != 0)
	{
		if(logger->isDebugEnabled())
		{
			ostringstream s;
			for(uint32_t i = 0; i < processing.size(); i++)
			{
				s << "(" << processing[i].first << "," << processing[i].second << ")";
			}
			LOG_DEBUG(logger, "Size of processing list: " << processing.size() << " contains:" << s.str());
		}

		CFGVertex actual_cfg;
		MSGVertex actual_msg;
		tie(actual_cfg, actual_msg) = processing.back();
		processing.pop_back();



		// ensure that each node is only handled once.
		bool node_already_processed = false;
		for(uint32_t j = 0; j<processed.size(); j++)
		{
			if(actual_cfg == processed[j])
			{
				node_already_processed = true;
			}
		}
		if(!node_already_processed)
		{
			if(actual_cfg != end)
			{
				if(get(nodeTypeNProp, actual_cfg) == CallPoint)
				{

					// XXX The inlining of functions is _really_ ugly.
					CFGVertex v;
					CFGVertex cfg_return_point = actual_cfg; // to temporally initialize the vertex
					cfgVertexIter vp;
					bool found_return = false;
					// find the right return point for the call point
					uint32_t context_addr = get(endAddrNProp, actual_cfg);
					for (vp = vertices(cfg); (vp.first != vp.second)&&(!found_return); ++vp.first)
					{
						v = *vp.first;
						if((get(nodeTypeNProp, v) == ReturnPoint) && (context_addr == get(endAddrNProp, v)))
						{
							found_return = true;
							cfg_return_point = v;
						}
					}
					assert(found_return);


					// lookup actual context id
					ContextMap::iterator pos = contextMap.find(context_addr);
					uint32_t context_id=0;

					if(pos != contextMap.end())
					{
						context_id = pos->second;
						pos->second = (context_id+1);

					}
					else
					{
						ContextMap::iterator ins_pos;
						bool ins_bool;
						tie(ins_pos, ins_bool) = contextMap.insert(make_pair(context_addr, context_id+1));
						assert(ins_bool);
					}

					LOG_DEBUG(logger, "Call context of: 0x" << hex <<  context_addr << dec << " node pair: callpoint: " << actual_cfg << " returnpoint: " << cfg_return_point << " context id: " << context_id);
					assert(found_return);


					// get the exit node!
					assert(in_degree(cfg_return_point, cfg) == 1);
					cfgInEdgeIter epi = in_edges(cfg_return_point, cfg);
					CFGEdge e_rt = *epi.first;
					CFGVertex cfg_exit_node = source(e_rt, cfg);
					LOG_DEBUG(logger, "Exit node is: " << cfg_exit_node);

					assert(out_degree(actual_cfg, cfg) == 1);
					ep = out_edges(actual_cfg, cfg);
					CFGEdge e_tg = *ep.first;
					CFGVertex target_cfg = target(e_tg, cfg);
					assert(get(edgeTypeEProp, e_tg) == Meta);
					MSGVertex new_msg = addMSGVertex(make_pair(actual_cfg, actual_msg), target_cfg, NULL, get(edgeTypeEProp, e_tg), e_tg);

					MSGVertex msg_exit_point = createVivuGraphForSequentialCode(make_pair(target_cfg, new_msg), cfg_exit_node);
//					processing.push_back(make_pair(cfg_exit_node, msg_exit_point));
					MSGVertex msg_return_point = addMSGVertex(make_pair(cfg_exit_node, msg_exit_point), cfg_return_point, NULL,get(edgeTypeEProp, e_rt), e_rt);
					processing.push_back(make_pair(cfg_return_point, msg_return_point));
					LOG_DEBUG(logger, "Returned to processing from: " << get(startAddrStringNProp, start.first) << " ( " << start.first << ", " << start.second <<  ")  to " << get(startAddrStringNProp, end) << " (" << end << ").");

					// set context ids (to identify the different inlined functions)
					put(contextIDNProp, actual_msg, context_id);
					put(contextIDNProp, msg_return_point, context_id);
				}
				else
				{
					bool found_bw = false; // determines if a backward edge was found and a new return msg node was delivered.
					CFGVertex loop_head = actual_cfg; // the target of a potentially found back edge, temporally initialize this variable

					MSGVertex msg_return = actual_msg; // temporally initialize this variable

					// check all back edges per node first, to unroll the loop represented by the back edge (starting at "target_cfg" ending at "actual_cfg")
					for(ep = out_edges(actual_cfg, cfg); ep.first != ep.second; ++ep.first) 
					{
						CFGEdge e = *ep.first;
						CFGVertex target_cfg;

						target_cfg = target(e, cfg);

						edge_type_t etype = get(edgeTypeEProp, e);

						LOG_DEBUG(logger, "From: " << get(startAddrStringNProp, actual_cfg) << " To: " << get(startAddrStringNProp, target_cfg) << " Type: " << etype);

						// handling for continue in loops (as jpegdct does in EDN):
						// - if multiple back edges targeting a loop head
						// - unroll the loop only once
						// - (forward) jump from break statement of 1st iteration to begin of rest iterations (may be has to be a ForwardStepUnroll since also the loop bound has to be added to this edge)
						// - direct the back edges to the head of the rest of iterations

						if(etype == BackwardJump)
						{
							// Unroll loops only if the loopbound is larger than 0 (the loop body is executed more than once after the initial run)
							if(conf->getBool(CONF_USE_FLOWFACT_GRAPH_ENRICHMENT) && (isLoopCausingBackEdge(e)) && (getLoopBoundForLoopHead(target_cfg, e) > 0))
							{
								if(target_cfg != start.first)
								{
									// if a nested loop is detected (by jumping back not to the current loop head), it will be handled recursively


									loop_head = target(e, cfg);
									if(!isLastUnhandledLoopEdge(&loop_exits, loop_head))
									{
										LOG_DEBUG(logger, "Found multiple back edges for node: 0x" << hex << get(startAddrNProp, loop_head) << " processed edge from 0x" << get(startAddrNProp, actual_cfg));

										registerLoopExitNodeToLoop(&loop_exits, actual_msg, loop_head);
									}
									else
									{
										LOG_DEBUG(logger, "Found last unhandled back edges for node: 0x" << hex << get(startAddrNProp, loop_head) << " processed edge from 0x" << get(startAddrNProp, actual_cfg));

										found_bw=true;

										// create msg vertex and connect it (Notice: no back edge is created) Notice the vertex is always created, no matter if it is already in the cfg_msg_map.
										CFGEdge injectingEdge = getLoopInjectingEdgeForLoopHead(target_cfg, e);
										MSGVertex new_msg = addMSGVertex(make_pair(actual_cfg, actual_msg), target_cfg, NULL, ForwardStepUnroll, injectingEdge); // added edgetype ForwardStepUnroll -> to mark that a backedge is transformed into a forward edge for unrolling the first loop iteration

										// convert also all other back edges of the first unrolled loop iteration, that target the start of the rest of loop iteration
										addForwardStepUnrollEdgesForUnrolledLoopHead(&loop_exits, target_cfg, injectingEdge, getFlowJoinNodeFromRestOfLoopHead(new_msg));

										// create the graph for the rest iterations of the loop
										msg_return = createVivuGraphForSequentialCode(make_pair(target_cfg, new_msg), actual_cfg);
										LOG_DEBUG(logger, "Returned to processing from: ( " << start.first << ", " << start.second <<  ") to " << end << ".");


										// add the back edge to the msg graph to allow flow analysis later on
										bool ins_edge;
										MSGEdge et;

										tie(et, ins_edge) = add_edge(msg_return, new_msg, msg);
										put(msgEdgeTypeEProp, et, etype);
										put(msgCircEprop, et, get(circEProp, e));
										assert(ins_edge);
									}

								}
								else
								{
									// do nothing, this case cannot appear
								}
							}
						}
					}

					// create for cfg vertexes connected by forward edges a mirror msg edge
					for(ep = out_edges(actual_cfg, cfg); ep.first != ep.second; ++ep.first) 
					{

						CFGEdge e = *ep.first;
						CFGVertex target_cfg;

						target_cfg = target(e, cfg);

						// check if the target node in connected to the exit node
//						bool node_connected_old = false;
//						for(uint32_t i = 0; (i < connected_nodes.size()) && !node_connected_old; i++)
//						{
//							if(target_cfg == connected_nodes[i])
//								node_connected_old = true;
//						}

						// check if the target node in connected to the start and exit node
						bool node_connected = true;
						if((cfg_exit != end) && (cfg_entry != start.first))
						{
							node_connected = isNodeOnPath(target_cfg, start.first, end, false);
						}

//						if(node_connected != node_connected_old)
//						{
//							LOG_WARN(logger, "Test connection on node failed: " << get(startAddrStringNProp, target_cfg) <<  " Is a connected node: " << ((node_connected_old)?("true"):("false")) << " Node on path: " << ((node_connected)?("true"):("false")));
//						}

						// if it is not connected to the exit node do not process it!
						if(!node_connected)
						{
//							LOG_DEBUG(logger, "Testing connection on node: " << get(startAddrStringNProp, target_cfg) <<  " Is a connected node: " << ((node_connected_old)?("true"):("false")) << " Node on path: " << ((node_connected)?("true"):("false")));
							LOG_DEBUG(logger, "Testing connection on node: " << get(startAddrStringNProp, target_cfg) <<  " Is a connected node: "  << ((node_connected)?("true"):("false")));

						}
						else
						{
							edge_type_t etype = get(edgeTypeEProp, e);

							if((etype == ForwardStep) || (etype == ForwardJump) || (etype == Meta) || ((etype == BackwardJump) && !isLoopCausingBackEdge(e)))
							{
								// the in edges for ReturnPoints are created in conjunction with handling CallPoints (see above)
								if(get(nodeTypeNProp, target_cfg) != ReturnPoint)
								{

									MSGVertex new_msg;
									//					if((etype == ForwardStep) && (found_bw))
									if(found_bw)
									{
										// create msg vertex and connect it to the one that was returned from the loop unfolding (see back edge handling above)
										new_msg = addMSGVertex(make_pair(actual_cfg, msg_return), target_cfg, &cfg_msg_map, etype, e);
									}
									else
									{
										// create msg vertex and connect it
										new_msg = addMSGVertex(make_pair(actual_cfg, actual_msg), target_cfg, &cfg_msg_map, etype, e);
									}
									// push_back to proccessing list
									processing.push_back(make_pair(target_cfg, new_msg));
								}
							}
							else if((etype == BackwardJump) && (target_cfg == start.first) && (actual_cfg != end))
							{
								bool ins_edge;
								MSGEdge et;

								tie(et, ins_edge) = add_edge(actual_msg, start.second, msg);
								put(msgEdgeTypeEProp, et, BackwardJump);
								put(msgCircEprop, et, -1);

								assert(ins_edge);
							}
						}
					}
				}
			}
			else
			{
				// marking MSG end node
				msg_end = actual_msg;

				LOG_DEBUG(logger, "Found end node");
			}
			processed.push_back(actual_cfg);
		}
	}

	return msg_end;
}

MSGVertex VivuGraphCreator::addMSGVertex(CFGMSGVPair source, CFGVertex target_cfg, CfgMsgMap *map, edge_type_t original_edge_type, CFGEdge correspondingEdge)
{
	MSGVertex new_msg;
	// TODO The special handling of the exit node possibly can be kicked out, somehow.
	if(target_cfg == cfg_exit)
	{
		new_msg = msg_exit;
	}
	else
	{
		if(map == NULL)
		{
			new_msg = add_vertex(msg);
			LOG_DEBUG(logger, "added msg vertex for " << hex << get(startAddrStringNProp, target_cfg));
		}
		else
		{
			CfgMsgMap::iterator pos;

			pos = map->find(target_cfg);

			if(pos != map->end())
			{
				new_msg = pos->second;
			}
			else
			{
				new_msg = add_vertex(msg);

				LOG_DEBUG(logger, "added new msg vertex for " << hex << get(startAddrStringNProp, target_cfg));

				CfgMsgMap::iterator ins_pos;
				bool ins_bool;
				tie(ins_pos, ins_bool) = map->insert(make_pair(target_cfg, new_msg));
				assert(ins_bool);
			}
		}
		put(mappedCFGVertexNProp, new_msg, target_cfg);
		put(memStateValNProp, new_msg, false);
		put(contextIDNProp, new_msg, 0);
	}

	bool ins_edge;
	MSGEdge et;

	if(original_edge_type == ForwardStepUnroll)
	{
		// insert additional FLOW_JOIN_NODE dummy node after 1st unrolled iteration and rest of the loop.
		// This is needed to combine the flow (loop bounds) of all back edges of the loop.

		MSGVertex dummy_msg = add_vertex(msg);
		CFGVertex dummy_cfg = add_vertex(cfg);

		put(startAddrStringNProp, dummy_cfg, FLOW_JOIN_NODE);
		put(nodeTypeNProp, dummy_cfg, FlowJoinNode);

		put(mappedCFGVertexNProp, dummy_msg, dummy_cfg);
		put(memStateValNProp, dummy_msg, false);
		put(contextIDNProp, dummy_msg, 0);


		// connect the dummy node as a ForwardStepUnroll
		tie(et, ins_edge) = add_edge(source.second, dummy_msg, msg);
		put(msgEdgeTypeEProp, et, ForwardStepUnroll);
		put(msgCircEprop, et, -1);

		// connect the dummy_msg with the new_msg as Meta edge with the loop bounds
		tie(et, ins_edge) = add_edge(dummy_msg, new_msg, msg);
		put(msgEdgeTypeEProp, et, Meta);

		// all loop bounds that are not at a ForwardStepUnroll edge must be the unrolled "first iteration", so set these flow values (if there are some) to one (for first loop iteration)
		int32_t circulation = get(circEProp, correspondingEdge);
		if(circulation > 0)
		{
			circulation--;
		}

		put(msgCircEprop, et, circulation);
		assert(ins_edge);


	}
	else
	{
		// connect the source node with the new_msg
		tie(et, ins_edge) = add_edge(source.second, new_msg, msg);
		put(msgEdgeTypeEProp, et, original_edge_type);

		// all loop bounds that are not at a ForwardStepUnroll edge must be the unrolled "first iteration", so set these flow values (if there are some) to one (for first loop iteration)
		int32_t circulation = get(circEProp, correspondingEdge);
		if(circulation > 0)
		{
			circulation = 1;
		}

		put(msgCircEprop, et, circulation);
		assert(ins_edge);
	}

	LOG_DEBUG(logger, "Inserted/connected node: " << new_msg << " with " << et << " in cfg: " << get(startAddrStringNProp, source.first) << " -> " << get(startAddrStringNProp, target_cfg));

	return new_msg;
}

CFGMSGVPair VivuGraphCreator::getEntry(void)
{
	return make_pair(cfg_entry, msg_entry);
}

CFGMSGVPair VivuGraphCreator::getExit(void)
{
	return make_pair(cfg_exit, msg_exit);
}

#ifndef VIVUG_CREATOR_USES_CFG_LOOP_HELPER
uint32_t VivuGraphCreator::getLoopBoundForLoopHead(CFGVertex loop_head, CFGEdge loop_causing_back_edge)
{
	CFGEdge injecting_edge = getLoopInjectingEdgeForLoopHead(loop_head, loop_causing_back_edge);

	int32_t no_of_loop_runs = get(circEProp, injecting_edge);

	if(no_of_loop_runs < 0)
	{
		no_of_loop_runs = 0;
	}


	LOG_DEBUG(logger, "Determined loop bound is: " << no_of_loop_runs);

	return no_of_loop_runs;
}

bool VivuGraphCreator::isLoopCausingBackEdge(CFGEdge back_edge)
{
	// TODO crosscheck this with the flow fact reader.
	
//	CFGVertex loop_head = target(back_edge, cfg);
//	cfgVertexIter vp;
//	CFGEdge e;
//	cfgInEdgeIter epi;
//
////#warning "This is just for testing dijkstra"
////	if(get(startAddrNProp, loop_head) == 0xa00011e2)
////		return false;
//
//	bool is_loop_b_edge = true;
//
//	if(get(edgeTypeEProp, back_edge) != BackwardJump)
//	{
//		is_loop_b_edge = false;
//	}
//
//	uint32_t no_in_edges = 0;
//	for(epi = in_edges(loop_head, cfg); epi.first != epi.second; ++epi.first) 
//	{
//		e = *epi.first;
//		if((get(edgeTypeEProp, e) == ForwardStep) || (get(edgeTypeEProp, e) == ForwardJump) || (get(edgeTypeEProp, e) == Meta))
//		{
//			// the inducting edge could be either a normal forward step, forward jump or a meta edge (e.g. if the loop head is the first bb of a function)
//			no_in_edges++;
//		}
//	}
//
//	bool found_tail_injecting_edge = false;
//	if((no_in_edges == 0) && (out_degree(loop_head, cfg) > 0))
//	{
//		// assuming that there is a loop inducting edge somewere else
//		found_tail_injecting_edge = true;
//	}
//
//	if(!(no_in_edges == 1 || found_tail_injecting_edge))
//	{
//		LOG_ERROR(logger, "Cannot find in injecting edge for basic block: " << get(startAddrStringNProp, loop_head));
//		is_loop_b_edge = false;
//	}
	

//	LOG_DEBUG(logger, "Checking conection: from " << get(startAddrStringNProp, source(back_edge, cfg)) << " to " << get(startAddrStringNProp, target(back_edge, cfg)));

	// if the target of the back edge is connected to the source the back edge spans a loop.
	bool is_connected = isConnectedViaPath(target(back_edge, cfg), source(back_edge, cfg));

//	if(is_connected != is_loop_b_edge)
//	{
//		LOG_WARN(logger, "Loop edge detection does not correspond to connective test: is_connected: " << is_connected << " is_loop_b_edge: " << is_loop_b_edge);
//	}
//	else
//	{
//		LOG_DEBUG(logger, "Loop edge detection is correct: is_connected: " << is_connected << " is_loop_b_edge: " << is_loop_b_edge);
//	}

	if(!is_connected)
	{
		LOG_DEBUG(logger, "Checking conection: from " << get(startAddrStringNProp, source(back_edge, cfg)) << " to " << get(startAddrStringNProp, target(back_edge, cfg)) << " failed the back_edge: " << back_edge << " is not causing a loop.");
	}
	else
	{
		LOG_DEBUG(logger, "Checking conection: from " << get(startAddrStringNProp, source(back_edge, cfg)) << " to " << get(startAddrStringNProp, target(back_edge, cfg)) << " success the back_edge: " << back_edge << " is causing a loop.");
	}

	return is_connected;
}

CFGEdge VivuGraphCreator::getLoopInjectingEdgeForLoopHead(CFGVertex loop_head, CFGEdge loop_causing_back_edge)
{
	CFGEdge result;

	vector<CFGVertex> path = getPath(target(loop_causing_back_edge, cfg), source(loop_causing_back_edge, cfg));
	if(!getInjectingEdgeForPath(path, &result))
	{
		LOG_ERROR(logger, "No Injecting edge found for: path from " << get(startAddrStringNProp, target(loop_causing_back_edge, cfg)) << " to " << get(startAddrStringNProp, source(loop_causing_back_edge, cfg)) << " ! ");
	}

	LOG_DEBUG(logger, "Inducting edge for: 0x" << hex << get(startAddrNProp, loop_head) << " is " << dec << result );

	return result;
}
#endif


uint32_t VivuGraphCreator::getBackEdgeInDegree(CFGVertex node)
{

	cfgInEdgeIter epi;
	CFGEdge e;

	uint32_t no_bedges = 0;
	for(epi = in_edges(node, cfg); epi.first != epi.second; ++epi.first) 
	{
		e = *epi.first;
		if(get(edgeTypeEProp, e) == BackwardJump)
		{
			no_bedges++;
		}
	}
	return no_bedges;
}

uint32_t VivuGraphCreator::getNumberOfHandledBackEdges(vector<loop_nodes_t> *loop_nodes, CFGVertex loop_head)
{
	for(uint32_t i=0; i < loop_nodes->size(); i++)
	{
		if(loop_nodes->at(i).loop_head == loop_head)
		{
			return loop_nodes->at(i).loop_exits.size();
		}
	}
	return 0;
}


void VivuGraphCreator::registerLoopExitNodeToLoop(vector<loop_nodes_t> *loop_nodes, MSGVertex loop_exit, CFGVertex loop_head)
{
	bool found_loop = false;
	for(uint32_t i=0; (i < loop_nodes->size()) && (!found_loop); i++)
	{
		if(loop_nodes->at(i).loop_head == loop_head)
		{
			found_loop=true;
			bool found_loop_exit = false;
			for(uint32_t j=0; (j < loop_nodes->at(i).loop_exits.size()) && (!found_loop_exit); j++)
			{
				if(loop_nodes->at(i).loop_exits[j] == loop_exit)
				{
					found_loop_exit = true;
				}
			}
			if(!found_loop_exit)
			{
				loop_nodes->at(i).loop_exits.push_back(loop_exit);
			}
		}
	}

	if(!found_loop)
	{
		loop_nodes_t tmp;
		tmp.loop_head = loop_head;
		tmp.loop_exits.push_back(loop_exit);
		loop_nodes->push_back(tmp);
	}
}

									
void VivuGraphCreator::addForwardStepUnrollEdgesForUnrolledLoopHead(vector<loop_nodes_t> *loop_nodes, CFGVertex loop_head, CFGEdge UNUSED_PARAMETER(loop_injecting_edge), MSGVertex loop_flow_join_node)
{
	bool found_loop = false;
	for(uint32_t i=0; (i < loop_nodes->size()) && (!found_loop); i++)
	{
		if(loop_nodes->at(i).loop_head == loop_head)
		{
			found_loop=true;
			for(uint32_t j=0; j < loop_nodes->at(i).loop_exits.size(); j++)
			{
				bool ins_edge;
				MSGEdge et;

				tie(et, ins_edge) = add_edge(loop_nodes->at(i).loop_exits[j], loop_flow_join_node, msg);
				put(msgEdgeTypeEProp, et, ForwardStepUnroll);
				put(msgCircEprop, et, -1);
				assert(ins_edge);
			}
		}
	}

}


bool VivuGraphCreator::isLastUnhandledLoopEdge(vector<loop_nodes_t> *loop_nodes, CFGVertex loop_head)
{
	return(getBackEdgeInDegree(loop_head) == (1+getNumberOfHandledBackEdges(loop_nodes, loop_head)));
}


MSGVertex VivuGraphCreator::getFlowJoinNodeFromRestOfLoopHead(MSGVertex rest_loop_head)
{
	assert(in_degree(rest_loop_head, msg) == 1);

	msgInEdgeIter epi;
	epi = in_edges(rest_loop_head, msg); 
	MSGEdge e = *epi.first;
	MSGVertex loop_flow_join_node = source(e, msg);


//	assert(get(startAddrStringNProp, get(mappedCFGVertexNProp, loop_flow_join_node)).compare(FLOW_JOIN_NODE)==0);
	assert(get(nodeTypeNProp, get(mappedCFGVertexNProp, loop_flow_join_node))==FlowJoinNode);

	return loop_flow_join_node;
}

#ifndef VIVUG_CREATOR_USES_CFG_LOOP_HELPER
bool VivuGraphCreator::isNodeOnPath(CFGVertex node, CFGVertex path_begin, CFGVertex path_end, bool same_stack_level)
{
	LOG_DEBUG(logger, "Checking if node " << get(startAddrStringNProp, node) << " is on a path from " << get(startAddrStringNProp, path_begin) << " to " << get(startAddrStringNProp, path_end));
	if((node == path_begin) || (node == path_end))
	{
		return true;
	}

	if(path_begin == path_end)
	{
		return false;
	}

	// for determining the predecessor and the successor use the path_end/path_begin node as barrier node, to prevent a false predecessor/successor detection. E.g. a successor and node should be checked, and  the out edges of the node lead out of the function and then, due to different call sites back into the function again, and then traversing to the possible successor. But when entering the funcition via a different call site the identified successor is not a real successor. To prevent this the isSuccessorNode (isPredecessorNode too) function is not allowed to search beyond a barrier node, which would be in this case the function entry point. Therefor the predecessor search has the path_end as barrier and the successor search the path begin.
	bool prec  = isPredecessorNode(path_begin, node, path_end, true, same_stack_level);
	bool suc =  isSuccessorNode(path_end, node, path_begin, true, same_stack_level);

	LOG_DEBUG(logger, "Prec: " << ((prec)?("True"):("False")) << " Suc: " << ((suc)?("True"):("False")));

	return prec && suc;
//	return isPredecessorNode(path_begin, node) && isSuccessorNode(path_end, node);
}

bool VivuGraphCreator::isPredecessorNode(CFGVertex predecessor, CFGVertex node, bool consider_context, bool same_stack_level)
{
	return isPredecessorNode(predecessor, node, cfg_exit, consider_context, same_stack_level);
}


bool VivuGraphCreator::isPredecessorNode(CFGVertex predecessor, CFGVertex node, CFGVertex barrier_node, bool consider_context, bool same_stack_level)
{

	// TODO: alternative implementation:
//	return isSuccessorNode(node, predecessor);


	stack<CFGVertex> processing;
	vector<node_context_t> processed;
	stack<CFGVertex> function_stack;
	ContextStack function_stack_addr;
	CFGVertex actual;
	CFGEdge e;
	cfgInEdgeIter epi;

	processing.push(node);

	while(!processing.empty())
	{
		actual = processing.top();

		LOG_TRACE(logger, "Stack is: 0x" << function_stack_addr.toString());

		bool in_processed = false;

		ContextStack chk_ctx = function_stack_addr;
		if(get(nodeTypeNProp, actual) == ReturnPoint)
		{
			// kick off the top element from context stack, if a return point detected
			chk_ctx.pop();
		}

		LOG_TRACE(logger, "Looking up processed list for actual node: " << get(startAddrStringNProp, actual) << " context is " << function_stack_addr.toString() << " check_ctx is " << chk_ctx.toString());

		for(uint32_t i = 0; (i < processed.size()) && !in_processed; i++)
		{
			if((actual == processed[i].node) && (!consider_context || (chk_ctx == processed[i].context)))
			{
				LOG_TRACE(logger, "Already processed node: " << get(startAddrStringNProp, actual) << " in context: " << chk_ctx.toString());
				in_processed = true;
			}
		}
		
		if(in_processed)
		{
			processing.pop();
			if(get(nodeTypeNProp, actual) == ReturnPoint)
			{
				// if a return point is correcly processed, pop it from the stack
				function_stack.pop();
				function_stack_addr.pop();
			}
			else if(get(nodeTypeNProp, actual) == CallPoint)
			{
				// if a call point is correcly processed, push the context back on the the stack (to correctly process the rest of the function
				function_stack.push(actual);
				function_stack_addr.push(get(endAddrNProp, actual));
			}
		}
		else
		{

			if(get(nodeTypeNProp, actual) == ReturnPoint)
			{
				uint32_t top = function_stack_addr.top();

				// push return point on stack if it is not already there (NOTE: recursion is not supported here)
				if(top != get(endAddrNProp, actual))
				{
					// if a return point is detected push it to the function stack.
					function_stack.push(actual);
					function_stack_addr.push(get(endAddrNProp, actual));
				}
			}
			else if(get(nodeTypeNProp, actual) == CallPoint)
			{
				uint32_t top = function_stack_addr.top();

				// pop from stack if the return point that corresponds to the call point is at top (NOTE: recursion is not supported here)
				if(top == get(endAddrNProp, actual))
				{
					if(function_stack.empty())
					{
						LOG_WARN(logger, "Wrong stack level.");
					}
					else
					{
						function_stack.pop();
					}
					if(function_stack_addr.empty())
					{
						LOG_WARN(logger, "Wrong ctx level.");
					}
					else
					{
						function_stack_addr.pop();
					}
				}
			}

			LOG_TRACE(logger, "Checking in edges of " << get(startAddrStringNProp, actual));
			bool check_in_edges = true;

			if(check_in_edges)
			{
				bool unhandled_edge = false;

				for(epi = in_edges(actual, cfg); (epi.first != epi.second); ++epi.first) 
				{
					e = *epi.first;
					if((get(edgeTypeEProp, e) == ForwardStep) || (get(edgeTypeEProp, e) == ForwardJump) || (get(edgeTypeEProp, e) == Meta) || ((get(edgeTypeEProp, e) == BackwardJump) && !isLoopCausingBackEdge(e)))
					{
						CFGVertex actual_src=source(e,cfg);
						if((actual_src == predecessor) && (!same_stack_level || function_stack_addr.empty()))
						{
							LOG_TRACE(logger, "Node " << get(startAddrStringNProp, predecessor) << " is predecessor of node " << get(startAddrStringNProp, node) << " via: " << get(startAddrStringNProp, actual));
							return true;
						}
						else
						{
							bool in_processed = false;

							ContextStack chk_ctx = function_stack_addr;
							if(get(nodeTypeNProp, actual_src) == CallPoint)
							{
								// kick the top element from context stack, if a call point detected
								chk_ctx.pop();
							}

							LOG_TRACE(logger, "Looking up processed list for actual_src node: " << get(startAddrStringNProp, actual_src) << " context is " << function_stack_addr.toString() << " check_context is " << chk_ctx.toString());

							for(uint32_t i = 0; (i < processed.size()) && !in_processed; i++)
							{

								if((actual_src == processed[i].node) && (!consider_context || (chk_ctx == processed[i].context)))
								{
									LOG_TRACE(logger, "Already processed actual_src node: " << get(startAddrStringNProp, actual_src) << " in context: " << chk_ctx.toString());
									in_processed = true;
								}
							}

							// ignore a call point with an invalid context address (do not add to processing list)
							bool ignore = false;
							if(get(nodeTypeNProp, actual_src) == CallPoint)
							{
//								if(!function_stack.empty() && get(endAddrNProp, function_stack.top()) != get(endAddrNProp, actual_src)) <- OLDER VERSION
//								if((!function_stack.empty()) && (actual_ctx != get(endAddrNProp, actual_src))) <- OLD VERSION
//								if((allow_function_leave) || (function_stack.empty())) || (actual_ctx != get(endAddrNProp, actual_src)))
								if((function_stack.empty()) || (function_stack_addr.top() != get(endAddrNProp, actual_src)))
								{
									LOG_TRACE(logger, "Ignoring call point: " << get(startAddrStringNProp, actual_src));
									ignore = true;
								}

							}

							if(actual_src == barrier_node)
							{
								LOG_TRACE(logger, "Ignoring barrier node: " << get(startAddrStringNProp, actual_src));
								ignore = true;
							}

							if(!in_processed && !ignore)
							{


								processing.push(actual_src);
								unhandled_edge = true;
							}
						}
					}
				}
				if(!unhandled_edge)
				{
					node_context_t tmp;
					tmp.node = actual;
					tmp.context = function_stack_addr;
					if(get(nodeTypeNProp, actual) == ReturnPoint)
					{
						// return points are executed in the context of the caller function, thus pop() the callee context
						tmp.context.pop();
					}
					processed.push_back(tmp);
					LOG_TRACE(logger, "Storing in processed list: " << get(startAddrStringNProp, actual) << " context: " << tmp.context.toString());
				}
			}
		}
	}
	return false;
}


bool VivuGraphCreator::isSuccessorNode(CFGVertex successor, CFGVertex node, bool consider_context, bool same_stack_level)
{
	return isSuccessorNode(successor, node, cfg_entry, consider_context, same_stack_level);
}

bool VivuGraphCreator::isSuccessorNode(CFGVertex successor, CFGVertex node, CFGVertex barrier_node, bool consider_context, bool same_stack_level)
{
	stack<CFGVertex> processing;
	vector<node_context_t> processed;
	stack<CFGVertex> function_stack;
	ContextStack function_stack_addr;
	CFGVertex actual;
	CFGEdge e;
	cfgOutEdgeIter epo;

	processing.push(node);

	while(!processing.empty())
	{
		actual = processing.top();

		LOG_TRACE(logger, "Stack is: " << function_stack_addr.toString());

		bool in_processed = false;

		ContextStack chk_ctx = function_stack_addr;
		if(get(nodeTypeNProp, actual) == CallPoint)
		{
			// kick the top element from context stack, if a call point detected
			chk_ctx.pop();
		}

		LOG_TRACE(logger, "Looking up processed list for actual node: " << get(startAddrStringNProp, actual) << " context is " << function_stack_addr.toString() << " check_ctx is " << chk_ctx.toString());

		for(uint32_t i = 0; (i < processed.size()) && !in_processed; i++)
		{
			if((actual == processed[i].node) && (!consider_context || (chk_ctx == processed[i].context)))
			{
				LOG_TRACE(logger, "Already processed node: " << get(startAddrStringNProp, actual) << " in context: " << chk_ctx.toString());
				in_processed = true;
			}
		}
		
		if(in_processed)
		{
			processing.pop();

			if(get(nodeTypeNProp, actual) == CallPoint)
			{
				// if a call point is correcly processed, pop it from the stack
				function_stack.pop();
				function_stack_addr.pop();
			}
			else if(get(nodeTypeNProp, actual) == ReturnPoint)
			{
				// if a return point is correcly processed, push the context back on the the stack (to correctly process the rest of the function
				function_stack.push(actual);
				function_stack_addr.push(get(endAddrNProp, actual));
			}
		}
		else
		{

			if(get(nodeTypeNProp, actual) == CallPoint)
			{
				uint32_t top = function_stack_addr.top();

				// push call point on stack if it is not already there (NOTE: recursion is not supported here)
				if(top != get(endAddrNProp, actual))
				{
					// if a call point is detected push it to the function stack.
					function_stack.push(actual);
					function_stack_addr.push(get(endAddrNProp, actual));
				}
			}
			else if(get(nodeTypeNProp, actual) == ReturnPoint)
			{
				uint32_t top = function_stack_addr.top();

				// pop from stack if the call point that corresponds to the return point is at top (NOTE: recursion is not supported here)
				if(top == get(endAddrNProp, actual))
				{
					if(function_stack.empty())
					{
						LOG_WARN(logger, "Wrong stack level.");
					}
					else
					{
						function_stack.pop();
					}
					if(function_stack_addr.empty())
					{
						LOG_WARN(logger, "Wrong ctx level.");
					}
					else
					{
						function_stack_addr.pop();
					}
				}
			}

			LOG_TRACE(logger, "Checking out edges of " << get(startAddrStringNProp, actual));
			bool check_out_edges = true;

			if(check_out_edges)
			{

				bool unhandled_edge = false;

				for(epo = out_edges(actual, cfg); (epo.first != epo.second); ++epo.first) 
				{
					e = *epo.first;
					if((get(edgeTypeEProp, e) == ForwardStep) || (get(edgeTypeEProp, e) == ForwardJump) || (get(edgeTypeEProp, e) == Meta) || ((get(edgeTypeEProp, e) == BackwardJump) && !isLoopCausingBackEdge(e)))
					{
						CFGVertex actual_tgt=target(e,cfg);
						if((actual_tgt == successor) && (!same_stack_level || function_stack_addr.empty()))
						{
							LOG_TRACE(logger, "Node " << get(startAddrStringNProp, successor) << " is successor of node " << get(startAddrStringNProp, node) << " via: " << get(startAddrStringNProp, actual));
							return true;
						}
						else
						{
							bool in_processed = false;

							ContextStack chk_ctx = function_stack_addr;
							if(get(nodeTypeNProp, actual_tgt) == ReturnPoint)
							{
								// kick the top element from context stack, if a return point detected
								chk_ctx.pop();
							}

							LOG_TRACE(logger, "Looking up processed list for actual_tgt node: " << get(startAddrStringNProp, actual_tgt) << " context is "  << function_stack_addr.toString() << " check_context is " << chk_ctx.toString());

							for(uint32_t i = 0; (i < processed.size()) && !in_processed; i++)
							{
								if((actual_tgt == processed[i].node) && (!consider_context || (chk_ctx == processed[i].context)))
								{
									LOG_TRACE(logger, "Already processed node: " << get(startAddrStringNProp, actual_tgt) << " in context: " <<  chk_ctx.toString());
									in_processed = true;
								}
							}

							// ignore a return point with an invalid context address (do not add to processing list)
							bool ignore = false;
							if(get(nodeTypeNProp, actual_tgt) == ReturnPoint)
							{
//								if(!function_stack.empty() && get(endAddrNProp, function_stack.top()) != get(endAddrNProp, actual_tgt))
//								if((allow_function_leave || (function_stack.empty())) || (actual_ctx != get(endAddrNProp, actual_tgt)))
								if((function_stack.empty()) || (function_stack_addr.top() != get(endAddrNProp, actual_tgt)))
								{
									LOG_TRACE(logger, "Ignoring return point: " << get(startAddrStringNProp, actual_tgt));
									ignore = true;
								}
							}

							if(actual_tgt == barrier_node)
							{
								LOG_TRACE(logger, "Ignoring barrier node: " << get(startAddrStringNProp, actual_tgt));
								ignore = true;
							}

							if(!in_processed && !ignore)
							{
								processing.push(actual_tgt);
								unhandled_edge = true;
							}
						}
					}
				}
				if(!unhandled_edge)
				{
					node_context_t tmp;
					tmp.node = actual;
					tmp.context = function_stack_addr;
					if(get(nodeTypeNProp, actual) == CallPoint)
					{
						// return points are executed in the context of the caller function, so pop the context of the callee
						tmp.context.pop();
					}
					processed.push_back(tmp);
					LOG_TRACE(logger, "Storing in processed list: " << get(startAddrStringNProp, actual) << " context: " << tmp.context.toString());
				}
			}
		}
	}
	return false;
}


bool VivuGraphCreator::isConnectedViaPath(CFGVertex start, CFGVertex end)
{
	vector<CFGVertex> path;
	path = getPath(start, end);
	if(path.size() > 0) 
	{
		stringstream spath;
		spath << get(startAddrStringNProp, path[0]);
		for(uint32_t i = 1; i < path.size(); i++)
		{
			spath << " -> " << get(startAddrStringNProp, path[i]);
		}
		LOG_DEBUG(logger, "Found path: " << spath.str());
		return true;
	}
	else
	{
		return false;
	}
}


vector<CFGVertex> VivuGraphCreator::getPath(CFGVertex start, CFGVertex end)
{

	// This function can be implemented stack sensitive. (such that start and end have to be on the same stack level, which is level 0)
	
	stack<CFGVertex> processing;
	vector<CFGVertex> processed;
	stack<CFGVertex> function_stack;
	CFGVertex actual;
	CFGEdge e;
	cfgOutEdgeIter epo;

	vector<CFGVertex> path;

	if(start == end)
	{
		// TODO check if start and end are connected via back edge.
		path.push_back(start);
		return path;
	}

	processing.push(start);

	LOG_TRACE(logger, "getPath() entry. From: " << get(startAddrStringNProp, start) << " To: " << get(startAddrStringNProp, end));

	while(!processing.empty())
	{
		actual = processing.top();

		bool in_processed = false;
//		LOG_DEBUG(logger, "Looking up processed list for actual node: " << get(startAddrStringNProp, actual));
		for(uint32_t i = 0; (i < processed.size()) && !in_processed; i++)
		{
			if(actual == processed[i])
			{
//				LOG_DEBUG(logger, "Already processed node: " << get(startAddrStringNProp, actual));
				in_processed = true;
			}
		}
		if(in_processed)
		{
			LOG_TRACE(logger, "Node already handled, ignoring it: " << get(startAddrStringNProp, actual));
			processing.pop();
			// kick out actual node from path, because a dead end was detected
			path.pop_back();

			if(get(nodeTypeNProp, actual) == CallPoint)
			{
				// if a call point is correcly processed, pop it from the stack
				function_stack.pop();
			}
			else if(get(nodeTypeNProp, actual) == ReturnPoint)
			{
				// if a return point is correcly processed, push the context back on the the stack (to correctly process the rest of the function
				function_stack.push(actual);
			}
		}
		else
		{
			// mark actual node as path, if not already active node on path
			if(path.empty() || ((!path.empty()) && (path.back() != actual)))
			{
				path.push_back(actual);
			}

			LOG_TRACE(logger, "Handling out edges of node : " << get(startAddrStringNProp, actual) << " path length: " << path.size());

//			LOG_DEBUG(logger, "Checking out edges of " << get(startAddrStringNProp, actual));
			bool check_out_edges = true;

			if(get(nodeTypeNProp, actual) == CallPoint)
			{
				// if a call point is detected push it to the function stack.
				function_stack.push(actual);
			}
			else if(get(nodeTypeNProp, actual) == ReturnPoint)
			{
// code not needed, because wrong return contexts are checked directly when testing the out edges.
				if(function_stack.empty())
				{
//					// wrong stack level
////					check_out_edges = false;
				}
//				else if(get(endAddrNProp, function_stack.top()) != get(endAddrNProp, actual))
//				{
//					// wrong function context exit
//					check_out_edges = false;
//				}
				else
				{
//					LOG_DEBUG(logger, "Stack head is: " << get(startAddrStringNProp, function_stack.top()));
					function_stack.pop();
				}
			}

			if(check_out_edges)
			{
				bool unhandled_edge = false;
				for(epo = out_edges(actual, cfg); (epo.first != epo.second); ++epo.first) 
				{
					e = *epo.first;
					if((get(edgeTypeEProp, e) == ForwardStep) || (get(edgeTypeEProp, e) == ForwardJump) || (get(edgeTypeEProp, e) == Meta))
					{
						CFGVertex actual_tgt=target(e,cfg);
						if(actual_tgt == end)
						{
							if(!function_stack.empty())
							{
								// TODO: check what should happen if the actual_tgt is a return point.
								LOG_WARN(logger, "Start and end node of getPath() _should_ be on the same stack level.");
							}
							path.push_back(actual_tgt);
							return path;
						}
						else
						{
							bool in_processed = false;
//							LOG_DEBUG(logger, "Looking up processed list for actual_tgt node: " << get(startAddrStringNProp, actual_tgt));
							for(uint32_t i = 0; (i < processed.size()) && !in_processed; i++)
							{
								if(actual_tgt == processed[i])
								{
//									LOG_DEBUG(logger, "Already processed node: " << get(startAddrStringNProp, actual_tgt));
									in_processed = true;
								}
							}

							// ignore a return point with an invalid context address (do not add to processing list)
							bool ignore = false;
							if(get(nodeTypeNProp, actual_tgt) == ReturnPoint)
							{
								if(!function_stack.empty() && get(endAddrNProp, function_stack.top()) != get(endAddrNProp, actual_tgt))
								{
									ignore = true;
								}
							}

//							if(actual_tgt == barrier_node)
//							{
//								LOG_DEBUG(logger, "Ignoring barrier node: " << get(startAddrStringNProp, actual_tgt));
//								ignore = true;
//							}

							if(!in_processed && !ignore)
							{
								LOG_DEBUG(logger, "Adding node: " << get(startAddrStringNProp, actual_tgt) << " to process list.");
								processing.push(actual_tgt);
								unhandled_edge = true;
							}
						}
					}
				}
				if(!unhandled_edge)
				{
					processed.push_back(actual);
//					LOG_DEBUG(logger, "Storing in processed list: " << get(startAddrStringNProp, actual));
				}
			}
		}
	}
	return path;
}

bool VivuGraphCreator::getInjectingEdgeForPath(vector<CFGVertex> path, CFGEdge *injecting_edge)
{
	stack<CFGVertex> function_stack;
	ContextStack function_stack_addr;

	// TODO: maybe add the strict context handling here like in isPredecessorNode/isSuccessorNode

	function_stack.push(path[0]);

	CFGVertex actual;

	LOG_DEBUG(logger, "Path size= " << path.size());
	for(int32_t i = path.size()-1; i >= 0; i--)
	{
		actual = path[i];
		LOG_DEBUG(logger, "Looking for injecting edge: " << get(startAddrStringNProp, actual));
		if(get(nodeTypeNProp, actual) == ReturnPoint)
		{
			function_stack.push(actual);
			function_stack_addr.push(get(endAddrNProp, actual));
		}
		else if(get(nodeTypeNProp, actual) == CallPoint)
		{
			function_stack.pop();
			function_stack_addr.pop();
		}

		if(function_stack_addr.top() == 0)
		{
			CFGEdge e;
			cfgInEdgeIter epi;
			for(epi = in_edges(actual, cfg); (epi.first != epi.second); ++epi.first) 
				{
					e = *epi.first;
					if((get(edgeTypeEProp, e) == ForwardStep) || (get(edgeTypeEProp, e) == ForwardJump) || (get(edgeTypeEProp, e) == Meta))
					{
						CFGVertex actual_src=source(e,cfg);
						LOG_DEBUG(logger, "Checking node: actual_src: " << get(startAddrStringNProp, actual_src));

						bool found_in_path = false;

						for(uint32_t j = 0; j < path.size(); j++)
						{
							if(actual_src == path[j])
							{
								found_in_path = true;
								break;
							}
						}
//						if(!found_in_path && !isNodeOnPath(actual_src, path.front(), path.back(), false))
						if(!found_in_path && !isNodeOnPath(actual_src, path.front(), path.back(), true))
						{
							LOG_DEBUG(logger, "Found injecting edge: " << e << " from: " << get(startAddrStringNProp, actual_src) << " to " << get(startAddrStringNProp, actual));
							*injecting_edge = e;
							return true;
						}
					}
				}
		}
	}

	return false;
}

#endif

