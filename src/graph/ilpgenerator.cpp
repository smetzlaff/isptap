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
#include "ilpgenerator.hpp"

LoggerPtr ILPGenerator::logger(Logger::getLogger("ILPGenerator"));

ILPGenerator::ILPGenerator(ControlFlowGraph cfgraph, CFGVertex cfg_entry, CFGVertex cfg_exit) : next_flow_var_id(1), flow_id_for_back_edge(0), wc_cost_value(0), lp_solution_type(SolutionNotCalculated)
{
	cfg = cfgraph;
	entry = cfg_entry;
	exit = cfg_exit;


	// get the properties for the graph (nodes and edges)
	costEProp = get(cost_t(), cfg);
	costOnChipEProp = get(cost_onchip_t(), cfg);
	costOffChipEProp = get(cost_offchip_t(), cfg);
	memPenaltyEProp = get(mem_penalty_t(), cfg);
	capacitylEProp = get(capacityl_t(), cfg);
	capacityhEProp = get(capacityh_t(), cfg);
	circEProp = get(circ_t(), cfg);
	actEProp = get(activation_t(), cfg);
	sflowEProp = get(static_flow_t(), cfg);
	edgeNameEProp = get(edgename_t(), cfg);
	edgeTypeEProp = get(edgetype_t(), cfg);
	nodeTypeNProp = get(nodetype_t(), cfg);
	startAddrNProp = get(startaddr_t(), cfg);
	startAddrStringNProp = get(startaddrstring_t(), cfg);
	endAddrNProp = get(endaddr_t(), cfg);
	contextIDNProp = get(context_id_t(), cfg);
	callLabelNProp = get(calllabel_t(), cfg);
	bbSizeNProp = get(bbsize_t(), cfg);
	cacheHitsNProp = get(cache_hits_t(), cfg);
	cacheMissesNProp = get(cache_misses_t(), cfg);
	cacheNCsNProp = get(cache_ncs_t(), cfg);

	conf = Configuration::getInstance();
}

ILPGenerator::~ILPGenerator()
{

	function_table.clear();

	object_function.clear();

	for(uint32_t i = 0; i < flow_conservation.size(); i++)
	{
		flow_conservation[i].left.clear();
		flow_conservation[i].right.clear();
	}
	flow_conservation.clear();
	flow_conservation_for_functions.clear();
	for(uint32_t i = 0; i < flow_facts.size(); i++)
	{
		flow_facts[i].loop_edges.clear();
	}
	flow_facts.clear();
	for(uint32_t i = 0; i < s_flow_facts.size(); i++)
	{
		s_flow_facts[i].edges.clear();
	}
	s_flow_facts.clear();

}

void ILPGenerator::createILP(void)
{
	// add back edge
	addBackEdge();

	// create object function (cost of every edge assigned to flow variables), also create the mappings for each edge to the corresponding flow variable
	buildObjectFunction(true);

	// create flow conservation equations (capacity in edges and out edges of each node should correspond)
	extractFlowConservation();

	// get flow conservation for function calls
	extractFlowConservationForFunctionCalls();

	// max flow for back loops (max loop counts)
	// TODO check flowfacts with "./isptap code_examples/main_compress_single_fsp.dump cl_hash" or similar
	getLoopBounds();

}

void ILPGenerator::updateObjectiveFunction(void)
{
	object_function.clear();
	ilp_formulation.clear();
	buildObjectFunction(false);
}


void ILPGenerator::writeILPFile(string filename)
{
	ofstream ilpfile;
	ilpfile.open(filename.c_str());

	assert(ilpfile.is_open());

	if(ilp_formulation.empty())
	{
		createILPFormulation();
	}

	ilpfile << ilp_formulation;

	ilpfile.close();
}

