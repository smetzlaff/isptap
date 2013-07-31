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
#include "bbsisp_jp_optimizer_wcp.hpp"
#include "dlp_factory.hpp"
#include "isa_factory.hpp"
#include "arch_cfg_factory.hpp"

BBSISPOptimizerJPWCP::BBSISPOptimizerJPWCP(ControlFlowGraph cfgraph, CFGVertex entry, CFGVertex exit) : BBSISPOptimizerWCP(cfgraph, entry, exit) 
{
	bbCodeNProp = get(bbcode_t(), cfg);

	arch_cfg = ArchConfigFactory::getInstance()->getArchConfigObject();
	dlp = DLPFactory::getInstance()->getDLPObject();
	isa = ISAFactory::getInstance()->getISAObject();

	ilp_solution_jump_penalty = 0;
}

BBSISPOptimizerJPWCP::~BBSISPOptimizerJPWCP()
{
	clear();
}
void BBSISPOptimizerJPWCP::clear(void)
{
	BBSISPOptimizerWCP::clear();
	jump_penalties.clear();
	binary_constraints.clear();
}

void BBSISPOptimizerJPWCP::calculateBlockAssignment()
{
	clear();
	generateBlockCostsConstraints();
	generateBlockSizeConstraint();
	generateBinaryDomain();
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
//	sort(cfg_ilps.begin(), cfg_ilps.end());
	for(it = cfg_ilps.begin(); it != cfg_ilps.end(); it++)
	{
		ilp_formulation << *it;
	}
	ilp_formulation << "\n// Basic block cost contraints:\n";
//	sort(block_cost_contraints.begin(), block_cost_contraints.end());
	for(it = block_cost_contraints.begin(); it != block_cost_contraints.end(); it++)
	{
		ilp_formulation << *it;
	}
	ilp_formulation << "\n// Overall jump penalties:\n";
	ilp_formulation << "jp = 0";
	for(it = jump_penalties.begin(); it != jump_penalties.end(); it++)
	{
		ilp_formulation << *it;
	}
	ilp_formulation << ";\n";
	ilp_formulation << "\n// Basic block size contraints:\n";
//	sort(block_size_constraints.begin(), block_size_constraints.end());
	for(it = block_size_constraints.begin(); it != block_size_constraints.end(); it++)
	{
		ilp_formulation << *it;
	}
	ilp_formulation << "\n// XOR & ANDN formulations:\n";
	sort(binary_constraints.begin(), binary_constraints.end());
	for(it = binary_constraints.begin(); it != binary_constraints.end(); it++)
	{
		ilp_formulation << *it;
	}
	ilp_formulation << "\n// Binary domains:\n";
//	sort(binary_domains.begin(), binary_domains.end());
	for(it = binary_domains.begin(); it != binary_domains.end(); it++)
	{
		ilp_formulation << *it;
	}

	LOG_INFO(logger, "Formulation: " << ilp_formulation.str());

	vector<lp_result_set> lp_result = writeAndSolveILPFile(ilp_formulation.str());
	setAssignment(lp_result);
	setVariables(lp_result);
}


void BBSISPOptimizerJPWCP::generateBlockCostsConstraints(void)
{
	if(!conf->getBool(CONF_BBSISP_WCP_SHRINK_ILP_FORMULATION))
	{
		for(cfgEdgeIter ep = edges(cfg); ep.first != ep.second; ++ep.first)
		{
			CFGEdge e = *ep.first;
			if(get(nodeTypeNProp, source(e, cfg)) == BasicBlock)
			{
				stringstream str;
				str << "ce" << source(e,cfg) << "t" << target(e ,cfg) << " = " << get(costOffChipEProp, e);
				if(get(costOffChipEProp, e) != get(costOnChipEProp, e))
				{
					str << " - " << (get(costOffChipEProp, e) - get(costOnChipEProp, e)) << " a" << source(e,cfg);
				}
				string penalty_string = getJumpPenaltyTermForBB(source(e, cfg), e);
				if(penalty_string != "")
				{
					str << penalty_string;
					bin_constraint_push_back(getXorConstraint(source(e, cfg), target(e, cfg)));
				}
				str << ";" << endl;
				block_cost_contraints.push_back(str.str());
			}
		}
	}
}

