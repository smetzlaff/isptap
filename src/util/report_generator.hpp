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
#ifndef _REPORT_GENERATOR_HPP_
#define _REPORT_GENERATOR_HPP_

#include "global.h"
#include "constants.h"
#include "configuration.hpp"
#include "graph_structure.h"
#include "wcpath_export.hpp"
#include "lpsolver.hpp"

class ReportGenerator {
	public:
		ReportGenerator(string file_name, bool file_append, mem_type_t memory, replacement_policy_t r_policy, analysis_metric_t use_metric, bool use_istats);
		virtual ~ReportGenerator();
		void setCodeSize(uint32_t codeSize);
		void setBlockAssignment(vector<uint32_t> assigned_blocks);
		void setBlockAssignment(vector<uint32_t> assigned_blocks, lp_solution_t solution_type);
		void setFunctionAssignment(vector<addr_label_t> assigned_functions);
		void setWCCostValue(uint64_t wc_cost_value);
		void setWCCostValue(uint64_t wc_cost_value, lp_solution_t solution_type);
		void setWCCostValue(uint64_t wc_cost_value, uint64_t wc_cost_value_improved);
		void setWCCostValue(uint64_t wc_cost_value, uint64_t wc_cost_value_improved, lp_solution_t solution_type, lp_solution_t solution_type_improved);
		void setMemCostValue(uint64_t mem_cost_value);
		void setMemCostValue(uint64_t mem_cost_value, lp_solution_t solution_type);
		void setMemSize(uint32_t size, uint32_t used);
		void setMemSize(uint32_t size);
		void setDFAStatistics(uint64_t state_count, uint64_t state_representation_count, uint64_t used_mem, uint64_t maintained_references);
		void setCacheHMStats(cache_hm_stat_t cache_stat);
		void generate_header(void);
		void generate_line(void);
		void generate(void);
		void setMappingFileName(string mappingFileName);
		void generateStaticFunctionMapping(void);
		void setWCPathInstructionStatistics(instr_stat_t istats);

	private:
		inline string printSolutionType(lp_solution_t solution_type);
		string report_file;
		string mapping_file_stub;
		bool report_append;
		mem_type_t memory_type;
		vector<uint32_t> blocks_assigned;
		lp_solution_t blocks_assigned_solution_type;
		vector<addr_label_t> functions_assigned;
		lp_solution_t functions_assigned_solution_type;
		uint32_t code_size;
		uint64_t wc_cost;
		lp_solution_t wc_cost_solution_type;
		uint64_t wc_cost_improved;
		lp_solution_t wc_cost_improved_solution_type;
		uint64_t mem_cost;
		lp_solution_t mem_cost_solution_type;
		uint32_t mem_size;
		uint32_t mem_size_used;
		uint64_t dfa_state_count;
		uint64_t dfa_state_representation_count;
		uint64_t dfa_mem_used;
		uint64_t dfa_maintained_references;
		cache_hm_stat_t cache_hm_stats;
		instr_stat_t i_stats;
		replacement_policy_t replacement_policy;
		analysis_metric_t metric;
		bool generate_instruction_statistics;

//		static LoggerPtr logger;
};

#endif
