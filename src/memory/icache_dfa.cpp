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
#include "icache_dfa.hpp"
#include "arch_cfg_factory.hpp"

// for setw() and setfill()
using std::setw;
using std::setfill;

/// cache_line_size has to be a power of two
#define calc_cache_line_addr(a) (a & (~(cache_parameters.line_size-1)))


#define FIX_STATE_UPDATE_BUG // to mark that this fix is untested
//#define DEBUG_STATE_UPDATE_BUG // code to debug the state update bug

LoggerPtr ICacheDataFlowAnalyzer::logger(Logger::getLogger("ICacheDataFlowAnalyzer"));

ICacheDataFlowAnalyzer::ICacheDataFlowAnalyzer(CFGMSGPair graph_pair, CFGMSGVPair entry_node, CFGMSGVPair exit_node)
{
	graph = graph_pair;
	entry = entry_node;
	exit = exit_node;

	// get node property structures for cfg
	nodeTypeNProp = get(nodetype_t(), graph.first);
	startAddrNProp = get(startaddr_t(), graph.first);
	startAddrStringNProp = get(startaddrstring_t(), graph.first);
	bbSizeNProp = get(bbsize_t(), graph.first);

	// get node property structures for msg
	memStateNProp = get(mem_state_t(), graph.second);
	memStateValNProp = get(mem_state_valid_t(), graph.second);
	mappedCFGVertexNProp = get(cfg_vertex_t(), graph.second);

	m_cacheHitsNProp = get(mem_cache_hits_t(), graph.second);
	m_cacheMissesNProp = get(mem_cache_misses_t(), graph.second);
	m_cacheNCsNProp = get(mem_cache_ncs_t(), graph.second);

	// get edge property structures for msg
	msgEdgeTypeEProp = get(msg_edgetype_t(), graph.second);
	dynamicMemPenaltyEProp = get(dynamic_mem_penalty_t(), graph.second);

	arch_cfg = ArchConfigFactory::getInstance()->getArchConfigObject();

	if((architecture_t)Configuration::getInstance()->getUint(CONF_ARCHITECTURE) == CARCORE)
	{
		if(!(dynamic_cast<CarCoreConfig *>(arch_cfg)->isFetchOptBranchAhead()))
		{
			LOG_WARN(logger, "Cache analysis will not be correct, iff the processor is allowed to fetch beyond an branch");
		}
	}

	cache_state_maintainer = NULL;

	MemoryParameters mp;
	cache_parameters = mp.getCacheParameters();
	setCacheParameters();
}

ICacheDataFlowAnalyzer::ICacheDataFlowAnalyzer(CFGMSGPair graph_pair, CFGMSGVPair entry_node, CFGMSGVPair exit_node, uint32_t mem_size)
{
	graph = graph_pair;
	entry = entry_node;
	exit = exit_node;

	// get node property structures for cfg
	nodeTypeNProp = get(nodetype_t(), graph.first);
	startAddrNProp = get(startaddr_t(), graph.first);
	startAddrStringNProp = get(startaddrstring_t(), graph.first);
	bbSizeNProp = get(bbsize_t(), graph.first);

	// get node property structures for msg
	memStateNProp = get(mem_state_t(), graph.second);
	memStateValNProp = get(mem_state_valid_t(), graph.second);
	mappedCFGVertexNProp = get(cfg_vertex_t(), graph.second);

	m_cacheHitsNProp = get(mem_cache_hits_t(), graph.second);
	m_cacheMissesNProp = get(mem_cache_misses_t(), graph.second);
	m_cacheNCsNProp = get(mem_cache_ncs_t(), graph.second);

	// get edge property structures for msg
	msgEdgeTypeEProp = get(msg_edgetype_t(), graph.second);
	dynamicMemPenaltyEProp = get(dynamic_mem_penalty_t(), graph.second);

	arch_cfg = ArchConfigFactory::getInstance()->getArchConfigObject();

	if((architecture_t)Configuration::getInstance()->getUint(CONF_ARCHITECTURE) == CARCORE)
	{
		if(!(dynamic_cast<CarCoreConfig *>(arch_cfg)->isFetchOptBranchAhead()))
		{
			LOG_WARN(logger, "Cache analysis will not be correct, iff the processor is allowed to fetch beyond an branch");
		}
	}

	cache_state_maintainer = NULL;

	MemoryParameters mp;
	cache_parameters = mp.getCacheParameters(mem_size);
	setCacheParameters();
}



