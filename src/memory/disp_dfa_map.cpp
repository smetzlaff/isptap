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
#include "disp_dfa_map.hpp"
#include "arch_cfg_factory.hpp"

LoggerPtr DISPDataFlowAnalyzerMap::logger(Logger::getLogger("DISPDFAMap"));

DISPDataFlowAnalyzerMap::DISPDataFlowAnalyzerMap(CFGMSGPair graph_pair, CFGMSGVPair entry_node, CFGMSGVPair exit_node, FunctionCallGraphObject *fcgo)
{
	graph = graph_pair;
	entry = entry_node;
	exit = exit_node;

	functions = fcgo;

	// get node property structures for cfg
	nodeTypeNProp = get(nodetype_t(), graph.first);
	startAddrNProp = get(startaddr_t(), graph.first);
	startAddrStringNProp = get(startaddrstring_t(), graph.first);
	bbSizeNProp = get(bbsize_t(), graph.first);

	// get node property structures for msg
	memStateValNProp = get(mem_state_valid_t(), graph.second);
	mappedCFGVertexNProp = get(cfg_vertex_t(), graph.second);

	// get edge property structures for msg
	msgEdgeTypeEProp = get(msg_edgetype_t(), graph.second);
	dynamicMemPenaltyEProp = get(dynamic_mem_penalty_t(), graph.second);

	arch_cfg = ArchConfigFactory::getInstance()->getArchConfigObject();

	disp_state_maintainer = NULL;

	MemoryParameters mp;
	disp_parameters = mp.getDispParameters();

	setMemParameters();
}

DISPDataFlowAnalyzerMap::DISPDataFlowAnalyzerMap(CFGMSGPair graph_pair, CFGMSGVPair entry_node, CFGMSGVPair exit_node, FunctionCallGraphObject *fcgo, uint32_t mem_size)
{
	graph = graph_pair;
	entry = entry_node;
	exit = exit_node;

	functions = fcgo;

	// get node property structures for cfg
	nodeTypeNProp = get(nodetype_t(), graph.first);
	startAddrNProp = get(startaddr_t(), graph.first);
	startAddrStringNProp = get(startaddrstring_t(), graph.first);
	bbSizeNProp = get(bbsize_t(), graph.first);

	// get node property structures for msg
	memStateValNProp = get(mem_state_valid_t(), graph.second);
	mappedCFGVertexNProp = get(cfg_vertex_t(), graph.second);

	// get edge property structures for msg
	msgEdgeTypeEProp = get(msg_edgetype_t(), graph.second);
	dynamicMemPenaltyEProp = get(dynamic_mem_penalty_t(), graph.second);

	arch_cfg = ArchConfigFactory::getInstance()->getArchConfigObject();

	disp_state_maintainer = NULL;

	MemoryParameters mp;
	disp_parameters = mp.getDispParameters(mem_size);
	setMemParameters();
}


DISPDataFlowAnalyzerMap::~DISPDataFlowAnalyzerMap()
{
	ctxMap.clear();
	functions = NULL;
	assert(disp_state_maintainer != NULL);
	delete(disp_state_maintainer);
}


void DISPDataFlowAnalyzerMap::analyzeDISP(void)
{
	vector<CFGMSGVPair> processing;
	processing.push_back(entry);

	assert(disp_parameters.size != 0);

	while(processing.size() != 0)
	{
		CFGMSGVPair actual = processing.back();
		processing.pop_back();

		if(isPredecessorMemStateKnown(actual) && (actual != exit))
		{
			LOG_DEBUG(logger, "Setting initial memory state for node: " << actual.second << " " << get(startAddrStringNProp, actual.first));
			// this code is used if the mem_state is representing the state of the cache _before_ call/return handling
			if(getForwardInEdgeCount(actual.second) > 1)
			{
				joinAndSetInitialMemState(actual);
			}
			else
			{
				setInitialMemState(actual);
			}

			if(logger->isDebugEnabled())
			{
				LOG_DEBUG(logger, "Initial memory state of node: " << dec << actual.second << " with addr: 0x" << hex << get(startAddrNProp, actual.first) << " state is valid: " << get(memStateValNProp, actual.second) << " in_degree: " << dec << in_degree(actual.second, graph.second) << " node type: " << get(nodeTypeNProp, actual.first));

				ostringstream s1;

				s1 << "Memory sets: ";
				disp_state_maintainer->printMemSet(&s1, actual.second);
				LOG_DEBUG(logger, s1.str());
			}

			ContextStack context = updateContextForNode(actual);

			msgOutEdgeIter ep;
			for(ep = out_edges(actual.second, graph.second); ep.first != ep.second; ++ep.first) 
			{
				if(get(msgEdgeTypeEProp, *ep.first) != BackwardJump)
				{
					CFGMSGVPair next;
					next.second = target(*ep.first, graph.second);
					next.first = get(mappedCFGVertexNProp, next.second);
					processing.push_back(next);
					addContextForNode(next, context);
				}
			}
		}
	}

	calculateMemPenalty();

}

