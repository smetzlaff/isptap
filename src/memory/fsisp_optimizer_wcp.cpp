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
#include "fsisp_optimizer_wcp.hpp"

FSISPOptimizerWCP::FSISPOptimizerWCP(ControlFlowGraph cfgraph, CFGVertex entry, CFGVertex exit, vector<function_graph_t> functions) : BBSISPOptimizerWCP(cfgraph, entry, exit) 
{
	ilp_solution_wcet_estimate = 0;
	fcgraphs = functions;
}

FSISPOptimizerWCP::~FSISPOptimizerWCP()
{
	clear();
}
void FSISPOptimizerWCP::clear(void)
{
	BBSISPOptimizerWCP::clear();
	assigned_functions.clear();
	function_membership_constraints.clear();
}

void FSISPOptimizerWCP::calculateBlockAssignment()
{
	clear();
	generateBlockCostsConstraints();
	generateBlockSizeConstraint();
	generateFunctionMembershipConstraints();
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
	ilp_formulation << "\n// Function membership constraints:\n";
	for(it = function_membership_constraints.begin(); it != function_membership_constraints.end(); it++)
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
	setFunctionAssignment(lp_result);
	setVariables(lp_result);
}


void FSISPOptimizerWCP::generateBinaryDomain(void)
{
	// this function is not needed in this implementation.
	return;
}


//string FSISPOptimizerWCP::getPenaltyForFunctionEntering(CFGVertex call_point, CFGVertex function_entry_node)
//{
//	// Do nothing. This function is only needed for BBSISPOptimizerJPWCP.
//	return "";
//}
//
//
//string FSISPOptimizerWCP::getPenaltyForFunctionExit(CFGVertex return_point, CFGVertex function_exit_node)
//{
//	// Do nothing. This function is only needed for BBSISPOptimizerJPWCP.
//	return "";
//}

void FSISPOptimizerWCP::setVariables(vector<lp_result_set> lp_result)
{
	for(uint32_t i = 0; i < lp_result.size(); i++)
	{
		if(lp_result[i].variable.compare("wentry") == 0)
		{
			// found the estimated WCET for the WCP-sensitive approach
			LOG_INFO(logger, "The FSISP assignment estimates the WCET with: " << lp_result[i].value);
			ilp_solution_wcet_estimate = lp_result[i].value;
		}
		else if(lp_result[i].variable.compare("sp") == 0)
		{
			// found the used bbsisp size
			LOG_INFO(logger, "Used FSISP size is: " << lp_result[i].value);
			ilp_solution_used_size = lp_result[i].value;
		}
	}
}

void FSISPOptimizerWCP::generateFunctionMembershipConstraints(void)
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
//							stringstream s;
//							s << "a" << w << " = f" << i << ";" << endl;
//							function_membership_constraints.push_back(s.str());
//
//							found_bb_in_function = true;
//						}
//					}
//
//				}
//			}
//		}
//		if(found_bb_in_function)
//		{
//			stringstream str;
//			str << "bin f" << i << ";" << endl; 
//			binary_domains.push_back(str.str());
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
					stringstream s;
					s << "a" << v << " = f" << i << ";" << endl;
					function_membership_constraints.push_back(s.str());

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
		stringstream str;
		str << "bin f" << *it << ";" << endl; 
		binary_domains.push_back(str.str());
	}
}

void FSISPOptimizerWCP::setFunctionAssignment(vector<lp_result_set> lp_result)
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

vector<addr_label_t> FSISPOptimizerWCP::getFunctionAssignment(void)
{
	return assigned_functions;
}