ICacheDataFlowAnalyzer::~ICacheDataFlowAnalyzer()
{
	delete(cache_state_maintainer);
}

void ICacheDataFlowAnalyzer::analyzeCache(void)
{
	vector<CFGMSGVPair> processing;
	processing.push_back(entry);

	assert(cache_parameters.line_no != 0);

	while(processing.size() != 0)
	{
		CFGMSGVPair actual = processing.back();
		processing.pop_back();

		if(isPredecessorMemStateKnown(actual) && (actual != exit))
		{
			// this code is used if the mem_state is representing the state of the cache _before_ basic block execution
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
				LOG_DEBUG(logger, "Initial memory state of node: " << actual.second << " with addr: 0x" << hex << get(startAddrNProp, actual.first) << " state is valid: " << get(memStateValNProp, actual.second) << " in_degree: " << dec << in_degree(actual.second, graph.second) );

				ostringstream s1, s2;
				abs_mem_set_t state = get(memStateNProp, actual.second);

				s1 << "Must set: ";
				cache_state_maintainer->printMemSet(&s1, &state.must_set);
				LOG_DEBUG(logger, s1.str());

				s2 << "May set: ";
				cache_state_maintainer->printMemSet(&s2, &state.may_set);
				LOG_DEBUG(logger, s2.str());


			}

			msgOutEdgeIter ep;
			for(ep = out_edges(actual.second, graph.second); ep.first != ep.second; ++ep.first) 
			{
				if(get(msgEdgeTypeEProp, *ep.first) != BackwardJump)
				{
					CFGMSGVPair next;
					next.second = target(*ep.first, graph.second);
					next.first = get(mappedCFGVertexNProp, next.second);
					processing.push_back(next);
				}
			}
		}
	}

	calculateCachePenalty();

}

bool ICacheDataFlowAnalyzer::isPredecessorMemStateKnown(CFGMSGVPair node)
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

void ICacheDataFlowAnalyzer::joinAndSetInitialMemState(CFGMSGVPair node)
{
	msgInEdgeIter ep;
	vector<abs_mem_set_t> prev_states;
	for(ep = in_edges(node.second, graph.second); ep.first != ep.second; ++ep.first) 
	{
		MSGEdge e = *ep.first;
		// ignore back edges
		if(get(msgEdgeTypeEProp, e) != BackwardJump)
		{
			MSGVertex predecessor = source(e, graph.second);
			abs_mem_set_t tmp = get(memStateNProp, predecessor);
			vector<uint32_t> cl_of_previous = getCacheLinesOfCFGNode(get(mappedCFGVertexNProp, predecessor));
			tmp = cache_state_maintainer->update(&tmp, &cl_of_previous, false);
			prev_states.push_back(tmp);
		}
	}
	LOG_DEBUG(logger, "Joining mem states of: " << get(startAddrStringNProp, node.first) << "/" << node.second);
	abs_mem_set_t new_state = cache_state_maintainer->join(&prev_states);
	put(memStateNProp, node.second, new_state);
	put(memStateValNProp, node.second, true);

}

void ICacheDataFlowAnalyzer::setInitialMemState(CFGMSGVPair node)
{
	if(node == entry)
	{
		abs_mem_set_t initial_state;
		cache_state_maintainer->setBlankMemState(&initial_state.must_set, &initial_state.may_set);
		put(memStateNProp, node.second, initial_state);
		put(memStateValNProp, node.second, true);
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

		MSGVertex predecessor = source(e, graph.second);
		abs_mem_set_t prev_state = get(memStateNProp, predecessor);
		vector<uint32_t> cl_of_previous = getCacheLinesOfCFGNode(get(mappedCFGVertexNProp, predecessor));
		prev_state = cache_state_maintainer->update(&prev_state, &cl_of_previous, true);
		put(memStateNProp, node.second, prev_state);
		put(memStateValNProp, node.second, true);
	}
}

vector<uint32_t> ICacheDataFlowAnalyzer::getCacheLinesOfCFGNode(CFGVertex node)
{
	vector<uint32_t> cache_lines;
	if(get(nodeTypeNProp, node) == BasicBlock)
	{
		if(cache_parameters.use_bbs_instead_of_cache_lines)
		{
			cache_lines.push_back(get(startAddrNProp, node));
		}
		else
		{
			uint32_t start_addr = get(startAddrNProp, node);
			uint32_t end_addr = get(startAddrNProp, node) + get(bbSizeNProp, node);
			uint32_t current_line_addr = start_addr & (~(cache_parameters.line_size-1));

			ostringstream s;

			while(current_line_addr < end_addr)
			{
				cache_lines.push_back(current_line_addr);
				if(logger->isDebugEnabled())
				{
					s << " 0x" << hex << current_line_addr;
				}
				current_line_addr += cache_parameters.line_size;
			}

			if(logger->isDebugEnabled())
			{
				LOG_DEBUG(logger, "Calculated cache line addresses of bb: 0x" << hex << start_addr << " to 0x" << end_addr << ":" << s.str())
			}
		}
	}
	return cache_lines;
}


