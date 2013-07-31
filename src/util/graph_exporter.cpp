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
#include "graph_exporter.hpp"

LoggerPtr GraphExporter::logger(Logger::getLogger("GraphExporter"));

GraphExporter *GraphExporter::singleton = NULL;

GraphExporter* GraphExporter::getInstance(void)
{
	if(singleton == NULL)
	{
		singleton = new GraphExporter;
	}
	return singleton;
}

GraphExporter::GraphExporter()
{
	conf = Configuration::getInstance();
}

GraphExporter::~GraphExporter()
{
}

void GraphExporter::exportGraphsToFile(string name, FunctionCallGraph fcg)
{
	switch((export_graph_format_t)conf->getUint(CONF_EXPORT_FORMAT))
	{
		case GRAPHML:
			{
				string exportfile_graphml("isptap_");
				exportfile_graphml += name;
				exportfile_graphml += ".graphml";

				GraphMLExport graphml_export(exportfile_graphml);

				if(graphml_export.exportGraph(fcg))
				{
					LOG_DEBUG(logger, "Exported " << name << "FCG to " << exportfile_graphml);
				}
				else
				{
					LOG_WARN(logger, "Export of " << name << "FCG to " << exportfile_graphml << " failed.");
				}
				break;
			}
		case GRAPHVIZ:
			{


				string exportfile_graphviz("isptap_");
				exportfile_graphviz += name;
				exportfile_graphviz += ".graphviz";

				GraphVizExport graphviz_export(exportfile_graphviz);

				if(graphviz_export.exportGraph(fcg))
				{
					LOG_DEBUG(logger, "Exported " << name << "FCG to " << exportfile_graphviz);
				}
				else
				{
					LOG_WARN(logger, "Export of " << name << "FCG to " << exportfile_graphviz << " failed.");
				}
				break;
			}
		default:
			LOG_ERROR(logger, "No valid export format defined");
	}
}

void GraphExporter::exportGraphsToFile(string name, ControlFlowGraph cfg)
{
	switch((export_graph_format_t)conf->getUint(CONF_EXPORT_FORMAT))
	{
		case GRAPHML:
			{
				string exportfile_graphml("isptap_");
				exportfile_graphml += name;
				exportfile_graphml += ".graphml";

				GraphMLExport graphml_export(exportfile_graphml);

				if(graphml_export.exportGraph(cfg))
				{
					LOG_DEBUG(logger, "Exported " << name << " CFG to " << exportfile_graphml);
				}
				else
				{
					LOG_WARN(logger, "Export of " << name << " CFG to " << exportfile_graphml << " failed.");
				}
				break;
			}

		case GRAPHVIZ:
			{
				string exportfile_graphviz("isptap_");
				exportfile_graphviz += name;
				exportfile_graphviz += ".graphviz";

				GraphVizExport graphviz_export(exportfile_graphviz);

				if(graphviz_export.exportGraph(cfg, startaddrstring_t(), edgename_t()))
				{
					LOG_DEBUG(logger, "Exported " << name << " CFG to " << exportfile_graphviz);
				}
				else
				{
					LOG_WARN(logger, "Export of " << name << " CFG to " << exportfile_graphviz << " failed.");
				}
				break;
			}
		default:
			LOG_ERROR(logger, "No valid export format defined");
	}

}

void GraphExporter::exportGraphsToFile(string name, MemoryStateGraph msg)
{
	Configuration *conf = Configuration::getInstance();
	switch((export_graph_format_t)conf->getUint(CONF_EXPORT_FORMAT))
	{
		case GRAPHML:
			{
				string exportfile_graphml("isptap_");
				exportfile_graphml += name;
				exportfile_graphml += ".graphml";

				GraphMLExport graphml_export(exportfile_graphml);

				if(graphml_export.exportGraph(msg))
				{
					LOG_DEBUG(logger, "Exported " << name << " MSG to " << exportfile_graphml);
				}
				else
				{
					LOG_WARN(logger, "Export of " << name << " MSG to " << exportfile_graphml << " failed.");
				}
				break;
			}
		case GRAPHVIZ:
			{
				string exportfile_graphviz("isptap_");
				exportfile_graphviz += name;
				exportfile_graphviz += ".graphviz";

				GraphVizExport graphviz_export(exportfile_graphviz);
				if(graphviz_export.exportGraph(msg))
				{
					LOG_DEBUG(logger, "Exported " << name << " MSG to " << exportfile_graphviz);
				}
				else
				{
					LOG_WARN(logger, "Export of " << name << " MSG to " << exportfile_graphviz << " failed.");
				}
				break;
			}
		default:
			LOG_ERROR(logger, "No valid export format defined");
	}
}
