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
#include "cost_exporter.hpp"

LoggerPtr CostExporter::logger(Logger::getLogger("CostExporter"));

CostExporter *CostExporter::singleton = NULL;

CostExporter* CostExporter::getInstance(void)
{
	if(singleton == NULL)
	{
		singleton = new CostExporter;
	}
	return singleton;
}

CostExporter::CostExporter()
{
}

CostExporter::~CostExporter()
{
}

void CostExporter::exportCostToFile(ControlFlowGraph cfg, string file_extension)
{
	BBCostExport bbce(cfg);
	bbce.exportCostToFile(file_extension);
}
