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
#include "sisp_optimizer_if.hpp"
#include "dlp_factory.hpp"
#include "isa_factory.hpp"
#include "arch_cfg_factory.hpp"

LoggerPtr SISPOptimizer_IF::logger(Logger::getLogger("SISPOptimizer_IF"));


SISPOptimizer_IF::SISPOptimizer_IF(ControlFlowGraph cfgraph, CFGVertex entry, CFGVertex exit) : CFGLoopHelper(cfgraph, entry, exit) , solution_type(SolutionNotCalculated), ilp_solution_used_size(0)
{

	// get node property structures
	nodeTypeNProp = get(nodetype_t(), cfg);
	startAddrNProp = get(startaddr_t(), cfg);
	startAddrStringNProp = get(startaddrstring_t(), cfg);
	bbSizeNProp = get(bbsize_t(), cfg);
	bbCodeNProp = get(bbcode_t(), cfg);

	// get edge property structures
	costEProp = get(cost_t(), cfg);
	costOnChipEProp = get(cost_onchip_t(), cfg);
	costOffChipEProp = get(cost_offchip_t(), cfg);
	memPenaltyEProp = get(mem_penalty_t(), cfg);
	actEProp = get(activation_t(), cfg);

	conf = Configuration::getInstance();


	arch_cfg = ArchConfigFactory::getInstance()->getArchConfigObject();
	dlp = DLPFactory::getInstance()->getDLPObject();
	isa = ISAFactory::getInstance()->getISAObject();
}

SISPOptimizer_IF::~SISPOptimizer_IF()
{
	assigned_bbaddrs.clear();
	ilp_solution_used_size = 0;
}

void SISPOptimizer_IF::setSize(uint32_t size)
{
	sisp_size = size;
	used_sisp_size = 0;
}

vector <uint32_t> SISPOptimizer_IF::getBlockAssignment(void)
{
	return assigned_bbaddrs;
}


uint32_t SISPOptimizer_IF::getUsedSispSize(void)
{
	uint32_t used_size=0;

	if(((mem_type_t)conf->getUint(CONF_MEMORY_TYPE) == BBSISP_JP) || ((mem_type_t)conf->getUint(CONF_MEMORY_TYPE) == BBSISP_JP_WCP) || (conf->getBool(CONF_MEMORY_BBSISP_ADD_JUMP_PENALTIES_TO_WCET)))
	{
		// obtain the size including the size penalties.
		used_size=getUsedSizeIncludingSizePenalties();
		LOG_INFO(logger, "Due to assignment of BBs a size penalty of " << (used_size - used_sisp_size) << " for the assigned BBs is considered.");
	}
	else
	{
		used_size=used_sisp_size;
	}

	return used_size;
}


vector<lp_result_set> SISPOptimizer_IF::writeAndSolveILPFile(vector<string> ilp_formulation)
{
	stringstream str;
	for(vector<string>::iterator it=ilp_formulation.begin(); it != ilp_formulation.end(); it++)
	{
		str << *it;
	}

	return writeAndSolveILPFile(str.str());
}

vector<lp_result_set> SISPOptimizer_IF::writeAndSolveILPFile(string ilp_formulation)
{
	ofstream ilpfile;
	string log_filename =  conf->getString(CONF_LOG_FILE);
	string ilpfile_name = regex_replace(log_filename, regex("[.period.][[:alnum:]]*$"), "", match_default | format_all) + "_bb_assignment.ilp";
	LOG_INFO(logger, "Writing LP to obtain the BB assignment to " << ilpfile_name);
	ilpfile.open(ilpfile_name.c_str());

	assert(ilpfile.is_open());

	ilpfile << ilp_formulation;

	ilpfile.close();

	LpSolver lps(ilp_formulation, conf->getString(CONF_LP_SOLVE_PARAMETERS));
	vector<lp_result_set> tmp =  lps.lpSolve();

	solution_type = lps.getSolutionType();

	return tmp;
}

