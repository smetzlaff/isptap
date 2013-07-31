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
#ifndef _FUNCTION_TABLE_CREATOR_HPP_
#define _FUNCTION_TABLE_CREATOR_HPP_

#include "global.h"
#include "constants.h"
#include "fcgobj.hpp"
#include <iostream>
#include <fstream>

class FunctionTableCreator {
	public:
		FunctionTableCreator(string file_name, bool old_format);
		virtual ~FunctionTableCreator();
		void createFunctionTable(FunctionCallGraphObject *fcgo);
		void createFunctionTable(vector<addr_name_size_t> functionMetaData);
	private:
		string function_table_file_name;
		bool old_ftab_format;

		static LoggerPtr logger;
};

#endif