void DISPDataFlowAnalyzerMap::categorizeMemAccesses(void)
{

	msgVertexIter mvi;
	MSGVertex mv;
	CFGVertex cv;

	uint32_t disp_call_hit = 0;
	uint32_t disp_call_nc = 0;
	uint32_t disp_call_nc_load_bytes = 0;
	uint32_t disp_call_miss = 0;
	uint32_t disp_call_miss_load_bytes = 0;
	uint32_t disp_return_hit = 0;
	uint32_t disp_return_nc = 0;
	uint32_t disp_return_nc_load_bytes = 0;
	uint32_t disp_return_miss = 0;
	uint32_t disp_return_miss_load_bytes = 0;

	for(mvi = vertices(graph.second); mvi.first != mvi.second; ++mvi.first)
	{
		mv = *mvi.first;
		cv = get(mappedCFGVertexNProp, mv);

		if(get(nodeTypeNProp, cv) == Entry)
		{
			// NOTE: the super entry has to be considered, else the root function will not be considered 
//			if(mv != entry.second)
			{
				uint32_t function_addr = getFunctionAddress(cv);
				uint32_t function_load_size = disp_state_maintainer->getFunctionMemSize(function_addr);
				bool in_must = disp_state_maintainer->isInMust(function_addr, mv);
				bool in_may = disp_state_maintainer->isInMay(function_addr, mv);
				if(in_must)
				{
					disp_call_hit++;
				}
				else if(in_may)
				{
					disp_call_nc++;
					disp_call_nc_load_bytes += function_load_size;
				}
				else
				{
					disp_call_miss++;
					disp_call_miss_load_bytes += function_load_size;
				}
			}
		}
		else if(get(nodeTypeNProp, cv) == Exit)
		{
			// NOTE: the super exit has to be considered, else the root function will not be considered 
//			if(mv != exit.second)
			{
				ContextStack ctx = getContextForNode(mv);

				if(!ctx.empty())
				{
					ctx.pop();
				}

				if(ctx.empty())
				{
					//LOG_DEBUG(logger, "no DISP penalty for exit node: " << mv << " because it leaves to root function.");
				}
				else
				{

					uint32_t function_addr =  ctx.top();
					uint32_t function_load_size = disp_state_maintainer->getFunctionMemSize(function_addr);
					bool in_must = disp_state_maintainer->isInMust(function_addr, mv);
					bool in_may = disp_state_maintainer->isInMay(function_addr, mv);
					if(in_must)
					{
						disp_return_hit++;
					}
					else if(in_may)
					{
						disp_return_nc++;
						disp_return_nc_load_bytes += function_load_size;
					}
					else
					{
						disp_return_miss++;
						disp_return_miss_load_bytes += function_load_size;
					}
				}
			}
		}
	}

	LOG_INFO(logger, "Classified DISP accesses: always hit: " << disp_call_hit+disp_return_hit << " (call: " << disp_call_hit << " ret: " << disp_return_hit << ") always miss: " << disp_call_miss+disp_return_miss << " (call: " << disp_call_miss << " ret: " << disp_return_miss << " bytes loaded call: " <<  disp_call_miss_load_bytes << " ret: " << disp_return_miss_load_bytes << " ) NC: " << disp_call_nc+disp_return_nc << " (call: " << disp_call_nc << " ret: " << disp_return_nc << " bytes loaded call: " <<  disp_call_nc_load_bytes << " ret: " << disp_return_nc_load_bytes << " )");

}

