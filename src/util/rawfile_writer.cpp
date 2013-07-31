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
#include "rawfile_writer.hpp"
#include "dlp_factory.hpp"
#include "isa_factory.hpp"

LoggerPtr RawFileWriter::logger(Logger::getLogger("RawFileWriter"));

bool sort_opcode_by_address(op_raw_t a, op_raw_t b) {if(a.address < b.address){return true;}else{return false;}}


RawFileWriter::RawFileWriter()
{
	conf = Configuration::getInstance();
	dlp = DLPFactory::getInstance()->getDLPObject();
	isa = ISAFactory::getInstance()->getISAObject();
}

RawFileWriter::~RawFileWriter()
{
	closeFiles();

	code_section.clear();
	data_section.clear();
}

void RawFileWriter::addCfgs(vector<ControlFlowGraph> cfgs)
{
	for(uint32_t i = 0; i < cfgs.size(); i++)
		addCfg(cfgs[i]);
}

void RawFileWriter::addCfg(ControlFlowGraph cfg)
{
	cfgVertexIter vp;
	CFGVertex v;
	op_raw_t tmp;

	// get node property structures
	property_map<ControlFlowGraph, nodetype_t>::type nodeTypeNProp = get(nodetype_t(), cfg);
//	property_map<ControlFlowGraph, bbsize_t>::type sizeNProp = get(bbsize_t(), cfg); // unused
//	property_map<ControlFlowGraph, startaddr_t>::type startAddrNProp = get(startaddr_t(), cfg); // unused
	property_map<ControlFlowGraph, bbcode_t>::type codeNProp = get(bbcode_t(), cfg);

	for(vp = vertices(cfg); vp.first != vp.second; ++vp.first)
	{
		v = *vp.first;

		if(get(nodeTypeNProp, v) == BasicBlock)
		{

			string bb_dump =  get(codeNProp, v);
			vector<string> instrs;

			split(instrs, bb_dump, boost::is_any_of("\r"));

			for(uint32_t i = 0 ; i < instrs.size(); i++)
			{
//				LOG_DEBUG(logger, "Token: " << instrs[i]);
				if(instrs[i] != "")
				{
					if(dlp->isCodeLine(instrs[i]))
					{
						tmp.address = dlp->getAddrFromCodeLine(instrs[i]);
						tmp.instr_raw = dlp->getInstructionFromCodeLine(instrs[i]);
						tmp.size = isa->getInstructionLength(tmp.instr_raw);
						tmp.instr_dump = instrs[i];
						code_section.push_back(tmp);
//						LOG_DEBUG(logger, "addr: 0x" << hex << tmp.address << " opcode: " << tmp.instr_raw << " length: " << tmp.size);
					}
					else if(dlp->isLabelLine(instrs[i]))
					{
						tmp.address = dlp->getAddrAndLabelFromLabelLine(instrs[i]).address;
						tmp.instr_raw = "00";
						tmp.size = 0;
						tmp.instr_dump = instrs[i];
						code_section.push_back(tmp);
					}
				}
			}
		}

	}

	code_section.sort(sort_opcode_by_address);

}


void RawFileWriter::addUnparsedDumpCode(vector<string> dump)
{

	op_raw_t tmp;
	for(uint32_t line=0; line<dump.size(); line++)
	{
		if(dlp->isCodeLine(dump[line]))
		{
			tmp.address = dlp->getAddrFromCodeLine(dump[line]);
			tmp.instr_raw = dlp->getInstructionFromCodeLine(dump[line]);
			tmp.size = isa->getInstructionLength(tmp.instr_raw);
			tmp.instr_dump = dump[line];
			code_section.push_back(tmp);
//			LOG_DEBUG(logger, "Unparsed: addr: 0x" << hex << tmp.address << " opcode: " << tmp.instr_raw << " length: " << tmp.size);
		}
		else if(dlp->isLabelLine(dump[line]))
		{
			tmp.address = dlp->getAddrAndLabelFromLabelLine(dump[line]).address;
			tmp.instr_raw = "00";
			tmp.size = 0;
			tmp.instr_dump = dump[line];
			code_section.push_back(tmp);
		}
	}

	code_section.sort(sort_opcode_by_address);

}

void RawFileWriter::addDataSection(vector<string> data)
{
	op_raw_t tmp;
	for(uint32_t line=0; line<data.size(); line++)
	{
		if(dlp->isCodeLine(data[line]))
		{
			tmp.address = dlp->getAddrFromCodeLine(data[line]);
			tmp.instr_raw = dlp->getInstructionFromCodeLine(data[line]);
			tmp.size = isa->getInstructionLength(tmp.instr_raw);
			tmp.instr_dump = data[line];
			data_section.push_back(tmp);
//			LOG_DEBUG(logger, "Unparsed: addr: 0x" << hex << tmp.address << " opcode: " << tmp.instr_raw << " length: " << tmp.size);
		}
		else if(dlp->isLabelLine(data[line]))
		{
			tmp.address = dlp->getAddrAndLabelFromLabelLine(data[line]).address;
			tmp.instr_raw = "00";
			tmp.size = 0;
			tmp.instr_dump = data[line];
			data_section.push_back(tmp);
		}
		else
		{
			tmp.address = UNKNOWN_ADDR;
			tmp.instr_raw = "00";
			tmp.size = 0;
			tmp.instr_dump = data[line];
			data_section.push_back(tmp);
		}
	}

}


