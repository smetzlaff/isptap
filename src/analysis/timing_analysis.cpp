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
#include "timing_analysis.hpp"

LoggerPtr TimingAnalysis::logger(Logger::getLogger("TimingAnalysis"));

TimingAnalysis::TimingAnalysis(Configuration *configuration, uint32_t analysis_id) : conf(configuration), id(analysis_id)
{
	gexporter = GraphExporter::getInstance();
	cexporter = CostExporter::getInstance();
	rchecker = ResultChecker::getInstance();
	mp = new MemoryParameters();

	string report_file_name = conf->getString(CONF_REPORT_FILE);
	report_file_name.append(id_appendix);
	report = new ReportGenerator(report_file_name,conf->getBool(CONF_REPORT_APPEND), mp->getMemType(), mp->getReplacementPolicy(), (analysis_metric_t)conf->getUint(CONF_USE_METRIC), conf->getBool(CONF_EXPORT_WC_PATH_INSTR_STATS));

	stringstream id_appendix_s;
	id_appendix_s << "_c" << id;
	id_appendix = id_appendix_s.str();

	stringstream id_log_prefix_s;
	id_log_prefix_s << "[c" << id << "]";
	id_log_prefix = id_log_prefix_s.str();

	pa_finished=false;
	ca_finished=false;
	ma_finished=false;
	ec_finished=false;

	baseline_calculated = false;
}

TimingAnalysis::~TimingAnalysis()
{
	pa_detected_labels.clear();

	ma_out_sisp_result.assigned_bbs.clear();

	delete(mp);
	delete(dp);
	delete(report);
}

bool TimingAnalysis::start(void)
{
	// FIXME: conf->getString(CONF_ENTRY_FUNCTION) is not safe when multiple functions should be analysed
	entry_function = conf->getString(CONF_ENTRY_FUNCTION);
	LOG_INFO(logger, id_log_prefix << " Entering application with start label: " << entry_function);

	analyseProgram();

	analysePipeline();

	if(conf->getBool(CONF_MEMORY_SIZE_STEPPING))
	{
		MemorySizeStepper mss;
		report->generate_header();
		while(mss.isValidSize())
		{
			analyseImem(mss.getMemorySize());
			calculateEstimate(mss.getMemorySize());
			report->generate_line();
			mss.increaseSize();
		}
	}
	else
	{
		analyseImem();

		calculateEstimate();

		report->generate();
	}


	if(conf->getBool(CONF_EXPORT_FUNCTION_TABLE))
	{
		if(!conf->getBool(CONF_DISP_INSTRUMENTATION))
		{
			FunctionTableCreator ftc(conf->getString(CONF_FUNCTION_TABLE_FILE), conf->getBool(CONF_OLD_FUNCTION_TABLE_FORMAT));
			ftc.createFunctionTable(dp->getFCGObj());
		}

	}

	return ec_finished;
}

bool TimingAnalysis::analyseProgram(void)
{
	switch ((architecture_t)conf->getUint(CONF_ARCHITECTURE))
	{
		case CARCORE:
			dp = new CarCoreDumpParser(id);
			break;
		case ARMV6M:
			dp = new Armv6mDumpParser(id);
			break;
		default:
			LOG_ERROR(logger, "Unknown architecture: " << conf->getString(CONF_ARCHITECTURE));
			assert(false);
	}

	dp->parse();

	pa_detected_labels = dp->getDetectedLabels();
	if(conf->getBool(CONF_EXPORT_FUNCTION_CFGS))
	{
		LOG_DEBUG(logger, id_log_prefix << "CFG creation for " << pa_detected_labels.size() << " functions.");
		for(uint i = 0; i < pa_detected_labels.size(); i++)
		{
			string function_name = pa_detected_labels[i].label;
			function_name.append(id_appendix);
			LOG_DEBUG(logger, id_log_prefix << "Creating graph output for " << function_name << " at 0x" << hex << pa_detected_labels[i].address);

			gexporter->exportGraphsToFile(function_name, dp->getCFG(pa_detected_labels[i].address));
		}
	}

	report->setCodeSize(dp->getCodeSize());
	LOG_INFO(logger, id_log_prefix << "Code size is: " << dp->getCodeSize() << " number of functions: " << dp->getFunctionCount());
	FunctionCallGraph fg = dp->getFCG();

	if(conf->getBool(CONF_EXPORT_FUNCTION_CALL_GRAPH))
	{
		string file_name("functions");
		file_name.append(id_appendix);
		gexporter->exportGraphsToFile(file_name, fg);
	}

	ControlFlowGraph scfg = dp->getSCFG();

	if(conf->getBool(CONF_EXPORT_SCFG))
	{
		string file_name("supergraph");
		file_name.append(id_appendix);
		gexporter->exportGraphsToFile(file_name, scfg);
	}



	// if there a flow fact file should be used add the loop bounds to the nodes, otherwise proceed without loop bounds
	FlowFactEnricher *ffe=NULL;
	if(conf->getBool(CONF_USE_FLOWFACT_GRAPH_ENRICHMENT))
	{
		assert(conf->getBool(CONF_USE_FLOWFACT_FILE));
		ffe = new FlowFactEnricher(dp->getSCFG(), dp->getSCFGEntry(), dp->getSCFGExit());
		ffe->setDetectedFunctions(pa_detected_labels);
		ffe->getLoopBounds();
		if(conf->getBool(CONF_EXPORT_FLOW_SCFG))
		{
			string file_name("supergraph_w_ff");
			file_name.append(id_appendix);
			gexporter->exportGraphsToFile(file_name, ffe->getGraphWithFlowFacts());
		}
		pa_out_scfg = ffe->getGraphWithFlowFacts();
		pa_out_scfg_entry = ffe->getEntry();
		pa_out_scfg_exit = ffe->getExit();
	}
	else
	{
		pa_out_scfg = dp->getSCFG();
		pa_out_scfg_entry = dp->getSCFGEntry();
		pa_out_scfg_exit = dp->getSCFGExit();
	}
	pa_out_function_cfgs = dp->getCFGForAllFunctions();
	pa_out_fcgo = *(dp->getFCGObj());
	pa_finished = true;

	if(ffe != NULL)
	{
		delete(ffe);
	}
	return pa_finished;
}

