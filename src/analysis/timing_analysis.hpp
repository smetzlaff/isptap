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
#ifndef _TIMING_ANALYSIS_HPP_
#define _TIMING_ANALYSIS_HPP_

#include <utility>                   // for std::pair
#include <algorithm>                 // for std::for_each
#include <iostream>                  // for std::cout
#include <time.h>
#include <boost/regex.hpp>
#include <boost/program_options.hpp>

#include "global.h"
#include "constants.h"


#include "graph/graph_structure.h"
#include "graph/flowfact_enrichment.hpp"
#include "graph/ilpgenerator.hpp"
#include "graph/vivug_creator.hpp"
#include "graph/msgtocfg_conv.hpp"

#include "parser/carcore_dump_parser.hpp"
#include "parser/armv6m_dump_parser.hpp"

#include "memory_params.hpp"
#include "memory_size_stepper.hpp"

#include "memory/bbsisp_optimizer.hpp"
#include "memory/bbsisp_jp_optimizer.hpp"
#include "memory/bbsisp_optimizer_wcp.hpp"
#include "memory/bbsisp_jp_optimizer_wcp.hpp"
#include "memory/fsisp_optimizer_old.hpp"
#include "memory/fsisp_optimizer.hpp"
#include "memory/fsisp_optimizer_wcp.hpp"

#include "memory/icache_dfa.hpp"
#include "memory/icache_dfa_map.hpp"

#include "memory/disp_dfa.hpp"
#include "memory/disp_dfa_map.hpp"

#include "instr/disp_instrumentator.hpp"

#include "util/graph_exporter.hpp"
#include "util/result_checker.hpp"

#include "util/graph_exporter.hpp"
#include "util/cost_exporter.hpp"
#include "util/function_table_creator.hpp"
#include "util/configuration.hpp"
#include "util/report_generator.hpp"
#include "util/rawfile_writer.hpp"
#include "util/wcpath_export.hpp"
#include "util/cost_exporter.hpp"

// for getHelpMessage()
#include "arch/arch_cfg.hpp"
#include "arch/carcore_cfg.hpp"
#include "arch/armv6m_cfg.hpp"
#include "util/jump_target_extractor.hpp"
#include "util/call_target_extractor.hpp"

class TimingAnalysis {
	public:
		TimingAnalysis(Configuration *configuration, uint32_t analysis_id);
		virtual ~TimingAnalysis();
		bool start(void);
		uint64_t getEstimate(void);
		lp_solution_t getSolutionType(void);

	private:
		bool analyseProgram(void);
		bool analysePipeline(void);
		bool analyseImem(void);
		bool analyseImem(uint32_t mem_size);
		bool analyseNomem(void);
		bool analyseVivuTest(void);
		bool analyseBBSISP(uint32_t sisp_size);
		bool analyseFSISP(uint32_t sisp_size);
		bool analyseFSISP_OLD(uint32_t sisp_size);
		bool analyseICache(uint32_t cache_size);
		bool analyseDISP(uint32_t disp_size);

//		bool analyseImem2(void);

		bool calculateEstimate(void);
		bool calculateEstimate(uint32_t mem_size);
		bool calculateBaselineEstimate(ControlFlowGraph ilp_in_graph, CFGVertex ilp_in_entry, CFGVertex ilp_in_exit);

		Configuration *conf;

		uint32_t id;
		string id_appendix;
		string id_log_prefix;

		string entry_function;

		uint64_t estimate;
		lp_solution_t estimate_solution_type;

		/// Indicates, if the program analysis is finished.
		bool pa_finished;
		/// Indicates, if the pipeline (core) analysis is finished.
		bool ca_finished;
		/// Indicates, if the instruction memory analysis is finished.
		bool ma_finished;
		/// Indicates, if the estimation calculation is finished.
		bool ec_finished;

		bool baseline_calculated;
		uint64_t baseline_timing_estimate;
		uint64_t baseline_mem_estimate;
		lp_solution_t baseline_solution_type;

		ReportGenerator *report;

		GraphExporter *gexporter;
		CostExporter *cexporter;
		ResultChecker *rchecker;
		MemoryParameters *mp;
		DumpParser *dp;

		vector<addr_label_t> pa_detected_labels;
		ControlFlowGraph pa_out_scfg;
		vector<function_graph_t> pa_out_function_cfgs;
		FunctionCallGraphObject pa_out_fcgo;
		CFGVertex pa_out_scfg_entry, pa_out_scfg_exit;

		ControlFlowGraph ca_out_scfg;
		CFGVertex ca_out_scfg_entry, ca_out_scfg_exit;

		ControlFlowGraph bl_out_scfg;
		CFGVertex bl_out_scfg_entry, bl_out_scfg_exit;

		ControlFlowGraph ma_out_scfg;
		CFGVertex ma_out_scfg_entry, ma_out_scfg_exit;
		sisp_result_t ma_out_sisp_result;

		/**
		 * \brief Pointer to the LOGCXX logger object.
		 */
		static LoggerPtr logger;
};

#endif
