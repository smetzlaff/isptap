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
#include "bbsisp_jp_optimizer.hpp"
#include "dlp_factory.hpp"
#include "isa_factory.hpp"
#include "arch_cfg_factory.hpp"

BBSISPOptimizerJP::BBSISPOptimizerJP(ControlFlowGraph cfgraph, CFGVertex entry, CFGVertex exit) : BBSISPOptimizer(cfgraph, entry, exit)
{
	bbCodeNProp = get(bbcode_t(), cfg);

	arch_cfg = ArchConfigFactory::getInstance()->getArchConfigObject();
	dlp = DLPFactory::getInstance()->getDLPObject();
	isa = ISAFactory::getInstance()->getISAObject();

	ilp_solution_jump_penalty = 0;
}



BBSISPOptimizerJP::~BBSISPOptimizerJP()
{
	clear();
}

void BBSISPOptimizerJP::clear(void)
{
	ilp_knapsack_formulation.clear();
	assigned_bbaddrs.clear();
	bb_data.clear();
	bb_connect.clear();
	bbidMap.clear();
	dispMap.clear();
	binary_constraints.clear();
}


void BBSISPOptimizerJP::calculateBlockAssignment(void)
{
	clear();
	generateBBRecords();
	generateBBConnections();
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

void BBSISPOptimizerJP::registerNodeIdMapping(CFGVertex v, uint32_t node_id)
{
	// register bb in BB ID CFG map
	BbidCfgMap::iterator ins_pos;
	bool ins_bool;
	tie(ins_pos, ins_bool) = bbidMap.insert(make_pair(v,node_id));
	assert(ins_bool);
}

void BBSISPOptimizerJP::generateBBConnections(void)
{
	CFGVertex src,tgt;
	CFGEdge e;
	cfgEdgeIter ep;
	BbidCfgMap::iterator pos;

	for (ep = edges(cfg); ep.first != ep.second; ++ep.first)
	{
		bb_connect_t tmp;
		e = *ep.first;
		edge_type_t etype = get(edgeTypeEProp, e);
		

		switch(etype)
		{
			case ForwardStep:
			case ForwardJump:
			case BackwardJump:
			case Meta:
				{
					src = source(e, cfg);
					tgt = target(e, cfg);

					// TODO: what if between two bbs is a meta block?!
					// XXX Does not work if between two bbs is a meta block
					node_type_t src_type = get(nodeTypeNProp, src);
					node_type_t tgt_type = get(nodeTypeNProp, tgt);
					if((src_type == BasicBlock) && (tgt_type == BasicBlock))
					{
						pos = bbidMap.find(src);
						tmp.src_id = pos->second;
						assert(pos != bbidMap.end());

						pos = bbidMap.find(tgt);
						tmp.tgt_id = pos->second;
						assert(pos != bbidMap.end());

						if(etype == ForwardStep)
						{
							tmp.type = ContinuousAdressing;
						}
						else
						{
							tmp.type = Jump;
						}
						tmp.activation_count = get(actEProp, e);
						bb_connect.push_back(tmp);

						LOG_DEBUG(logger, "Generating BB connection src cfg id: " <<  src << " src id: a" << tmp.src_id << " tgt cfg id: " << tgt << " tgt id: a" << tmp.tgt_id << " connection type: " << tmp.type);
					}
					else if(tgt_type == CallPoint)
					{
						assert(src_type == BasicBlock);

						pos = bbidMap.find(src);
						assert(pos != bbidMap.end());
						tmp.src_id = pos->second;

						CFGVertex bb_f_first = getFunctStartBBlock(tgt);
						pos = bbidMap.find(bb_f_first);
						assert(pos != bbidMap.end());
						tmp.tgt_id = pos->second;

						tmp.type = Call;
						tmp.activation_count = get(actEProp, e);

						bb_connect.push_back(tmp);

						LOG_DEBUG(logger, "Generating BB connection for Call src cfg id: " <<  src << " src id: a" << tmp.src_id << " tgt cfg id: " << bb_f_first << " tgt id: a" << tmp.tgt_id << " connection type: " << tmp.type);
					}
					else if(src_type == ReturnPoint)
					{
						assert(tgt_type == BasicBlock);

						CFGVertex bb_f_last = getFunctEndBBlock(src);
						pos = bbidMap.find(bb_f_last);
						assert(pos != bbidMap.end());
						tmp.src_id = pos->second;

						pos = bbidMap.find(tgt);
						assert(pos != bbidMap.end());
						tmp.tgt_id = pos->second;

						tmp.type = Return;
						tmp.activation_count = get(actEProp, e);

						bb_connect.push_back(tmp);

						LOG_DEBUG(logger, "Generating BB connection for Return src cfg id: " <<  src << " src id: a" << tmp.src_id << " tgt cfg id: " << bb_f_last << " tgt id: a" << tmp.tgt_id << " connection type: " << tmp.type);
					}
					else
					{
						// cases currently not distinguished
					}
					break;
				}
			default:
				{
				}
		}
	}
}

void BBSISPOptimizerJP::generateKnapsackILPFormulation(void)
{
	stringstream ilp_knapsack_formulation_stream;

	stringstream objective_function;
	stringstream jp_var;
	stringstream size_constraints;
	stringstream block_sizes;
	stringstream set_binary_domains;

	assert(bb_data.size() > 0);

	// strip off the first basic block, for a more beautiful ILP
	objective_function << " max: " << endl << bb_data[0].benefit << " a" << bb_data[0].id << endl;
	size_constraints << "sp" << bb_data[0].id << endl;
	set_binary_domains << "bin a" << bb_data[0].id << ";" << endl;


	block_sizes << getBBSizeWithPenaltyConstraint(0);

	// proceed with all other basic blocks
	for(uint32_t i = 1; i < bb_data.size(); i++)
	{
		objective_function << " + " << bb_data[i].benefit << " a" << bb_data[i].id << endl;
		size_constraints << " + sp" << bb_data[i].id << endl;
		set_binary_domains << "bin a" << bb_data[i].id << ";" << endl;
		block_sizes << getBBSizeWithPenaltyConstraint(i);
	}


	// add jump penalties
	objective_function << " - jp;" << endl;
#ifndef ENABLE_IW_FLUSH_PENALTY
	jp_var << "jp = "<<  arch_cfg->getJumpPenalty(ContinuousAdressing, NoDisplacement) << " jpca - " << arch_cfg->getJumpPenalty(Jump, disp4) << " jpj4 - " << arch_cfg->getJumpPenalty(Jump, disp8)  << " jpj8 - " << arch_cfg->getJumpPenalty(Jump, disp15)  << " jpj15 - " << arch_cfg->getJumpPenalty(Jump, disp24)  << " jpj24 - " << arch_cfg->getJumpPenalty(Jump, indirect)  << " jpji - " << arch_cfg->getJumpPenalty(Call, disp8)  << " jpc8 - " << arch_cfg->getJumpPenalty(Call, disp24)  << " jpc24 - " << arch_cfg->getJumpPenalty(Call, indirect)  << " jpci;" << endl;
#else
	jp_var << "jp = "<<  arch_cfg->getJumpPenalty(ContinuousAdressing, NoDisplacement) << " jpca - " << " 6 jpca_iwflush - " << arch_cfg->getJumpPenalty(Jump, disp4) << " jpj4 - " << arch_cfg->getJumpPenalty(Jump, disp8)  << " jpj8 - " << arch_cfg->getJumpPenalty(Jump, disp15)  << " jpj15 - " << arch_cfg->getJumpPenalty(Jump, disp24)  << " jpj24 - " << arch_cfg->getJumpPenalty(Jump, indirect)  << " jpji - " << arch_cfg->getJumpPenalty(Call, disp8)  << " jpc8 - " << arch_cfg->getJumpPenalty(Call, disp24)  << " jpc24 - " << arch_cfg->getJumpPenalty(Call, indirect)  << " jpci;" << endl;
#endif

	size_constraints << " = sp;" << endl;
	size_constraints << "sp <= " << sisp_size << ";" << endl;
	
	ostringstream jpca, jpj4, jpj8, jpj15, jpj24, jpji, jpc8, jpc24, jpci;

	jpca << "jpca = 0";
	jpj4 << "jpj4 = 0";
	jpj8 << "jpj8 = 0";
	jpj15 << "jpj15 = 0";
	jpj24 << "jpj24 = 0";
	jpji << "jpji = 0";
	jpc8 << "jpc8 = 0";
	jpc24 << "jpc24 = 0";
	jpci << "jpci = 0";

#ifdef ENABLE_IW_FLUSH_PENALTY
	ostringstream jpca_iwflush;
	jpca_iwflush << "jpca_iwflush = 0";
#endif
	
	for(uint32_t i = 0; i < bb_connect.size(); i++)
	{
		if(bb_connect[i].type == ContinuousAdressing)
		{
			if(bb_connect[i].activation_count != 0) // ignore the constraint if it is not on the WCET path
			{
				jpca << " + " << bb_connect[i].activation_count <<  " xor" << bb_connect[i].src_id << "00" << bb_connect[i].tgt_id;
				bin_constraint_push_back(getXorConstraint(i));

#ifdef ENABLE_IW_FLUSH_PENALTY
				if(isNodeEnteredOnlyByContinuousAdressing(bb_connect[i].tgt_id))
				{
					jpca_iwflush << " + " << bb_connect[i].activation_count <<  " xor" << bb_connect[i].src_id << "00" << bb_connect[i].tgt_id;
				}
#endif
			}
		}
		else if(bb_connect[i].type == Jump)
		{
			if(bb_connect[i].activation_count != 0) // ignore the constraint if it is not on the WCET path
			{
				displacement_type_t disp = getDisplacementTypeOfLastBBInstruction(bb_connect[i].src_id);
				ostringstream *jp_print;

				switch(disp)
				{
					case disp4:
						{
							jp_print = &jpj4;
							break;
						}
					case disp11:
						// FIXME: Using the same penalty for ARMv6M instructions with 11 bit displacement as assumed for Carcore instructions with 8 bit displacement, since both instructions are 16 bit instructions.
					case disp8:
						{
							jp_print = &jpj8;
							break;
						}
					case disp15:
						{
							jp_print = &jpj15;
							break;
						}
					case disp24:
						{
							jp_print = &jpj24;
							break;
						}
					case indirect:
						{
							jp_print = &jpji;
							break;
						}
					default:
						assert(false); // No valid jump displacement!
				}

				*jp_print << " + " << bb_connect[i].activation_count << " xor" << bb_connect[i].src_id << "00" << bb_connect[i].tgt_id;
				bin_constraint_push_back(getXorConstraint(i));
			}
		}
		else if(bb_connect[i].type == Call)
		{
			if(bb_connect[i].activation_count != 0) // ignore the constraint if it is not on the WCET path
			{
				displacement_type_t disp = getDisplacementTypeOfLastBBInstruction(bb_connect[i].src_id);
				ostringstream *jp_print;

				switch(disp)
				{
					case disp11:
						// FIXME: Using the same penalty for ARMv6M instructions with 11 bit displacement as assumed for Carcore instructions with 8 bit displacement, since both instructions are 16 bit instructions.
					case disp8:
						{
							jp_print = &jpc8;
							break;
						}
					case disp24:
						{
							jp_print = &jpc24;
							break;
						}
					case indirect:
						{
							jp_print = &jpci;
							break;
						}
					default:
						assert(false); // No valid call displacement!
				}
				*jp_print << " + " << bb_connect[i].activation_count << " xor" << bb_connect[i].src_id << "00" << bb_connect[i].tgt_id;
				bin_constraint_push_back(getXorConstraint(i));
			}
		} 
		else if(bb_connect[i].type == Return)
		{
			// do nothing ... no penalty for returns
		}	

	}

	jpca << ";";
#ifdef ENABLE_IW_FLUSH_PENALTY
	jpca_iwflush << ";";
#endif
	jpj4 << ";";
	jpj8 << ";";
	jpj15 << ";";
	jpj24 << ";";
	jpji << ";";
	jpc8 << ";";
	jpc24 << ";";
	jpci << ";";
	
	ilp_knapsack_formulation.push_back("\n// Object function:\n");
	ilp_knapsack_formulation.push_back(objective_function.str());
	ilp_knapsack_formulation.push_back("\n// Jump penalties:\n");
	ilp_knapsack_formulation.push_back(jp_var.str());
	ilp_knapsack_formulation.push_back("\n// Size constraints:\n");
	ilp_knapsack_formulation.push_back(size_constraints.str());
	ilp_knapsack_formulation.push_back("\n// Block sizes including penalties:\n");
	ilp_knapsack_formulation.push_back(block_sizes.str());
	ilp_knapsack_formulation.push_back("\n// Jump penalties for additional jumps:\n");
	ilp_knapsack_formulation.push_back(jpca.str());
#ifdef ENABLE_IW_FLUSH_PENALTY
	ilp_knapsack_formulation.push_back("\n// Jump penalties for additional jumps and flushes of the IW:\n");
	ilp_knapsack_formulation.push_back(jpca_iwflush.str());
#endif
	ilp_knapsack_formulation.push_back("\n// Jump penalties for changed jumps:\n");
	ilp_knapsack_formulation.push_back(jpj4.str());
	ilp_knapsack_formulation.push_back("\n// Jump penalties for changed calls:\n");
	ilp_knapsack_formulation.push_back(jpj8.str());
	ilp_knapsack_formulation.push_back("\n");
	ilp_knapsack_formulation.push_back(jpj15.str());
	ilp_knapsack_formulation.push_back("\n");
	ilp_knapsack_formulation.push_back(jpj24.str());
	ilp_knapsack_formulation.push_back("\n");
	ilp_knapsack_formulation.push_back(jpji.str());
	ilp_knapsack_formulation.push_back("\n// Jump penalties for changed calls:\n");
	ilp_knapsack_formulation.push_back(jpc8.str());
	ilp_knapsack_formulation.push_back("\n");
	ilp_knapsack_formulation.push_back(jpc24.str());
	ilp_knapsack_formulation.push_back("\n");
	ilp_knapsack_formulation.push_back(jpci.str());
	ilp_knapsack_formulation.push_back("\n// XOR & ANDN formulations:\n");
	sort(binary_constraints.begin(), binary_constraints.end());
	stringstream bin_constraint_stream;
	for(vector<string>::iterator it = binary_constraints.begin(); it != binary_constraints.end(); it++)
	{
		bin_constraint_stream << *it;
	}
	ilp_knapsack_formulation.push_back(bin_constraint_stream.str());
	ilp_knapsack_formulation.push_back("\n// Binary domains:\n");
	ilp_knapsack_formulation.push_back(set_binary_domains.str());
	ilp_knapsack_formulation.push_back("\n");
}

void BBSISPOptimizerJP::generateBBRecords(void)
{
	cfgVertexIter vp;
	cfgOutEdgeIter epo;
	cfgInEdgeIter epi;
	CFGVertex v;
	CFGEdge e;

	// group bb_address, wcet activation count  (represented by the flow value of the in_edges) and cost (isp/SDRAM) (represented as the costs of the out_edges)


	// cycle all nodes and get properties from every bb
	for (vp = vertices(cfg); vp.first != vp.second; ++vp.first)
	{
		v = *vp.first;
//		LOG_DEBUG(logger, "Checking node: " <<  (get(startAddrStringNProp, v)).c_str() << " 0x" << hex << get(startAddrNProp, v) << " type: " << get(nodeTypeNProp, v));
		if(get(nodeTypeNProp, v) == BasicBlock)
		{
			bb_record_t tmp;

			// set assignment variable id (needed for ilp generation)
			tmp.id = v;

			// get address
			tmp.address = get(startAddrNProp, v);

			// get size
			tmp.size = get(bbSizeNProp, v);

			tmp.benefit = 0;


			uint32_t total_edge_activation=0;

#ifdef BB_COST_DO_NOT_DEPEND_ON_BB_EXIT
			uint32_t offchip_cost = 0;
			uint32_t onchip_cost = 0;
			uint32_t cur_cost = 0;
			for(epo = out_edges(v, cfg); epo.first != epo.second; ++epo.first) 
			{
				e = *epo.first;
				uint32_t cost = get(costEProp, e);
				uint32_t mem_penalty = get(memPenaltyEProp, e);
//				LOG_DEBUG(logger, "cost: " << cost << " mem_penalty: " << mem_penalty);
				if(cur_cost == 0)
				{
					cur_cost = cost + mem_penalty;
				}
				else
				{
					// all out edges from one basic block have the same cost (because the cost of the bb is assigned to it's out edges)
					assert(cur_cost == cost + mem_penalty);
				}

				cost = get(costOffChipEProp, e);
				if(offchip_cost == 0)
				{
					offchip_cost = cost;
				}
				else
				{
					// all out edges from one basic block have the same cost (because the cost of the bb is assigned to it's out edges)
					assert(offchip_cost == cost);
				}

				cost = get(costOnChipEProp, e);
				if(onchip_cost == 0)
				{
					onchip_cost = cost;
				}
				else
				{
					// all out edges from one basic block have the same cost (because the cost of the bb is assigned to it's out edges)
					assert(onchip_cost == cost);
				}
			}

			// get the activation count for the wcet critical path
			for(epi = in_edges(v, cfg); epi.first != epi.second; ++epi.first)
			{
				e = *epi.first;
				total_edge_activation += get(actEProp, e);
			}

			switch((analysis_metric_t) conf->getUint(CONF_USE_METRIC))
			{
				case WCET_RATIO_FILES:
				case WCET:
					{
						tmp.benefit = (cur_cost - onchip_cost) * total_edge_activation;
						break;
					}
				case MDIC:
					{
						tmp.benefit = cur_cost * total_edge_activation;
						break;
					}
				case MPL:
					{
						tmp.benefit = cur_cost;
					}
				default:
					{
						assert(false);
					}
			}
#else
			for(epo = out_edges(v, cfg); epo.first != epo.second; ++epo.first) 
			{
				e = *epo.first;
				uint32_t cost = get(costEProp, e);
				uint32_t mem_penalty = get(memPenaltyEProp, e);
				uint32_t edge_activation = get(actEProp, e);
				uint32_t offchip_cost = get(costOffChipEProp, e);
				uint32_t onchip_cost = get(costOnChipEProp, e);
				uint32_t cur_cost = cost + mem_penalty;


				LOG_DEBUG(logger, "node: " << "0x" << hex << tmp.address << dec << " cost: " << cost << " mem_penalty: " << mem_penalty << " activation: " << edge_activation);

				// only get the cost of the activated edges (the edge that is on the WCP)
				if(edge_activation != 0)
				{
					switch((analysis_metric_t) conf->getUint(CONF_USE_METRIC))
					{
						case WCET_RATIO_FILES:
						case WCET:
							{
								assert(cur_cost == offchip_cost);
								assert(mem_penalty == offchip_cost - onchip_cost);
								tmp.benefit += (mem_penalty) * edge_activation;
								break;
							}
						case MDIC:
							{
								tmp.benefit += cur_cost * edge_activation;
								break;
							}
						case MPL:
						default:
							{
								assert(false);
							}
					}
				}
			}

#endif


			LOG_DEBUG(logger, "Creating bb_record: a" << tmp.id << " addr: 0x" << hex << tmp.address << " Size: " << dec << tmp.size << " activation_count: " << total_edge_activation << " benefit: " << tmp.benefit);

			bb_data.push_back(tmp);

			registerNodeIdMapping(v, tmp.id);

			registerBlockLastInstrDisplacement(tmp.address, get(bbCodeNProp, v));
		}
	}

}

void BBSISPOptimizerJP::setAssignment(vector<lp_result_set> lp_result)
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
				bool found=false;
				for(uint32_t j = 0; j < bb_data.size(); j++)
				{
					if(cur_lp_var_id == bb_data[j].id)
					{
						assigned_bbaddrs.push_back(bb_data[j].address);
						used_sisp_size += bb_data[j].size;
						found = true;
						break;
					}
				}
				assert(found);
			}
		}
	}


	// TODO instrument the code ?!! update .elf-file, reparse it .... 
}