bool TimingAnalysis::analysePipeline(void)
{
	assert(pa_finished);

	ControlFlowGraphCostCalculator cfgcc(pa_out_scfg, pa_out_scfg_entry, pa_out_scfg_exit);
	cfgcc.calculateCost();
	ca_out_scfg = cfgcc.getCFG();
	ca_out_scfg_entry = cfgcc.getCFGEntry();
	ca_out_scfg_exit = cfgcc.getCFGExit();
	ca_finished = true;

	return ca_finished;

}

bool TimingAnalysis::analyseImem(void)
{
	assert(ca_finished);

	switch(mp->getMemType())
	{
		case BBSISP:
		case BBSISP_JP:
		case BBSISP_WCP:
		case BBSISP_JP_WCP:
		case FSISP_OLD:
		case FSISP:
		case FSISP_WCP:
			return analyseImem(mp->getSispParameters().size);
			break;
		case ICACHE:
			return analyseImem(mp->getCacheParameters().size);
			break;
		case DISP:
			return analyseImem(mp->getDispParameters().size);
			break;
		case NO_MEM:
			return analyseNomem();
			break;
		case VIVU_TEST:
			return analyseVivuTest();
			break;
		default:
			LOG_ERROR(logger, id_log_prefix << "No memory type specified.");
			assert(false);
	}
	return false;
}

bool TimingAnalysis::analyseImem(uint32_t mem_size)
{
	assert(ca_finished);

	switch(mp->getMemType())
	{
		case BBSISP:
		case BBSISP_JP:
		case BBSISP_WCP:
		case BBSISP_JP_WCP:
			return analyseBBSISP(mem_size);
			break;
		case FSISP_OLD:
			return analyseFSISP_OLD(mem_size);
			break;
		case FSISP:
		case FSISP_WCP:
			return analyseFSISP(mem_size);
			break;
		case ICACHE:
			return analyseICache(mem_size);
			break;
		case DISP:
			return analyseDISP(mem_size);
			break;
		default:
			LOG_ERROR(logger, id_log_prefix << "No memory type specified.");
			assert(false);
	}
	return false;
}

bool TimingAnalysis::analyseNomem(void)
{
	assert(ca_finished);

	LOG_DEBUG(logger, id_log_prefix << "Analyzing program without memory.");

	ma_out_scfg = ca_out_scfg;
	ma_out_scfg_entry = ca_out_scfg_entry;
	ma_out_scfg_exit = ca_out_scfg_exit;
	ma_finished = true;

	report->setMemSize(mp->getUsableMemorySize());

	return ma_finished;
}