string BBSISPOptimizerJPWCP::getEdgeCostConstraint(CFGEdge e)
{
	stringstream str;
	str << get(costOffChipEProp, e);
	uint32_t benefit = get(costOffChipEProp, e) - get(costOnChipEProp, e);
	if(benefit != 0)
	{
		str << " - " << benefit << " a" << source(e,cfg);
	}
	string penalty_string = getJumpPenaltyTermForBB(source(e, cfg), e);
	if(penalty_string != "")
	{
		str << penalty_string;
		bin_constraint_push_back(getXorConstraint(source(e, cfg), target(e, cfg)));
	}

	return str.str();
}


string BBSISPOptimizerJPWCP::getEdgeCostConstraint(CFGEdge e, uint32_t multiplicator)
{
	stringstream str;
	str << (multiplicator * get(costOffChipEProp, e));
	uint32_t benefit = get(costOffChipEProp, e) - get(costOnChipEProp, e);
	if(benefit != 0)
	{
		str << " - " << (multiplicator * benefit) << " a" << source(e,cfg);
	}
	string penalty_string = getJumpPenaltyTermForBB(source(e, cfg), e, multiplicator);
	if(penalty_string != "")
	{
		str << penalty_string;
		bin_constraint_push_back(getXorConstraint(source(e, cfg), target(e, cfg)));
	}

	return str.str();
}


void BBSISPOptimizerJPWCP::generateBlockSizeConstraint(void)
{
	stringstream block_size_stream;
	cfgVertexIter vp = vertices(cfg);
	cfgOutEdgeIter eop;
	assert(vp.first != vp.second);
	CFGVertex v = *vp.first;

	// search as long as the first bb was found
	while((get(nodeTypeNProp, v) != BasicBlock) && (vp.first != vp.second))
	{
		++vp.first;
		assert(vp.first != vp.second);
		v= *vp.first;
	}

	assert(get(nodeTypeNProp, v) == BasicBlock);
	if(!conf->getBool(CONF_BBSISP_WCP_SHRINK_ILP_FORMULATION))
	{
		block_size_stream << " sp" << v << endl; 
		block_size_constraints.push_back(getBBSizeWithPenaltyConstraint(v));

		++vp.first;

		for(; vp.first != vp.second; ++vp.first)
		{
			v= *vp.first;
			if(get(nodeTypeNProp, v) == BasicBlock)
			{
				block_size_stream << " + " << " sp" << v << endl; 
				block_size_constraints.push_back(getBBSizeWithPenaltyConstraint(v));
			}
		}

		block_size_stream << " = sp;" << endl;
		block_size_stream << "sp <= " << sisp_size << ";" << endl << endl;

		// but the sum into the font of the vector
		block_size_constraints.insert(block_size_constraints.begin(), block_size_stream.str());
	}
	else // conf->getBool(CONF_BBSISP_WCP_SHRINK_ILP_FORMULATION)
	{
		block_size_stream << getBBSizeWithPenaltyConstraint(v) << endl;

		++vp.first;

		for(; vp.first != vp.second; ++vp.first)
		{
			v= *vp.first;
			if(get(nodeTypeNProp, v) == BasicBlock)
			{
				block_size_stream << " + " << getBBSizeWithPenaltyConstraint(v) << endl;
			}
		}

		block_size_stream << " = sp;" << endl;
		block_size_stream << "sp <= " << sisp_size << ";" << endl << endl;

		// but the sum into the font of the vector
		block_size_constraints.insert(block_size_constraints.begin(), block_size_stream.str());
	}
}