vector<uint32_t> BBSISPOptimizerJP::getBlockConnectionIds(uint32_t src_id)
{
	vector<bb_connect_t>::iterator it;
	vector<uint32_t> connected_to_src_id;

	for(uint32_t i = 0; i < bb_connect.size(); i++)
	{
		if(bb_connect[i].src_id == src_id)
		{
			connected_to_src_id.push_back(i);
		}
	}

	return connected_to_src_id;
}


displacement_type_t BBSISPOptimizerJP::getDisplacementTypeOfLastBBInstruction(uint32_t id)
{
	displacement_type_t return_val = UnknownDisplacementType;

	uint32_t bb_address = UNKNOWN_ADDR;

	vector<bb_record_t>::iterator it;

	for(it = bb_data.begin(); it < bb_data.end(); it++)
	{
		if(it->id == id)
		{
			bb_address = it->address;
		}
	}
	assert(bb_address != UNKNOWN_ADDR);

	DisplacementMap::iterator pos;
	pos = dispMap.find(bb_address);

	if(pos != dispMap.end())
	{
		return_val = pos->second;
	}
	else
	{
		return_val = UnknownDisplacementType;
	}

	return return_val;
}


void BBSISPOptimizerJP::registerBlockLastInstrDisplacement(uint32_t bb_addr, string bb_code)
{
	displacement_type_t displacement;
	vector<string> instrs;
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

	DisplacementMap::iterator ins_pos;
	bool ins_bool;
	tie(ins_pos, ins_bool) = dispMap.insert(make_pair(bb_addr,displacement));
	assert(ins_bool);
}