bool TimingAnalysis::analyseVivuTest(void)
{
	LOG_DEBUG(logger, id_log_prefix << "Creating memory state graph (VIVU) ...");
	VivuGraphCreator *vgc = new VivuGraphCreator(ca_out_scfg, ca_out_scfg_entry, ca_out_scfg_exit);
	CFGMSGPair vivu_graph = vgc->createVivuGraph();
	MsgToCfgConverter msg2cfg = MsgToCfgConverter(vivu_graph, vgc->getEntry(), vgc->getExit());
	ControlFlowGraph vivu_cfg = msg2cfg.getCfg();

	if(conf->getBool(CONF_EXPORT_VIVU_GRAPH))
	{
		LOG_DEBUG(logger, id_log_prefix << "Exporting memory state graph (VIVU) ...");
		gexporter->exportGraphsToFile("vivu_orig_graph", vivu_graph.second);
		gexporter->exportGraphsToFile("vivu_graph", vivu_cfg);
	}

	ma_out_scfg = vivu_cfg;
	ma_out_scfg_entry = msg2cfg.getEntry();
	ma_out_scfg_exit = msg2cfg.getExit();
	ma_finished = true;

	report->setMemSize(mp->getUsableMemorySize());

	return ma_finished;
}

bool TimingAnalysis::analyseBBSISP(uint32_t sisp_size)
{
	assert(ca_finished);

	if(!baseline_calculated)
	{
		// calculate baseline estimate for static memories
		calculateBaselineEstimate(ca_out_scfg, ca_out_scfg_entry, ca_out_scfg_exit);
		assert(baseline_calculated);

		if(conf->getBool(CONF_MEMORY_SIZE_STEPPING))
		{
			report->setWCCostValue(baseline_timing_estimate, baseline_solution_type);
			report->setMemCostValue(baseline_mem_estimate, baseline_solution_type);
			report->setMemSize(0, 0);
			report->generate_line();
		}
	}

	SISPOptimizer_IF *bbsisp;

	if(conf->getBool(CONF_EXPORT_SOLVED_FLOW_SCFG_WITH_ASSIGNMENT))
	{
		stringstream file_name;
		file_name << "scfg_preBBSISP_" << sisp_size << id_appendix;
		gexporter->exportGraphsToFile(file_name.str(), bl_out_scfg);
	}

	switch(mp->getMemType())
	{
		case BBSISP:
			LOG_INFO(logger, id_log_prefix << "Starting Knapsack-based BBSISP allocation");
			bbsisp = new BBSISPOptimizer(bl_out_scfg, bl_out_scfg_entry, bl_out_scfg_exit);
			break;
		case BBSISP_JP:
			LOG_INFO(logger, id_log_prefix << "Starting Knapsack-based BBSISP allocation with penalties");
			bbsisp = new BBSISPOptimizerJP(bl_out_scfg, bl_out_scfg_entry, bl_out_scfg_exit);
			break;
		case BBSISP_WCP:
			LOG_INFO(logger, id_log_prefix << "Starting WCP-aware BBSISP allocation");
			bbsisp = new BBSISPOptimizerWCP(bl_out_scfg, bl_out_scfg_entry, bl_out_scfg_exit);
			break;
		case BBSISP_JP_WCP:
			LOG_INFO(logger, id_log_prefix << "Starting WCP-aware BBSISP allocation with penalties");
			bbsisp = new BBSISPOptimizerJPWCP(bl_out_scfg, bl_out_scfg_entry, bl_out_scfg_exit);
			break;
		default:
			assert(false);
	}

	bbsisp->setSize(sisp_size);
	bbsisp->calculateBlockAssignment();
	vector<uint32_t> assigned_blocks = bbsisp->getBlockAssignment();
	lp_solution_t bb_solution_type = bbsisp->getSolutionType();

	LOG_DEBUG(logger, id_log_prefix << "Block assignement for sisp size of " << sisp_size << " byte (used " << bbsisp->getUsedSispSize() << ")");
	sort(assigned_blocks.begin(), assigned_blocks.end());
	for(uint32_t i = 0; i < assigned_blocks.size(); i++)
	{
		LOG_DEBUG(logger, id_log_prefix << "0x" << hex << assigned_blocks[i]);
	}


	// update the CFG with the assigned basic blocks
	ControlFlowGraphCostCalculator cfgcc(bl_out_scfg, bl_out_scfg_entry, bl_out_scfg_exit);
	if((mp->getMemType() == BBSISP_JP) || (mp->getMemType() == BBSISP_JP_WCP) || (mp->getSispParameters().use_jump_penalties))
	{
		// recalculate the bb costs after assignment (since the jump penalties may change the costs of the bbs)
		cfgcc.considerMemoryAssignment(assigned_blocks, true);
	}
	else
	{
		// just set the basic blocks, and do not recalculate their cost (no jump penalties are considered)
		cfgcc.considerMemoryAssignment(assigned_blocks, false);
	}
	ma_out_scfg = cfgcc.getCFG();
	ma_out_scfg_entry = cfgcc.getCFGEntry();
	ma_out_scfg_exit = cfgcc.getCFGExit();
	ma_out_sisp_result = bbsisp->getResults();
	ma_finished = true;


	if(conf->getBool(CONF_EXPORT_SOLVED_FLOW_SCFG_WITH_ASSIGNMENT))
	{
		stringstream file_name;
		file_name << "ilpgraph_BBSISP_" << sisp_size << id_appendix;
		gexporter->exportGraphsToFile(file_name.str(), ma_out_scfg);
	}


	report->setBlockAssignment(bbsisp->getBlockAssignment(), bb_solution_type);
	report->setMemSize(sisp_size, bbsisp->getUsedSispSize());

	delete(bbsisp);

	return ma_finished;
}

