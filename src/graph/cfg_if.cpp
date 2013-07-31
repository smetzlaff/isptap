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
#include "cfg_if.hpp"

#include "carcore_timing.hpp"
#include "armv6m_core_timing.hpp"

LoggerPtr ControlFlowGraphObject_IF::logger(Logger::getLogger("ControlFlowGraphObject"));

ControlFlowGraphObject_IF::ControlFlowGraphObject_IF()
{
	conf = Configuration::getInstance();

	bb_cost_container_onchip = NULL;
	bb_cost_container_offchip = NULL;

	switch((architecture_t)conf->getUint(CONF_ARCHITECTURE))
	{
		case CARCORE:
			core_timing_onchip = (CoreTiming *) new CarCoreTiming(ONCHIP);
			core_timing_offchip = (CoreTiming *) new CarCoreTiming(OFFCHIP);
			break;
		case ARMV6M:
			core_timing_onchip = (CoreTiming *) new Armv6mCoreTiming(ONCHIP);
			core_timing_offchip = (CoreTiming *) new Armv6mCoreTiming(OFFCHIP);
			break;
		default:
			assert(false);
	}

	memory_type = (mem_type_t)conf->getUint(CONF_MEMORY_TYPE);

}

ControlFlowGraphObject_IF::~ControlFlowGraphObject_IF()
{
	avMap.clear();

	delete(core_timing_onchip);
	delete(core_timing_offchip);
}

ControlFlowGraph ControlFlowGraphObject_IF::getCFG(void)
{
	return cfg;
}

CFGVertex ControlFlowGraphObject_IF::getCFGEntry(void)
{
	return entry;
}

CFGVertex ControlFlowGraphObject_IF::getCFGExit(void)
{
	return exit;
}


uint32_t ControlFlowGraphObject_IF::getBBSizeFromAddr(uint32_t address)
{
	AddrVertexMap::iterator pos;
	uint32_t bb_size=0;

	pos = avMap.find(address);

	if(pos != avMap.end())
	{
		bb_size = get(bbSizeNProp, pos->second);
	}
	else
	{
		LOG_WARN(logger, "BB for address 0x" << address << " not found.")
	}
	return bb_size;
}


uint32_t ControlFlowGraphObject_IF::getInstrCountFromAddr(uint32_t address)
{
	AddrVertexMap::iterator pos;
	uint32_t bb_instr_count=0;

	pos = avMap.find(address);

	if(pos != avMap.end())
	{
		bb_instr_count = get(bbInstrsNProp, pos->second);
	}
	else
	{
		LOG_WARN(logger, "BB for address 0x" << address << " not found.")
	}
//	LOG_DEBUG(logger, "number of instructions fopr BB 0x" << hex << address << " is: " << dec << bb_instr_count);

	return bb_instr_count;
}

void ControlFlowGraphObject_IF::setRatioFileReaders(RatioFileReader *rfr_onchip, RatioFileReader *rfr_offchip)
{
	bb_cost_container_onchip = rfr_onchip;
	bb_cost_container_offchip = rfr_offchip;
}


void ControlFlowGraphObject_IF::addBBCostToCfg(void)
{

//	assuming an edge u -> v the cost for executing u is assigned to that edge!

	vector<CFGVertex> processing;
	vector<CFGVertex> processed;
	processing.push_back(getCFGEntry());

	while(processing.size() != 0)
	{
		CFGVertex v = processing.back();
		// delete/clear processed node
		processing.pop_back();

		// ensure that each node is only handled once.
		bool node_already_processed = false;
		for(vector<CFGVertex>::iterator it = processed.begin(); it != processed.end(); it++)
		{
			if(v == *it)
			{
				node_already_processed = true;
			}
		}

		if(!node_already_processed)
		{
			if(get(nodeTypeNProp, v) == BasicBlock)
			{
				calculateCostForBB(v);
			}

			for(cfgOutEdgeIter ep = out_edges(v, cfg); ep.first != ep.second; ++ep.first) 
			{
				CFGEdge e = *ep.first;
				processing.push_back(target(e,cfg));
			}

			processed.push_back(v);
		}
	}
}