void DISPDataFlowAnalyzerMap::calculateMemPenalty(void)
{
	// The mem penalty for calls is added to the out edge of the entry node resp. in edge of the first bb. For returns the mem penalty is added to the out edge of the last bb node resp. in edge of the exit node.

	msgVertexIter mvi;
	MSGVertex mv;
	CFGVertex cv;
	MSGEdge me;
	msgOutEdgeIter meo;
	msgInEdgeIter mei;


	for(mvi = vertices(graph.second); mvi.first != mvi.second; ++mvi.first)
	{
		mv = *mvi.first;
		cv = get(mappedCFGVertexNProp, mv);

		if(get(nodeTypeNProp, cv) == Entry)
		{
			// NOTE: the super entry has to be considered, else the root function will not be considered 
//			if(mv != entry.second)
			{
				uint32_t disp_cost = 0;
				uint32_t function_addr = getFunctionAddress(cv);
				bool in_must = disp_state_maintainer->isInMust(function_addr, mv);
				bool in_may = disp_state_maintainer->isInMay(function_addr, mv);
				if(in_must)
				{
					// no cost - its a hit
					disp_cost = getDispHitPenalty(true);
				}
				else if(in_may)
				{
					// add function miss costs
					disp_cost = getDispMissAndLoadPenalty(disp_state_maintainer->getFunctionMemSize(function_addr), CALL);
				}
				else
				{
					// add function miss costs
					disp_cost = getDispMissAndLoadPenalty(disp_state_maintainer->getFunctionMemSize(function_addr), CALL);
				}

				assert(out_degree(mv, graph.second) == 1);
				for(meo = out_edges(mv, graph.second); meo.first != meo.second; ++meo.first) 
				{
					me = *meo.first;
					put(dynamicMemPenaltyEProp, me, disp_cost);
				}

				ostringstream os;
				os << " {";
				disp_state_maintainer->printMemSet(&os, mv);
				os << "}";
				LOG_DEBUG(logger, "DISP penalty for entry node: " << mv << " called function with address 0x" << hex << function_addr << " with size: " << dec << functions->getFunctionSize(function_addr) << "/" << disp_state_maintainer->getFunctionMemSize(function_addr) << " is " << dec << disp_cost << " " << ((in_must)?("[HIT]"):("")) << ((!in_must && in_may)?("[NC]"):("")) << ((!in_must && !in_may)?("[MISS]"):("")) << os.str());

			}
		}
		else if(get(nodeTypeNProp, cv) == Exit)
		{
			// NOTE: the super exit has to be considered, else the root function will not be considered 
//			if(mv != exit.second)
			{
				uint32_t disp_cost = 0;

				ContextStack ctx =  getContextForNode(mv);

				if(!ctx.empty())
				{
					ctx.pop();
				}

				if(ctx.empty())
				{
					LOG_DEBUG(logger, "no DISP penalty for exit node: " << mv << " because it leaves to root function.");
				}
				else
				{

					uint32_t function_addr = ctx.top();
					bool in_must = disp_state_maintainer->isInMust(function_addr, mv);
					bool in_may = disp_state_maintainer->isInMay(function_addr, mv);
					if(in_must)
					{
						// no cost - its a hit
						disp_cost = getDispHitPenalty(false);
					}
					else if(in_may)
					{
						// add function miss costs
						disp_cost = getDispMissAndLoadPenalty(disp_state_maintainer->getFunctionMemSize(function_addr), RETURN);
					}
					else
					{
						// add function miss costs
						disp_cost = getDispMissAndLoadPenalty(disp_state_maintainer->getFunctionMemSize(function_addr), RETURN);
					}

					assert(in_degree(mv, graph.second) == 1);
					for(mei = in_edges(mv, graph.second); mei.first != mei.second; ++mei.first) 
					{
						me = *mei.first;
						put(dynamicMemPenaltyEProp, me, disp_cost);
					}

					ostringstream os;
					os << " {";
					disp_state_maintainer->printMemSet(&os, mv);
					os << "}";
					LOG_DEBUG(logger, "DISP penalty for exit node: " << mv << " returning to function with address 0x" << hex << function_addr << " with size: " << dec << functions->getFunctionSize(function_addr) << "/" << disp_state_maintainer->getFunctionMemSize(function_addr) << " is " << dec << disp_cost << " " << ((in_must)?("[HIT]"):("")) << ((!in_must && in_may)?("[NC]"):("")) << ((!in_must && !in_may)?("[MISS]"):("")) << os.str());
				}
			}
		}
	}

}