void RawFileWriter::printCode(void)
{
	list<op_raw_t>::iterator it;
	if(logger->isDebugEnabled())
	{
		for (it=code_section.begin(); it!=code_section.end(); ++it)
		{
			LOG_DEBUG(logger, it->instr_dump);
		}
	}
}


void RawFileWriter::generateRaw(void)
{
	code_section.sort(sort_opcode_by_address);

//	printCode();

	openFiles();


	list<op_raw_t>::iterator code_it;
	code_it=code_section.begin();
	uint32_t next_addr = code_it->address + code_it->size;

	writeInstructionAsRaw(code_it->instr_raw, code_it->size);
//	writeDumpLine(string("Disassembly of section .startup:"));
	writeDumpLine(code_it->instr_dump);
	++code_it;
	for (; code_it!=code_section.end(); ++code_it)
	{
		LOG_DEBUG(logger, code_it->instr_dump << " next_addr: 0x" << hex << next_addr << " this 0x" << code_it->address);
		if(next_addr != code_it->address)
		{
			assert(next_addr < code_it->address);
			bridgeMemoryHole(next_addr, code_it->address);
		}


		writeInstructionAsRaw(code_it->instr_raw, code_it->size);
		writeDumpLine(code_it->instr_dump);

		next_addr = code_it->address + code_it->size;
	}

	if(data_section.size() != 0)
	{
		// write also the data section

		vector<op_raw_t>::iterator data_it;
		// select the first valid code/data line (no comment line)
		for(data_it = data_section.begin(); data_it != data_section.end(); data_it++)
		{
			if(data_it->address != UNKNOWN_ADDR)
			{
				break;
			}
		}

		// check if there is an overlapping between code and data
		assert(next_addr <= data_it->address);
		bridgeMemoryHole(next_addr, data_it->address);
		next_addr = data_it->address;

		// write data section
		for(data_it = data_section.begin(); data_it != data_section.end(); data_it++)
		{
			// check for valid line
			if(data_it->address != UNKNOWN_ADDR)
			{
				LOG_DEBUG(logger, data_it->instr_dump << " next_addr: 0x" << hex << next_addr << " this 0x" << data_it->address);
				if(next_addr != data_it->address)
				{
					assert(next_addr < data_it->address);
					bridgeMemoryHole(next_addr, data_it->address);
				}
				next_addr = data_it->address + data_it->size;
				writeInstructionAsRaw(data_it->instr_raw, data_it->size);
			}

			writeDumpLine(data_it->instr_dump);
		}
	}

	closeFiles();

}

void RawFileWriter::openFiles(void)
{
	if(!dump_file.is_open())
	{
		string dump_file_name = string(conf->getString(CONF_RAW_FILE)+".dump");
		dump_file.open(dump_file_name.c_str());
		assert(dump_file.is_open());
	}


	if(!raw_file.is_open())
	{
		string raw_file_name = string(conf->getString(CONF_RAW_FILE));
		raw_file.open(raw_file_name.c_str(),ios::out | ios::binary);
		assert(raw_file.is_open());
	}
}

void RawFileWriter::writeInstructionAsRaw(string opcode, uint32_t opcode_length)
{
	if(opcode_length == 0)
	{
		// this is a label line, do not write anything
		return;
	}

	for(uint32_t i=0; i < opcode_length*2; i+=2)
	{
		char byte[3];
		byte[0] = (opcode.c_str())[i];
		byte[1] = (opcode.c_str())[i+1];
		byte[2] = '\0';
		char bin = strtoul(byte, NULL,16);
//		LOG_DEBUG(logger, "OP is: 0x" << " 0x" << byte << " 0x" << hex << bin);
		raw_file.write(&bin, 1);
	}
}

void RawFileWriter::writeDumpLine(string line)
{
	assert(dump_file.is_open());
	if(dlp->isLabelLine(line))
	{
		dump_file << "\n";
	}
	dump_file << line << "\n";
}

void RawFileWriter::closeFiles(void)
{
	if(dump_file.is_open())
	{
		dump_file.close();
	}

	if(raw_file.is_open())
	{
		raw_file.close();
	}
}

void RawFileWriter::bridgeMemoryHole(uint32_t start_addr, uint32_t end_addr)
{
	if(start_addr == end_addr)
		return;

	assert(start_addr < end_addr);

	for(uint32_t i = 0; i < end_addr-start_addr; i+=2)
	{
		writeInstructionAsRaw(string("0000"), 2);
		stringstream s;
		uint32_t addr = start_addr + i;
		s << hex << addr << ":\t00 00     \tinserted nop to bridge memory hole from 0x" << start_addr << " to 0x" << end_addr;
		writeDumpLine(s.str());
	}
}
