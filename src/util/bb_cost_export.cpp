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
#include "bb_cost_export.hpp"

LoggerPtr BBCostExport::logger(Logger::getLogger("BBCostExport"));

BBCostExport::BBCostExport(ControlFlowGraph cfgraph)
{
	conf = Configuration::getInstance();

	cfg = cfgraph;

	// get node property structures
	nodeTypeNProp = get(nodetype_t(), cfg);
	startAddrNProp = get(startaddr_t(), cfg);

	// get edge property structures
	costEProp = get(cost_t(), cfg);
	memPenaltyEProp = get(mem_penalty_t(), cfg);
	actEProp = get(activation_t(), cfg);

}

BBCostExport::~BBCostExport()
{
}


void BBCostExport::exportCostToFile(string filename_extension)
{
	if(!conf->getBool(CONF_EXPORT_BB_COST))
	{
		return;
	}

	stringstream filename;
	filename << conf->getString(CONF_EXPORT_BB_COST_FILEPREFIX) << "_" << filename_extension << ".cst";

	ofstream exp_cost_file;
	exp_cost_file.open(filename.str().c_str());

	assert(exp_cost_file.is_open());

	exp_cost_file << "BB id:\tBB addr:\tEdge id:\tCost:\tActivation count:\n";

	cfgVertexIter vp;
	for (vp = vertices(cfg); vp.first != vp.second; ++vp.first)
	{
		CFGVertex v = *vp.first;

		if(get(nodeTypeNProp, v) == BasicBlock)
		{
			uint32_t bb_id = v;
			uint32_t bb_addr = get(startAddrNProp, v);

			cfgOutEdgeIter epo;
			for(epo = out_edges(v, cfg); epo.first != epo.second; ++epo.first) 
			{
				CFGEdge e = *epo.first;
				uint32_t cost =  get(costEProp, e);
				uint32_t mem_penalty = get(memPenaltyEProp, e);
				uint32_t activation_count = get(actEProp, e);
				uint32_t edge_cost=0;

				switch((analysis_metric_t) conf->getUint(CONF_USE_METRIC))
				{
					case WCET_RATIO_FILES:
					case WCET:
						{
							// for the execution time the memory penalty (of cache or any other memory) has to be taken into account
							edge_cost = cost + mem_penalty;
							break;
						}
					case MDIC:
					case MPL:
						{
							// for WCIC and WCPL only the number of instructions resp. the size of the block is of interest
							// in ControlFlowGraphObject::addBBCostToCfg() the instruction count resp. the block size is put into the cost edge property
							edge_cost = cost;
							break;
						}
					default:
						{
							LOG_ERROR(logger, "Wrong analysis metric.");
							assert(false);
						}
				}


				exp_cost_file << bb_id << "\t0x" << hex << bb_addr << dec << "\t" << e << "\t" << edge_cost << "\t" << activation_count << "\n";
					

			}
			
		}
	}

	exp_cost_file.close();

}