void ICacheDataFlowAnalyzer::setCacheParameters(void)
{
	assert((cache_parameters.size != 0) && (cache_parameters.line_size != 0));

	LOG_DEBUG(logger, "Icache size is: " << cache_parameters.size << " bytes. Line size is: " << cache_parameters.line_size << " bytes (2^" << cache_parameters.line_size_bit << ") . Number of cache lines is: " << cache_parameters.line_no << "(ways: " << cache_parameters.ways << " sets: " << cache_parameters.sets << " ) Replacement policy is: " << cache_parameters.rpol);

	if(cache_state_maintainer != NULL)
	{
		LOG_WARN(logger, "Multiple initializations of CacheStateMaintainer!");
	}

	switch(cache_parameters.rpol)
	{
		case LRU:
			{
				cache_state_maintainer = new LRUCacheStateMaintainer(cache_parameters);
				break;
			}
		case DIRECT_MAPPED:
			{
				cache_state_maintainer = new DirectMappedCacheStateMaintainer(cache_parameters);
				break;
			}
		default:
			{
				LOG_ERROR(logger, "Wrong replacement policy: " << cache_parameters.rpol << " is not implemented yet.")
				assert(false);
			}
	}
}

void ICacheDataFlowAnalyzer::categorizeCacheAccesses(void)
{
	msgVertexIter vp;
	MSGVertex mv;
	CFGVertex cv;
	uint32_t cache_hit = 0;
	uint32_t cache_miss = 0;
	uint32_t cache_nc = 0;
	vector<uint32_t> unique_lines;

#ifdef DEBUG_STATE_UPDATE_BUG
	bool debug = false;
#endif

	for(vp = vertices(graph.second); vp.first != vp.second; ++vp.first)
	{
		mv = *vp.first;
		cv = get(mappedCFGVertexNProp, mv);

		if(get(nodeTypeNProp, cv) == BasicBlock)
		{
			abs_mem_set_t mem = get(memStateNProp, mv);
			vector<uint32_t> cache_addrs = getCacheLinesOfCFGNode(cv);
			for(uint32_t i = 0; i < cache_addrs.size(); i++)
			{
				bool in_must = cache_state_maintainer->isInSet(cache_addrs[i], &mem.must_set);
#ifdef DEBUG_STATE_UPDATE_BUG
				if((debug == false) && (in_must == true) && (i>0))
				{
					LOG_DEBUG(logger, "CHECK CASE");
				}
				debug = in_must;
#endif
				bool in_may = cache_state_maintainer->isInSet(cache_addrs[i], &mem.may_set);
				if(in_must)
				{
					cache_hit++;
				}
				else if(in_may)
				{
					cache_nc++;
				}
				else
				{
					cache_miss++;
				}
#ifdef FIX_STATE_UPDATE_BUG
				// update the memory state after considering the cache address
				mem = cache_state_maintainer->update(&mem, cache_addrs[i], false);
#endif

				// add all accessed cache addresses to a list
				bool found = false;
				for(uint32_t j = 0; (j < unique_lines.size())&&(!found); j++)
				{
					if(unique_lines[j] == cache_addrs[i])
					{
						found = true;
					}
				}
				if(!found)
				{
					unique_lines.push_back(cache_addrs[i]);
				}
			}
		}
	}
	
	LOG_INFO(logger, "Classified cache accesses: always hit: " << cache_hit << " always miss: " << cache_miss << " NC: " << cache_nc << " number of all cache line addresses: " << unique_lines.size());
}