bool TimingAnalysis::analyseFSISP(uint32_t sisp_size)
{
	assert(ca_finished);

	if(!baseline_calculated)
	{
		// calculate baseline estimate for static memories
		calculateBaselineEstimate(ca_out_scfg, ca_out_scfg_entry, ca_out_scfg_exit);
		assert(baseline_calculated);

		if(conf->getBool(CONF_MEMORY_SIZE_STEPPING))
		{
			report->setWCCostValue(baseline_timing_estimate, baseline_solution_type);
			report->setMemCostValue(baseline_mem_estimate, baseline_solution_type);
			report->setMemSize(0, 0);
			report->generate_line();
		}
	}

	SISPOptimizer_IF *fsisp;

	vector<addr_label_t> assigned_functions;

	if(conf->getBool(CONF_EXPORT_SOLVED_FLOW_SCFG_WITH_ASSIGNMENT))
	{
		stringstream file_name;
		file_name << "ilpgraph_preFSISP_" << sisp_size << id_appendix;
		gexporter->exportGraphsToFile(file_name.str(), bl_out_scfg);
	}

	switch(mp->getMemType())
	{
		case FSISP:
			LOG_INFO(logger, id_log_prefix << "Starting Knapsack-based FSISP allocation");
			fsisp = new FSISPOptimizer(bl_out_scfg, bl_out_scfg_entry, bl_out_scfg_exit, pa_out_function_cfgs);
			fsisp->setSize(sisp_size);
			fsisp->calculateBlockAssignment();
			assigned_functions = ((FSISPOptimizer*)fsisp)->getFunctionAssignment();
			break;
		case FSISP_WCP:
			LOG_INFO(logger, id_log_prefix << "Starting WCP-aware FSISP allocation");
			fsisp = new FSISPOptimizerWCP(bl_out_scfg, bl_out_scfg_entry, bl_out_scfg_exit, pa_out_function_cfgs);
			fsisp->setSize(sisp_size);
			fsisp->calculateBlockAssignment();
			assigned_functions = ((FSISPOptimizerWCP*)fsisp)->getFunctionAssignment();
			break;
		default:
			assert(false);
	}

	stringstream str_assigned_functions;
	for(vector<addr_label_t>::iterator al_it = assigned_functions.begin(); al_it != assigned_functions.end(); al_it++)
	{
		str_assigned_functions << (*al_it).label << " ( 0x" << hex << (*al_it).address << dec << " ) ";
	}

	LOG_DEBUG(logger, id_log_prefix << id_log_prefix << "Assigned the following functions to fsisp: " << str_assigned_functions.str());

	vector<uint32_t> assigned_blocks = fsisp->getBlockAssignment();

	LOG_DEBUG(logger, id_log_prefix << id_log_prefix << "Block assignement for sisp size of " << sisp_size << " byte (used " << fsisp->getUsedSispSize() << ")");
	sort(assigned_blocks.begin(), assigned_blocks.end());
	for(uint32_t i = 0; i < assigned_blocks.size(); i++)
	{
		LOG_DEBUG(logger, id_log_prefix << id_log_prefix << "0x" << hex << assigned_blocks[i]);
	}

	// update the CFG with the assigned functions
	ControlFlowGraphCostCalculator cfgcc(bl_out_scfg, bl_out_scfg_entry, bl_out_scfg_exit);
	cfgcc.considerMemoryAssignment(assigned_blocks, false);

	ma_out_scfg = cfgcc.getCFG();
	ma_out_scfg_entry = cfgcc.getCFGEntry();
	ma_out_scfg_exit = cfgcc.getCFGExit();
	ma_out_sisp_result = fsisp->getResults();
	ma_finished= true;

	if(conf->getBool(CONF_EXPORT_SOLVED_FLOW_SCFG_WITH_ASSIGNMENT))
	{
		stringstream file_name;
		file_name << "ilpgraph_FSISP_" << sisp_size << id_appendix;
		gexporter->exportGraphsToFile(file_name.str(), ma_out_scfg);
	}

	report->setFunctionAssignment(assigned_functions);
	report->setMemSize(sisp_size, fsisp->getUsedSispSize());

	if(conf->getBool(CONF_STATIC_MAPPING_REPORT))
	{
		report->generateStaticFunctionMapping();
	}

	delete(fsisp);

	return ma_finished;
}