bool DISPDataFlowAnalyzerMap::isPredecessorMemStateKnown(CFGMSGVPair node)
{
	// shortcut for entry node
	if(node.second == entry.second)
	{
		assert(in_degree(node.second, graph.second) == 0);
		return true;
	}

	// checks if all nodes on incomming edges have a valid cache state
	msgInEdgeIter ep;
	assert(in_degree(node.second, graph.second) > 0);
	for(ep = in_edges(node.second, graph.second); ep.first != ep.second; ++ep.first) 
	{
		MSGEdge e = *ep.first;
		// ignore back edges, because the memory effects of loops are taken into account by unrolling the first loop iteration in the msg
		if(get(msgEdgeTypeEProp, e) != BackwardJump)
		{
			if(get(memStateValNProp, source(e, graph.second))==false)
			{
				return false;
			}
		}
	}
	return true;
}

void DISPDataFlowAnalyzerMap::joinAndSetInitialMemState(CFGMSGVPair node)
{
	msgInEdgeIter ep;
	vector<MSGVertex> prev_states;
	for(ep = in_edges(node.second, graph.second); ep.first != ep.second; ++ep.first) 
	{
		MSGEdge e = *ep.first;
		// ignore back edges
		if(get(msgEdgeTypeEProp, e) != BackwardJump)
		{
			MSGVertex predecessor_m = source(e, graph.second);
			CFGVertex predecessor_c = get(mappedCFGVertexNProp, predecessor_m);
			if(get(nodeTypeNProp, predecessor_c) == Entry)
			{
				// cannot happen, since functions are inlined
				LOG_ERROR(logger, "No join possible for node: " << node.second << " " << get(startAddrStringNProp, node.first) << " because multiple entry points and entry node as predecessor: " << predecessor_m << " " << get(startAddrStringNProp, predecessor_c));
				assert(false);
			}
			else if(get(nodeTypeNProp, predecessor_c) == Exit)
			{
				// cannot happen, since functions are inlined
				LOG_ERROR(logger, "No join possible for node: " << node.second << " " << get(startAddrStringNProp, node.first) << " because multiple entry points and exit node as predecessor: " << predecessor_m << " " << get(startAddrStringNProp, predecessor_c));
				assert(false);
			}
			else
			{
				// nothing to be done here
			}
			prev_states.push_back(predecessor_m);
		}
	}
	LOG_DEBUG(logger, "Joining mem states of: " << get(startAddrStringNProp, node.first) << "/" << node.second);
	disp_state_maintainer->join(&prev_states, node.second);
	put(memStateValNProp, node.second, true);

}