void ICacheDataFlowAnalyzer::calculateCachePenalty(void)
{
	msgVertexIter mvi;
	MSGVertex mv;
	CFGVertex cv;
	MSGEdge me;
	msgOutEdgeIter meo;


	for(mvi = vertices(graph.second); mvi.first != mvi.second; ++mvi.first)
	{
		mv = *mvi.first;
		cv = get(mappedCFGVertexNProp, mv);

		if(get(nodeTypeNProp, cv) == BasicBlock)
		{
			abs_mem_set_t mem = get(memStateNProp, mv);
			uint32_t cache_cost = 0;
			vector<uint32_t> cache_addrs = getCacheLinesOfCFGNode(cv);
			uint16_t hits = 0;
			uint16_t misses = 0;
			uint16_t ncs = 0;
#ifdef DEBUG_STATE_UPDATE_BUG
			bool debug = false;
#endif


			ostringstream os;
			os << " { MUST: ";
			cache_state_maintainer->printMemSet(&os, &mem.must_set);
			os << ", MAY: ";
			cache_state_maintainer->printMemSet(&os, &mem.may_set);
			os << "}";


			for(uint32_t i = 0; i < cache_addrs.size(); i++)
			{
				bool in_must = cache_state_maintainer->isInSet(cache_addrs[i], &mem.must_set);
#ifdef DEBUG_STATE_UPDATE_BUG
				if((debug == false) && (in_must == true) && (i>0))
				{
					LOG_DEBUG(logger, "CHECK CASE");
				}
				debug = in_must;
#endif
				bool in_may = cache_state_maintainer->isInSet(cache_addrs[i], &mem.may_set);
				if(in_must)
				{
					// no cost - its a hit
					hits++;
				}
				else if(in_may)
				{
					// add cache miss costs
					cache_cost += getCacheMissCost();
					ncs++;
				}
				else
				{
					// add cache miss costs
					cache_cost += getCacheMissCost();
					misses++;
				}
#ifdef FIX_STATE_UPDATE_BUG
				// update the memory state after considering the cache address
				mem = cache_state_maintainer->update(&mem, cache_addrs[i], false);
#endif
			}

#ifdef DEBUG_STATE_UPDATE_BUG
			abs_mem_set_t mem2 = get(memStateNProp, mv);
			bool must_differ = false;
			bool may_differ = false;
			if(mem.must_set.size() == mem2.must_set.size())
			{
				for(uint32_t i = 0; i < mem.must_set.size(); i++)
				{
					if(mem2.must_set[i].address != mem.must_set[i].address)
					{
						must_differ = true;
					}
				}
			}
			else
			{
				must_differ = true;
			}
			if(mem.may_set.size() == mem2.may_set.size())
			{
				for(uint32_t i = 0; i < mem.may_set.size(); i++)
				{
					if(mem2.may_set[i].address != mem.may_set[i].address)
					{
						may_differ = true;
					}
				}
			}
			else
			{
				may_differ = true;
			}
			LOG_DEBUG(logger, "MUST: " << ((must_differ)?("differs"):("same")) << " MAY: " << ((may_differ)?("differs"):("same")));
#endif

			for(meo = out_edges(mv, graph.second); meo.first != meo.second; ++meo.first) 
			{
				me = *meo.first;
				put(dynamicMemPenaltyEProp, me, cache_cost);
			}

			put(m_cacheHitsNProp, mv, hits);
			put(m_cacheMissesNProp, mv, misses);
			put(m_cacheNCsNProp, mv, ncs);

//			string bb_desc = get(startAddrStringNProp, cv);
//			stringstream s;
//			s << "(H:" << hits << "|M:" << misses << "|N:" << ncs << ")";
//			bb_desc += s.str();
//			put(startAddrStringNProp, cv, bb_desc);


			LOG_DEBUG(logger, "Cache penalty for node: " << mv << " with start address 0x" << hex << get(startAddrNProp, cv) << " is " << dec << cache_cost << " (H:" << hits << "|M:" << misses << "|N:" << ncs << ")" << os.str());

		}
	}
	
}

uint32_t ICacheDataFlowAnalyzer::getCacheMissCost()
{
	return arch_cfg->getMissLatency(ICACHE)+1;
}

uint32_t ICacheDataFlowAnalyzer::getForwardInEdgeCount(MSGVertex v)
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

uint64_t ICacheDataFlowAnalyzer::getMemoryStateCount(void)
{
	return cache_state_maintainer->getMemoryStateCount();
}

uint64_t ICacheDataFlowAnalyzer::getRepresentationStateCount(void)
{
	return cache_state_maintainer->getRepresentationStateCount();
}

uint64_t ICacheDataFlowAnalyzer::getUsedMemSize(void)
{
	return cache_state_maintainer->getUsedSize();
}

uint64_t ICacheDataFlowAnalyzer::getUsedMemReferences(void)
{
	return cache_state_maintainer->getUsedMemReferences();
}
