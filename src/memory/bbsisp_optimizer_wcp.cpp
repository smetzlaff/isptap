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
#include "bbsisp_optimizer_wcp.hpp"

BBSISPOptimizerWCP::BBSISPOptimizerWCP(ControlFlowGraph cfgraph, CFGVertex entry, CFGVertex exit) : SISPOptimizer_IF(cfgraph, entry, exit) 
{
	ilp_solution_wcet_estimate = 0;
}

BBSISPOptimizerWCP::~BBSISPOptimizerWCP()
{
	clear();
}
void BBSISPOptimizerWCP::clear(void)
{
	cfg_ilps.clear();
	block_cost_contraints.clear();
	block_size_constraints.clear();
	binary_domains.clear();
	functionMap.clear();
	assigned_bbaddrs.clear();
	loopMap.clear();
	ilp_solution_wcet_estimate = 0;
}

void BBSISPOptimizerWCP::calculateBlockAssignment()
{
	clear();
	generateBlockCostsConstraints();
	generateBlockSizeConstraint();
	generateBinaryDomain();
	generateOptimalILPFormulation();

	stringstream ilp_formulation;
	vector<string>::iterator it;

	ilp_formulation << "\n// Objective function:\n";
	if(!conf->getBool(CONF_BBSISP_WCP_FILL_ISP_UP))
	{
		ilp_formulation << "min: wentry;\n";
	}
	else
	{
		ilp_formulation << "min: 1e10 wentry - sp;\n";
	}
	ilp_formulation << "\n// Flow constraints:\n";
//	sort(cfg_ilps.begin(),cfg_ilps.end());
	for(it = cfg_ilps.begin(); it != cfg_ilps.end(); it++)
	{
		ilp_formulation << *it;
	}
	ilp_formulation << "\n// Basic block cost contraints:\n";
//	sort(block_cost_contraints.begin(),block_cost_contraints.end());
	for(it = block_cost_contraints.begin(); it != block_cost_contraints.end(); it++)
	{
		ilp_formulation << *it;
	}
	ilp_formulation << "\n// Basic block size contraints:\n";
//	sort(block_size_constraints.begin(),block_size_constraints.end());
	for(it = block_size_constraints.begin(); it != block_size_constraints.end(); it++)
	{
		ilp_formulation << *it;
	}
	ilp_formulation << "\n// Binary domains:\n";
//	sort(binary_domains.begin(),binary_domains.end());
	for(it = binary_domains.begin(); it != binary_domains.end(); it++)
	{
		ilp_formulation << *it;
	}

	LOG_INFO(logger, "Formulation: " << ilp_formulation.str());

	vector<lp_result_set> lp_result = writeAndSolveILPFile(ilp_formulation.str());
	setAssignment(lp_result);
	setVariables(lp_result);
}


void BBSISPOptimizerWCP::generateOptimalILPFormulation(void)
{
	stringstream cfg_ilp;
	uint32_t running_id = 0;
	cfg_ilp << "wentry" << " >= cl" << running_id << ";" << endl;
	cfg_ilp << "cl" << running_id << " = w" << cfg_entry << ";" << endl;
	cfg_ilps.push_back(cfg_ilp.str());
	running_id++;

	registerAllLoopHeads();
	vector<CFGEdge> leaving_edges;
	generateOptimalILPFormulationForSequentialCode(cfg_entry, cfg_exit, running_id, leaving_edges);
	assert(leaving_edges.empty());
}