void DISPDataFlowAnalyzerMap::setInitialMemState(CFGMSGVPair node)
{
	if(node == entry)
	{
		disp_state_maintainer->setBlankMemState(node.second);
		put(memStateValNProp, node.second, true);
		ContextStack context;
		context.clear();
		addContextForNode(node, context);
	}
	else
	{
		assert(getForwardInEdgeCount(node.second) == 1);
		msgInEdgeIter ep = in_edges(node.second, graph.second);
	
		// if there is a back edge in the in_edges ignore it!
		// XXX this is not beautiful!
		while(get(msgEdgeTypeEProp, *ep.first) == BackwardJump)
		{
			++ep.first;
		}
	
		MSGEdge e = *ep.first;

		MSGVertex predecessor_msg = source(e, graph.second);

		assert(get(memStateValNProp, predecessor_msg));

		CFGMSGVPair predecessor;
		predecessor.second = predecessor_msg;
		predecessor.first = get(mappedCFGVertexNProp, predecessor_msg);


		node_type_t predecessor_type = get(nodeTypeNProp, predecessor.first);
		if(predecessor_type == Entry)
		{
			// NOTE: the super entry has to be considered, else the root function will not be considered 
		//	if(predecessor != entry) // the super entry is ignored
			{
				LOG_DEBUG(logger, "Updating DISP content on Entry node.");
				disp_state_maintainer->update(predecessor.second, node.second, getFunctionAddress(predecessor), CALL, getContextForNode(predecessor).top());
			}
		}
		else if(predecessor_type == Exit)
		{
			ContextStack ctx =  getContextForNode(predecessor);

			// strip the context of the current function, that is exiting
			uint32_t callee = ctx.pop();

			if(!ctx.empty())
			{
				LOG_DEBUG(logger, "Updating DISP content on Exit node.");
				disp_state_maintainer->update(predecessor.second, node.second, ctx.top(), RETURN, callee);
			}
			else
			{
				LOG_DEBUG(logger, "Returning to root function, no changes in DISP");
			}
		}
		else
		{
			disp_state_maintainer->transfer(predecessor.second, node.second);
			// do nothing
		}
		put(memStateValNProp, node.second, true);
	}
}

ContextStack DISPDataFlowAnalyzerMap::getContextForNode(MSGVertex mnode)
{
	ContextStack context;
	NodeContextMap::iterator pos;

	pos = ctxMap.find(mnode);

	if(pos == ctxMap.end())
	{
		LOG_ERROR(logger, "No context found");
	}
	else
	{
		context = pos->second;
	}

	return context;

}

ContextStack DISPDataFlowAnalyzerMap::getContextForNode(CFGMSGVPair node)
{
	return getContextForNode(node.second);
}


ContextStack DISPDataFlowAnalyzerMap::updateContextForNode(CFGMSGVPair node)
{
	ContextStack context;
	context = getContextForNode(node);

	if(get(nodeTypeNProp, node.first) == Entry)
	{
		context.push(getFunctionAddress(node));

		if(node.second == entry.second) 
		{
			// setted the context of the super entry:
			LOG_DEBUG(logger, "Setting root context for super_entry node.");
		}
	}
	else if(get(nodeTypeNProp, node.first) == Exit)
	{
		assert(!context.empty());
		context.pop();
	}

	return context;
}

void DISPDataFlowAnalyzerMap::addContextForNode(CFGMSGVPair node, ContextStack context)
{
	NodeContextMap::iterator pos;
	bool ins_bool;


	if(logger->isDebugEnabled())
	{
		string s;
		if(context.empty())
		{
			s = " empty";
		}
		else
		{
			s = context.toString();
		}
		LOG_DEBUG(logger, "Inserting: " << node.second << " with context:" << s << " to context map");
	}

	
	pos = ctxMap.find(node.second);

	if(pos == ctxMap.end())
	{
		tie(pos, ins_bool) = ctxMap.insert(make_pair(node.second, context));
		assert(ins_bool);
	}
	else
	{
		// context was already inserted.
	}


}

uint32_t DISPDataFlowAnalyzerMap::getFunctionAddress(CFGVertex cnode)
{
	assert(get(nodeTypeNProp, cnode) == Entry);
	assert(out_degree(cnode, graph.first)==1);

	cfgOutEdgeIter epo = out_edges(cnode, graph.first);
	CFGEdge e = *epo.first;
	CFGVertex tgt = target(e, graph.first);

	return get(startAddrNProp, tgt);
}

uint32_t DISPDataFlowAnalyzerMap::getFunctionAddress(CFGMSGVPair node)
{
	return getFunctionAddress(node.first);
}

