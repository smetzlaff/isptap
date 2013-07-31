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
#include "wcpath_export.hpp"
#include "dlp_factory.hpp"

// FIXME:
// - dynamic worst case paths are not supported, e.g. in the first iteration of a loop path A is taken and for further iterations path B
// - not all kinds of loops are supported, e.g. loops that are injected by a forward edge cannot be identified and traversed correctly

LoggerPtr WCPathExporter::logger(Logger::getLogger("WCPathExporter"));

WCPathExporter::WCPathExporter(ControlFlowGraph cfggraph, CFGVertex cfg_entry, CFGVertex cfg_exit, string wcpath_out_file, string wchist_out_file) : CFGLoopHelper(cfggraph, cfg_entry, cfg_exit)
{
	wcpath_output_file_name = wcpath_out_file;
	wchist_output_file_name = wchist_out_file;


	dlp = DLPFactory::getInstance()->getDLPObject();

	// get the properties for the graph (nodes and edges)
	bbCodeNProp = get(bbcode_t(), cfg);
	actEProp = get(activation_t(), cfg);
#ifdef PRINT_CACHE_STATS
	cacheHitsNProp = get(cache_hits_t(), cfg);
	cacheMissesNProp = get(cache_misses_t(), cfg);
	cacheNCsNProp = get(cache_ncs_t(), cfg);
#endif
}

WCPathExporter::~WCPathExporter()
{
	if(wcpath_output_file.is_open())
	{
		wcpath_output_file.close();
	}
	if(wchist_output_file.is_open())
	{
		wchist_output_file.close();
	}
}

void WCPathExporter::traverseAndPrintWCPath(void)
{
	wcpath_output_file.open(wcpath_output_file_name.c_str());
	assert(wcpath_output_file.is_open());

	ContextStack ctx;
	traverseWCPathForSequentialCode(cfg_entry, cfg_exit, 1, &ctx);

	wcpath_output_file.close();
}

void WCPathExporter::printWCHist(void)
{
	wchist_output_file.open(wchist_output_file_name.c_str());
	assert(wchist_output_file.is_open());

	// using map here, since the vivu transformation splits basic block by context, here these contextes are merges for the histogram
	map<uint32_t, uint32_t> bb_map;
	map<uint32_t, uint32_t>::iterator bb_map_it;

#ifdef PRINT_CACHE_STATS
	// map for cache statistics
	map<uint32_t, cache_hm_stat_t> bb_cache_map;
	map<uint32_t, cache_hm_stat_t>::iterator bb_cache_map_it;
#endif

	bool ins_bool;

	cfgVertexIter vp;
	CFGVertex v;
	cfgInEdgeIter epi;
	CFGEdge e;

	for (vp = vertices(cfg); vp.first != vp.second; ++vp.first)
	{
		v = *vp.first;
		if(get(nodeTypeNProp, v) == BasicBlock)
		{
			uint32_t activation_count = 0;
			for(epi = in_edges(v, cfg); epi.first != epi.second; ++epi.first)
			{
				e = *epi.first;
				activation_count += get(actEProp, e);
			}

			uint32_t bb_addr = get(startAddrNProp, v);
			
			bb_map_it = bb_map.find(bb_addr);
			if(bb_map_it == bb_map.end())
			{
				tie(bb_map_it, ins_bool) = bb_map.insert(make_pair(bb_addr,activation_count));
			}
			else
			{
				bb_map_it->second += activation_count;
			}

#ifdef PRINT_CACHE_STATS
			uint16_t hit = get(cacheHitsNProp,v);
			uint16_t miss = get(cacheMissesNProp,v);
			uint16_t nc = get(cacheNCsNProp,v);
			cache_hm_stat_t tmp;
			tmp.hit = hit*activation_count;
			tmp.miss = miss*activation_count;
			tmp.nc = nc*activation_count;
			bb_cache_map_it = bb_cache_map.find(bb_addr);
			if(bb_cache_map_it == bb_cache_map.end())
			{
				tie(bb_cache_map_it, ins_bool) = bb_cache_map.insert(make_pair(bb_addr,tmp));
			}
			else
			{
				bb_cache_map_it->second.hit += tmp.hit;
				bb_cache_map_it->second.miss += tmp.miss;
				bb_cache_map_it->second.nc += tmp.nc;
			}
#endif

		}
	}

	bb_map_it = bb_map.begin();
	while(bb_map_it != bb_map.end())
	{
		wchist_output_file << hex << "0x" << bb_map_it->first << "\t" << dec << bb_map_it->second;
#ifdef PRINT_CACHE_STATS
		bb_cache_map_it = bb_cache_map.find(bb_map_it->first);
		if(bb_cache_map_it != bb_cache_map.end())
		{
			wchist_output_file << "\t" << bb_cache_map_it->second.hit <<  "\t" << bb_cache_map_it->second.miss << "\t" << bb_cache_map_it->second.nc;
		}
#endif
		wchist_output_file << endl;
			
		bb_map_it++;
	}

	wchist_output_file.close();
}

