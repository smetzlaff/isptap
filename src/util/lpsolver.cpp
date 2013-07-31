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
#include "lpsolver.hpp"

#define UNBOUND_NUMBER numeric_limits<uint32_t>::max()

LoggerPtr LpSolver::logger(Logger::getLogger("LpSolver"));

// regular expression to find in the lp_solve output the assignments of the flow variables. Notice the last part distinguished between a normal number or a number in exponential writing (e.g. 1.200e+03)
regex LpSolver::re_varass("^[a-z]+[0-9]*[[:space:]]+(([0-9]+)|([0-9]+(\\.)?[0-9]*e[\\+\\-][0-9]+))$");

// regular expression to identify the objective function value
regex LpSolver::re_objfuncval("Value of objective function: [0-9]+");

// regular expression to identify an unbound integer value
regex LpSolver::re_int_val_with_e("[0-9]+(\\.)?[0-9]*e[\\+\\-][0-9]+");


bool sort_string_by_variable(string a, string b){return (a.compare(b) <= 0);}
#define sort_by_variable(x) sort(x.begin(), x.end(), sort_string_by_variable)

LpSolver::LpSolver(string ilp_formulation, string parameters) : lp_solution_type(SolutionNotCalculated)
{
	this->lp_solve_parameters = parameters;

	conf = Configuration::getInstance();

	char c[] = "isptap_lpsolve_XXXXXX";
	if(mkstemp(c) == -1)
	{
		assert(false);
	}
	stringstream s;
	s << c;
	ilp_file = s.str();

	// write ilp to temp file
	ofstream ilpfile;
	ilpfile.open(ilp_file.c_str());
	assert(ilpfile.is_open());
	assert(!ilp_formulation.empty());
	ilpfile << ilp_formulation;
	ilpfile.close();

	LOG_DEBUG(logger, "Created tempfile: " << ilp_file);
}

LpSolver::~LpSolver()
{
	lp_result.clear();

	if(conf->getBool(CONF_LP_SOLVE_DONT_DELETE_TEMPFILES))
	{
		LOG_INFO(logger, "KEEPING tempfiles: " << ilp_file << ", " << ilp_file << ".out" << " and " << ilp_file << ".err");
	}
	else
	{
		// delete temp file
		int removed_ilp_file = remove(ilp_file.c_str());
		assert(removed_ilp_file == 0);
		string tmp = ilp_file + ".out";
		removed_ilp_file = remove(tmp.c_str());
		assert(removed_ilp_file == 0);
		tmp = ilp_file + ".err";
		removed_ilp_file = remove(tmp.c_str());
		assert(removed_ilp_file == 0);

		LOG_DEBUG(logger, "Deleted tempfiles: " << ilp_file << ", " << ilp_file << ".out" << " and " << ilp_file << ".err");
	}

}

vector<lp_result_set> LpSolver::lpSolve(void)
{

	runLpSolve();

	if(parseLpSolveOutput())
	{
	}
	else
	{
		LOG_ERROR(logger, "Problem is unbounded.");
	}

	return lp_result;
}

void LpSolver::runLpSolve(void)
{
	ostringstream command;
	command << "lp_solve " << lp_solve_parameters << " " << ilp_file << " > "<< ilp_file << ".out 2> " << ilp_file << ".err";
	LOG_DEBUG(logger, "Executing lp_solve with command line: " << command.str());
	int rc = system(command.str().c_str());
	if(rc != 0)
	{
		LOG_WARN(logger, "Something went wrong, when trying to solve ilp, rc = " << rc);
		if(rc == -1)
		{
			LOG_ERROR(logger, "system() failed errno: " << errno);
		}

	}
}

