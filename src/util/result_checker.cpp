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
#include "result_checker.hpp"


LoggerPtr ResultChecker::logger(Logger::getLogger("ResultChecker"));

ResultChecker *ResultChecker::singleton = NULL;

ResultChecker* ResultChecker::getInstance(void)
{
	if(singleton == NULL)
	{
		singleton = new ResultChecker;
	}
	return singleton;
}

ResultChecker::ResultChecker()
{
	conf = Configuration::getInstance();
}

ResultChecker::~ResultChecker()
{
}

void ResultChecker::check_result(uint32_t calculated_value, string property_to_compare)
{
	uint32_t value_to_compare = conf->getUint(property_to_compare);

	if(value_to_compare != 0)
	{
		if(value_to_compare != calculated_value)
		{
			LOG_WARN(logger, "Test failed: Calculated " << property_to_compare << " does not equal the value given in the config file. Result is " << calculated_value << ", but should be " << value_to_compare);
		}
		else
		{
			stringstream s;
			if(property_to_compare.compare(CONF_OUTPUT_WCET) == 0)
			{
				uint32_t real_execution_time = Configuration::getInstance()->getUint(CONF_OUTPUT_ET_SIM);
				if(real_execution_time != 0)
				{
					double overestimation = (((double)calculated_value) / ((double)real_execution_time) - 1) * 100;
					s << " Overestimation is " << overestimation << "%";
				}
			}
			LOG_INFO(logger, "Test passed: Calculated " << property_to_compare << " equals the value given in the config file." << s.str());
		}
	}

}

void ResultChecker::check_result(uint32_t calculated_value, string property_to_compare, uint32_t mem_size)
{
	uint32_t value_to_compare = conf->getUint(property_to_compare, mem_size);

	if(value_to_compare != 0)
	{
		if(value_to_compare != calculated_value)
		{
			LOG_WARN(logger, "Test failed for mem size " << mem_size << ": Calculated " << property_to_compare << " does not equal the value given in the config file. Result is " << calculated_value << ", but should be " << value_to_compare);
		}
		else
		{
			LOG_INFO(logger, "Test passed for mem size " << mem_size << ": Calculated " << property_to_compare << " equals the value given in the config file.");
		}
	}

}


void ResultChecker::check_sisp_assignment(mem_type_t mtype, string mtype_s, SISPOptimizer_IF *sisp, ILPGenerator* ilpg, bool disable_asserts)
{
	sisp_result_t tmp;
	tmp.used_size = sisp->getUsedSispSize();
	tmp.estimated_used_size = sisp->getEstimatedUsedSize();
	tmp.assigned_bbs = sisp->getBlockAssignment();
	tmp.solution_type = sisp->getSolutionType();

	if((mtype == BBSISP_WCP) || (mtype == BBSISP_JP_WCP) || (mtype == FSISP_WCP))
	{
		tmp.estimated_timing = (dynamic_cast<BBSISPOptimizerWCP *>(sisp))->getEstimatedWCET();
	}
	else
	{
		tmp.estimated_timing = numeric_limits<uint64_t>::max();
	}

	if(mtype == BBSISP_JP)
	{
		tmp.estimated_jump_penalty = (dynamic_cast<BBSISPOptimizerJP *>(sisp))->getEstimatedJumpPenalty();
	}
	else if(mtype == BBSISP_JP_WCP)
	{
		tmp.estimated_jump_penalty = (dynamic_cast<BBSISPOptimizerJPWCP *>(sisp))->getEstimatedJumpPenalty();
	}
	else
	{
		tmp.estimated_jump_penalty = numeric_limits<uint32_t>::max();
	}

	return check_sisp_assignment(mtype, mtype_s, &tmp, ilpg, disable_asserts);

}

void ResultChecker::check_sisp_assignment(mem_type_t mtype, string mtype_s, sisp_result_t* sisp_result, ILPGenerator* ilpg, bool disable_asserts)
{
	uint32_t sisp_used_size_cfg = ilpg->getSizeOfBlocks(sisp_result->assigned_bbs);

	if(sisp_result->estimated_used_size != sisp_result->used_size)
	{
		LOG_ERROR(logger, "The used bytes of the " << mtype_s << " calculated by the assignment ILP does not correspond to the updated (jump enhanced) basic blocks: " << sisp_result->estimated_used_size << " != " << sisp_result->used_size);
		if(!disable_asserts)
			assert(false);
	}
	else if(sisp_result->used_size != sisp_used_size_cfg)
	{
		LOG_ERROR(logger, "The used bytes of the " << mtype_s << " calculated in the memory optimizer does not correspond the updated CFG: " << sisp_result->used_size << " != " << sisp_used_size_cfg);
		if(!disable_asserts)
			assert(false);

	}
	else
	{
		LOG_DEBUG(logger, "Used size estimated by " << mtype_s << " assignment ILP fits the calculated size of : " << sisp_result->used_size);
	}

	if((mtype == BBSISP_WCP) || (mtype == BBSISP_JP_WCP) || (mtype == FSISP_WCP))
	{
		LOG_INFO(logger, mtype_s << " estimated WCET: " << sisp_result->estimated_timing);
		if(sisp_result->estimated_timing != ilpg->getWCCostValue())
		{
			LOG_WARN(logger, "WCET estimate calculated by ILPG differs from the estimation from " << mtype_s << " by: " << ((((float)ilpg->getWCCostValue()-(float)sisp_result->estimated_timing)/(float)ilpg->getWCCostValue())*100.0) << "%. Thus the selection might be non optimal.");
		}
	}

	if((mtype == BBSISP_JP) || (mtype == BBSISP_JP_WCP))
	{
		LOG_DEBUG(logger, mtype_s << " estimated jump penalty of: " << sisp_result->estimated_jump_penalty);
	}
}