void SISPOptimizer_IF::setAssignment(vector<lp_result_set> lp_result)
{
	used_sisp_size = 0;
	uint32_t cur_lp_var_id;
	for(uint32_t i = 0; i < lp_result.size(); i++)
	{
		// process only the variables that represent the assignment of the blocks: a[0-9]+
		if(boost::regex_search(lp_result[i].variable, boost::regex("^a[0-9]+$")))
		{
			if(lp_result[i].value == 1)
			{
				cur_lp_var_id = strtoul(((lp_result[i].variable).substr(1,(lp_result[i].variable).length()-1)).c_str(), NULL, 10);
				LOG_DEBUG(logger, "Assignment of basic block a" << cur_lp_var_id << " found.");
				CFGVertex v = cur_lp_var_id;
				assigned_bbaddrs.push_back(get(startAddrNProp, v));
				used_sisp_size += get(bbSizeNProp, v);
			}
		}
	}


	// TODO instrument the code ?!! update .elf-file, reparse it .... 
}


CFGVertex SISPOptimizer_IF::getFunctStartBBlock(CFGVertex v)
{
	CFGVertex processing = v;
	cfgOutEdgeIter eop;

	assert((get(nodeTypeNProp, processing) == CallPoint) || (get(nodeTypeNProp, processing) == Entry));

	if(get(nodeTypeNProp, processing) == CallPoint)
	{
		// get the functions entry node
		assert(out_degree(processing, cfg) == 1);
		eop = out_edges(processing, cfg);
		processing = target(*eop.first, cfg);
	}

	if(get(nodeTypeNProp, processing) == Entry)
	{
		// get the functions start basic block
		assert(out_degree(processing, cfg) == 1);
		eop = out_edges(processing, cfg);
		processing = target(*eop.first, cfg);
	}
	else
	{
		assert(false);
	}

	assert(get(nodeTypeNProp, processing) == BasicBlock);
	return processing;
}


CFGVertex SISPOptimizer_IF::getFunctEndBBlock(CFGVertex v)
{
	CFGVertex processing = v;
	cfgInEdgeIter eip;

	assert((get(nodeTypeNProp, processing) == ReturnPoint) || (get(nodeTypeNProp, processing) == Exit));

	if(get(nodeTypeNProp, processing) == ReturnPoint)
	{
		// get the functions entry node
		assert(in_degree(processing, cfg) == 1);
		eip = in_edges(processing, cfg);
		processing = source(*eip.first, cfg);
	}

	if(get(nodeTypeNProp, processing) == Exit)
	{
		// get the functions start basic block
		assert(in_degree(processing, cfg) == 1);
		eip = in_edges(processing, cfg);
		processing = source(*eip.first, cfg);
	}
	else
	{
		assert(false);
	}

	assert(get(nodeTypeNProp, processing) == BasicBlock);
	return processing;
}


CFGVertex SISPOptimizer_IF::getFunctCallingBBlock(CFGVertex v)
{
	CFGVertex processing = v;
	cfgInEdgeIter eip;

	assert(get(nodeTypeNProp, processing) == CallPoint);

	if(get(nodeTypeNProp, processing) == CallPoint)
	{
		// get the functions entry node
		assert(in_degree(processing, cfg) == 1);
		eip = in_edges(processing, cfg);
		processing = source(*eip.first, cfg);
	}
	else
	{
		assert(false);
	}

	assert(get(nodeTypeNProp, processing) == BasicBlock);
	return processing;
}

CFGVertex SISPOptimizer_IF::getFunctReturnToBB(CFGVertex v)
{
	CFGVertex processing = v;
	cfgOutEdgeIter eop;

	assert(get(nodeTypeNProp, processing) == ReturnPoint);

	if(get(nodeTypeNProp, processing) == ReturnPoint)
	{
		// get the functions entry node
		assert(out_degree(processing, cfg) == 1);
		eop = out_edges(processing, cfg);
		processing = target(*eop.first, cfg);
	}
	else
	{
		assert(false);
	}

	assert(get(nodeTypeNProp, processing) == BasicBlock);
	return processing;
}