string BBSISPOptimizerJP::getBBSizeWithPenaltyConstraint(uint32_t id)
{
	// build size penalty terms in this format:
	// sp2 = 22 a2 + 4 andn2003; // the bb has a size of 22 but if it is located in the scratchpad but a3 not a connected edge there has to be added (with a size penalty of 4)
	// sp3 = 16 a3 + 4 andn3004 + 2 andn3005; // the bb has a size of 16 but if it is located in the scratchpad and a4 and a2 not a size penalty of 6 is to be considered.

	stringstream stream;
	stringstream stream2;
	uint32_t bb_size=  bb_data[id].size;
	stream << "sp" << bb_data[id].id << " = ";
	vector<uint32_t> connections_of_i = getBlockConnectionIds(bb_data[id].id);
	for(uint32_t j = 0; j < connections_of_i.size(); j++)
	{
		assert(bb_data[id].id == bb_connect[connections_of_i[j]].src_id);
		displacement_type_t disp = UnknownDisplacementType;
		if(bb_connect[connections_of_i[j]].type == ContinuousAdressing)
		{
			disp = NoDisplacement;
		}
		else
		{
			disp = getDisplacementTypeOfLastBBInstruction(bb_data[id].id);
		}
		// FIXME Round the costs up to multiples of the fetch word size
		uint32_t penalty =  arch_cfg->getSizePenalty(bb_connect[connections_of_i[j]].type, disp);
		if( penalty != 0)
		{
			stream2 << " + " << penalty << " andn" << bb_connect[connections_of_i[j]].src_id << "00" << bb_connect[connections_of_i[j]].tgt_id;
			bin_constraint_push_back(getAndNConstraint(connections_of_i[j]));
		}
	}
	stream << bb_size << " a" << bb_data[id].id << stream2.str() << ";" << endl;

	return stream.str();
}