bool TimingAnalysis::analyseFSISP_OLD(uint32_t sisp_size)
{
	assert(ca_finished);

	if(!baseline_calculated)
	{
		// calculate baseline estimate for static memories
		calculateBaselineEstimate(ca_out_scfg, ca_out_scfg_entry, ca_out_scfg_exit);
		assert(baseline_calculated);

		if(conf->getBool(CONF_MEMORY_SIZE_STEPPING))
		{
			report->setWCCostValue(baseline_timing_estimate, baseline_solution_type);
			report->setMemCostValue(baseline_mem_estimate, baseline_solution_type);
			report->setMemSize(0, 0);
			report->generate_line();
		}
	}

	FSISPOptimizerOLD fsisp;
	fsisp.setSize(sisp_size);
	fsisp.setFunctions(pa_out_fcgo.getFCG(), pa_out_function_cfgs);
	fsisp.calculateFunctionBenefitsWithSCFGProperties(bl_out_scfg);
	fsisp.calculateKnapsackFunctionAssignment();

	vector<uint32_t> assigned_blocks = fsisp.getBlockAssignment();

	LOG_DEBUG(logger, id_log_prefix << "Block assignement for sisp size of " << sisp_size << " byte (used " << fsisp.getUsedSispSize() << ")");
	sort(assigned_blocks.begin(), assigned_blocks.end());
	for(uint32_t i = 0; i < assigned_blocks.size(); i++)
		LOG_DEBUG(logger, id_log_prefix << "0x" << hex << assigned_blocks[i]);

	// update the CFG with the assigned functions
	ControlFlowGraphCostCalculator cfgcc(bl_out_scfg, bl_out_scfg_entry, bl_out_scfg_exit);
	cfgcc.considerMemoryAssignment(assigned_blocks, false);

	ma_out_scfg = cfgcc.getCFG();
	ma_out_scfg_entry = cfgcc.getCFGEntry();
	ma_out_scfg_exit = cfgcc.getCFGExit();
	ma_finished = true;

	if(conf->getBool(CONF_EXPORT_SOLVED_FLOW_SCFG_WITH_ASSIGNMENT))
	{
		stringstream file_name;
		file_name << "ilpgraph_FSISP_" << sisp_size << id_appendix;
		gexporter->exportGraphsToFile(file_name.str(), ma_out_scfg);
	}

	if(conf->getBool(CONF_STATIC_MAPPING_REPORT))
	{
		report->generateStaticFunctionMapping();
	}

	return ma_finished;
}

bool TimingAnalysis::analyseICache(uint32_t cache_size)
{
	LOG_DEBUG(logger, id_log_prefix << "Creating memory state graph (VIVU) ...");
	VivuGraphCreator *vgc = new VivuGraphCreator(ca_out_scfg, ca_out_scfg_entry, ca_out_scfg_exit);
	CFGMSGPair vivu_graph = vgc->createVivuGraph();

	if(conf->getBool(CONF_EXPORT_VIVU_GRAPH))
	{
		LOG_DEBUG(logger, id_log_prefix << "Exporting memory state graph (VIVU) ...");

		{
			string file_name("vivu_orig_graph");
			file_name.append(id_appendix);
			gexporter->exportGraphsToFile(file_name, vivu_graph.second);
		}

		MsgToCfgConverter msg2cfg = MsgToCfgConverter(vivu_graph, vgc->getEntry(), vgc->getExit());
		ControlFlowGraph vivu_cfg = msg2cfg.getCfg();

		{
			string file_name("vivu_graph");
			file_name.append(id_appendix);
			gexporter->exportGraphsToFile(file_name, vivu_cfg);
		}
	}

	LOG_DEBUG(logger, id_log_prefix << "Starting data flow analysis for cache ...");
	ICacheDataFlowAnalyzer_IF *icdfa;
	if(mp->getReplacementPolicy() != FIFO)
	{
		icdfa = new ICacheDataFlowAnalyzer(vivu_graph, vgc->getEntry(), vgc->getExit(), cache_size);
	}
	else
	{
		icdfa = new ICacheDataFlowAnalyzerMap(vivu_graph, vgc->getEntry(), vgc->getExit(), cache_size);
	}

	icdfa->analyzeCache();
	icdfa->categorizeCacheAccesses();

	LOG_DEBUG(logger, id_log_prefix << "DFA created " << dec << icdfa->getRepresentationStateCount() << " (" << icdfa->getMemoryStateCount() << ")" << " concrete memory state objects with size: " << icdfa->getUsedMemSize() << " number of maintained memory references: " << icdfa->getUsedMemReferences());

	CFGMSGPair vivu_cache_graph = icdfa->getGraph();
	MsgToCfgConverter msg2cfg = MsgToCfgConverter(vivu_cache_graph, icdfa->getEntry(), icdfa->getExit());
	ControlFlowGraph vivu_cache_cfg = msg2cfg.getCfg();

	if(conf->getBool(CONF_EXPORT_VIVU_GRAPH))
	{
		LOG_DEBUG(logger, id_log_prefix << "Exporting memory state graph (VIVU) with cache costs ...");
		string file_name("vivu_cache_graph");
		file_name.append(id_appendix);
		gexporter->exportGraphsToFile(file_name, vivu_cache_cfg);
	}

	ma_out_scfg = vivu_cache_cfg;
	ma_out_scfg_entry = msg2cfg.getEntry();
	ma_out_scfg_exit = msg2cfg.getExit();
	ma_finished = true;

	report->setMemSize(cache_size);
	report->setDFAStatistics(icdfa->getMemoryStateCount(), icdfa->getRepresentationStateCount(), icdfa->getUsedMemSize(), icdfa->getUsedMemReferences());

	delete(icdfa);

	return ma_finished;
}