uint32_t SISPOptimizer_IF::getUsedSizeIncludingSizePenalties(void)
{
	uint32_t used_size=0;

	for(cfgVertexIter vp = vertices(cfg); vp.first != vp.second; ++vp.first)
	{
		CFGVertex v = *vp.first;
		if(get(nodeTypeNProp, v) == BasicBlock)
		{
			if(isAssigned(get(startAddrNProp, v)))
			{
				uint32_t bb_cost = get(bbSizeNProp, v);
				uint32_t bb_penalty = getSizePenaltyForAssignedBasicBlock(v);
				uint32_t block_cost = bb_cost + bb_penalty;

				LOG_DEBUG(logger, "cost incl. penalties for bb: " << v << " 0x" << hex << get(startAddrNProp, v) << dec << " cost: " << block_cost << " (" << bb_cost << "+" << bb_penalty << ")");
				used_size += block_cost;
			}

		}
	}

	return used_size;
}

uint32_t SISPOptimizer_IF::getSizePenaltyForAssignedBasicBlock(CFGVertex v)
{
	bool size_penalty_ca = false; // penalty for continuous addressing (an jump was added)
	bool size_penalty_js = false; // penalty for extending a short jump instruction
	bool size_penalty_cs = false; // penalty for extenting a short call instruciton

	uint32_t size_penalty=0;
	for(cfgOutEdgeIter eop = out_edges(v, cfg); eop.first != eop.second; ++eop.first)
	{	
		CFGEdge e = *eop.first;
		CFGVertex tgt = target(e, cfg);

		// Notice: only direct connections and calls without any Meta node between two basic blocks are handled (currently there should be no scenario that is not covered)
		if(get(nodeTypeNProp, tgt) == BasicBlock)
		{
			// The basic block v gets a penalty if the following basic block is not in the BBSISP, and therefore a connecting jump has to be added or a short jump was extended.
			if(!isAssigned(get(startAddrNProp, tgt)))
			{
				displacement_type_t displacement; 
				connection_type_t connection; 
				tie(connection, displacement) = getConnectionAndDisplacementType(e);
				if((connection != UnknownConnectionType) && (connection != Return))
				{
					if(connection == ContinuousAdressing)
					{
						assert((size_penalty_ca == false) && (size_penalty_cs == false)); // calls are not possible for a bb that is connected with ContinuousAdressing. And there has to be only one ContinuousAdressing penalty!
						size_penalty_ca = true;
						size_penalty += arch_cfg->getSizePenalty(connection, displacement);
					}
					else if(connection == Jump)
					{
						assert(size_penalty_cs == false); // calls are not possible for a bb that is connected with a Jump.
						if(size_penalty_js == false) // Add the size penalty for a short jump only once. (Due to indirect jumps it is possible that there are multiple jump targets, i.e. multiple jump out edges for one BB)
						{
							size_penalty += arch_cfg->getSizePenalty(connection, displacement);
						}
						size_penalty_js = true;
					}
				}
			}
		}
		else if(get(nodeTypeNProp, tgt) == CallPoint)
		{
			// get the first bb of the called function
			assert(out_degree(tgt, cfg) == 1);
			cfgOutEdgeIter eoi = out_edges(tgt, cfg);
			CFGVertex function_entry = target(*eoi.first, cfg);
			assert(get(nodeTypeNProp, function_entry) == Entry);
			assert(out_degree(function_entry, cfg) == 1);
			eoi = out_edges(function_entry, cfg);
			CFGVertex c_tgt = target(*eoi.first, cfg);
			assert(get(nodeTypeNProp, c_tgt) == BasicBlock);

			// The basic block v gets a penalty if the first bb of the called function is not in the BBSISP, and therefore a short call is extended.
			if(!isAssigned(get(startAddrNProp, c_tgt)))
			{
				displacement_type_t displacement; 
				connection_type_t connection; 
				tie(connection, displacement) = getConnectionAndDisplacementType(e);
				if(connection == Call)
				{
					assert((size_penalty_ca == false) && (size_penalty_js == false)); // a bb can only be terminated by ONE call instr.
					if(size_penalty_cs == false) // Because for indirect calls it is possible that multiple targets are present, but the penalty can only be charged once.
					{
						size_penalty += arch_cfg->getSizePenalty(connection, displacement);
					}
					size_penalty_cs = true;
				}
			}

		}
	}
	return size_penalty;
}