uint32_t BBSISPOptimizerWCP::generateOptimalILPFormulationForSequentialCode(CFGVertex start, CFGVertex end, uint32_t running_id, vector<CFGEdge>& leavingEdges)
{
	vector<CFGVertex> processing;
	vector<CFGVertex> processed;

	processing.push_back(start);

	cfgOutEdgeIter ep;

	LOG_DEBUG(logger, "Processing from: " << start  << " [" << get(startAddrStringNProp, start) << "]" << " to " << end << " [" << get(startAddrStringNProp, end) << "]" << ".");

	while(processing.size() != 0)
	{
		if(logger->isDebugEnabled())
		{
			ostringstream s;
			for(uint32_t i = 0; i < processing.size(); i++)
			{
				s << "(" << processing[i] << ")";
			}
			LOG_DEBUG(logger, "Size of processing list: " << processing.size() << " contains:" << s.str());
		}

		CFGVertex actual_cfg = processing.back();
		processing.pop_back();

		LOG_DEBUG(logger, "Processing " << actual_cfg << " " << get(startAddrStringNProp, actual_cfg));

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
					// create virtual node for function
					CFGVertex v;
					uint32_t function_id = running_id;
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


					stringstream cfg_ilp;

					assert(out_degree(actual_cfg, cfg) == 1);
					cfgOutEdgeIter eop = out_edges(actual_cfg, cfg);
					CFGVertex function_entry_cfg = target(*eop.first, cfg);
					assert(in_degree(cfg_return_point, cfg) == 1);
					cfgInEdgeIter eip = in_edges(cfg_return_point, cfg);
					CFGVertex function_exit_cfg = source(*eip.first, cfg);

					FunctionMap::iterator pos = functionMap.find(function_entry_cfg);

					if(pos == functionMap.end())
					{
						vector<CFGEdge> function_leaving_edges;
						// create ilp for the function body
						running_id = generateOptimalILPFormulationForSequentialCode(function_entry_cfg, function_exit_cfg, ++running_id, function_leaving_edges);
						LOG_DEBUG(logger, "Returned from function to processing from: " << start << " to " << end << ".");
						assert(function_leaving_edges.empty());
						
						FunctionMap::iterator ins_pos;
						bool ins_bool;
						tie(ins_pos, ins_bool) = functionMap.insert(make_pair(function_entry_cfg, function_id));
						assert(ins_bool);
						// create cost for the function with function_id
						cfg_ilp << "cf" << function_id << " = " << " w" << function_entry_cfg  << ";" << endl;

					}
					else
					{
						function_id = pos->second;
					}
					// connect call point with virtual function node wfXctxY
					cfg_ilp << "w" << actual_cfg << " >= wf" << function_id << "c" << hex << context_addr << dec << getPenaltyForFunctionEntering(actual_cfg, function_entry_cfg) << ";" << endl;
					// connect virtual function node with return point, and taking cost of function into account
					// NOTICE the cost of the function exit node (which is an Exit node) to the node to which it is returned (which is an ReturnPoint node) does not need to be considered, because it is free of cost.
					cfg_ilp << "wf" << function_id << "c" << hex << context_addr << dec <<  " >= w" << cfg_return_point << " + cf" << function_id << getPenaltyForFunctionExit(cfg_return_point, function_exit_cfg) << ";" << endl;

					cfg_ilps.push_back(cfg_ilp.str());
					processing.push_back(cfg_return_point);

				}
				else
				{
					for(ep = out_edges(actual_cfg, cfg); ep.first != ep.second; ++ep.first) 
					{
						bool found_loop_head = false;
						uint32_t loop_id = running_id;
						vector<CFGVertex> loop_exits;
						vector<CFGEdge> irregular_loop_exit_edges;
						CFGEdge eo = *ep.first;
						CFGEdge back_edge;
						CFGVertex target_cfg = target(eo, cfg);

						edge_type_t etype = get(edgeTypeEProp, eo);
						if((etype == ForwardStep) || (etype == ForwardJump) || (etype == Meta))
						{

							LOG_DEBUG(logger, "Checking out-edges From: " << actual_cfg << " " << get(startAddrStringNProp, actual_cfg) << " To: " << target_cfg << " " << get(startAddrStringNProp, target_cfg) << " Type: " << etype << " Edge " << eo);

							LoopDataMap::iterator pos = loopMap.find(target_cfg);
							if(pos != loopMap.end())
							{
								if((target_cfg != start) &&  (pos->second.exitNode != end))
								{
									LOG_DEBUG(logger, "Target of out edge " <<  eo << ". " << target_cfg << " is a loop head. Backedge is: " <<  pos->second.backEdge << " exit node is: " <<  pos->second.exitNode);

									loop_exits.push_back(pos->second.exitNode);
									found_loop_head = true;

									stringstream cfg_ilp;
									int32_t loop_bound = getLoopBoundForLoopHead(target_cfg, pos->second.backEdge);
									cfg_ilp << "cl" << loop_id << " = " << loop_bound+1 /* the value from the flow facts determines the number of invocations of the back_edge, i.e. the number of times the loop is entered  */ << " w" << pos->second.startNode;
									if(get(nodeTypeNProp, pos->second.exitNode) == BasicBlock)
									{
										// charge the cost of the loop conserving edge, i.e. of the bottom node of the loop body
										// The cost of this basic block for the loop exiting edge is charged, on connection of the virtual loop node with the code after the loop (see calculation of wlXX and the usage of the variable loop_exits).
										if(!conf->getBool(CONF_BBSISP_WCP_SHRINK_ILP_FORMULATION))
										{
											cfg_ilp << " + " << loop_bound << " ce" << source(pos->second.backEdge,cfg) << "t" << target(pos->second.backEdge, cfg);
										}
										else // conf->getBool(CONF_BBSISP_WCP_SHRINK_ILP_FORMULATION)
										{
											cfg_ilp << " + " << getEdgeCostConstraint(pos->second.backEdge, loop_bound);
										}
									}
									else
									{
										assert(false); // could there be a loop with a non basic block node at the end??
									}

									cfg_ilp << ";" << endl;
									cfg_ilps.push_back(cfg_ilp.str());

									// create ilp for the loop body
									running_id = generateOptimalILPFormulationForSequentialCode(pos->second.startNode, pos->second.exitNode, ++running_id, irregular_loop_exit_edges);
									LOG_DEBUG(logger, "Returned loop to processing from: " << start << " to " << end << ".");
								}
							}

							stringstream cfg_ilp;

							if(!found_loop_head)
							{
								bool is_on_path = isNodeOnPath(target_cfg, start, end, false);
								LOG_DEBUG(logger, "Successor of actual node " << actual_cfg << " " << get(startAddrStringNProp, actual_cfg) << " (edge: " << eo << ") is no loop head. The node is " << ((!is_on_path)?("not "):("")) << "within the sequential code part");
								cfg_ilp << "w" << actual_cfg << " >= ";
								if(is_on_path)
								{
									cfg_ilp << "w" << target_cfg;
								}
								if(get(nodeTypeNProp, actual_cfg) == BasicBlock)
								{
									if(is_on_path)
									{
										cfg_ilp << " + ";
									}

									if(!conf->getBool(CONF_BBSISP_WCP_SHRINK_ILP_FORMULATION))
									{
										cfg_ilp << "ce" << source(eo,cfg) << "t" << target(eo, cfg);
									}
									else // conf->getBool(CONF_BBSISP_WCP_SHRINK_ILP_FORMULATION)
									{
										cfg_ilp << getEdgeCostConstraint(eo);
									}
								}


								cfg_ilp << ";" << endl;

								cfg_ilps.push_back(cfg_ilp.str());
								
								// checking if target node is within loop body
								if(is_on_path)
								{
									LOG_DEBUG(logger, "Pushing " << target_cfg << " " << get(startAddrStringNProp, target_cfg) << " to process list");
									processing.push_back(target_cfg);
								}
								else
								{
									LOG_DEBUG(logger, "Node " << target_cfg << " " << get(startAddrStringNProp, target_cfg) << " is not within currently processing code part (function or loop body), cannot add to process list.");
									leavingEdges.push_back(eo);
								}

							}
							else
							{
								LOG_DEBUG(logger, "Successor of actual node " << actual_cfg << " " << get(startAddrStringNProp, actual_cfg) << " (edge: " << eo << ") is loop head");
								cfg_ilp << "w" << actual_cfg << " >= wl" << loop_id;
								if(get(nodeTypeNProp, actual_cfg) == BasicBlock)
								{
									if(!conf->getBool(CONF_BBSISP_WCP_SHRINK_ILP_FORMULATION))
									{
										cfg_ilp << " + ce" << source(eo,cfg) << "t" << target(eo, cfg);
									}
									else // conf->getBool(CONF_BBSISP_WCP_SHRINK_ILP_FORMULATION)
									{
										cfg_ilp << " + " << getEdgeCostConstraint(eo);
									}
								}
								cfg_ilp << ";" <<  endl;
								cfg_ilps.push_back(cfg_ilp.str());

								for(uint32_t i = 0; i < loop_exits.size(); i++)
								{
									LOG_DEBUG(logger, "Loop exit nodes are: " << loop_exits[i]);
									for(cfgOutEdgeIter ep2 = out_edges(loop_exits[i], cfg); ep2.first != ep2.second; ++ep2.first) 
									{
										CFGEdge el = *ep2.first;
										CFGVertex post_loop_node = target(el, cfg);

										edge_type_t etype = get(edgeTypeEProp, el);
										if((etype == ForwardStep) || (etype == ForwardJump) || (etype == Meta))
										{
											stringstream tmp;
											// the wcet of the loop is the wcet of the following node + the cost of the loop + the cost of the loop out edge

											if(!conf->getBool(CONF_BBSISP_WCP_SHRINK_ILP_FORMULATION))
											{
												tmp << "wl" << loop_id << " >= w" << post_loop_node <<  " + cl" << loop_id << " + ce" << source(el,cfg) << "t" << target(el, cfg) << ";" << endl;
											}
											else // conf->getBool(CONF_BBSISP_WCP_SHRINK_ILP_FORMULATION)
											{
												tmp << "wl" << loop_id << " >= w" << post_loop_node <<  " + cl" << loop_id << " + " << getEdgeCostConstraint(el) << ";" << endl;
											}

											cfg_ilps.push_back(tmp.str());

											// checking if target node is within loop body
											if(isNodeOnPath(target_cfg, start, end, false))
											{
												LOG_DEBUG(logger, "Pushing post loop node " << post_loop_node << " " << get(startAddrStringNProp, post_loop_node) << " to process list");
												processing.push_back(post_loop_node);
											}
											else
											{
												LOG_DEBUG(logger, "Post loop node " << target_cfg << " " << get(startAddrStringNProp, target_cfg) << " is not on path, cannot add to process list.");
												leavingEdges.push_back(eo);
												assert(false);
											}
										}
									}

								}
								for(uint32_t i = 0; i < irregular_loop_exit_edges.size(); i++)
								{
									CFGEdge e = irregular_loop_exit_edges[i];
									LOG_DEBUG(logger, "Irregular loop exit edge: " << e);
									// checking if the target of the loop leaving edge can be found whithin the currently activw loop body
									if(isNodeOnPath(target(e, cfg), start, end, false))
									{

										stringstream tmp;
										// If a loop is unexpectively left by an edge e, the target of e may be reached by the loop body (in the worst case at the end of the loop, since the structure of the loop is hidden by the cost of the loop (clXX).
										// The cost of the irregular loop exit edge does not need to be charged here, because it is implicitely already in the cost of the loop (which is maximizes over all possible paths, including the dead end of an irregular leaving edge.
										tmp << "wl" << loop_id << " >= w" << target(e, cfg) <<  " + cl" << loop_id << ";" << endl; 
										cfg_ilps.push_back(tmp.str());
									}
									else
									{
										// The irregular loop leaving edge leaves multiple levels of loop nests. Delegate handling to next loop level.
										leavingEdges.push_back(e);
										assert(false);
									}
								}


							}

						}
					}
				}
			}
			else
			{
				LOG_DEBUG(logger, "Found end node");

				stringstream cfg_ilp;

				// The cost of the loop conserving edge (the bottom basic block) is charged in the loop cost variable: clXX = loop_bound+1 * wLoopHead + ceLoopConservingEdgeXX
				// This is because the loop body is ececuted loop_bound+1, whereas the cost of the loop conserving edge needs to be taken only loop_bound times into account, because the last iteration of the loop uses another edge, which is charged in wlXX >= wYY + clXX + ceExitEdgeOfLoopXX
				cfg_ilp << "w" << actual_cfg << " = 0;" << endl;

				cfg_ilps.push_back(cfg_ilp.str());
			}
			processed.push_back(actual_cfg);
		}
	}
	return running_id;

}