void ILPGenerator::createILPFormulation(void)
{
	stringstream ilp_formulation_stream;

	assert(object_function.size() > 0);
	stringstream object_fcn;
	object_fcn << "max:\n  " << object_function[0].weight << " f" << object_function[0].var_id << endl;

	for(uint32_t i=1; i < object_function.size(); i++)
	{
		object_fcn << "+ " << object_function[i].weight << " f" << object_function[i].var_id << endl;
	}
	object_fcn << ";" << endl;

//	LOG_DEBUG(logger, "//Object function is:");
//	LOG_DEBUG(logger, "\n" << object_fcn.str());

	stringstream injecting_flow;
	injecting_flow << "f" << flow_id_for_back_edge << " = 1;" << endl;

//	LOG_DEBUG(logger, "//Injection flow equation is:");
//	LOG_DEBUG(logger, "\n" << injecting_flow.str());

	stringstream flow_cons;
	for(uint32_t i = 0; i < flow_conservation.size(); i++)
	{
		assert(flow_conservation[i].left.size()>0);
		flow_cons << "f" << flow_conservation[i].left[0];
		for(uint32_t j = 1; j < flow_conservation[i].left.size(); j++)
		{
			flow_cons << " + f" << flow_conservation[i].left[j];
		}
		flow_cons << " = ";
		assert(flow_conservation[i].right.size()>0);
		flow_cons << "f" << flow_conservation[i].right[0];
		for(uint32_t j = 1; j < flow_conservation[i].right.size(); j++)
		{
			flow_cons << " + f" << flow_conservation[i].right[j];
		}
		flow_cons << ";" << endl;
	}

//	LOG_DEBUG(logger, "//Flow conservation equations are:");
//	LOG_DEBUG(logger, "\n" << flow_cons.str());

	stringstream flow_cons_func;

	for(uint32_t i = 0; i < flow_conservation_for_functions.size(); i ++)
	{
		flow_cons_func << "f" << flow_conservation_for_functions[i].left << " = f" << flow_conservation_for_functions[i].right << ";" << endl;
	}

//	LOG_DEBUG(logger, "Flow conservation for functions: " << flow_cons_func.str());

	stringstream flow_facts_string;

	for(uint32_t i = 0; i < flow_facts.size(); i ++)
	{
		flow_facts_string << "f" << flow_facts[i].loop_edges[0];

		for(uint32_t j = 1; j < flow_facts[i].loop_edges.size(); j++)
		{
			flow_facts_string << " + f" << flow_facts[i].loop_edges[j];
		}

		flow_facts_string << " = " << flow_facts[i].loop_bound <<  " * f" << flow_facts[i].inducting_edges[0];
		for(uint32_t j = 1; j < flow_facts[i].inducting_edges.size(); j++)
		{
			flow_facts_string << " + " << flow_facts[i].loop_bound <<  " * f" << flow_facts[i].inducting_edges[j];
		}
		flow_facts_string << ";" << endl;
	}

//	LOG_DEBUG(logger, "Flow facts for loops: " << flow_facts_string.str());

	stringstream s_flow_facts_string;

	for(uint32_t i = 0; i < s_flow_facts.size(); i ++)
	{
		s_flow_facts_string << "f" << s_flow_facts[i].edges[0];

		for(uint32_t j = 1; j < s_flow_facts[i].edges.size(); j++)
		{
			s_flow_facts_string << " + f" << s_flow_facts[i].edges[j];
		}

		switch(s_flow_facts[i].flow_type)
		{
			case EXACT:
				s_flow_facts_string << " = ";
				break;
			case MAX:
				s_flow_facts_string << " <= ";
				break;
			case MIN:
				s_flow_facts_string << " >= ";
				break;
			default:
				assert(false);
		}

		s_flow_facts_string << s_flow_facts[i].flow_bound << ";" << endl;
	}

//	LOG_DEBUG(logger, "Statically defined flow facts: " << s_flow_facts_string.str());


	stringstream set_integer_domains;

	for(uint32_t i = 0; i < next_flow_var_id; i++)
	{
		set_integer_domains << "int f" << i << ";" << endl;
	}

	ilp_formulation_stream << "// Object function:" << endl;
	ilp_formulation_stream << object_fcn.str();
	ilp_formulation_stream << endl << "// injection flow from back edge" << endl;
	ilp_formulation_stream << injecting_flow.str();
	ilp_formulation_stream << endl << "// flow conservation" << endl;
	ilp_formulation_stream << flow_cons.str();
	ilp_formulation_stream << endl << "// flow conservation for functions" << endl;
	ilp_formulation_stream << flow_cons_func.str();
	ilp_formulation_stream << endl << "// flow facts (loop bounds)" << endl;
	ilp_formulation_stream << flow_facts_string.str();
	ilp_formulation_stream << endl << "// flow facts (statically defined)" << endl;
	ilp_formulation_stream << s_flow_facts_string.str();
	ilp_formulation_stream << endl << "// set all flow variables as integer domains" << endl;
	ilp_formulation_stream << set_integer_domains.str();
	ilp_formulation_stream << endl;

	ilp_formulation = ilp_formulation_stream.str();
	LOG_INFO(logger, "ILP to calculate WCET is:\n" << ilp_formulation);
}