void ControlFlowGraphObject_IF::calculateCostForBB(CFGVertex v)
{
	cycles_t cost_onchip, cost_offchip;
	cost_onchip.jump=0;
	cost_onchip.forward_step=0;
	cost_offchip.jump=0;
	cost_offchip.forward_step=0;

	switch((analysis_metric_t) conf->getUint(CONF_USE_METRIC))
	{
		case WCET_RATIO_FILES:
			{
				// get cost from ratio file (create otawa ratio file reader)
				cost_offchip.forward_step = bb_cost_container_offchip->getBBCost(get(startAddrNProp, v));
				// here it is not distinguished how the block is leaved
				cost_offchip.jump = cost_offchip.forward_step;
				if(cost_offchip.forward_step == 0)
				{
					LOG_INFO(logger, "Cannot obtain offchip cost for block address 0x" << hex << get(startAddrNProp, v));
				}

				cost_onchip.forward_step = bb_cost_container_onchip->getBBCost(get(startAddrNProp, v));
				// here it is not distinguished how the block is leaved
				cost_onchip.jump = cost_onchip.forward_step;
				if(cost_onchip.forward_step == 0)
				{
					LOG_INFO(logger, "Cannot obtain onchip cost for block address 0x" << hex << get(startAddrNProp, v));
				}

				break;

			}
		case WCET:
			{
				if(conf->getBool(CONF_USE_ARCH_CFG_FILE))
				{
					cfgInEdgeIter epi;
					bool bb_entered_by_forward_step_only = true;
					for(epi = in_edges(v, cfg); epi.first != epi.second; ++epi.first) 
					{
						CFGEdge e = *epi.first;
						if(get(edgeTypeEProp, e) != ForwardStep)
						{
							bb_entered_by_forward_step_only = false;
						}
					}

					if(in_degree(v, cfg) == 0)
					{
						bb_entered_by_forward_step_only = false;
						LOG_WARN(logger, "The basic block: " << get(startAddrStringNProp, v) << " id: " << v << " does not have an input edge! (CFG entry node is: 0x" << hex << get(callLabelNProp, getCFGEntry()) << ")");
					}

					LOG_DEBUG(logger, "Calculating cycle cost for bb: " << get(startAddrStringNProp, v) << " with id: " << v);

					// get cost for basic block, if the code is in the onchip mem:
#ifdef BB_COST_DO_NOT_DEPEND_ON_BB_EXIT
					cost_onchip = core_timing_onchip->getCycleCountForInstructions(get(bbCodeNProp, v), bb_entered_by_forward_step_only, get(startAddrNProp, v), get(endAddrNProp,v));
#else
					cost_onchip = core_timing_onchip->getCycleCountForInstructionsWithBlockExitSensitivity(get(bbCodeNProp, v), bb_entered_by_forward_step_only, get(startAddrNProp, v), get(endAddrNProp,v));
#endif

					// get cost for basic block, if the code is in the offchip mem:
#ifdef BB_COST_DO_NOT_DEPEND_ON_BB_EXIT
					cost_offchip = core_timing_offchip->getCycleCountForInstructions(get(bbCodeNProp, v), bb_entered_by_forward_step_only, get(startAddrNProp, v), get(endAddrNProp,v));
#else
					cost_offchip = core_timing_offchip->getCycleCountForInstructionsWithBlockExitSensitivity(get(bbCodeNProp, v), bb_entered_by_forward_step_only, get(startAddrNProp, v), get(endAddrNProp,v));
#endif
				}
				else
				{
					// cannot use timing model, it is not activated by user.
					assert(false);
				}
				break;
			}
		case MDIC:
			{
				cost_offchip.forward_step = getInstrCountFromAddr(get(startAddrNProp, v));
				// here it is not distinguished how the block is leaved
				cost_offchip.jump = cost_offchip.forward_step;
				cost_onchip = cost_offchip;
				if(cost_offchip.forward_step == 0)
				{
					LOG_INFO(logger, "Cannot obtain instruction count for block address 0x" << hex << get(startAddrNProp, v));
				}
				break;
			}
		case MPL:
			{
				cost_offchip.forward_step = getBBSizeFromAddr(get(startAddrNProp, v));
				// here it is not distinguished how the block is leaved
				cost_offchip.jump = cost_offchip.forward_step;
				cost_onchip = cost_offchip;
				break;
			}
		default:
			{
				LOG_ERROR(logger, "Unknown analysis metric.");
				assert(false);
			}
	}


	cfgOutEdgeIter epo;
	for(epo = out_edges(v, cfg); epo.first != epo.second; ++epo.first) 
	{
		CFGEdge e = *epo.first;

		uint32_t ecost_offchip, ecost_onchip;

		edge_type_t e_type = get(edgeTypeEProp, e);
		if((e_type == ForwardStep) || (e_type == Meta))
		{
			ecost_onchip = cost_onchip.forward_step;
			ecost_offchip = cost_offchip.forward_step;
		}
		else if((e_type == ForwardJump) || (e_type == BackwardJump))
		{
			ecost_onchip = cost_onchip.jump;
			ecost_offchip = cost_offchip.jump;
		}
		else
		{
			assert(false);
		}

		// compare values to the ones in the ratio file:
		LOG_DEBUG(logger, "Cost for bb: 0x" << hex << get(startAddrNProp, v) << " - 0x" << get(endAddrNProp, v) << dec << " Onchip: " << ecost_onchip << "\tOffchip: " <<  ecost_offchip << "\tMemPenalty: " << ecost_offchip-ecost_onchip << "\tEdge type: " << e_type);

		// set the costs:
		put(costOnChipEProp, e, ecost_onchip);
		put(costOffChipEProp, e, ecost_offchip);
		if((memory_type == NO_MEM) || (memory_type == VIVU_TEST))
		{
			// the cost of the block is the cost for off-chip memory without any further memory penalty
			put(costEProp, e, ecost_offchip);
			put(memPenaltyEProp, e, 0);
		}
		else if(IS_STATIC_MEM(memory_type))
		{
			// the cost of the block is the cost for on-chip memory and the difference between off- and on-chip memory as memory penalty
			put(costEProp, e, ecost_onchip);
			put(memPenaltyEProp, e, ecost_offchip-ecost_onchip);
		}
		else
		{
			// the cost of the block is the cost for on-chip memory. The memory penalty is set to 0 because it is calculated via DFA later.
			put(costEProp, e, ecost_onchip);
			put(memPenaltyEProp, e, 0);
		}
	}

}