string BBSISPOptimizerJP::getXorConstraint(uint32_t id)
{
	stringstream stream;

	// formulate the XOR assignment of src_id and tgt_id
	stream << "xor" << bb_connect[id].src_id << "00" << bb_connect[id].tgt_id << " <= a" << bb_connect[id].src_id << " + a" << bb_connect[id].tgt_id << ";" << endl;
	stream << "xor" << bb_connect[id].src_id << "00" << bb_connect[id].tgt_id << " >= a" << bb_connect[id].src_id << " - a" << bb_connect[id].tgt_id << ";" << endl;
	stream << "xor" << bb_connect[id].src_id << "00" << bb_connect[id].tgt_id << " >= - a" << bb_connect[id].src_id << " + a" << bb_connect[id].tgt_id << ";" << endl;
	stream << "xor" << bb_connect[id].src_id << "00" << bb_connect[id].tgt_id << " <= 2 - a" << bb_connect[id].src_id << " - a" << bb_connect[id].tgt_id << ";" << endl;
	// TODO: Need to set variable as binary. Otherwise if this will cause significant extra lp_solve run-time, cross check results if they are really binary.
//	stream << "bin xor" << bb_connect[id].src_id << "00" << bb_connect[id].tgt_id << ";" << endl;
	stream << endl;


	return stream.str();
}