bool LpSolver::parseLpSolveOutput(void)
{

	// TODO handle potential error output
	string ilp_file_err;
	ilp_file_err = ilp_file;
	ilp_file_err += ".err";

	ifstream lpsolve_result;

	string ilp_file_out;
	ilp_file_out = ilp_file;
	ilp_file_out += ".out";


	// set to no solution type;
	lp_solution_type = SolutionNotCalculated;

	lpsolve_result.open(ilp_file_out.c_str());

	if(lpsolve_result.fail())
	{
		LOG_ERROR(logger, "lp_solve outfile " << ilp_file_out << " cannot be opened");
		return false;
	}

	assert(lpsolve_result.is_open());


	// read the whole solution file to sort the results
	// this is done to minimize the diff in the log file of different analysis runs
	vector<string> tmp_vec;
	while(!lpsolve_result.eof())
	{
		string str;
		getline(lpsolve_result, str);
		if(str != "")
		{
			tmp_vec.push_back(str);
		}
	}
	sort_by_variable(tmp_vec);
	vector<string>::iterator it;

	lpsolve_result.close();

	// start parsing the output
	// TODO: Improve the parsing such that all variables after \"Actual values of the variables:\" are correctly processed.
	for(it = tmp_vec.begin(); it != tmp_vec.end(); it++)
	{
		string str = *it;

		if(str.compare("This problem is unbounded") == 0)
		{
			LOG_INFO(logger, "The generated ILP problem is unbounded!");
			// something went wrong, set solution type to error
			lp_solution_type = ProblemUnbound;
			return false;
		}

		if(regex_search(str, regex("Suboptimal solution")))
		{
			LOG_WARN(logger, "lp_solve was aborted and found only a suboptimal solution");
			// only a suboptimal solution was found
			assert(lp_solution_type == SolutionNotCalculated);
			lp_solution_type = SuboptimalSolution;
		}

		if(isVarAssignmentLine(str))
		{
			lp_result_set tmp;
			boost::regex re("[[:space:]]+");
			boost::sregex_token_iterator line_iterator(str.begin(), str.end(), re, -1);

			tmp.variable = *line_iterator;
			line_iterator++;
			string ilp_value = *line_iterator;

			tmp.value = parseObjFuncVal(ilp_value);

			if(tmp.value != UNBOUND_NUMBER)
			{
				LOG_DEBUG(logger, "Found for variable: " << tmp.variable << " the value: " << tmp.value);
			}
			else
			{
				LOG_WARN(logger, "Variable is unbound: " << tmp.variable << " has value: " << ilp_value << " setting it to: " << UNBOUND_NUMBER);
				assert((lp_solution_type == SolutionNotCalculated) || (lp_solution_type == ProblemUnbound));
				lp_solution_type = ProblemUnbound;
			}
			lp_result.push_back(tmp);

		}
		else if(isObjectiveFunctionValueLine(str))
		{
			uint32_t length_of_prefix = 29; // see re_objfuncval
			string obj_func_value = str.substr(length_of_prefix, str.size()-length_of_prefix);
			lp_objective_function_value = parseObjFuncVal(obj_func_value);

			if(lp_objective_function_value != UNBOUND_NUMBER)
			{
				LOG_DEBUG(logger, "Setted the value of the objective function: " << lp_objective_function_value);
			}
			else
			{
				LOG_ERROR(logger, "Objective function is unbound: " << obj_func_value << " assigning: " << UNBOUND_NUMBER);
				assert((lp_solution_type == SolutionNotCalculated) || (lp_solution_type == ProblemUnbound));
				lp_solution_type = ProblemUnbound;
			}
		}
	}

	if(lp_solution_type == SolutionNotCalculated)
	{
		// if the solution type was not changed
		lp_solution_type = OptimalSolution;
	}

	ifstream lpsolve_out;

	lpsolve_out.open(ilp_file_err.c_str());

	if(lpsolve_out.fail())
	{
		LOG_ERROR(logger, "lp_solve error outfile " << lpsolve_out << " cannot be opened");
		// something went wrong, set solution type to error
		lp_solution_type = ErrorWhileSolving;
		return false;
	}

	assert(lpsolve_out.is_open());

	stringstream s;
	while(!lpsolve_out.eof())
	{
		string str;
		getline(lpsolve_out, str);
		s << str << endl;
	}
	LOG_INFO(logger, "Output of lp_solve error file: \n" << s.str());
	return true;
}

bool LpSolver::isVarAssignmentLine(string line)
{
	return (regex_search(line, re_varass));
}

bool LpSolver::isObjectiveFunctionValueLine(string line)
{
	return regex_search(line, re_objfuncval);
}

uint32_t LpSolver::parseObjFuncVal(string obj_func_value)
{
	if(!regex_search(obj_func_value, re_int_val_with_e))
	{
		return strtoul(obj_func_value.c_str(), NULL, 10);
	}
	else
	{
		double value = strtod(obj_func_value.c_str(), NULL);
		uint64_t uint64_value = (uint64_t)value;
		uint32_t uint32_value = UNBOUND_NUMBER;
		// XXX The comparison with the uint64_t is ugly:
		if((uint64_value > uint32_value) || ((double)(UNBOUND_NUMBER) < value))
		{
			LOG_ERROR(logger, "Variable assignment to " << value << " is to large to fit 32bit value. Setting it to " << UNBOUND_NUMBER);
			return UNBOUND_NUMBER;
		}
		else
		{
			return (uint32_t)uint64_value;
		}
	}

	return UNBOUND_NUMBER;
}

uint32_t LpSolver::getObjectiveFunctionValue(void)
{
	return lp_objective_function_value;
}

lp_solution_t LpSolver::getSolutionType(void)
{
	return lp_solution_type;
}