void BBSISPOptimizerWCP::generateBlockCostsConstraints(void)
{
	if(!conf->getBool(CONF_BBSISP_WCP_SHRINK_ILP_FORMULATION))
	{
		for(cfgEdgeIter ep = edges(cfg); ep.first != ep.second; ++ep.first)
		{
			CFGEdge e = *ep.first;
			if(get(nodeTypeNProp, source(e, cfg)) == BasicBlock)
			{
				stringstream str;
				str << "ce" << source(e,cfg) << "t" << target(e,cfg) << " = " << get(costOffChipEProp, e);
				if(get(costOffChipEProp, e) != get(costOnChipEProp, e))
				{
					str << " - " << (get(costOffChipEProp, e) - get(costOnChipEProp, e)) << " a" << source(e,cfg);
				}
				str << ";" << endl;
				block_cost_contraints.push_back(str.str());
			}
		}

		block_cost_contraints.push_back(" \n");
	}
}

string BBSISPOptimizerWCP::getEdgeCostConstraint(CFGEdge e)
{
	stringstream str;
	str << get(costOffChipEProp, e);
	uint32_t benefit = get(costOffChipEProp, e) - get(costOnChipEProp, e);
	if(benefit != 0)
	{
		str << " - " << benefit << " a" << source(e,cfg);
	}
	return str.str();
}