string BBSISPOptimizerJP::getAndNConstraint(uint32_t id)
{
	stringstream stream;

	// formulate the ANDN assignment of src_id and tgt_id
	stream << "andn" << bb_connect[id].src_id << "00" << bb_connect[id].tgt_id << " <= a" << bb_connect[id].src_id  << ";" << endl;
	stream << "andn" << bb_connect[id].src_id << "00" << bb_connect[id].tgt_id << " <= 1 - a" << bb_connect[id].tgt_id << ";" << endl;
	stream << "andn" << bb_connect[id].src_id << "00" << bb_connect[id].tgt_id << " + 1 >= a" << bb_connect[id].src_id << " + 1 - a" << bb_connect[id].tgt_id << ";" << endl;
	// TODO: Need to set variable as binary. Otherwise if this will cause significant extra lp_solve run-time, cross check results if they are really binary.
//	stream << "bin andn" << bb_connect[id].src_id << "00" << bb_connect[id].tgt_id << ";" << endl;
	stream << endl;


	return stream.str();
}

void BBSISPOptimizerJP::bin_constraint_push_back(string bin_constraint)
{
	if(bin_constraint != "")
	{
		binary_constraints.push_back(bin_constraint);
	}
}

void BBSISPOptimizerJP::setVariables(vector<lp_result_set> lp_result)
{
	for(uint32_t i = 0; i < lp_result.size(); i++)
	{
		if(lp_result[i].variable.compare("sp") == 0)
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

uint32_t BBSISPOptimizerJP::getEstimatedJumpPenalty(void)
{
	return ilp_solution_jump_penalty;
}

#ifdef ENABLE_IW_FLUSH_PENALTY
bool BBSISPOptimizerJP::isNodeEnteredOnlyByContinuousAdressing(uint32_t node)
{
	for(uint32_t i = 0; i < bb_connect.size(); i++)
	{
		if(bb_connect[i].tgt_id == node)
		{
			if(bb_connect[i].type != ContinuousAdressing)
			{
				return false;
			}
		}
	}
	return true;
}
#endif


sisp_result_t BBSISPOptimizerJP::getResults(void)
{
	sisp_result_t tmp = SISPOptimizer_IF::getResults();
	tmp.estimated_timing = numeric_limits<uint64_t>::max();
	tmp.estimated_jump_penalty = getEstimatedJumpPenalty();

	return tmp;
}