void WCPathExporter::traverseWCPathForSequentialCode(CFGVertex start, CFGVertex end, uint32_t count, ContextStack *context)
{
	CFGVertex v;
	CFGEdge e_f, e_b;
	cfgOutEdgeIter epo;

	// intialisize e_f and e_b to suppress compiler warnings:
	e_f = CFGEdge();
	e_b = CFGEdge();

	for(uint32_t i = 0; i < count; i++)
	{
		v = start;
		LOG_DEBUG(logger, i << " / " << count);

		while(v != end)
		{

			if(get(nodeTypeNProp, v) == CallPoint)
			{
				LOG_DEBUG(logger, "Found entry node: ctx is 0x" << hex << get(endAddrNProp, v));
				context->push(get(endAddrNProp, v));
			}

			printNode(v);


			bool found_fw_edge = false;
			bool found_bw_edge = false;

			for(epo = out_edges(v, cfg); epo.first != epo.second; ++epo.first) 
			{
				CFGEdge e = *epo.first;
				if(get(actEProp, e) != 0)
				{
					if(get(edgeTypeEProp, e) == BackwardJump)
					{
						if(target(e,cfg) != start) // forbid to find the back edge that caused the printSCC() call
						{
							if(found_bw_edge == true)
							{
								LOG_ERROR(logger, "There was already one backward edge found, dynammic WCPaths are currently not supported!.");
							}
							// TODO: The WCPathExporter currently does not support WCPathes that are not fix, e.g. the first iteration of a loop runs into an intialization but the next iteration does not, as e.g. for the CRC benchmark from the maelardalen suite

							found_bw_edge = true;
							e_b = e;
						}
					}
					else
					{
						if(get(nodeTypeNProp, v) == Exit)
						{
							uint32_t call_point = context->top();
							if(get(endAddrNProp, target(e, cfg)) == call_point)
							{
								assert(found_fw_edge == false);
								LOG_DEBUG(logger, "Found for call point 0x" << hex << call_point << " the corresponding return point");
								context->pop();
								found_fw_edge = true;
								e_f = e;
							}
						}
						else
						{
							if(found_fw_edge == true)
							{
								LOG_ERROR(logger, "There was already one forward edge found, dynammic WCPaths are currently not supported!.");
							}
							assert(found_fw_edge == false);
							// TODO: The WCPathExporter currently does not support WCPathes that are not fix, e.g. the first iteration of a loop runs into an intialization but the next iteration does not, as e.g. for the CRC benchmark from the maelardalen suite
							found_fw_edge = true;
							e_f = e;
						}
					}
				}
			}

			assert(found_fw_edge || found_bw_edge); // If this happens, either the graph has no activation_t properties that mark the WC-path (generated by ILPGenerator) or something weird has happend.


			// TODO: Loops without injecting back edges cannot be traversed correctly. Adapt the WCP export to also support this type of loops.
			if(found_fw_edge && found_bw_edge)
			{
				int32_t loop_bound = getLoopBoundForLoopHead(target(e_b, cfg), e_b);
				if(loop_bound > 0)
				{
					LOG_DEBUG(logger, "Printing loop: from " << get(startAddrStringNProp, v) << " to " <<  get(startAddrStringNProp, target(e_b, cfg)) << " " << loop_bound << " times");
					// circle through nodes, since loop was found
					traverseWCPathForSequentialCode(target(e_b, cfg), v, loop_bound, context);
					LOG_DEBUG(logger, "Returned from loop  from " << get(startAddrStringNProp, v) << " to " <<  get(startAddrStringNProp, target(e_b, cfg)));
				}
				else
				{
					LOG_ERROR(logger, "Loop bound for loop:  from " << get(startAddrStringNProp, v) << " to " <<  get(startAddrStringNProp, target(e_b, cfg)) << " was not found. Ignoring it.");
				}
				v = target(e_f, cfg);
				LOG_DEBUG(logger, "Following alternative fw edge to " << get(startAddrStringNProp, v));
			}
			else if(found_fw_edge)
			{
				v = target(e_f, cfg);
				LOG_DEBUG(logger, "Following fw edge to " << get(startAddrStringNProp, v));
			}
			else
			{
				v = target(e_b, cfg);
				LOG_DEBUG(logger, "Following bw edge to " << get(startAddrStringNProp, v));
			}
		}

		printNode(v);
	}
}

