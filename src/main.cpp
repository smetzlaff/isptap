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
#include <utility>                   // for std::pair
#include <algorithm>                 // for std::for_each
#include <iostream>                  // for std::cout
#include <time.h>
#include <boost/regex.hpp>
#include <boost/program_options.hpp>

#include "config.h"
#include "global.h"
#include "constants.h"

#include "analysis/timing_analysis.hpp"

// for getHelpMessage()
#include "arch/arch_cfg.hpp"
#include "arch/carcore_cfg.hpp"
#include "arch/armv6m_cfg.hpp"
#include "util/jump_target_extractor.hpp"
#include "util/call_target_extractor.hpp"


LoggerPtr logger(Logger::getLogger("Main"));

#define ISPTAP_VERSION PACKAGE_VERSION
#define COPYRIGHT "Copyright (C) 2013 Stefan Metzlaff, University of Augsburg, Germany"

struct core_wcet_t {
	uint32_t core_id;
	string entry_function;
	uint64_t wcet_estimate;
	lp_solution_t solution_type;
};

void isptap_main(Configuration *conf);
void printVersion(void);

int main(int argc, char** argv)
{
	uint64_t cstart, cend;
	double cdiff;

	cstart = clock();

	Configuration *conf;

	// TODO Check scfg creation and optimization, because edge types are changed in for jumps and calls from ForwardStep to ForwardJump, BackwardJump


	// TODO Documentation needed! (in main function and in all header files)

	program_options::options_description op_required("Required options");
	op_required.add_options()
		("config-file,c", program_options::value<string>(), "configuration file (required)")
		;

	program_options::options_description op_basic("");
	op_basic.add_options()
		("help,h", "This help message")
		("help-config", "Help message for the format of the configuration files.")
		("help-architecture", "Help message for the format of the architecture files.")
		("help-carcore", "Help message for the format of the CarCore config files.")
		("help-armv6m", "Help message for the format of the ARMv6M config files.")
		("help-flow-facts", "Help message for the format of the flow-fact files.")
		("version,v", "version")
		;

	program_options::options_description op_add("Additional parameters to overwrite/set specific config file settings");
	op_add.add_options()
		("set-arch-file,a", program_options::value<string>(), "set architectural configuration file")
		("set-log-file,l", program_options::value<string>(), "set log output file")
		("set-rpt-file,r", program_options::value<string>(), "set report file")
		("set-log-dir,L", program_options::value<string>(), "set log output dir (parameter is added as prefix)")
		("set-rpt-dir,R", program_options::value<string>(), "set report dir (parameter is added as prefix)")
		("set-out-dir,D", program_options::value<string>(), "set log and report dir (parameter is added as prefix, do not use with -L or -R)")
		;

	program_options::options_description op("All Options");
	op.add(op_required).add(op_basic).add(op_add);

	stringstream s;
	s << endl << "Usage:\n" << argv[0] << " configs/config_file.cfg\n\nParameters";
	program_options::options_description op_desc(s.str());
	op_desc.add(op_basic).add(op_add);

	program_options::positional_options_description pos_op;
	pos_op.add("config-file", -1);

	program_options::variables_map vm;
	vector<string> wrong_parameters;

	try {
		// parse parameters
		program_options::parsed_options parsed_params = program_options::command_line_parser(argc, argv).options(op).positional(pos_op).allow_unregistered().run();

		// propagate to variables_map
		program_options::store(parsed_params, vm);
		program_options::notify(vm);

		// check for wrong parameters
		wrong_parameters = collect_unrecognized(parsed_params.options, program_options::include_positional);
	}
	catch(program_options::ambiguous_option e)
	{
		cout << endl << "Wrong parameter: " << e.what() << endl;
		cout << op_desc << endl;
		exit(1);
	}


	if(wrong_parameters.size() != 0)
	{
		if (vm.count("config-file") && wrong_parameters.size()==1)
		{
			// Somehow the positional option is also recognized as unrecognized. 
			// If the config file is the only unknown parameter, proceed with normal operation.
			goto read_config;
		}
		cout << "Wrong parameter(s): ";
		for(vector<string>::iterator it = wrong_parameters.begin(); it != wrong_parameters.end(); ++it)
		{
			cout << *it << " "; 
		}
		cout << endl;
		cout << op_desc << endl;

		exit(1);
	}
	else if(vm.count("help"))
	{
		cout << op_desc << endl;
		exit(1);
	}
	else if(vm.count("help-config"))
	{
		cout << Configuration::getInstanceWithHelpMessages()->getHelpMessage() << endl;
		exit(1);
	}
	else if(vm.count("help-architecture"))
	{
		cout << ArchConfig::getBasicHelpMessage() << endl;
		exit(1);
	}
	else if(vm.count("help-carcore"))
	{
		cout << CarCoreConfig::getInstanceWithHelpMessages()->getHelpMessage() << endl;
		exit(1);
	}
	else if(vm.count("help-armv6m"))
	{
		cout << Armv6mConfig::getInstanceWithHelpMessages()->getHelpMessage() << endl;
		exit(1);
	}
	else if(vm.count("help-flow-facts"))
	{
		cout << FlowFactReader::getHelpMessage() << endl;
		cout << CallTargetExtractor::getHelpMessage() << endl;
		cout << JumpTargetExtractor::getHelpMessage() << endl;
		exit(1);
	}
	else if(vm.count("version"))
	{
		printVersion();
		exit(1);
	}
	else if (vm.count("config-file")) 
	{
read_config:
		conf = Configuration::getInstance();
		cout << "Processing config file: " << vm["config-file"].as<string>() << endl;
		// set up isptap configuration and logging support
		conf->parseConfigFile(vm["config-file"].as<string>());

		if(vm.count("set-arch-file"))
		{
			cout << "Manually setting architectual configuration file: " <<  vm["set-arch-file"].as<string>() << endl;
			conf->setProperty(CONF_USE_ARCH_CFG_FILE, true);
			conf->setProperty(CONF_ARCH_CFG_FILE, vm["set-arch-file"].as<string>());
		}
		if(vm.count("set-log-file"))
		{
			cout << "Manually setting log output file: " <<  vm["set-log-file"].as<string>() << endl;
			conf->setProperty(CONF_LOG_FILE, vm["set-log-file"].as<string>());
		}
		if(vm.count("set-rpt-file"))
		{
			cout << "Manually setting report file: " <<  vm["set-rpt-file"].as<string>() << endl;
			conf->setProperty(CONF_REPORT_FILE, vm["set-rpt-file"].as<string>());
		}
		if(vm.count("set-log-dir"))
		{
			cout << "Manually setting log output dir: " <<  vm["set-log-dir"].as<string>() << endl;
			string log_file = conf->getString(CONF_LOG_FILE);
			log_file = vm["set-log-dir"].as<string>() + log_file;
			cout << "Log file is now: " <<  log_file << endl;
			conf->setProperty(CONF_LOG_FILE, log_file);
		}
		if(vm.count("set-rpt-dir"))
		{
			cout << "Manually setting report dir: " <<  vm["set-rpt-dir"].as<string>() << endl;
			string rpt_file = conf->getString(CONF_REPORT_FILE);
			rpt_file = vm["set-rpt-dir"].as<string>() + rpt_file;
			cout << "Report file is now: " <<  rpt_file << endl;
			conf->setProperty(CONF_REPORT_FILE, rpt_file);
		}
		if(vm.count("set-out-dir"))
		{
			if(vm.count("set-log-dir") || vm.count("set-rpt-dir"))
			{
				cout << op_desc;
				exit(1);
			}
			cout << "Manually setting output dir: " <<  vm["set-out-dir"].as<string>() << endl;
			string out_dir = vm["set-out-dir"].as<string>();

			string log_file = conf->getString(CONF_LOG_FILE);
			log_file = out_dir + log_file;
			cout << "Log file is now: " <<  log_file << endl;
			conf->setProperty(CONF_LOG_FILE, log_file);

			string rpt_file = conf->getString(CONF_REPORT_FILE);
			rpt_file =out_dir + rpt_file;
			cout << "Report file is now: " <<  rpt_file << endl;
			conf->setProperty(CONF_REPORT_FILE, rpt_file);
		}

		// it could be possible that the log file was changed by an overwrite/set parameter.
		conf->initializeLogging();
	} 
	else 
	{
		cout << op_desc;
		exit(1);
	}

	isptap_main(conf);

	cend = clock();
	cdiff = (cend-cstart) / 1000000.0;

	LOG_INFO(logger, "Estimated ISPTAP execution time: " << setprecision(3) << cdiff << " seconds.");

	return 0;

}

void isptap_main(Configuration *conf)
{
	if(!conf->getBool(CONF_DISP_INSTRUMENTATION))
	{
		if(conf->getInt(CONF_NUMBER_OF_CORES) == 1)
		{
			TimingAnalysis ta(conf, 0);

			if(ta.start())
			{
				LOG_INFO(logger, "Calculated estimate is " << ta.getEstimate() <<  " solution type: " << ta.getSolutionType());
			}
			else
			{
				LOG_ERROR(logger, "Could not calculate estimate. Something went wrong.");
			}

		}
		else
		{
			// tbd.
		}
	}
	else
	{
		// Instrumentator ins(conf);
		// ins.start();
		assert(false);
	}
}

void printVersion(void)
{
	cout << "ISPTAP version " << ISPTAP_VERSION << endl;
	cout << COPYRIGHT << endl;
}
