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
#include "fsisp_optimizer_old.hpp"

LoggerPtr FSISPOptimizerOLD::logger(Logger::getLogger("FSISPOptimizerOLD"));


FSISPOptimizerOLD::FSISPOptimizerOLD()
{
	conf = Configuration::getInstance();
	string log_filename =  conf->getString(CONF_LOG_FILE);
	ilpfile_name = "isptap_"+ regex_replace(log_filename, regex("[.period.][[:alnum:]]*$"), "", match_default | format_all) + "_f_assignment.ilp";
}


FSISPOptimizerOLD::~FSISPOptimizerOLD()
{
	function_data.clear();
}


void FSISPOptimizerOLD::setSize(uint32_t size)
{
	LOG_DEBUG(logger, "Setting FSISP size to " << size << " bytes.");
	sisp_size = size;
	used_sisp_size = 0;
}


void FSISPOptimizerOLD::setFunctions(FunctionCallGraph fcgraph, const vector<function_graph_t> fcgraphs)
{
	fcg = fcgraph;

	for(uint32_t i=0; i < fcgraphs.size(); i++)
	{
		function_record_t tmp;

		tmp.id = i;
		tmp.cfg = fcgraphs[i].cfg->getCFG();
		tmp.name = fcgraphs[i].name;
		tmp.size = fcgraphs[i].cfg->getCodeSize();
		tmp.address = fcgraphs[i].address;
		tmp.end_address = tmp.address + tmp.size;
		tmp.cur_cost = 0;
		tmp.onchip_cost = 0;
		tmp.offchip_cost = 0;
		tmp.benefit  = 0;
//		tmp.activation_count = 0;

		LOG_DEBUG(logger, "Adding " << tmp.name << " to function list");

		function_data.push_back(tmp);
	}
	LOG_DEBUG(logger, function_data.size() << " functions added.");

}