bool SISPOptimizer_IF::isAssigned(uint32_t addr)
{
	for(vector<uint32_t>::iterator it = assigned_bbaddrs.begin(); it < assigned_bbaddrs.end(); it++)
	{
		if(*it == addr)
		{
			return true;
		}
	}
	return false;
}


displacement_type_t SISPOptimizer_IF::getDisplacementTypeOfBB(CFGVertex node)
{
 	displacement_type_t displacement;
	vector<string> instrs;

	assert(get(nodeTypeNProp, node) == BasicBlock);

	string bb_code = get(bbCodeNProp, node);
	split(instrs, bb_code, boost::is_any_of("\r"));
	string last_instr = instrs.back();
	while(last_instr == "" && !instrs.empty())
	{
		instrs.pop_back();
		last_instr = instrs.back();
	}
	assert(!instrs.empty() && dlp->isCodeLine(last_instr) );
	if(dlp->isBranchInstr(last_instr))
	{
		displacement = isa->getDisplacementType(dlp->getInstructionFromCodeLine(last_instr));
		assert(displacement != UnknownDisplacementType);
	}
	else
	{
		displacement = NoDisplacement;
	}
	return displacement;
}


pair<connection_type_t, displacement_type_t> SISPOptimizer_IF::getConnectionAndDisplacementType(CFGEdge e)
{
	connection_type_t connection = UnknownConnectionType; 
	displacement_type_t displacement = UnknownDisplacementType; 

	assert(get(nodeTypeNProp, source(e, cfg)) == BasicBlock);

	switch(get(edgeTypeEProp, e))
	{
		case ForwardStep: 
			{
				connection = ContinuousAdressing;
				displacement = NoDisplacement;
				break;
			}
		case ForwardJump: 
		case BackwardJump: 
		case Meta:
			{
				switch(get(nodeTypeNProp, target(e, cfg)))
				{
					case BasicBlock:
						{
							connection = Jump;
							displacement = getDisplacementTypeOfBB(source(e, cfg));
							break;
						}
					case CallPoint:
						{
							connection = Call;
							displacement = getDisplacementTypeOfBB(source(e, cfg));
							break;
						}
					case ReturnPoint:
						{
							connection = Return;
							displacement = NoDisplacement;
							break;
						}
					case Exit:
						{
							connection = UnknownConnectionType;
							displacement = NoDisplacement;
							break;
						}
					default:
						{
							assert(false);
						}
				}
				break;
			}
		default:
			{
				assert(false);
			}
	}
	return make_pair(connection, displacement);
}

lp_solution_t SISPOptimizer_IF::getSolutionType(void)
{
	return solution_type;
}


void SISPOptimizer_IF::setVariables(vector<lp_result_set> lp_result)
{
	for(uint32_t i = 0; i < lp_result.size(); i++)
	{
		if(lp_result[i].variable.compare("sp") == 0)
		{
			// found the used bbsisp size
			LOG_INFO(logger, "Used BBSISP size is: " << lp_result[i].value);
			ilp_solution_used_size = lp_result[i].value;
		}
	}
}

uint32_t SISPOptimizer_IF::getEstimatedUsedSize(void)
{
	return ilp_solution_used_size;
}

sisp_result_t SISPOptimizer_IF::getResults(void)
{
	sisp_result_t tmp;
	tmp.used_size = getUsedSispSize();
	tmp.estimated_used_size = getEstimatedUsedSize();
	tmp.assigned_bbs = getBlockAssignment();
	tmp.solution_type = getSolutionType();
	tmp.estimated_timing = numeric_limits<uint32_t>::max();
	tmp.estimated_jump_penalty = numeric_limits<uint64_t>::max();

	return tmp;
}