string BBSISPOptimizerWCP::getEdgeCostConstraint(CFGEdge e, uint32_t multiplicator)
{
	stringstream str;
	str << (multiplicator * get(costOffChipEProp, e));
	uint32_t benefit = get(costOffChipEProp, e) - get(costOnChipEProp, e);
	if(benefit != 0)
	{
		str << " - " << (multiplicator * benefit) << " a" << source(e,cfg);
	}
	return str.str();
}

void BBSISPOptimizerWCP::generateBlockSizeConstraint(void)
{
	stringstream block_size_stream;
	cfgVertexIter vp = vertices(cfg);
	assert(vp.first != vp.second);
	CFGVertex v = *vp.first;

	// search as long as the first bb was found
	while((get(nodeTypeNProp, v) != BasicBlock) && (vp.first != vp.second))
	{
		++vp.first;
		assert(vp.first != vp.second);
		v= *vp.first;
	}

	assert(get(nodeTypeNProp, v) == BasicBlock);
	block_size_stream << get(bbSizeNProp, v) << " a" << v << endl; 

	++vp.first;

	for(; vp.first != vp.second; ++vp.first)
	{
		v= *vp.first;
		if(get(nodeTypeNProp, v) == BasicBlock)
		{
			block_size_stream << " + " << get(bbSizeNProp, v) << " a" << v << endl; 
		}
	}

	block_size_stream << " = sp;" << endl;
	block_size_stream << "sp <= " << sisp_size << ";" << endl;

	block_size_constraints.push_back(block_size_stream.str());
}