bool TimingAnalysis::analyseDISP(uint32_t disp_size)
{
	LOG_DEBUG(logger, id_log_prefix << "Creating memory state graph (VIVU) ...");
	VivuGraphCreator *vgc = new VivuGraphCreator(ca_out_scfg, ca_out_scfg_entry, ca_out_scfg_exit);
	CFGMSGPair vivu_graph = vgc->createVivuGraph();
	if(conf->getBool(CONF_EXPORT_VIVU_GRAPH))
	{
		LOG_DEBUG(logger, id_log_prefix << "Exporting memory state graph (VIVU) ...");

		{
			string file_name("vivu_orig_graph");
			file_name.append(id_appendix);
			gexporter->exportGraphsToFile(file_name, vivu_graph.second);
		}

		MsgToCfgConverter msg2cfg = MsgToCfgConverter(vivu_graph, vgc->getEntry(), vgc->getExit());
		ControlFlowGraph vivu_cfg = msg2cfg.getCfg();

		{
			string file_name("vivu_graph");
			file_name.append(id_appendix);
			gexporter->exportGraphsToFile(file_name, vivu_cfg);
		}
	}

	LOG_DEBUG(logger, id_log_prefix << "Starting data flow analysis for DISP ...");
	DISPDataFlowAnalyzer_IF *disp_dfa;

	if((mp->getReplacementPolicy() != FIFO) && (mp->getReplacementPolicy() != STACK))
	{
		disp_dfa = new DISPDataFlowAnalyzer(vivu_graph, vgc->getEntry(), vgc->getExit(), &pa_out_fcgo, disp_size);
	}
	else
	{
		disp_dfa = new DISPDataFlowAnalyzerMap(vivu_graph, vgc->getEntry(), vgc->getExit(), &pa_out_fcgo, disp_size);
	}

	disp_dfa->analyzeDISP();
	disp_dfa->categorizeMemAccesses();

	LOG_DEBUG(logger, id_log_prefix << "DFA created " << dec << disp_dfa->getRepresentationStateCount() << " (" << disp_dfa->getMemoryStateCount() << ")"  << " concrete memory state objects with size: " << disp_dfa->getUsedMemSize() << " number of maintained memory references: " << disp_dfa->getUsedMemReferences());

	CFGMSGPair vivu_disp_graph = disp_dfa->getGraph();
	MsgToCfgConverter msg2cfg = MsgToCfgConverter(vivu_disp_graph, disp_dfa->getEntry(), disp_dfa->getExit());
	ControlFlowGraph vivu_disp_cfg = msg2cfg.getCfg();

	if(conf->getBool(CONF_EXPORT_VIVU_GRAPH))
	{
		LOG_DEBUG(logger, id_log_prefix << "Exporting memory state graph (VIVU) with DISP costs ...");
		string file_name("vivu_disp_graph");
		file_name.append(id_appendix);
		gexporter->exportGraphsToFile(file_name, vivu_disp_cfg);
	}

	ma_out_scfg = vivu_disp_cfg;
	ma_out_scfg_entry = msg2cfg.getEntry();
	ma_out_scfg_exit = msg2cfg.getExit();
	ma_finished = true;

	report->setMemSize(disp_size);
	report->setDFAStatistics(disp_dfa->getMemoryStateCount(), disp_dfa->getRepresentationStateCount(), disp_dfa->getUsedMemSize(), disp_dfa->getUsedMemReferences());

	delete(disp_dfa);

	return ma_finished;

}