ControlFlowGraph ILPGenerator::getILPCFG(void)
{
	return cfg;
}

void ILPGenerator::addBackEdge(void)
{

	if(out_degree(exit, cfg) != 0)
	{
		remove_edge(exit, entry, cfg);
	}

	bool inserted;
	EdgeFlowVarMap::iterator pos;
	FlowVarEdgeMap::iterator pos_rev;
	CFGEdge back_edge;

	tie(back_edge, inserted) = add_edge(exit, entry, cfg);
	assert(inserted);
	put(costEProp, back_edge, 0);
	put(capacitylEProp, back_edge, 1);
	put(capacityhEProp, back_edge, 1);
	// XXX is circ_t of -1 here correct? Can the capacity and the circ be joined here?
	put(circEProp, back_edge, -1);
	put(edgeTypeEProp, back_edge, InductingBackEdge);
	// set injecting flow for the back edge
	flow_id_for_back_edge = 0;
	put(edgeNameEProp, back_edge, string("f0 (0;1/1)"));
	tie(pos, inserted) = efvMap.insert(make_pair(back_edge, flow_id_for_back_edge));
	assert(inserted);
	tie(pos_rev, inserted) = fevMap.insert(make_pair(flow_id_for_back_edge, back_edge));
	assert(inserted);

}

void ILPGenerator::buildObjectFunction(bool createFlowVarMappings)
{
	cfgVertexIter vp;
	bool inserted;
	EdgeFlowVarMap::iterator pos;
	FlowVarEdgeMap::iterator pos_rev;
	CFGEdge e, back_edge;
	cfgOutEdgeIter epo;

	// walk through all edges, starting at BEGIN, assign flow var and combine it with the edge_cost
	for (vp = vertices(cfg); vp.first != vp.second; ++vp.first)
	{
		LOG_DEBUG(logger, "vertex: " << *vp.first);
		for(epo = out_edges(*vp.first, cfg); epo.first != epo.second; ++epo.first) 
		{
			e = *epo.first;

			if(!isBackEdge(e))
			{
				uint32_t flow_var_id;
				obj_func_entry_t tmp;
				tmp.weight = getWeightOfEdge(e);
				if(createFlowVarMappings)
				{
					// registering edge in mapping of edge to flow variable
					tie(pos, inserted) = efvMap.insert(make_pair(e, next_flow_var_id));
					assert(inserted);
					tie(pos_rev, inserted) = fevMap.insert(make_pair(next_flow_var_id, e));
					assert(inserted);
					flow_var_id = next_flow_var_id;
					next_flow_var_id++;
				}
				else
				{
					// lookup flow variable id for edge
					pos = efvMap.find(e);
					assert(pos != efvMap.end());
					flow_var_id = pos->second;
				}

				tmp.var_id = flow_var_id;
				object_function.push_back(tmp);
				LOG_DEBUG(logger, "creating entry for object function: " << dec << tmp.weight << " f" << flow_var_id);

				ostringstream edgename;
				edgename << "f" << flow_var_id << " (" << get(costEProp, e) << "+" << get(memPenaltyEProp, e) << ";" << get(capacitylEProp, e) << "/" << get(capacityhEProp, e) << ")";
				put(edgeNameEProp, e, edgename.str());
			}
			else
			{
				// back edge is already registered (independed from createFlowVarMappings parameter)
				obj_func_entry_t tmp;
				tmp.weight = getWeightOfEdge(e);
				tmp.var_id = flow_id_for_back_edge;
				object_function.push_back(tmp);
				LOG_DEBUG(logger, "creating entry for object function (back edge): " << tmp.weight << " f" << flow_id_for_back_edge);
			}
		}
	}
}

