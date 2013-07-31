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
#include "cfgloophelper.hpp"


LoggerPtr CFGLoopHelper::logger(Logger::getLogger("CFGLoopHelper"));

CFGLoopHelper::CFGLoopHelper(ControlFlowGraph cfggraph, CFGVertex cfgentry, CFGVertex cfgexit)
{
	cfg = cfggraph;
	cfg_entry = cfgentry;
	cfg_exit = cfgexit;

	// get the properties for the graph (nodes and edges)
	circEProp = get(circ_t(), cfg);
	edgeTypeEProp = get(edgetype_t(), cfg);
	startAddrNProp = get(startaddr_t(), cfg);
	startAddrStringNProp = get(startaddrstring_t(), cfg);
	nodeTypeNProp = get(nodetype_t(), cfg);
	endAddrNProp = get(endaddr_t(), cfg);
}



CFGLoopHelper::~CFGLoopHelper()
{
}



int32_t CFGLoopHelper::getLoopBoundForLoopHead(CFGVertex loop_head, CFGEdge loop_causing_back_edge)
{
	LOG_DEBUG(logger, "Determining loop bound for 0x" << hex << get(startAddrStringNProp, loop_head) << " (" << dec << loop_head << ") with loop casuing edge: " << loop_causing_back_edge);
	CFGEdge injecting_edge = getLoopInjectingEdgeForLoopHead(loop_head, loop_causing_back_edge);

	int32_t no_of_loop_runs = get(circEProp, injecting_edge);

	LOG_DEBUG(logger, "Determined loop bound is: " << no_of_loop_runs);

	return no_of_loop_runs;
}

bool CFGLoopHelper::isLoopCausingBackEdge(CFGEdge back_edge)
{
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

CFGEdge CFGLoopHelper::getLoopInjectingEdgeForLoopHead(CFGVertex loop_head, CFGEdge loop_causing_back_edge)
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



bool CFGLoopHelper::isNodeOnPath(CFGVertex node, CFGVertex path_begin, CFGVertex path_end, bool same_stack_level)
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

	// for determining the predecessor and the successor use the path_end/path_begin node as barrier node, to prevent a false predecessor/successor detection. E.g. a successor and node should be checked, and  the out edges of the node lead out of the function and then, due to different call sites back into the function again, and then traversing to the possible successor. But when entering the funcition via a different call site the identified successor is not a real successor. To prevent this the isSuccessorNode (isPredecessorNode too) function is not allowed to search beyond a barrier node, which would be in this case the function entry point. Therefore the predecessor search has the path_end as barrier and the successor search the path_begin.
	LOG_TRACE(logger, "Checking for predecessor.");
	bool prec  = isPredecessorNode(path_begin, node, path_end, true, same_stack_level);
	LOG_TRACE(logger, "Checking for successor.");
	bool suc =  isSuccessorNode(path_end, node, path_begin, true, same_stack_level);

	LOG_DEBUG(logger, "Prec: " << ((prec)?("True"):("False")) << " Suc: " << ((suc)?("True"):("False")));

	return prec && suc;
//	return isPredecessorNode(path_begin, node) && isSuccessorNode(path_end, node);
}

bool CFGLoopHelper::isPredecessorNode(CFGVertex predecessor, CFGVertex node, bool consider_context, bool same_stack_level)
{
	return isPredecessorNode(predecessor, node, cfg_exit, consider_context, same_stack_level);
}


bool CFGLoopHelper::isPredecessorNode(CFGVertex predecessor, CFGVertex node, CFGVertex barrier_node, bool consider_context, bool same_stack_level)
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


bool CFGLoopHelper::isSuccessorNode(CFGVertex successor, CFGVertex node, bool consider_context, bool same_stack_level)
{
	return isSuccessorNode(successor, node, cfg_entry, consider_context, same_stack_level);
}

bool CFGLoopHelper::isSuccessorNode(CFGVertex successor, CFGVertex node, CFGVertex barrier_node, bool consider_context, bool same_stack_level)
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

bool CFGLoopHelper::isConnectedViaPath(CFGVertex start, CFGVertex end)
{
	return isConnectedViaPath(start, end, true);
}

bool CFGLoopHelper::isConnectedViaPath(CFGVertex start, CFGVertex end, bool same_stack_level)
{
	vector<CFGVertex> path;
	path = getPath(start, end, same_stack_level);
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

vector<CFGVertex> CFGLoopHelper::getPath(CFGVertex start, CFGVertex end)
{
	return getPath(start, end, true);
}

vector<CFGVertex> CFGLoopHelper::getPath(CFGVertex start, CFGVertex end, bool same_stack_level)
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
							if(!function_stack.empty() && same_stack_level)
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

bool CFGLoopHelper::getInjectingEdgeForPath(vector<CFGVertex> path, CFGEdge* injecting_edge)
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
						LOG_DEBUG(logger, "Checking node: actual_src: " << get(startAddrStringNProp, actual_src) << " edge: " << e);

						bool found_in_path = false;

						for(uint32_t j = 0; j < path.size(); j++)
						{
							if(actual_src == path[j])
							{
								found_in_path = true;
								break;
							}
						}
						if(!found_in_path && !isNodeOnPath(actual_src, path.front(), path.back(), true))
						{
							LOG_DEBUG(logger, "Found possible injecting edge: " << e << " from: " << get(startAddrStringNProp, actual_src) << " to " << get(startAddrStringNProp, actual));
							// It is not absolutely sure that this is the injecting edge, because there could be multiple entry points to the loop body, like in the duff benchmark for function duffcopy.
							// Therefore it it checked if the edge has a valid circulation entry that was set by the processing of the flowfacts file.
							if(get(circEProp, e) != -1)
							{
								LOG_DEBUG(logger, "Verified injecting edge: " << e << " from: " << get(startAddrStringNProp, actual_src) << " to " << get(startAddrStringNProp, actual) << " by correct loop constraint.");
								*injecting_edge = e;
								return true;
								
							}
						}
					}
				}
		}
	}

	return false;
}