uint32_t DISPDataFlowAnalyzerMap::getForwardInEdgeCount(MSGVertex v)
{
	msgInEdgeIter ep;
	uint32_t fw_edge_count =0;
	for(ep = in_edges(v, graph.second); ep.first != ep.second; ++ep.first) 
	{
		if(get(msgEdgeTypeEProp, *ep.first) != BackwardJump)
		{
			fw_edge_count++;
		}
	}
	return fw_edge_count;
}


void DISPDataFlowAnalyzerMap::setMemParameters()
{
	assert((disp_parameters.size != 0) && (disp_parameters.block_size != 0));


	assert(disp_state_maintainer == NULL);

	switch(disp_parameters.rpol)
	{
		case FIFO:
			{
				LOG_DEBUG(logger, "Setting up DISP abstract state maintainer with FIFO as replacement policy");
				disp_state_maintainer = new BFDISPStateMaintainerFIFO(disp_parameters, functions);
				break;
			}
		case STACK:
		{
				LOG_DEBUG(logger, "Setting up DISP abstract state maintainer with STACK as replacement policy");
				disp_state_maintainer = new BFDISPStateMaintainerStack(disp_parameters, functions);
				break;
		}
		default:
			{
				assert(false);
			}

	}
}

uint32_t DISPDataFlowAnalyzerMap::getDispHitPenalty(bool call)
{
	// the minimal call/return latency is considered here (when memory is ONCHIP, i.e. getCallLatency(true) and getReturnLatency(true)) to maximise the penalty
	uint32_t call_ret_handling_pipeline_latency = (call)?(arch_cfg->getCallLatency(true)+1):(arch_cfg->getReturnLatency(true)+1); // needed cycles to call a or return to a function
	uint32_t disp_hit_penalty = arch_cfg->getDISPCtrlHitCycles(); // needed cycles to handle a disp hit
	
	// if the disp hit penalty is smaller than the time needed by the pipeline to handle the call/return, no additional penalty is added by the DISP. Otherwise the difference has to be taken as penalty into account.
	if(disp_hit_penalty > call_ret_handling_pipeline_latency)
	{
		disp_hit_penalty -= call_ret_handling_pipeline_latency;
	}
	else
	{
		disp_hit_penalty = 0;
	}
	return disp_hit_penalty;
}

uint32_t DISPDataFlowAnalyzerMap::getDispMissAndLoadPenalty(uint32_t function_size_in_mem, activation_type_t act)
{
	uint32_t block_load_cost = arch_cfg->getMissLatency(DISP)+1;
	// the minimal call/return latency is considered here (when memory is ONCHIP, i.e. getCallLatency(true) and getReturnLatency(true)) to maximise the penalty
	uint32_t call_ret_handling_pipeline_latency = (act==CALL)?(arch_cfg->getCallLatency(true)+1):(arch_cfg->getReturnLatency(true)+1);
	uint32_t function_load_cycles = (function_size_in_mem/disp_parameters.block_size) * block_load_cost;
	uint32_t disp_miss_penalty = arch_cfg->getDISPCtrlMissCycles();
	uint32_t func_load_penalty;
	// the function loading is done while the the call/ret handling is done, if the fetch memory is separated this in done in the slipstream of the function load.
	if(arch_cfg->isFetchMemIndependent())
	{
		if(function_load_cycles + disp_miss_penalty > call_ret_handling_pipeline_latency)
		{
			func_load_penalty = function_load_cycles + disp_miss_penalty - call_ret_handling_pipeline_latency;
		}
		else
		{
			func_load_penalty = 0;
		}
	}
	else
	{
		func_load_penalty = function_load_cycles + disp_miss_penalty;
	}
	return func_load_penalty;
}

uint64_t DISPDataFlowAnalyzerMap::getMemoryStateCount(void)
{
	return disp_state_maintainer->getMemoryStateCount();
}

uint64_t DISPDataFlowAnalyzerMap::getRepresentationStateCount(void)
{
	return disp_state_maintainer->getRepresentationStateCount();
}

uint64_t DISPDataFlowAnalyzerMap::getUsedMemSize(void)
{
	return disp_state_maintainer->getUsedSize();
}

uint64_t DISPDataFlowAnalyzerMap::getUsedMemReferences(void)
{
	return disp_state_maintainer->getUsedMemReferences();
}
