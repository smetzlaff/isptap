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
#ifndef _RESULT_CHECKER_HPP_
#define _RESULT_CHECKER_HPP_

#include "global.h"
#include "constants.h"

#include "graph/ilpgenerator.hpp"

#include "memory/sisp_optimizer_if.hpp"
#include "memory/bbsisp_jp_optimizer.hpp"
#include "memory/bbsisp_jp_optimizer_wcp.hpp"

class ResultChecker {
	public:
		static ResultChecker* getInstance(void);

		/*!
		 * \brief Compares the result that was calculated by isptap with a given result from the config file.
		 * This function is used to be aware of any changes that occur due to source code changes.
		 * \param calculated_value The value that was calculated by isptap run.
		 * \param property_to_compare The measure that is compared.
		 */
		void check_result(uint32_t calculated_value, string property_to_compare);
		/*!
		 * \brief Compares the result that was calculated by isptap with a given result from the config file.
		 * This function is used to be aware of any changes that occur due to source code changes.
		 * \param calculated_value The value that was calculated by isptap run.
		 * \param property_to_compare The measure that is compared.
		 * \param mem_size The size of the memory if memory stepping is used.
		 */
		void check_result(uint32_t calculated_value, string property_to_compare, uint32_t mem_size);
		void check_sisp_assignment(mem_type_t mtype, string mtype_s, SISPOptimizer_IF *sisp, ILPGenerator* ilpg, bool disable_asserts);
		void check_sisp_assignment(mem_type_t mtype, string mtype_s, sisp_result_t *sisp_result, ILPGenerator* ilpg, bool disable_asserts);

	private:
		ResultChecker();
		virtual ~ResultChecker();
		ResultChecker(const ResultChecker&);                 // Prevent copy-construction
		ResultChecker& operator=(const ResultChecker&);      // Prevent assignment
	
		static ResultChecker *singleton;
		static LoggerPtr logger;
		Configuration *conf;
};

#endif
