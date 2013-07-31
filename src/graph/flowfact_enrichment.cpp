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
#include "flowfact_enrichment.hpp"


LoggerPtr FlowFactEnricher::logger(Logger::getLogger("FlowFactEnricher"));

FlowFactEnricher::FlowFactEnricher(ControlFlowGraph cfgraph, CFGVertex cfg_entry, CFGVertex cfg_exit):sflow_id(0)
{

	cfg = cfgraph;
	entry = cfg_entry;
	exit = cfg_exit;

	// get the properties for the graph (nodes and edges)
	circEProp = get(circ_t(), cfg);
	sflowEProp = get(static_flow_t(), cfg);
	edgeNameEProp = get(edgename_t(), cfg);
	edgeTypeEProp = get(edgetype_t(), cfg);
	nodeTypeNProp = get(nodetype_t(), cfg);
	startAddrNProp = get(startaddr_t(), cfg);
	startAddrStringNProp = get(startaddrstring_t(), cfg);
	endAddrNProp = get(endaddr_t(), cfg);

	conf = Configuration::getInstance();
}

FlowFactEnricher::~FlowFactEnricher()
{
	for(uint32_t i = 0; i < flow_facts.size(); i++)
	{
		flow_facts[i].loop_edges.clear();
	}
	flow_facts.clear();
}

void FlowFactEnricher::getLoopBounds(void)
{
	cfgVertexIter vp;
	CFGEdge e;
	cfgInEdgeIter epi;

	if(conf->getBool(CONF_USE_FLOWFACT_FILE))
	{

		flowfact.setFunctionTable(function_table);
		flowfact.setFile(conf->getString(CONF_FLOWFACT_FILE));

		// check for all basic blocks if there is an entry in the flowfacts file
		for (vp = vertices(cfg); vp.first != vp.second; ++vp.first)
		{
			CFGVertex v = *vp.first;

			if(get(nodeTypeNProp, v) == BasicBlock)
			{
				int32_t cap = flowfact.getLoopBoundCapacity(get(startAddrNProp,v));
				if(cap != -1)
				{
					// capacity contraint found in flow facts
					// get both in egdes and distinguish inducting and loop edge

					LOG_DEBUG(logger, "Processing node: " << get(startAddrStringNProp, v));
					assert(in_degree(v, cfg)>=2); // two ore more edges are allowed: 1 inducting edge and multiple loop edges

					vector<CFGEdge> loop_e;
					CFGEdge inducting_e;
					inducting_e = CFGEdge();

					if(!flowfact.isLoopDecisionAtTail(get(startAddrNProp,v)))
					{
						for(epi = in_edges(v, cfg); epi.first != epi.second; ++epi.first) 
						{
							e = *epi.first;

							LOG_DEBUG(logger, "Processing edges using fwstep, fw jump or meta edges as induction: " << get(edgeNameEProp, e) << " Type: " << get(edgeTypeEProp, e) );

							if((get(edgeTypeEProp, e) == ForwardStep)  || (get(edgeTypeEProp, e) == ForwardJump) || (get(edgeTypeEProp, e) == Meta))
							{
								// the inducting edge could be either a normal forward step edge or a meta edge (e.g. if the loop head is the first bb of a function)
								inducting_e = e;
								// add the flow value of the loop to the inducting edge
								put(circEProp, e, cap);
								ostringstream s;
								s << get(edgeNameEProp, e);
								s << " (ff: " << dec << cap << ")";
								put(edgeNameEProp, e, s.str());
							}
							else if (get(edgeTypeEProp, e) == BackwardJump)
							{
								// the loop edge is always the backward jump:
								//							loop_e = e;
								loop_e.push_back(e);
							}
							else
							{
								LOG_WARN(logger, "Wrong edge type.");
							}
						}
					}
					else
					{
						for(epi = in_edges(v, cfg); epi.first != epi.second; ++epi.first) 
						{
							e = *epi.first;

							uint32_t inducting_bb = flowfact.getInductingBB(get(startAddrNProp,v));
							LOG_DEBUG(logger, "Processing edges using special edge from 0x" << hex << inducting_bb << dec << " as induction: " << get(edgeNameEProp, e) << " Type: " << get(edgeTypeEProp, e) << " targets: 0x" << hex << get(startAddrNProp, source(e, cfg)));

							if(inducting_bb == get(startAddrNProp, source(e, cfg)))
							{
								// the inducting edge could be either a normal forward step edge or a meta edge (e.g. if the loop head is the first bb of a function)
								inducting_e = e;
								// add the flow value of the loop to the inducting edge
								put(circEProp, e, cap);
								ostringstream s;
								s << get(edgeNameEProp, e);
								s << " (ff: " << dec << cap << ")";
								put(edgeNameEProp, e, s.str());
							}
							else
							{
								// the loop edge is always the backward jump:
								loop_e.push_back(e);
							}
						}

					}

					assert(loop_e.size()>0);
					if(logger->isDebugEnabled())
					{
						ostringstream debug;
						debug << "found loopbound for edges: f" <<  loop_e[0];
						for(uint32_t i = 1; i < loop_e.size(); i++)
						{
							debug << " + f" << loop_e[i];
						}
						debug << " = " << cap <<  " * " << inducting_e;
						LOG_DEBUG(logger, debug.str());
					}

					for(uint32_t i = 0; i < loop_e.size(); i++)
					{
						assert(loop_e[i] != inducting_e);
					}

					flow_fact_edges_t tmp;
					tmp.inducting_edge = inducting_e;
					tmp.loop_edges = loop_e;
					tmp.loop_bound = cap;

					flow_facts.push_back(tmp);

				}

				static_flow_constraint_t static_flow = flowfact.getStaticFlowConstraint(get(startAddrNProp,v));
				if(static_flow.flow_type != UNKNOWN)
				{
					LOG_DEBUG(logger, "Processing node: " << get(startAddrStringNProp, v) << "with " << in_degree(v, cfg) << " in edges");
					assert(in_degree(v, cfg) > 0);
					for(epi = in_edges(v, cfg); epi.first != epi.second; ++epi.first) 
					{
						e = *epi.first;

						ostringstream s;
						s << get(edgeNameEProp, e);
						s << " (sff: " << ((static_flow.flow_type == EXACT)?("="):((static_flow.flow_type == MAX)?("<="):(">="))) << dec << static_flow.flow_bound << ")";
						put(edgeNameEProp, e, s.str());

						// Register the static flow constrains in structure and add them to the ilp
						static_flow_constraint_t sflow;
						sflow.flow_bound = static_flow.flow_bound;
						sflow.flow_type = static_flow.flow_type;
						sflow.id = sflow_id;
						put(sflowEProp, e, sflow);
					}
					// use the same id for the static flow constraint to allow merging of the two constraints to: "f1 + f2 flow_type flow_bound"
					sflow_id++;
				}
			}
		}
	}
}



ControlFlowGraph FlowFactEnricher::getGraphWithFlowFacts(void)
{
	return cfg;
}

vector<flow_fact_edges_t> FlowFactEnricher::getFlowFacts(void)
{
	return flow_facts;
}

CFGVertex FlowFactEnricher::getEntry(void)
{
	return entry;
}

CFGVertex FlowFactEnricher::getExit(void)
{
	return exit;
}

void FlowFactEnricher::setDetectedFunctions(vector<addr_label_t> detected_functions)
{
	function_table = detected_functions;
}