string BBSISPOptimizerJPWCP::getJumpPenaltyTermForBB(CFGVertex src, CFGVertex tgt, node_type_t node_type)
{
	stringstream stream;

	assert(get(nodeTypeNProp, src) == BasicBlock);
	assert(get(nodeTypeNProp, tgt) == BasicBlock);

	if(node_type == CallPoint)
	{
		displacement_type_t displacement = getDisplacementTypeOfBB(src);
		// ignore constraint if the penalty is zero
		uint32_t penalty = arch_cfg->getJumpPenalty(Call, displacement);
		if(penalty != 0)
		{
			stream << " + " << penalty << " xor" << src << "00" << tgt;
		}
	}
	else if(node_type == ReturnPoint)
	{
		// Do Nothing
	}
	else
	{
		assert(false);
	}

	jump_penalties.push_back(stream.str());
	return stream.str();
}
string BBSISPOptimizerJPWCP::getJumpPenaltyTermForBB(CFGVertex src, CFGEdge e)
{
	stringstream stream;

	assert(get(nodeTypeNProp, src) == BasicBlock);

	displacement_type_t displacement; 
	connection_type_t connection;

	tie(connection, displacement) = getConnectionAndDisplacementType(e);
	
	if(connection != UnknownConnectionType)
	{
		LOG_DEBUG(logger, "Building jump constraint for " << src << " to " << target(e, cfg) << " connection: " << connection << " displacement: " << displacement << " edge: " << get(edgeTypeEProp, e));
		if(connection == Call)
		{
			// ignore constraint if the penalty is zero
			uint32_t penalty = arch_cfg->getJumpPenalty(connection, displacement);
			if(penalty != 0)
			{
				stream << " + " << penalty << " xor" << src << "00" << getFunctStartBBlock(target(e, cfg));
			}
		}
		else if(connection == Return)
		{
			// do nothing
		}
		else
		{
			assert(get(nodeTypeNProp, target(e, cfg)) == BasicBlock);
			// ignore constraint if the penalty is zero
			uint32_t penalty = arch_cfg->getJumpPenalty(connection, displacement);
			if(penalty != 0)
			{
				stream << " + " << penalty << " xor" << src << "00" << target(e, cfg);
			}
		}
	}

	jump_penalties.push_back(stream.str());
	return stream.str();
}

string BBSISPOptimizerJPWCP::getJumpPenaltyTermForBB(CFGVertex src, CFGEdge e, uint32_t multiplicator)
{
	stringstream stream;

	assert(get(nodeTypeNProp, src) == BasicBlock);

	displacement_type_t displacement; 
	connection_type_t connection;

	tie(connection, displacement) = getConnectionAndDisplacementType(e);
	
	if(connection != UnknownConnectionType)
	{
		LOG_DEBUG(logger, "Building jump constraint for " << src << " to " << target(e, cfg) << " connection: " << connection << " displacement: " << displacement << " edge: " << get(edgeTypeEProp, e));
		if(connection == Call)
		{
			// ignore constraint if the penalty is zero
			uint32_t penalty = arch_cfg->getJumpPenalty(connection, displacement);
			if(penalty != 0)
			{
				stream << " + " << (multiplicator * penalty) << " xor" << src << "00" << getFunctStartBBlock(target(e, cfg));
			}
		}
		else if(connection == Return)
		{
			// do nothing
		}
		else
		{
			assert(get(nodeTypeNProp, target(e, cfg)) == BasicBlock);
			// ignore constraint if the penalty is zero
			uint32_t penalty = arch_cfg->getJumpPenalty(connection, displacement);
			if(penalty != 0)
			{
				stream << " + " << (multiplicator * penalty) << " xor" << src << "00" << target(e, cfg);
			}
		}
	}

	jump_penalties.push_back(stream.str());
	return stream.str();
}