void ILPGenerator::extractFlowConservation(void)
{
	cfgVertexIter vp;
	EdgeFlowVarMap::iterator pos;
	CFGEdge e;
	cfgOutEdgeIter epo;
	cfgInEdgeIter epi;

	for (vp = vertices(cfg); vp.first != vp.second; ++vp.first)
	{
		vector<uint32_t> left, right;
		for(epi = in_edges(*vp.first, cfg); epi.first != epi.second; ++epi.first) 
		{
			e = *epi.first;
			pos = efvMap.find(e);
			assert(pos != efvMap.end());
			left.push_back(pos->second);
		}
		for(epo = out_edges(*vp.first, cfg); epo.first != epo.second; ++epo.first) 
		{
			e = *epo.first;
			pos = efvMap.find(e);
			assert(pos != efvMap.end());
			right.push_back(pos->second);
		}

		assert((left.size() != 0) && (right.size() != 0));

		flow_equation_t fc_tmp;
		fc_tmp.left = left;
		fc_tmp.right = right;
		flow_conservation.push_back(fc_tmp);

	}
}

void ILPGenerator::extractFlowConservationForFunctionCalls(void)
{
	cfgVertexIter vp;
	EdgeFlowVarMap::iterator pos;
	CFGEdge e, back_edge;
	cfgOutEdgeIter epo;
	cfgInEdgeIter epi;
	vector<context_flow_var_id_t> call_point_contexts, return_point_contexts;

	// the flow variables of the out edge of a call point has to match the flow variablee of the in edge of a return point
	for (vp = vertices(cfg); vp.first != vp.second; ++vp.first)
	{
		CFGVertex v = *vp.first;

		if(get(nodeTypeNProp, v) == CallPoint)
		{
			context_flow_var_id_t tmp;
			tmp.context_addr = get(endAddrNProp, v);
			tmp.context_id = get(contextIDNProp, v);
			tmp.called_function_addr = get(callLabelNProp, v);

			LOG_DEBUG(logger, "Found Call context: " << hex << tmp.context_addr << " id: " << tmp.context_id << " to function: " << tmp.called_function_addr);

			// a call point can have only one out edge
			assert(out_degree(v, cfg)==1);

			for(epo = out_edges(v, cfg); epo.first != epo.second; ++epo.first) 
			{
				e = *epo.first;
				pos = efvMap.find(e);
				assert(pos != efvMap.end());
				tmp.flow_var_id = pos->second;
			}

			call_point_contexts.push_back(tmp);

		}
		else if(get(nodeTypeNProp, v) == ReturnPoint)
		{
			context_flow_var_id_t tmp;
			tmp.context_addr = get(endAddrNProp, v);
			tmp.context_id = get(contextIDNProp, v);
			tmp.called_function_addr = get(callLabelNProp, v);

			LOG_DEBUG(logger, "Found Return context: " << hex << tmp.context_addr << " id: " << tmp.context_id << " to function: " << tmp.called_function_addr);

			// a return point can have only one in edge
			assert(in_degree(v, cfg)==1);

			for(epi = in_edges(v, cfg); epi.first != epi.second; ++epi.first) 
			{
				e = *epi.first;
				pos = efvMap.find(e);
				assert(pos != efvMap.end());
				tmp.flow_var_id = pos->second;
			}

			return_point_contexts.push_back(tmp);
		}
	}

	// call and return point context should match
	assert(call_point_contexts.size() == return_point_contexts.size());

	// create flow conservation equations for function
	for(uint32_t i = 0; i < call_point_contexts.size(); i++)
	{
		for(uint32_t j = 0; j < return_point_contexts.size(); j++)
		{
			if((call_point_contexts[i].context_addr == return_point_contexts[j].context_addr)&&(call_point_contexts[i].called_function_addr == return_point_contexts[j].called_function_addr)&&(call_point_contexts[i].context_id == return_point_contexts[j].context_id))
			{
				flow_var_id_match_t tmp;
				tmp.left = call_point_contexts[i].flow_var_id;
				tmp.right = return_point_contexts[j].flow_var_id;
				flow_conservation_for_functions.push_back(tmp);
				break;
			}
		}
	}
}

