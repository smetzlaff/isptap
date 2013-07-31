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
#ifndef _LPSOLVER_HPP_
#define _LPSOLVER_HPP_

#include "global.h"
#include "graph_structure.h"
#include "configuration.hpp"
#include <iostream>
#include <fstream>
#include <boost/regex.hpp>
#include <errno.h>

struct lp_result_set {
	string variable;
	uint32_t value;
};

enum lp_solution_t { OptimalSolution=0, SuboptimalSolution, ErrorWhileSolving, ProblemUnbound, SolutionNotCalculated};

class LpSolver {
	public:
		LpSolver(string ilp_formulation, string parameters);
		virtual ~LpSolver();
		vector<lp_result_set> lpSolve(void);
		lp_solution_t getSolutionType(void);
		uint32_t getObjectiveFunctionValue(void);
	private:
		void runLpSolve(void);
		bool parseLpSolveOutput(void);
		bool isVarAssignmentLine(string line);
		bool isObjectiveFunctionValueLine(string line);
		uint32_t parseObjFuncVal(string obj_func_value);

		string ilp_file;
		string lp_solve_parameters;
		vector<lp_result_set> lp_result;
		uint32_t lp_objective_function_value;
		lp_solution_t lp_solution_type;

		// regular expression to find in the lp_solve output the assignments of the flow variables
		static regex re_varass;
		// regular expression to identify the objective function value
		static regex re_objfuncval;
		// regular expression to identify an unbound integer value
		static regex re_int_val_with_e;

		Configuration *conf;

		static LoggerPtr logger;
};

#endif