bool TimingAnalysis::calculateBaselineEstimate(ControlFlowGraph ilp_in_graph, CFGVertex ilp_in_entry, CFGVertex ilp_in_exit)
{

	assert(ca_finished);

	ControlFlowGraph ilpcfg;
	ILPGenerator *ilpg;
	string ilp_file;

	// generate ILP only for static mems
	ilpg = new ILPGenerator(ilp_in_graph, ilp_in_entry, ilp_in_exit);
	ilpg->setDetectedFunctions(pa_detected_labels);
	ilpg->createILP();


	ilpcfg = ilpg->getILPCFG();

	if(conf->getBool(CONF_EXPORT_FLOW_SCFG))
	{
		string file_name("ilpgraph_WOflows_baseline");
		file_name.append(id_appendix);
		gexporter->exportGraphsToFile(file_name, ilpcfg);
	}

	if(conf->getBool(CONF_EXPORT_ILPS))
	{
		ilp_file = "isptap_";
		string function_name = entry_function;
		ilp_file += function_name;
		ilp_file += "baseline";
		ilp_file += ".ilp";
		ilpg->writeILPFile(ilp_file);
	}
	
	ilpg->solveILP();

	if(conf->getBool(CONF_EXPORT_BB_COST))
	{
		string file_name("nomem_wo_assignment");
		file_name.append(id_appendix);
		cexporter->exportCostToFile(ilpg->getILPCFG(), file_name);
	}

	rchecker->check_result(ilpg->getWCCostValue(), CONF_OUTPUT_WCET_WO_OPT);
	rchecker->check_result(ilpg->getMemCostValue(), CONF_OUTPUT_MEM_COST_WO_OPT);

	ilpcfg = ilpg->getILPCFG();
	if(conf->getBool(CONF_EXPORT_SOLVED_FLOW_SCFG))
	{
		string file_name("ilpgraph_baseline");
		file_name.append(id_appendix);
		gexporter->exportGraphsToFile(file_name, ilpcfg);
	}
	if(conf->getBool(CONF_EXPORT_WC_PATH) || conf->getBool(CONF_EXPORT_WC_PATH_HIST) || conf->getBool(CONF_EXPORT_WC_PATH_INSTR_STATS))
	{
		// original WC path, without consideration of the knapsack optimization
		string wcpath_file = conf->getString(CONF_EXPORT_WC_PATH_FILEPREFIX)+"baseline.wcp";
		string wchist_file = conf->getString(CONF_EXPORT_WC_PATH_FILEPREFIX)+"baseline.wch";
		WCPathExporter wcpw(ilpcfg, ilpg->getEntry(), ilpg->getExit(), wcpath_file, wchist_file);

		if(conf->getBool(CONF_EXPORT_WC_PATH))
		{
			wcpw.traverseAndPrintWCPath();
		}
		if(conf->getBool(CONF_EXPORT_WC_PATH_HIST))
		{
			wcpw.printWCHist();
		}
		if(conf->getBool(CONF_EXPORT_WC_PATH_INSTR_STATS))
		{
			report->setWCPathInstructionStatistics(wcpw.getInstrStats());
		}
	}



	bl_out_scfg = ilpg->getILPCFG();
	bl_out_scfg_entry = ilpg->getEntry();
	bl_out_scfg_exit = ilpg->getExit();

	baseline_calculated = true;
	baseline_timing_estimate = ilpg->getWCCostValue();
	baseline_mem_estimate = ilpg->getMemCostValue();
	baseline_solution_type = ilpg->getSolutionType();

	delete ilpg;

	return baseline_calculated;
}

bool TimingAnalysis::calculateEstimate(void)
{
	assert(ca_finished);

	switch(mp->getMemType())
	{
		case BBSISP:
		case BBSISP_JP:
		case BBSISP_WCP:
		case BBSISP_JP_WCP:
		case FSISP_OLD:
		case FSISP:
		case FSISP_WCP:
			return calculateEstimate(mp->getSispParameters().size);
			break;
		case ICACHE:
			return calculateEstimate(mp->getCacheParameters().size);
			break;
		case DISP:
			return calculateEstimate(mp->getDispParameters().size);
			break;
		case NO_MEM:
		case VIVU_TEST:
			return calculateEstimate(0);
			break;
		default:
			LOG_ERROR(logger, id_log_prefix << "No memory type specified.");
			assert(false);
	}
	return false;
}