void FSISPOptimizerOLD::calculateFunctionBenefitsWithSCFGProperties(ControlFlowGraph scfg)
{
	// get node property structures
	property_map<ControlFlowGraph, nodetype_t>::type scfg_nodeTypeNProp = get(nodetype_t(), scfg);
	property_map<ControlFlowGraph, startaddr_t>::type scfg_startAddrNProp = get(startaddr_t(), scfg);
//	property_map<ControlFlowGraph, startaddrstring_t>::type scfg_startAddrStringNProp = get(startaddrstring_t(), scfg); // unused
//	property_map<ControlFlowGraph, bbsize_t>::type scfg_bbSizeNProp = get(bbsize_t(), scfg); // unused
//	property_map<ControlFlowGraph, calllabel_t>::type scfg_calllabel_t = get(calllabel_t(), scfg); // unused

	// get edge property structures
	property_map<ControlFlowGraph, cost_t>::type scfg_costEProp = get(cost_t(), scfg);
	property_map<ControlFlowGraph, cost_onchip_t>::type scfg_costOnChipEProp = get(cost_onchip_t(), scfg);
	property_map<ControlFlowGraph, cost_offchip_t>::type scfg_costOffChipEProp = get(cost_offchip_t(), scfg);
	property_map<ControlFlowGraph, mem_penalty_t>::type scfg_memPenaltyEProp = get(mem_penalty_t(), scfg);
	property_map<ControlFlowGraph, activation_t>::type scfg_actEProp = get(activation_t(), scfg);

	cfgVertexIter vp;
	cfgOutEdgeIter epo;
	cfgInEdgeIter epi;
	CFGVertex v;
	CFGEdge e;

	LOG_DEBUG(logger, function_data.size() << " functions found.");
	// cycle through all vertices of the supergraph and set the max flow (activation count) and the costs for each basic block for each function (stored in function_data) 
	for(vp = vertices(scfg); vp.first != vp.second; ++vp.first)
	{
		v = *vp.first;

		if(get(scfg_nodeTypeNProp, v) == BasicBlock)
		{
			uint32_t bb_addr = get(scfg_startAddrNProp, v);
			uint32_t affiliated_function=0;

			bool block_found = false;
			// assign the wcet costs for that block to the function it belongs to
			for(uint32_t i = 0; i < function_data.size(); i++)
			{
				if((bb_addr >= function_data[i].address) && (function_data[i].end_address > bb_addr))
				{

					block_found = true;
					LOG_DEBUG(logger, "Assigning block cost of bb: 0x" << hex << bb_addr << "  for function " <<  function_data[i].name);
					affiliated_function = i;
					break;
				}
			}

			assert(block_found);

			for(epo = out_edges(v, scfg); epo.first != epo.second; ++epo.first) 
			{
				e = *epo.first;
				uint32_t cur_cost = get(scfg_costEProp, e);
				uint32_t off_cost = get(scfg_costOffChipEProp, e);
				uint32_t on_cost = get(scfg_costOnChipEProp, e);
				uint32_t mem_penalty = get(scfg_memPenaltyEProp, e);
				uint32_t activation_count = get(scfg_actEProp,e);

				switch((analysis_metric_t) conf->getUint(CONF_USE_METRIC))
				{
					case WCET_RATIO_FILES:
					case WCET:
						{
							function_data[affiliated_function].cur_cost += activation_count * cur_cost;
							function_data[affiliated_function].offchip_cost += activation_count * off_cost;
							function_data[affiliated_function].onchip_cost += activation_count * on_cost;
							assert(mem_penalty == (off_cost - on_cost));
							function_data[affiliated_function].benefit += activation_count * (off_cost - on_cost);
							break;
						}
					case MDIC:
						{
							function_data[affiliated_function].cur_cost += activation_count * cur_cost;
							function_data[affiliated_function].offchip_cost += activation_count * off_cost;
							function_data[affiliated_function].onchip_cost += activation_count * on_cost;
							function_data[affiliated_function].benefit += function_data[affiliated_function].cur_cost;
							break;
						}
					case MPL:
						{
							function_data[affiliated_function].cur_cost += cur_cost;
							function_data[affiliated_function].offchip_cost += off_cost;
							function_data[affiliated_function].onchip_cost += on_cost;
							function_data[affiliated_function].benefit += function_data[affiliated_function].cur_cost;
							// this method is _not_ to be used, so fall down to assert(false)
//							break; 
						}
					default:
						{
							assert(false);
						}
				}

			}
		}
	}

	LOG_DEBUG(logger, function_data.size() << " functions found.");
	
	for(uint32_t i = 0; i < function_data.size(); i++)
	{
		LOG_DEBUG(logger, "Function id: " << function_data[i].id << " name: " << function_data[i].name << " addr: 0x" << hex << function_data[i].address << " end addr: 0x" << function_data[i].end_address << " size: "  << dec << function_data[i].size << " on/offchip cost: " << function_data[i].onchip_cost << "/" << function_data[i].offchip_cost << " benefit: " << function_data[i].benefit);
	} 
}

void FSISPOptimizerOLD::calculateKnapsackFunctionAssignment(void)
{
	if(ilp_knapsack_formulation.empty())
	{
		generateKnapsackILPFormulation();
	}

	writeKnapsackILPFile();

	LpSolver lps(ilp_knapsack_formulation, conf->getString(CONF_LP_SOLVE_PARAMETERS));
//	LpSolver lps(ilpfile_name, conf->getString(CONF_LP_SOLVE_PARAMETERS));
	setAssignment(lps.lpSolve());
}


vector<addr_label_t> FSISPOptimizerOLD::getFunctionAssignment(void)
{
	return assigned_functions;
}

vector<uint32_t> FSISPOptimizerOLD::getBlockAssignment(void)
{
	return assigned_blocks;
}