void WCPathExporter::printNode(CFGVertex v)
{
	LOG_TRACE(logger, "WCPath: " << dec << v << " " << get(startAddrStringNProp, v));
	wcpath_output_file << "; Node: " << dec << v << " Type: " << get(nodeTypeNProp, v) << " Name: " << get(startAddrStringNProp, v) << endl;
	if(get(nodeTypeNProp, v) == BasicBlock)
	{
		string bbcode = get(bbCodeNProp, v);
		vector<string> bbcode_lines;
		split(bbcode_lines, bbcode, boost::is_any_of("\r"));

		for(uint32_t i = 0; i < bbcode_lines.size(); i++)
		{
			if(dlp->isCodeLine(bbcode_lines[i]))
			{
				uint32_t opcode_addr = dlp->getAddrFromCodeLine(bbcode_lines[i]);
				string opcode_string = dlp->getCommentFromCodeLine(bbcode_lines[i]);
				wcpath_output_file << hex << opcode_addr << " \"" << opcode_string << "\"" << endl;
			}
		}
	}
}

void WCPathExporter::countAndCategorizeInstruction(instr_stat_t *instr_stats, string instruction, uint32_t count)
{
	ostringstream os;

	instr_stats->instructions+=count;
	switch(dlp->getInstructionType(instruction))
	{
		case I_ARITHMETIC:
			{
				instr_stats->arithmetic_instructions+=count;
				os << "arithmetic";
				break;
			}
		case I_UNCOND_BRANCH:
		case I_UNCOND_INDIRECT_BRANCH:
			{
				instr_stats->branch_instructions+=count;
				os << "unconditional branch";
				break;
			}
		case I_COND_BRANCH:
		case I_COND_INDIRECT_BRANCH:
			{
				instr_stats->condbranch_instructions+=count;
				os << "conditional branch";
				break;
			}
		case I_CALL:
		case I_INDIRECT_CALL:
			{
				instr_stats->call_instructions+=count;
				os << "call";
				break;
			}
		case I_RETURN:
			{
				instr_stats->return_instructions+=count;
				os << "return";
				break;
			}
		case I_LOAD:
			{
				instr_stats->load_instructions+=count;
				os << "load";
				break;
			}
		case I_STORE:
			{
				instr_stats->store_instructions+=count;
				os << "store";
				break;
			}
		case I_SYNC:
			{
				instr_stats->sync_instructions+=count;
				os << "synchronization";
				break;
			}
		case I_DEBUG:
			{
				instr_stats->debug_instructions+=count;
				os << "debug";
				break;
			}
		case I_OTHERS:
			{
				instr_stats->other_instructions+=count;
				os << "other";
				break;
			}
		default:
			{
				instr_stats->unknown_instructions+=count;
				os << "unknown";
			}
	}
	LOG_DEBUG(logger, "Identified " << instruction << " as " << os.str() << " instruction");
}


instr_stat_t WCPathExporter::getInstrStats(void)
{
	cfgVertexIter vp;
	CFGVertex v;
	cfgInEdgeIter epi;
	CFGEdge e;
	instr_stat_t instruction_statistics;
	instruction_statistics.instructions = 0;
	instruction_statistics.arithmetic_instructions = 0;
	instruction_statistics.branch_instructions = 0;
	instruction_statistics.condbranch_instructions = 0;
	instruction_statistics.call_instructions = 0;
	instruction_statistics.return_instructions = 0;
	instruction_statistics.load_instructions = 0;
	instruction_statistics.store_instructions = 0;
	instruction_statistics.sync_instructions = 0;
	instruction_statistics.debug_instructions = 0;
	instruction_statistics.other_instructions = 0;
	instruction_statistics.unknown_instructions = 0;

	for (vp = vertices(cfg); vp.first != vp.second; ++vp.first)
	{
		v = *vp.first;
		if(get(nodeTypeNProp, v) == BasicBlock)
		{
			uint32_t activation_count = 0;
			for(epi = in_edges(v, cfg); epi.first != epi.second; ++epi.first)
			{
				e = *epi.first;
				activation_count += get(actEProp, e);
			}

			// only categorize the instructions if they are on the WC-path
			if(activation_count > 0)
			{
				string bbcode = get(bbCodeNProp, v);
				vector<string> bbcode_lines;
				split(bbcode_lines, bbcode, boost::is_any_of("\r"));

				for(uint32_t i = 0; i < bbcode_lines.size(); i++)
				{
					if(dlp->isCodeLine(bbcode_lines[i]))
					{
						countAndCategorizeInstruction(&instruction_statistics, bbcode_lines[i], activation_count);
					}
				}
			}
		}
	}

	return instruction_statistics;
}