void ILPGenerator::getLoopBounds(void)
{

	cfgVertexIter vp;
	EdgeFlowVarMap::iterator pos;
	CFGEdge e;
	cfgInEdgeIter epi;

	if(conf->getBool(CONF_USE_FLOWFACT_FILE))
	{

		// check for all basic blocks if there is an entry in the flowfacts file
		for (vp = vertices(cfg); vp.first != vp.second; ++vp.first)
		{
			CFGVertex v = *vp.first;

			vector<uint32_t> loop_fs;
			vector<uint32_t> inducting_fs;
			int32_t cap =0;

			for(epi = in_edges(v, cfg); epi.first != epi.second; ++epi.first) 
			{
				e = *epi.first;

				if(get(circEProp, e) >= 0)
				{
					cap = get(circEProp,e);
					pos = efvMap.find(e);
					assert(pos != efvMap.end());
					inducting_fs.push_back(pos->second);
				}
				else
				{
					pos = efvMap.find(e);
					assert(pos != efvMap.end());
					loop_fs.push_back(pos->second);
				}
			}
			// if an inducting edge was found and if there is any loop edge (e.g. for the "first iteration" of an unrolled loop in the vivu graph there is no loop edge)
			if((inducting_fs.size() > 0) && (loop_fs.size() > 0))
			{
				if(logger->isDebugEnabled())
				{
					ostringstream debug;
					debug << "found loopbound for edges: f" <<  loop_fs[0];
					for(uint32_t i = 1; i < loop_fs.size(); i++)
					{
						debug << " + f" << loop_fs[i];
					}
					debug << " = " << cap <<  " * f" << inducting_fs[0];
					for(uint32_t j = 1; j < inducting_fs.size(); j++)
					{
						debug << " + " << cap << " * f" << inducting_fs[j];
					}
					LOG_DEBUG(logger, debug.str());
				}

				for(uint32_t i = 0; i < loop_fs.size(); i++)
				{
					for(uint32_t j = 0; j < inducting_fs.size(); j++)
					{
						assert(loop_fs[i] != inducting_fs[j]);
					}
				}

				flow_fact_t tmp;
				tmp.inducting_edges = inducting_fs;
				tmp.loop_edges = loop_fs;
				tmp.loop_bound = cap;

				flow_facts.push_back(tmp);
			}


			static_flow_constraint_t s_flow;
			for(epi = in_edges(v, cfg); epi.first != epi.second; ++epi.first) 
			{
				// TODO: Consistency check missing: Verify that for all in edges the same s_flow.id is used.
				e = *epi.first;

				s_flow.flow_type = UNKNOWN;
				s_flow.flow_bound = -1;
				s_flow.id = -1;
				bool found_sflow = false;

				s_flow = get(sflowEProp, e);

				if(s_flow.flow_type != UNKNOWN)
				{

					for(uint32_t i=0; i < s_flow_facts.size(); i++)
					{
						// If a static flow constraint with the same id is already registered, the flow constraint is combined by multiple edges (e.g. f1 + fx [>|<]= value).
						// This is necessary to support the static flow constraints also for VIVU transformed graphs in which the loops are unrolled an one edge with a constraint may be duplicated.
						if(s_flow_facts[i].id == s_flow.id)
						{
							found_sflow = true;
							assert((s_flow_facts[i].flow_type = s_flow.flow_type) && (s_flow_facts[i].flow_bound = s_flow.flow_bound));
							pos = efvMap.find(e);
							assert(pos != efvMap.end());
							s_flow_facts[i].edges.push_back(pos->second);

						}
					}

					if(!found_sflow) // the static flow constraint is not yet registered - create it.
					{
						s_flow_fact_t tmp;
						tmp.flow_type = s_flow.flow_type;
						tmp.flow_bound = s_flow.flow_bound;
						tmp.id = s_flow.id;
						pos = efvMap.find(e);
						assert(pos != efvMap.end());
						tmp.edges.push_back(pos->second);
						s_flow_facts.push_back(tmp);
					}
				}
			}
		}
	}
}