bool TimingAnalysis::calculateEstimate(uint32_t mem_size)
{
	assert(ma_finished);

	ILPGenerator *ilpg = new ILPGenerator(ma_out_scfg, ma_out_scfg_entry, ma_out_scfg_exit);
	ilpg->setDetectedFunctions(pa_detected_labels);
	ilpg->createILP();

	if(conf->getBool(CONF_EXPORT_FLOW_SCFG))
	{
		string file_name("ilpgraph_WOflows");
		file_name.append(id_appendix);
		gexporter->exportGraphsToFile(file_name, ilpg->getILPCFG());
	}


	if(conf->getBool(CONF_EXPORT_ILPS))
	{
		string ilp_file = "isptap_";
		string function_name = entry_function;
		ilp_file += function_name;
		ilp_file += ".ilp";
		ilpg->writeILPFile(ilp_file);
	}

	ilpg->solveILP();


	estimate = ilpg->getWCCostValue();
	estimate_solution_type = ilpg->getSolutionType();
	if(estimate_solution_type == ErrorWhileSolving)
	{
		LOG_ERROR(logger, id_log_prefix << "ILP could not be solved: Unknown error occured while solving.");
		return false;
	}
	else if(estimate_solution_type == ProblemUnbound)
	{
		LOG_ERROR(logger, id_log_prefix << "ILP could not be solved: Problem is unbound.");
		return false;
	}
	else if(estimate_solution_type == SolutionNotCalculated)
	{
		LOG_ERROR(logger, id_log_prefix << "ILP could not be solved: Solution was not calculated.");
		return false;
	}

	ec_finished = true;

	if(conf->getBool(CONF_EXPORT_BB_COST))
	{
		stringstream costExpFileExt;
		costExpFileExt << "m" << mp->getMemType() << "_size_" << mem_size;
		cexporter->exportCostToFile(ilpg->getILPCFG(), costExpFileExt.str());
	}

	if(conf->getBool(CONF_MEMORY_SIZE_STEPPING))
	{
		rchecker->check_result(ilpg->getWCCostValue(), CONF_OUTPUT_WCET_STEP, mem_size);
		rchecker->check_result(ilpg->getMemCostValue(), CONF_OUTPUT_MEM_COST_STEP, mem_size);
	}
	else
	{
		rchecker->check_result(ilpg->getWCCostValue(), CONF_OUTPUT_WCET);
		rchecker->check_result(ilpg->getMemCostValue(), CONF_OUTPUT_MEM_COST);
	}

	switch(mp->getMemType())
	{
		case BBSISP:
		case BBSISP_WCP:
		case BBSISP_JP:
		case BBSISP_JP_WCP:
		case FSISP:
		case FSISP_WCP:
			rchecker->check_sisp_assignment(mp->getMemType(), mp->getMemTypeString(), &ma_out_sisp_result, ilpg, mp->getSispParameters().use_jump_penalties);
			break;
		default:
			break;
	}

	if(conf->getBool(CONF_EXPORT_SOLVED_FLOW_SCFG))
	{
		string file_name("ilpgraph");
		file_name.append(id_appendix);
		gexporter->exportGraphsToFile(file_name, ilpg->getILPCFG());
	}
	if(conf->getBool(CONF_EXPORT_WC_PATH) || conf->getBool(CONF_EXPORT_WC_PATH_HIST) || conf->getBool(CONF_EXPORT_WC_PATH_INSTR_STATS))
	{
		string wcpath_file = conf->getString(CONF_EXPORT_WC_PATH_FILEPREFIX)+".wcp";
		string wchist_file = conf->getString(CONF_EXPORT_WC_PATH_FILEPREFIX)+".wch";
		WCPathExporter wcpw(ilpg->getILPCFG(), ilpg->getEntry(), ilpg->getExit(), wcpath_file, wchist_file);

		if(conf->getBool(CONF_EXPORT_WC_PATH))
		{
			wcpw.traverseAndPrintWCPath();
		}
		if(conf->getBool(CONF_EXPORT_WC_PATH_HIST))
		{
			wcpw.printWCHist();
		}

		if(conf->getBool(CONF_EXPORT_WC_PATH_INSTR_STATS))
		{
			report->setWCPathInstructionStatistics(wcpw.getInstrStats());
		}
	}

	if(IS_STATIC_MEM(mp->getMemType()))
	{
		assert(baseline_calculated);
		report->setWCCostValue(baseline_timing_estimate, ilpg->getWCCostValue(), baseline_solution_type, ilpg->getSolutionType());
	}
	else
	{
		report->setWCCostValue(ilpg->getWCCostValue(), ilpg->getSolutionType());
	}
	report->setMemCostValue(ilpg->getMemCostValue(), ilpg->getSolutionType());
	if(mp->getMemType() == ICACHE)
	{
		report->setCacheHMStats(ilpg->getCacheHMStatsForWCP());
	}

	delete(ilpg);

	return ec_finished;

}

uint64_t TimingAnalysis::getEstimate(void)
{
	return estimate;
}

lp_solution_t TimingAnalysis::getSolutionType(void)
{
	return estimate_solution_type;
}