string BBSISPOptimizerJPWCP::getXorConstraint(CFGVertex src, CFGVertex tgt)
{
	stringstream stream;

	if((get(nodeTypeNProp, src) == BasicBlock) && (get(nodeTypeNProp, tgt) == BasicBlock))
	{

		LOG_DEBUG(logger, "Adding XOR constraint for " << src << " and " << tgt);

		// formulate the XOR assignment of src_id and tgt_id
		stream << "xor" << src << "00" << tgt << " <= a" << src << " + a" << tgt << ";" << endl;
		stream << "xor" << src << "00" << tgt << " >= a" << src << " - a" << tgt << ";" << endl;
		stream << "xor" << src << "00" << tgt << " >= - a" << src << " + a" << tgt << ";" << endl;
		stream << "xor" << src << "00" << tgt << " <= 2 - a" << src << " - a" << tgt << ";" << endl;
		// TODO: Need to set variable as binary. Otherwise if this will cause significant extra lp_solve run-time, cross check results if they are really binary.
//		stringstream bstream;
//		bstream << "bin xor" << src << "00" << tgt << ";" << endl;
//		binary_domains.push_back(bstream.str());
		stream << endl;
	}

	return stream.str();
}

string BBSISPOptimizerJPWCP::getAndNConstraint(CFGVertex src, CFGVertex tgt)
{
	stringstream stream;

	if((get(nodeTypeNProp, src) == BasicBlock) && (get(nodeTypeNProp, tgt) == BasicBlock))
	{

		LOG_DEBUG(logger, "Adding ANDN constraint for " << src << " and " << tgt);

		// formulate the ANDN assignment of src_id and tgt_id
		stream << "andn" << src << "00" << tgt << " <= a" << src << ";" << endl;
		stream << "andn" << src << "00" << tgt << " <= 1 - a" << tgt << ";" << endl;
		stream << "andn" << src << "00" << tgt << " + 1 >= a" << src << " + 1 - a" << tgt << ";" << endl;
		// TODO: Need to set variable as binary. Otherwise if this will cause significant extra lp_solve run-time, cross check results if they are really binary.
//		stringstream bstream;
//		bstream << "bin andn" << src << "00" << tgt << ";" << endl;
//		binary_domains.push_back(bstream.str());
		stream << endl;
	}

	return stream.str();
}

string BBSISPOptimizerJPWCP::getBBSizeWithPenaltyConstraint(CFGVertex v)
{
	stringstream stream;
	stringstream stream2;

	assert(get(nodeTypeNProp, v) == BasicBlock);
	if(!conf->getBool(CONF_BBSISP_WCP_SHRINK_ILP_FORMULATION))
	{
		stream << "sp" << v << " = ";
	}

	uint32_t bb_size = get(bbSizeNProp, v);
	for(cfgOutEdgeIter eop = out_edges(v, cfg); eop.first != eop.second; ++eop.first)
	{
		CFGEdge e = *eop.first;
		CFGVertex tgt = target(e, cfg);
		// FIXME Only direct connections without any Meta node between two basic blocks can be handled.
		// TODO: Add support for calls

		if(get(nodeTypeNProp, tgt) == BasicBlock)
		{
			displacement_type_t displacement; 
			connection_type_t connection; 
			tie(connection, displacement) = getConnectionAndDisplacementType(e);

			if((connection != UnknownConnectionType) && (connection != Return))
			{
				// FIXME Round the costs up to multiples of the fetch word size
				uint32_t penalty = arch_cfg->getSizePenalty(connection, displacement);
				// ignore constraint if the penalty is zero
				if(penalty != 0)
				{
					stream2 << " + " << penalty << " andn" << v << "00" << tgt;
					bin_constraint_push_back(getAndNConstraint(v, tgt));
				}
			}
		}
		else if(get(nodeTypeNProp, tgt) == CallPoint)
		{
			displacement_type_t displacement; 
			connection_type_t connection; 
			tie(connection, displacement) = getConnectionAndDisplacementType(e);

			if(connection == Call)
			{
				uint32_t penalty = arch_cfg->getSizePenalty(connection, displacement);
				// ignore constraint if the penalty is zero
				if(penalty != 0)
				{
					// get the first basic block of the called function
					assert(get(nodeTypeNProp, tgt) == CallPoint);
					assert(out_degree(tgt, cfg) == 1);
					cfgOutEdgeIter eoi = out_edges(tgt, cfg);
					CFGVertex function_entry = target(*eoi.first, cfg);
					assert(get(nodeTypeNProp, function_entry) == Entry);
					assert(out_degree(function_entry, cfg) == 1);
					eoi = out_edges(function_entry, cfg);
					CFGVertex c_tgt = target(*eoi.first, cfg);
					assert(get(nodeTypeNProp, c_tgt) == BasicBlock);

					stream2 << " + " << penalty << " andn" << v << "00" << c_tgt;
					bin_constraint_push_back(getAndNConstraint(v, c_tgt));
				}
			}
		}
	}

	stream << bb_size << " a" << v << stream2.str();

	if(!conf->getBool(CONF_BBSISP_WCP_SHRINK_ILP_FORMULATION))
	{
		stream << ";" << endl;
	}

	return stream.str();
}