bool ILPGenerator::isBackEdge(CFGEdge e)
{
	EdgeFlowVarMap::iterator pos;
	pos = efvMap.find(e);
	if(pos != efvMap.end())
	{
		if(flow_id_for_back_edge == pos->second)
		{
			return true;
		}
	}
	return false;
}

lp_solution_t ILPGenerator::solveILP(void)
{
	if(ilp_formulation.empty())
	{
		createILPFormulation();
	}

	LpSolver lp(ilp_formulation, conf->getString(CONF_LP_SOLVE_PARAMETERS));

	vector<lp_result_set> lp_result = lp.lpSolve();
	lp_solution_type = lp.getSolutionType();

	if(lp_result.size() == 0)
	{
		LOG_ERROR(logger, "Result from lpsolver is empty");
		if((lp_solution_type == SolutionNotCalculated) || (lp_solution_type == OptimalSolution) || (lp_solution_type == SuboptimalSolution))
		{
			lp_solution_type = ErrorWhileSolving;
			return ErrorWhileSolving;
		}
	}

	if(lp_solution_type == ErrorWhileSolving)
	{
		LOG_ERROR(logger, "Error detected on solving the ILP. Cannot proceed");
		return lp_solution_type;
	}
	else if(lp_solution_type == ProblemUnbound)
	{
		LOG_ERROR(logger, "The ILP is unbound. No solution was found.");
		return lp_solution_type;
	}

	for(uint32_t i=0; i < lp_result.size(); i++)
	{
		setFlowValue(lp_result[i].variable, lp_result[i].value);
	}

	if(wc_cost_value == 0)
	{
		wc_cost_value = lp.getObjectiveFunctionValue();
		switch((analysis_metric_t) conf->getUint(CONF_USE_METRIC))
		{
			case WCET_RATIO_FILES:
			case WCET:
				{
					uint32_t graph_wcet = getGraphCost();
					LOG_INFO(logger, "WCET of ILP is: " << wc_cost_value << " (" << graph_wcet << ")"); // worst case execution time
					LOG_INFO(logger, "MEM_COST of ILP is: " << getGraphMemCost());

					if(wc_cost_value != graph_wcet)
					{
						LOG_WARN(logger, "WCET differs!");
					}
					break;
				}
			case MDIC:
				{
					uint32_t graph_wcet = getGraphCost();
					LOG_INFO(logger, "WCIC of ILP is: " << wc_cost_value << " (" << graph_wcet << ")"); // worst case instruction count
					if(wc_cost_value != graph_wcet)
					{
						LOG_WARN(logger, "WCIC differs!");
					}
					break;
				}
			case MPL:
				{
					LOG_INFO(logger, "WCPL of ILP is: " << wc_cost_value); // worst case path length
					break;
				}
			default:
				{
					LOG_ERROR(logger, "Wrong analysis metric.");
					assert(false);
				}
		}
	}
	else
	{
		uint32_t new_wc_cost_value = lp.getObjectiveFunctionValue();
		switch((analysis_metric_t) conf->getUint(CONF_USE_METRIC))
		{
			case WCET_RATIO_FILES:
			case WCET:
				{
					uint32_t graph_wcet = getGraphCost();
					LOG_INFO(logger, "WCET was improved by " << wc_cost_value - new_wc_cost_value << " old: " << wc_cost_value << " new: " << new_wc_cost_value << " (" << graph_wcet << ")");
					LOG_INFO(logger, "MEM_COST of ILP is: " << getGraphMemCost());
					if(wc_cost_value != graph_wcet)
					{
						LOG_WARN(logger, "WCET differs!");
					}
					break;
				}
			case MDIC:
				{
					uint32_t graph_wcet = getGraphCost();
					LOG_INFO(logger, "WCIT was improved by " << wc_cost_value - new_wc_cost_value << " old: " << wc_cost_value << " new: " << new_wc_cost_value << " (" << graph_wcet << ")");
					if(wc_cost_value != graph_wcet)
					{
						LOG_WARN(logger, "WCIT differs!");
					}
					break;
				}
			case MPL:
				{
					LOG_INFO(logger, "WCPL was improved by " << wc_cost_value - new_wc_cost_value << " old: " << wc_cost_value << " new: " << new_wc_cost_value);
					break;
				}
			default:
				{
					LOG_ERROR(logger, "Wrong analysis metric.");
					assert(false);
				}
		}
		wc_cost_value = new_wc_cost_value;
	}
	return lp_solution_type;
}

