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
#ifndef _CARCORE_DUMP_PARSER_HPP_
#define _CARCORE_DUMP_PARSER_HPP_

#include "dump_parser.hpp"


#define STARTUP_CODE_BEGIN_LABEL "_start"

class CarCoreDumpParser : public DumpParser {
	public:
		CarCoreDumpParser(uint32_t core_number);
		virtual ~CarCoreDumpParser();

	protected:
		void parseLabels(void);

		/**
		 * \brief The dump file contains some startup code (from STARTUP_BEGIN_LABEL to a final debug instruction) that has to be ignored mainly.
		 */
		ControlFlowGraphObject *parseCfgForStartLabel(addr_label_t function);
};
#endif