void BBSISPOptimizerWCP::generateBinaryDomain(void)
{
	for(cfgVertexIter vp = vertices(cfg); vp.first != vp.second; ++vp.first)
	{
		CFGVertex v = *vp.first;
		if(get(nodeTypeNProp, v) == BasicBlock)
		{
			stringstream str;
			str << "bin a" << v << ";" << endl; 
			// TODO: Need to set variable as binary. Otherwise if this will cause significant extra lp_solve run-time, cross check results if they are really binary.
//			str << "bin na" << v << ";" << endl; 
			binary_domains.push_back(str.str());
		}
	}
}


string BBSISPOptimizerWCP::getPenaltyForFunctionEntering(CFGVertex UNUSED_PARAMETER(call_point), CFGVertex UNUSED_PARAMETER(function_entry_node))
{
	// Do nothing. This function is only needed for BBSISPOptimizerJPWCP.
	return "";
}


string BBSISPOptimizerWCP::getPenaltyForFunctionExit(CFGVertex UNUSED_PARAMETER(return_point), CFGVertex UNUSED_PARAMETER(function_exit_node))
{
	// Do nothing. This function is only needed for BBSISPOptimizerJPWCP.
	return "";
}

void BBSISPOptimizerWCP::registerAllLoopHeads(void)
{
	for(cfgEdgeIter eit = edges(cfg); eit.first != eit.second; ++eit.first)
	{
		CFGEdge e = *eit.first;
		if(get(edgeTypeEProp, e) == BackwardJump)
		{
			if(isLoopCausingBackEdge(e))
			{
				if(getLoopBoundForLoopHead(target(e, cfg), e) > 0)
				{
					CFGVertex loop_entry = target(getLoopInjectingEdgeForLoopHead(target(e, cfg), e), cfg);
					CFGVertex loop_end = source(e, cfg);
					LOG_DEBUG(logger, "Loop causing edge: " << e << " loop entry: " << loop_entry << " start: " << target(e, cfg) << " exit: " << loop_end);
					loop_data_t tmp;
					tmp.exitNode = loop_end;
					tmp.backEdge = e;
					tmp.startNode = target(e, cfg);
					tmp.entryNode = loop_entry;
					bool ins;
					LoopDataMap::iterator pos;
					tie(pos, ins) = loopMap.insert(make_pair(loop_entry, tmp));
					assert(ins == true);
				}
			}
		}
	}
}


void BBSISPOptimizerWCP::setVariables(vector<lp_result_set> lp_result)
{
	for(uint32_t i = 0; i < lp_result.size(); i++)
	{
		if(lp_result[i].variable.compare("wentry") == 0)
		{
			// found the estimated WCET for the WCP-sensitive approach
			LOG_INFO(logger, "The bbsisp assignment estimates the WCET with: " << lp_result[i].value);
			ilp_solution_wcet_estimate = lp_result[i].value;
		}
		else if(lp_result[i].variable.compare("sp") == 0)
		{
			// found the used bbsisp size
			LOG_INFO(logger, "Used BBSISP size is: " << lp_result[i].value);
			ilp_solution_used_size = lp_result[i].value;
		}
	}
}

uint32_t BBSISPOptimizerWCP::getEstimatedWCET(void)
{
	return ilp_solution_wcet_estimate;
}


sisp_result_t BBSISPOptimizerWCP::getResults(void)
{
	sisp_result_t tmp = SISPOptimizer_IF::getResults();
	tmp.estimated_timing = getEstimatedWCET();
	tmp.estimated_jump_penalty = numeric_limits<uint32_t>::max();

	return tmp;
}