void ILPGenerator::setFlowValue(string flowvar, uint32_t flowvar_value)
{
	setFlowValue(strtoul((flowvar.substr(1,flowvar.length()-1)).c_str(), NULL, 10), flowvar_value);
}

void ILPGenerator::setFlowValue(uint32_t flowvar_id, uint32_t flowvar_value)
{
	FlowVarEdgeMap::iterator pos;
	pos = fevMap.find(flowvar_id);
	assert(pos != fevMap.end());
	CFGEdge e = pos->second;

	// TODO check if circ is the right property (or if it should be renamed)
	put(actEProp, e, flowvar_value);

	// also update the edgename property (which is depicted in the exported graphs)
	ostringstream edgename;
	edgename << "f" << flowvar_id << " (" << get(costEProp, e) << "+" << get(memPenaltyEProp, e)  << ";" << get(capacitylEProp, e) << "/" << get(capacityhEProp, e) << ") " << flowvar_value;
	put(edgeNameEProp, e, edgename.str());

}

uint64_t ILPGenerator::getGraphCost(void)
{
	// FIXME the output of getGraphCost() should be the same as the wc_cost_value, but in some cases there is a small unexplanable difference
	uint64_t cost = 0;
	cfgEdgeIter ep;
	CFGEdge e;

	for(ep = edges(cfg); ep.first != ep.second; ++ep.first)
	{
		e = *ep.first;
		uint32_t edge_cost = get(actEProp, e) * getWeightOfEdge(e);
//		LOG_DEBUG(logger, "Adding " << edge_cost << " for " << get(edgeNameEProp, e));
		cost += edge_cost;
	}

	return cost;
}

uint64_t ILPGenerator::getGraphMemCost(void)
{
	uint64_t mem_cost = 0;
	cfgEdgeIter ep;
	CFGEdge e;

	for(ep = edges(cfg); ep.first != ep.second; ++ep.first)
	{
		e = *ep.first;
		uint32_t edge_mem_cost = get(actEProp, e) * getMemPenaltyOfEdge(e);
		mem_cost += edge_mem_cost;
	}

	return mem_cost;
}

uint64_t ILPGenerator::getWCCostValue(void)
{
	return getGraphCost();
}

void ILPGenerator::setWCCostValue(uint64_t wcost)
{
	wc_cost_value = wcost;
}

uint64_t ILPGenerator::getMemCostValue(void)
{
	return getGraphMemCost();
}



void ILPGenerator::setAssignedBlocks(vector<uint32_t> assigned_blocks)
{
	cfgVertexIter vp;
	CFGVertex v;
	cfgOutEdgeIter epo;
	CFGEdge e;

	// this method is only usable for static memory types
	assert(IS_STATIC_MEM((mem_type_t)conf->getUint(CONF_MEMORY_TYPE)));

	// search for matching blocks and set the onchip cost for their outedges
	for(uint32_t i = 0; i < assigned_blocks.size(); i++)
	{
		for (vp = vertices(cfg); vp.first != vp.second; ++vp.first)
		{
			v = *vp.first;
			if((get(nodeTypeNProp, v) == BasicBlock) && get(startAddrNProp, v) == assigned_blocks[i])
			{
				string str;
				str = "X ";
				str += get(startAddrStringNProp, v);
				put(startAddrStringNProp, v, str);
				for(epo = out_edges(*vp.first, cfg); epo.first != epo.second; ++epo.first) 
				{
					e = *epo.first;
					
					// for assigned blocks the memory penalty is cleared so the blocks execution time is the onchip cost only
					put(memPenaltyEProp, e, 0); 

//					put(costEProp, e, get(costOnChipEProp,e));
				}
			}
		}
	}
}

