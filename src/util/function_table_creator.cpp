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
#include "function_table_creator.hpp"


LoggerPtr FunctionTableCreator::logger(Logger::getLogger("FunctionTableCreator"));

bool sort_funcs_by_addr_uint32(uint32_t a, uint32_t b) {return (a < b);}
bool sort_funcs_by_addr_struct(addr_name_size_t a, addr_name_size_t b) {return (a.address < b.address);}
#define sort_by_addr(x) sort(x.begin(), x.end(), sort_funcs_by_addr_uint32)
#define sort_by_addr2(x) sort(x.begin(), x.end(), sort_funcs_by_addr_struct)

FunctionTableCreator::FunctionTableCreator(string file_name, bool old_format)
{
	function_table_file_name  = file_name;
	old_ftab_format = old_format;
}

FunctionTableCreator::~FunctionTableCreator()
{

}

void FunctionTableCreator::createFunctionTable(FunctionCallGraphObject *fcgo)
{
	ofstream ftab_file;
	ftab_file.open(function_table_file_name.c_str());
	assert(ftab_file.is_open());


	vector<uint32_t> functions = fcgo->getFunctions();


	if(functions.size() == 0)
	{
		LOG_WARN(logger, "Sorry no functions to create a function table.");
		return;
	}

	sort_by_addr(functions);

	for(uint32_t i = 0; i < functions.size(); i++)
	{
		if(old_ftab_format)
		{
			ftab_file << hex << functions[i] << " <" << fcgo->getFunctionName(functions[i]) << ">:" << endl;
		}
		else
		{
			ftab_file << "0x" << hex << functions[i] << "\t" << fcgo->getFunctionName(functions[i]) << "\t" << dec << fcgo->getFunctionSize(functions[i]) << endl;
		}
	}
	if(old_ftab_format)
	{
		ftab_file << hex << (functions.back() + fcgo->getFunctionSize(functions.back())) << " <END_TAG>:" << endl;
	}
	else
	{
		ftab_file << "0x" << hex << (functions.back() + fcgo->getFunctionSize(functions.back())) << "\tEND_TAG\t0" << endl;
	}

	ftab_file.close();

}

void FunctionTableCreator::createFunctionTable(vector<addr_name_size_t> functionMetaData)
{
	ofstream ftab_file;
	ftab_file.open(function_table_file_name.c_str());
	assert(ftab_file.is_open());


	vector<addr_name_size_t> functions = functionMetaData;


	if(functions.size() == 0)
	{
		LOG_WARN(logger, "Sorry no functions to create a function table.");
		return;
	}

	sort_by_addr2(functions);

	for(uint32_t i = 0; i < functions.size(); i++)
	{
		if(old_ftab_format)
		{
			ftab_file << hex << functions[i].address << " <" << functions[i].name << ">:" << endl;
		}
		else
		{
			ftab_file << "0x" << hex << functions[i].address << "\t" << functions[i].name << "\t" << dec << functions[i].size << endl;
		}
	}
	if(old_ftab_format)
	{
		ftab_file << hex << (functions.back().address + functions.back().size) << " <END_TAG>:" << endl;
	}
	else
	{
		ftab_file << "0x" << hex << (functions.back().address + functions.back().size) << "\tEND_TAG\t0" << endl;
	}

	ftab_file.close();

}
