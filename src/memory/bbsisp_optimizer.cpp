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
#include "bbsisp_optimizer.hpp"

//bool sort_bbrecords_by_benefit(bb_record_t a, bb_record_t b) {if(a.benefit == b.benefit){return (a.size < b.size);}else{return (a.benefit > b.benefit);}}
//bool sort_bbrecords_by_size(bb_record_t a, bb_record_t b){return (a.size < b.size);}
//bool sort_bbrecords_by_activation_count(bb_record_t a, bb_record_t b){return (a.activation_count > b.activation_count);}
//#define sort_by_benefit(x) sort(x.begin(), x.end(), sort_bbrecords_by_benefit)
//#define sort_by_size(x) sort(x.begin(), x.end(), sort_bbrecords_by_size)
//#define sort_by_activation_count(x) sort(x.begin(), x.end(), sort_bbrecords_by_activation_count)

BBSISPOptimizer::BBSISPOptimizer(ControlFlowGraph cfgraph, CFGVertex entry, CFGVertex exit) : SISPOptimizer_IF(cfgraph, entry, exit)
{
}

BBSISPOptimizer::~BBSISPOptimizer()
{
	clear();
}

void BBSISPOptimizer::clear(void)
{
	assigned_bbaddrs.clear();
	ilp_knapsack_formulation.clear();
}

void BBSISPOptimizer::calculateBlockAssignment(void)
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
	setVariables(lp_result);
}

void BBSISPOptimizer::generateKnapsackILPFormulation(void)
{
	stringstream ilp_knapsack_formulation_stream;

	stringstream objective_function;
	stringstream size_constraints;
	stringstream set_binary_domains;

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
	set_binary_domains << "bin a" << v << ";" << endl;

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
			set_binary_domains << "bin a" << v << ";" << endl;
		}
	}
	objective_function << ";" << endl;
	size_constraints << " = sp;" << endl;
	size_constraints << "sp <= " << sisp_size << ";" << endl;

	ilp_knapsack_formulation.push_back("\n// Object function:\n");
	ilp_knapsack_formulation.push_back(objective_function.str());
	ilp_knapsack_formulation.push_back("\n// Size constraints:\n");
	ilp_knapsack_formulation.push_back(size_constraints.str());
	ilp_knapsack_formulation.push_back("\n// Binary domains:\n");
	ilp_knapsack_formulation.push_back(set_binary_domains.str());
	ilp_knapsack_formulation.push_back("\n");

}
