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
#include "fsisp_optimizer.hpp"

FSISPOptimizer::FSISPOptimizer(ControlFlowGraph cfgraph, CFGVertex entry, CFGVertex exit, vector<function_graph_t> functions) : SISPOptimizer_IF(cfgraph, entry, exit)
{
	fcgraphs = functions;
}

FSISPOptimizer::~FSISPOptimizer()
{
	clear();
}

void FSISPOptimizer::clear(void)
{
	assigned_bbaddrs.clear();
	assigned_functions.clear();
	ilp_knapsack_formulation.clear();
}

void FSISPOptimizer::calculateBlockAssignment(void)
{
	clear();
	generateKnapsackILPFormulation();

	stringstream str;
	for(uint32_t i = 0; i < ilp_knapsack_formulation.size(); i++)
	{
		str << ilp_knapsack_formulation[i];
	}
	LOG_INFO(logger, "Formulation: " << str.str());

	vector<lp_result_set> lp_result = writeAndSolveILPFile(ilp_knapsack_formulation);
	setAssignment(lp_result);
	setFunctionAssignment(lp_result);
	setVariables(lp_result);
}

void FSISPOptimizer::generateKnapsackILPFormulation(void)
{
	stringstream ilp_knapsack_formulation_stream;

	stringstream objective_function;
	stringstream size_constraints;
	stringstream set_binary_domains;
	stringstream function_constraints;

	objective_function << " max: " << endl;
	cfgVertexIter vp = vertices(cfg);
	assert(vp.first != vp.second);

	CFGVertex v = *vp.first;
	// search as long as the first bb was found
	while(get(nodeTypeNProp, v) != BasicBlock)
	{
		++vp.first;
		assert(vp.first != vp.second);
		v= *vp.first;
	}


	// then write the first line (without + in front)
	uint32_t benefit = 0;
	for(cfgOutEdgeIter eop = out_edges(v, cfg); eop.first != eop.second; ++eop.first)
	{	
		CFGEdge e = *eop.first;
		switch((analysis_metric_t) conf->getUint(CONF_USE_METRIC))
		{
			case WCET_RATIO_FILES:
			case WCET:
				{
					benefit += (get(costOffChipEProp, e) - get(costOnChipEProp, e)) * get(actEProp, e);
					break;
				}
			case MDIC:
				{
					benefit += get(costEProp, e) * get(actEProp, e);
					break;
				}
			case MPL:
			default:
				{
					assert(false);
				}
		}

	}

	objective_function << benefit << " a" << v << endl;
	size_constraints << get(bbSizeNProp, v) << " a" << v << endl;

	++vp.first;

	// now all following lines can be written
	for(; vp.first != vp.second; ++vp.first)
	{
		v = *vp.first;
		if(get(nodeTypeNProp, v) == BasicBlock)
		{
			benefit = 0;
			for(cfgOutEdgeIter eop = out_edges(v, cfg); eop.first != eop.second; ++eop.first)
			{	
				CFGEdge e = *eop.first;
				switch((analysis_metric_t) conf->getUint(CONF_USE_METRIC))
				{
					case WCET_RATIO_FILES:
					case WCET:
						{
							assert(get(memPenaltyEProp, e) == get(costOffChipEProp, e) - get(costOnChipEProp, e));
							benefit += (get(costOffChipEProp, e) - get(costOnChipEProp, e)) * get(actEProp, e);
							break;
						}
					case MDIC:
						{
							benefit += get(costEProp, e) * get(actEProp, e);
							break;
						}
					case MPL:
					default:
						{
							assert(false);
						}
				}

			}
			objective_function << " + " << benefit << " a" << v << endl;
			size_constraints << " + " << get(bbSizeNProp, v) << " a" << v << endl;
		}
	}
	objective_function << ";" << endl;
	size_constraints << " = sp;" << endl;
	size_constraints << "sp <= " << sisp_size << ";" << endl;

	generateFunctionMembershipConstraints(set_binary_domains, function_constraints);


	ilp_knapsack_formulation.push_back("\n// Object function:\n");
	ilp_knapsack_formulation.push_back(objective_function.str());
	ilp_knapsack_formulation.push_back("\n// Size constraints:\n");
	ilp_knapsack_formulation.push_back(size_constraints.str());
	ilp_knapsack_formulation.push_back("\n// Function membership constraints:\n");
	ilp_knapsack_formulation.push_back(function_constraints.str());
	ilp_knapsack_formulation.push_back("\n// Binary domains:\n");
	ilp_knapsack_formulation.push_back(set_binary_domains.str());
	ilp_knapsack_formulation.push_back("\n");

}