void ILPGenerator::setDetectedFunctions(vector<addr_label_t> detected_functions)
{
	function_table = detected_functions;
}

uint32_t ILPGenerator::getWeightOfEdge(CFGEdge e)
{
	uint32_t weight = 0;
	uint32_t cost =  get(costEProp, e);
	uint32_t mem_penalty = get(memPenaltyEProp, e);

	switch((analysis_metric_t) conf->getUint(CONF_USE_METRIC))
	{
		case WCET_RATIO_FILES:
		case WCET:
			{
				// for the execution time the memory penalty (of cache or any other memory) has to be taken into account
				weight = cost + mem_penalty;
				break;
			}
		case MDIC:
		case MPL:
			{
				// for WCIC and WCPL only the number of instructions resp. the size of the block is of interest
				// in ControlFlowGraphObject::addBBCostToCfg() the instruction count resp. the block size is put into the cost edge property
				weight = cost;
				break;
			}
		default:
			{
				LOG_ERROR(logger, "Wrong analysis metric.");
				assert(false);
			}
	}

	return weight;

}

uint32_t ILPGenerator::getMemPenaltyOfEdge(CFGEdge e)
{
	uint32_t penalty = 0;
	mem_type_t memory_type = (mem_type_t) conf->getUint(CONF_MEMORY_TYPE);
	if((memory_type == NO_MEM) || (memory_type == VIVU_TEST))
	{
		// the mem penalty is the number of cycles that would be safed if the block would be executed in the onchip memory
		penalty = get(costEProp, e) - get(costOnChipEProp, e);
	}
	else if(IS_STATIC_MEM(memory_type))
	{
		penalty = get(memPenaltyEProp, e);
	}
	else
	{
		// the memory penalty is the number of cycles needed to by cache or disp obtain the instructions of the block (calculated by DFA)
		penalty = get(memPenaltyEProp, e);
	}
	return penalty;
}

CFGVertex ILPGenerator::getEntry(void)
{
	return entry;
}

CFGVertex ILPGenerator::getExit(void)
{
	return exit;
}


lp_solution_t ILPGenerator::getSolutionType(void)
{
	return lp_solution_type;
}


uint32_t ILPGenerator::getSizeOfBlocks(vector<uint32_t> blocks)
{
	cfgVertexIter vp;
	CFGVertex v;
	cfgOutEdgeIter epo;
	CFGEdge e;
	uint32_t used_size=0;

	// this method is only usable for static memory types
	assert(IS_STATIC_MEM((mem_type_t)conf->getUint(CONF_MEMORY_TYPE)));

	// search for matching blocks and set the onchip cost for their outedges
	for(uint32_t i = 0; i < blocks.size(); i++)
	{
		for (vp = vertices(cfg); vp.first != vp.second; ++vp.first)
		{
			v = *vp.first;
			if((get(nodeTypeNProp, v) == BasicBlock) && get(startAddrNProp, v) == blocks[i])
			{
				used_size += get(bbSizeNProp, v);
			}
		}
	}
	return used_size;
}


cache_hm_stat_t ILPGenerator::getCacheHMStatsForWCP(void)
{
	cfgEdgeIter ep;
	CFGEdge e;
	CFGVertex v;

	cache_hm_stat_t cache_hm;

	cache_hm.hits = 0;
	cache_hm.misses = 0;
	cache_hm.ncs = 0;

	if((mem_type_t)conf->getUint(CONF_MEMORY_TYPE) == ICACHE)
	{

		for(ep = edges(cfg); ep.first != ep.second; ++ep.first)
		{
			e = *ep.first;
			v = source(e, cfg);

			if(get(nodeTypeNProp, v) == BasicBlock)
			{
				cache_hm.hits += get(actEProp, e) * get(cacheHitsNProp, v);
				cache_hm.misses += get(actEProp, e) * get(cacheMissesNProp, v);
				cache_hm.ncs += get(actEProp, e) * get(cacheNCsNProp, v);
			}
		}
	}

	return cache_hm;
}