void BBSISPOptimizerJPWCP::bin_constraint_push_back(string bin_constraint)
{
	if(bin_constraint != "")
	{
		binary_constraints.push_back(bin_constraint);
	}
}


string BBSISPOptimizerJPWCP::getPenaltyForFunctionEntering(CFGVertex call_point, CFGVertex function_entry_node)
{
	// jump penalty from calling basic block to first basic block in the function (the penalty occurs if one of those block, but not both, is assigned to the BBSISP)
	CFGVertex bb_calling_f = getFunctCallingBBlock(call_point);
	CFGVertex bb_f_start = getFunctStartBBlock(function_entry_node);
	bin_constraint_push_back(getXorConstraint(bb_calling_f, bb_f_start));
	return getJumpPenaltyTermForBB(bb_calling_f, bb_f_start, CallPoint);
}


string BBSISPOptimizerJPWCP::getPenaltyForFunctionExit(CFGVertex return_point, CFGVertex function_exit_node)
{
	// jump penalty from the terminating basic block of the function to the first basic block after return point (the penalty occurs if one of those block, but not both, is assigned to the BBSISP)
	CFGVertex bb_f_last = getFunctEndBBlock(function_exit_node);
	CFGVertex bb_returningfrom_f = getFunctReturnToBB(return_point);
	bin_constraint_push_back(getXorConstraint(bb_f_last, bb_returningfrom_f));
	return getJumpPenaltyTermForBB(bb_f_last, bb_returningfrom_f, ReturnPoint);
}

void BBSISPOptimizerJPWCP::setVariables(vector<lp_result_set> lp_result)
{
	for(uint32_t i = 0; i < lp_result.size(); i++)
	{
		if(lp_result[i].variable.compare("wentry") == 0)
		{
			// found the estimated WCET for the WCP-sensitive approach
			LOG_INFO(logger, "The bbsisp assignment estimates the WCET with: " << lp_result[i].value);
			ilp_solution_wcet_estimate = lp_result[i].value;
		}
		else if(lp_result[i].variable.compare("sp") == 0)
		{
			// found the used bbsisp size
			LOG_INFO(logger, "Used BBSISP size is: " << lp_result[i].value);
			ilp_solution_used_size = lp_result[i].value;
		}
		else  if(lp_result[i].variable.compare("jp") == 0)
		{
			// found the used bbsisp size
			LOG_INFO(logger, "Overall BBSISP jump penalty is: " << lp_result[i].value);
			ilp_solution_jump_penalty = lp_result[i].value;
		}
	}
}

uint32_t BBSISPOptimizerJPWCP::getEstimatedJumpPenalty(void)
{
	return ilp_solution_jump_penalty;
}

sisp_result_t BBSISPOptimizerJPWCP::getResults(void)
{
	sisp_result_t tmp = SISPOptimizer_IF::getResults();
	tmp.estimated_timing = getEstimatedWCET();
	tmp.estimated_jump_penalty = getEstimatedJumpPenalty();

	return tmp;
}