void FSISPOptimizerOLD::generateKnapsackILPFormulation(void)
{
	stringstream ilp_knapsack_formulation_stream;

	// generate knapsack ilp file from function_data
	stringstream objective_function;
	stringstream size_constraints;
	stringstream set_binary_domains;

	assert(function_data.size() > 0);

	objective_function << " max: " << endl << function_data[0].benefit << " a" << function_data[0].id << endl;
	size_constraints << function_data[0].size << " a" << function_data[0].id << endl;
	set_binary_domains << "bin a" << function_data[0].id << ";" << endl;
	for(uint32_t i = 1; i < function_data.size(); i++)
	{
		objective_function << " + " << function_data[i].benefit << " a" << function_data[i].id << endl;
		size_constraints << " + " << function_data[i].size << " a" << function_data[i].id << endl;
		set_binary_domains << "bin a" << function_data[i].id << ";" << endl;
	}
	objective_function << ";" << endl;
	size_constraints << " <= " << sisp_size << ";" << endl;

	ilp_knapsack_formulation_stream << "// Objective function:" << endl;
	ilp_knapsack_formulation_stream << objective_function.str();
	ilp_knapsack_formulation_stream << "// Size constraints:" << endl;
	ilp_knapsack_formulation_stream << size_constraints.str();
	ilp_knapsack_formulation_stream << "// Binary domains:" << endl;
	ilp_knapsack_formulation_stream << set_binary_domains.str();
	ilp_knapsack_formulation_stream << endl;

	ilp_knapsack_formulation = ilp_knapsack_formulation_stream.str();
}

void FSISPOptimizerOLD::writeKnapsackILPFile(void)
{
	// write knapsack ilp file from function_data
	if(ilp_knapsack_formulation.empty())
	{
		generateKnapsackILPFormulation();
	}

	ofstream ilpfile;
	LOG_INFO(logger, "Writing LP to obtain the function assignment to " << ilpfile_name);
	ilpfile.open(ilpfile_name.c_str());

	assert(ilpfile.is_open());

	ilpfile << ilp_knapsack_formulation;

	ilpfile.close();
}

void FSISPOptimizerOLD::setAssignment(vector<lp_result_set> lp_result)
{
	used_sisp_size = 0;

	uint32_t cur_lp_var_id;
	for(uint32_t i = 0; i < lp_result.size(); i++)
	{
		if(lp_result[i].value == 1)
		{
			cur_lp_var_id = strtoul(((lp_result[i].variable).substr(1,(lp_result[i].variable).length()-1)).c_str(), NULL, 10);
			for(uint32_t j = 0; j < function_data.size(); j++)
			{
				if(cur_lp_var_id == function_data[j].id)
				{
					// set function assignments
					addr_label_t tmp;
					tmp.address = function_data[j].address;
					tmp.label = function_data[j].name;
					assigned_functions.push_back(tmp);

					used_sisp_size += function_data[j].size;

					// also assign basic blocks to fsisp memory
					addBBsOfFunctionToAssignedBlocks(j);

					break;
				}
			}
		}
	}

	LOG_DEBUG(logger, "Assigned functions for FSISP with size of " << sisp_size << " :");
	for(uint32_t i = 0; i < assigned_functions.size(); i++)
	{
		LOG_DEBUG(logger, "Name: " << assigned_functions[i].label << " addr: 0x" << hex << assigned_functions[i].address);
	}

	LOG_DEBUG(logger, "Assigned blocks for FSISP with size of " << sisp_size << " :");
	for(uint32_t i = 0; i < assigned_blocks.size(); i++)
	{
		LOG_DEBUG(logger, "0x" << hex << assigned_blocks[i]);
	}

}

void FSISPOptimizerOLD::addBBsOfFunctionToAssignedBlocks(uint32_t function_data_id)
{
	assert(function_data_id < function_data.size());

	cfgVertexIter vp;
	CFGVertex v;

	// get node property structures
	property_map<ControlFlowGraph, nodetype_t>::type cfg_nodeTypeNProp = get(nodetype_t(), function_data[function_data_id].cfg);
	property_map<ControlFlowGraph, startaddr_t>::type cfg_startAddrNProp = get(startaddr_t(), function_data[function_data_id].cfg);


	// cycle through all vertices of the functions cfg and add all basic blocks to assigned_block vector 
	for(vp = vertices(function_data[function_data_id].cfg); vp.first != vp.second; ++vp.first)
	{
		v = *vp.first;
		if(get(cfg_nodeTypeNProp, v) == BasicBlock)
		{
			assigned_blocks.push_back(get(cfg_startAddrNProp, v));
		}
	}

}

uint32_t FSISPOptimizerOLD::getUsedSispSize(void)
{
	return used_sisp_size;
}