void FSISPOptimizer::generateFunctionMembershipConstraints(stringstream &set_binary_domains, stringstream &function_constraints)
//{
//	for(uint32_t i=0; i < fcgraphs.size(); i++)
//	{
//		bool found_bb_in_function = false; // checks if there is a bb that is considered by the analysis that is assigned to the function.
//		function_graph_t fg = fcgraphs[i];
//
//		ControlFlowGraph func_cfg = fg.cfg->getCFG();
//
//		// XXX inefficient implementation:
//		for(cfgVertexIter vp = vertices(func_cfg); vp.first != vp.second; ++vp.first)
//		{
//			CFGVertex v = *vp.first;
//			property_map<ControlFlowGraph, startaddr_t>::type startAddrNProp_func_cfg;
//			startAddrNProp_func_cfg = get(startaddr_t(), func_cfg);
//			property_map<ControlFlowGraph, nodetype_t>::type nodeTypeNProp_func_cfg;
//			nodeTypeNProp_func_cfg = get(nodetype_t(), func_cfg);
//			if(get(nodeTypeNProp_func_cfg, v) == BasicBlock)
//			{
//				for(cfgVertexIter wp = vertices(cfg); wp.first != wp.second; ++wp.first)
//				{
//					CFGVertex w = *wp.first;
//					if(get(nodeTypeNProp, w) == BasicBlock)
//					{
//						if(get(startAddrNProp_func_cfg, v) == get(startAddrNProp, w))
//						{
//							// the block with id w is part of function i
//							function_constraints << "a" << w << " = f" << i << ";" << endl;
//							found_bb_in_function = true;
//						}
//					}
//
//				}
//			}
//		}
//		if(found_bb_in_function)
//		{
//			set_binary_domains << "bin f" << i << ";" << endl;
//		}
//	}
//}
{
	vector<uint32_t> used_functions;

	for(cfgVertexIter vp = vertices(cfg); vp.first != vp.second; ++vp.first)
	{
		CFGVertex v = *vp.first;
		if(get(nodeTypeNProp, v) == BasicBlock)
		{
			uint32_t bb_addr = get(startAddrNProp, v);
			for(uint32_t i=0; i < fcgraphs.size(); i++)
			{
				if((bb_addr >= fcgraphs[i].address) && (bb_addr < fcgraphs[i].address + fcgraphs[i].cfg->getCodeSize()))
				{
					// the block with id w is part of function i
					function_constraints << "a" << v << " = f" << i << ";" << endl;

					bool already_pushed=false;
					for(vector<uint32_t>::iterator it=used_functions.begin(); it != used_functions.end(); it++)
					{
						if((*it) == i)
						{
							already_pushed = true;
							break;
						}
					}
					if(!already_pushed)
					{
						used_functions.push_back(i);
					}
				}
			}

		}
	}

	sort(used_functions.begin(), used_functions.end());

	for(vector<uint32_t>::iterator it=used_functions.begin(); it != used_functions.end(); it++)
	{
			set_binary_domains << "bin f" << *it << ";" << endl;
	}
}

void FSISPOptimizer::setFunctionAssignment(vector<lp_result_set> lp_result)
{
	uint32_t cur_lp_var_id;
	for(uint32_t i = 0; i < lp_result.size(); i++)
	{
		// process only the variables that represent the assignment of the blocks: a[0-9]+
		if(boost::regex_search(lp_result[i].variable, boost::regex("^f[0-9]+$")))
		{
			if(lp_result[i].value == 1)
			{
				cur_lp_var_id = strtoul(((lp_result[i].variable).substr(1,(lp_result[i].variable).length()-1)).c_str(), NULL, 10);
				LOG_DEBUG(logger, "Assignment of function f" << cur_lp_var_id << " found " << fcgraphs[cur_lp_var_id].name << ".");
				addr_label_t tmp;
				tmp.address = fcgraphs[cur_lp_var_id].address;
				tmp.label = fcgraphs[cur_lp_var_id].name;
				assigned_functions.push_back(tmp);
			}
		}
	}
}

vector<addr_label_t> FSISPOptimizer::getFunctionAssignment(void)
{
	return assigned_functions;
}
