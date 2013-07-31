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
#include "tricore_dumpline_parser.hpp"
#include "regex_parser_tokens.h"

using namespace boost;

LoggerPtr TriCoreDumpLineParser::logger(Logger::getLogger("TriCoreTriCoreDumpLineParser"));

TriCoreDumpLineParser *TriCoreDumpLineParser::singleton = NULL;

TriCoreDumpLineParser* TriCoreDumpLineParser::getInstance(void)
{
	if(singleton == NULL)
	{
		singleton = new TriCoreDumpLineParser;
	}
	return singleton;
}

TriCoreDumpLineParser::TriCoreDumpLineParser()
{
	initializeRegExs();
	testInstrClassification();
}

TriCoreDumpLineParser::~TriCoreDumpLineParser()
{
}

bool TriCoreDumpLineParser::isBranchInstr(string str)
{
	return regex_search(str,re_cf_change);
}

bool TriCoreDumpLineParser::isReturnInstr(string str)
{
	return regex_search(str,re_cf_change_return);
}

bool TriCoreDumpLineParser::isDebugInstr(string str)
{
	return regex_search(str,re_cf_change_debug);
}

bool TriCoreDumpLineParser::isCallInstr(string str)
{
	return regex_search(str,re_cf_change_call);
}

bool TriCoreDumpLineParser::isCondBranchInstr(string str)
{
//	if(isBranchInstr(str))
//	{
		return !(regex_search(str,re_cf_change_uncond)||regex_search(str,re_cf_change_call)||regex_search(str,re_cf_change_return));
//	}
//	return false;
}

bool TriCoreDumpLineParser::isIndirectBranchInstr(string str)
{
	return regex_search(str,re_cf_change_indirect);
}

bool TriCoreDumpLineParser::isIndirectCallInstr(string str)
{
	return regex_search(str,re_cf_change_call_indirect);
}

string TriCoreDumpLineParser::getCommentFromCodeLine(string str)
{
	vector<string> strs;
	split(strs, str, boost::is_any_of("\t"));

	return strs.back();
}

string TriCoreDumpLineParser::assembleCodeLine(uint32_t opcode_address, string opcode_raw, string opcode_dump)
{
	stringstream result;
	char address[8+1];
	sprintf(address, "%08x", opcode_address);

	result << address << ":\t";
	if(opcode_raw.size() == 4)
	{
		result << opcode_raw[0] << opcode_raw[1] << " " << opcode_raw[2] << opcode_raw[3];
		result << "      ";
	}
	else if(opcode_raw.size() == 8)
	{
		result << opcode_raw[0] << opcode_raw[1] << " " << opcode_raw[2] << opcode_raw[3];
		result << " " << opcode_raw[4] << opcode_raw[5] << " " << opcode_raw[6] << opcode_raw[7];
	}
	else
	{
		// sorry wrong opcode length
		LOG_DEBUG(logger, "wrong opcode length: " << opcode_raw);
		assert(false);
	}

	result << "\t" << opcode_dump;

	return result.str();
}

string TriCoreDumpLineParser::updateAddressInDumpInstruction(string opcode_dump, uint32_t address)
{
	char newAddr[8+1];
	sprintf(newAddr, "%08x", address);
	string result;
	result = regex_replace(opcode_dump, regex(HEX_TOKEN"{8}"), string(newAddr), match_default | format_first_only);
	return result;
}

instr_type_t TriCoreDumpLineParser::getInstructionType(string str)
{
	assert(isCodeLine(str));

	if(regex_search(str, re_cf_change_call))
	{
		if(regex_search(str, re_cf_change_call_indirect))
		{
			return I_INDIRECT_CALL;
		}
		else
		{
			return I_CALL;
		}
	}
	else if(regex_search(str, re_cf_change_return))
	{
		return I_RETURN;
	}
	else if(regex_search(str, re_cf_change_uncond))
	{
		if(regex_search(str, re_cf_change_indirect))
		{
			return I_UNCOND_INDIRECT_BRANCH;
		}
		else
		{
			return I_UNCOND_BRANCH;
		}
	}
	else if(regex_search(str, re_cf_change))
	{
		return I_COND_BRANCH;
	}
	else if(regex_search(str, re_store))
	{
		return I_STORE;
	}
	else if(regex_search(str, re_load))
	{
		return I_LOAD;
	}
	else if(regex_search(str, re_sync))
	{
		return I_SYNC;
	}
	else if(regex_search(str, re_cf_change_debug))
	{
		return I_DEBUG;
	}
	else if(regex_search(str, re_others) || regex_search(str, re_core_reg_instrs))
	{
		return I_OTHERS;
	}
	else
	{
		return I_ARITHMETIC;
	}

	return I_UNKNOWN;
}

void TriCoreDumpLineParser::testInstrClassification(void)
{

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("abs")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("abs.b")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("abs.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("absdif")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("absdif.b")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("absdif.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("absdifs")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("absdifs.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("abss")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("abss.h")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("add")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("add.a")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("add.b")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("add.b")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("add.g")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("add.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("addc")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("addi")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("addih")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("addih.a")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("adds")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("adds.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("adds.hu")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("adds.u")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("addsc.a")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("addsc.at")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("addx")));
	
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("and")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("and.and.t")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("and.andn.t")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("and.eq")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("and.ge")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("and.ge.u")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("and.lt")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("and.lt.u")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("and.ne")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("and.nor.t")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("and.or.t")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("and.t")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("andn")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("andn.t")));

	assert(I_OTHERS == getInstructionType(createTestInstrLine("bisr")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("bmerge")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("bsplit")));

	assert(I_OTHERS == getInstructionType(createTestInstrLine("cachea.i")));
	assert(I_OTHERS == getInstructionType(createTestInstrLine("cachea.w")));
	assert(I_OTHERS == getInstructionType(createTestInstrLine("cachea.wi")));
	assert(I_OTHERS == getInstructionType(createTestInstrLine("cachei.w")));
	assert(I_OTHERS == getInstructionType(createTestInstrLine("cachei.wi")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("cadd")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("caddn")));

	assert(I_CALL == getInstructionType(createTestInstrLine("call")));
	assert(I_CALL == getInstructionType(createTestInstrLine("calla")));
	assert(I_INDIRECT_CALL == getInstructionType(createTestInstrLine("calli")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("clo")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("clo.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("cls")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("cls.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("clz")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("clz.h")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("cmov")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("cmovn")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("csub")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("csubn")));

	assert(I_DEBUG == getInstructionType(createTestInstrLine("debug")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("dextr")));

	assert(I_OTHERS == getInstructionType(createTestInstrLine("disable")));
	
	assert(I_OTHERS == getInstructionType(createTestInstrLine("dsync")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("dvadj")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("dvinit")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("dvinit.u")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("dvinit.b")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("dvinit.bu")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("dvinit.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("dvinit.hu")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("dvstep")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("dvstep.u")));

	assert(I_OTHERS == getInstructionType(createTestInstrLine("enable")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("eq")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("eq.a")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("eq.b")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("eq.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("eq.w")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("eqany.b")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("eqany.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("eqz.a")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("extr")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("extr.u")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("ge")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("ge.u")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("ge.a")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("imask")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("ins.t")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("insn.t")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("insert")));

	assert(I_OTHERS == getInstructionType(createTestInstrLine("isync")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("ixmax")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("ixmax.u")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("ixmin")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("ixmin.u")));

	assert(I_UNCOND_BRANCH == getInstructionType(createTestInstrLine("j")));
	assert(I_UNCOND_BRANCH == getInstructionType(createTestInstrLine("ja")));
	assert(I_UNCOND_INDIRECT_BRANCH == getInstructionType(createTestInstrLine("ji")));
	assert(I_UNCOND_BRANCH == getInstructionType(createTestInstrLine("jl")));
	assert(I_UNCOND_BRANCH == getInstructionType(createTestInstrLine("jla")));
	assert(I_UNCOND_INDIRECT_BRANCH == getInstructionType(createTestInstrLine("jli")));

	assert(I_COND_BRANCH == getInstructionType(createTestInstrLine("jeq")));
	assert(I_COND_BRANCH == getInstructionType(createTestInstrLine("jeq.a")));
	assert(I_COND_BRANCH == getInstructionType(createTestInstrLine("jge")));
	assert(I_COND_BRANCH == getInstructionType(createTestInstrLine("jge.u")));
	assert(I_COND_BRANCH == getInstructionType(createTestInstrLine("jgez")));
	assert(I_COND_BRANCH == getInstructionType(createTestInstrLine("jgtz")));
	assert(I_COND_BRANCH == getInstructionType(createTestInstrLine("jlez")));
	assert(I_COND_BRANCH == getInstructionType(createTestInstrLine("jlt")));
	assert(I_COND_BRANCH == getInstructionType(createTestInstrLine("jlt.u")));
	assert(I_COND_BRANCH == getInstructionType(createTestInstrLine("jltz")));
	assert(I_COND_BRANCH == getInstructionType(createTestInstrLine("jne")));
	assert(I_COND_BRANCH == getInstructionType(createTestInstrLine("jne.a")));
	assert(I_COND_BRANCH == getInstructionType(createTestInstrLine("jned")));
	assert(I_COND_BRANCH == getInstructionType(createTestInstrLine("jnei")));
	assert(I_COND_BRANCH == getInstructionType(createTestInstrLine("jnz")));
	assert(I_COND_BRANCH == getInstructionType(createTestInstrLine("jnz.a")));
	assert(I_COND_BRANCH == getInstructionType(createTestInstrLine("jnz.t")));
	assert(I_COND_BRANCH == getInstructionType(createTestInstrLine("jz")));
	assert(I_COND_BRANCH == getInstructionType(createTestInstrLine("jz.a")));
	assert(I_COND_BRANCH == getInstructionType(createTestInstrLine("jz.t")));

	assert(I_LOAD == getInstructionType(createTestInstrLine("ld.a")));
	assert(I_LOAD == getInstructionType(createTestInstrLine("ld.b")));
	assert(I_LOAD == getInstructionType(createTestInstrLine("ld.bu")));
	assert(I_LOAD == getInstructionType(createTestInstrLine("ld.d")));
	assert(I_LOAD == getInstructionType(createTestInstrLine("ld.da")));
	assert(I_LOAD == getInstructionType(createTestInstrLine("ld.du"))); // this instr is not in ISA spec
	assert(I_LOAD == getInstructionType(createTestInstrLine("ld.h")));
	assert(I_LOAD == getInstructionType(createTestInstrLine("ld.hu")));
	assert(I_LOAD == getInstructionType(createTestInstrLine("ld.q")));
	assert(I_LOAD == getInstructionType(createTestInstrLine("ld.w")));
	assert(I_LOAD == getInstructionType(createTestInstrLine("ld.t"))); // this instr is not in ISA spec
	assert(I_LOAD == getInstructionType(createTestInstrLine("ldlcx")));
	assert(I_LOAD == getInstructionType(createTestInstrLine("lducx")));

	assert(I_SYNC == getInstructionType(createTestInstrLine("ldmst")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("lea")));


	assert(I_COND_BRANCH == getInstructionType(createTestInstrLine("loop")));
	assert(I_COND_BRANCH == getInstructionType(createTestInstrLine("loopu")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("lt")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("lt.u")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("lt.a")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("lt.b")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("lt.bu")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("lt.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("lt.hu")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("lt.w")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("lt.wu")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("madd")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("madds")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("madd.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("madds.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("madd.q")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("madds.q")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("madd.u")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("madds.u")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("maddm.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("maddms.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("maddr.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("maddrs.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("maddr.q")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("maddrs.q")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("maddsu.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("maddsus.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("maddsum.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("maddsums.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("maddsur.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("maddsurs.h")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("max")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("max.u")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("max.b")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("max.bu")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("max.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("max.hu")));

	assert(I_OTHERS == getInstructionType(createTestInstrLine("mfcr")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("min")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("min.u")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("min.b")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("min.bu")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("min.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("min.hu")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("mov")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("mov.a")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("mov.aa")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("mov.d")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("mov.u")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("movh")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("movh.a")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("msub")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("msubs")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("msub.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("msubs.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("msub.q")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("msubs.q")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("msub.u")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("msubs.u")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("msubad.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("msubads.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("msubadm.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("msubadms.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("msubadr.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("msubadrs.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("msubm.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("msubms.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("msubr.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("msubrs.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("msubr.q")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("msubrs.q")));

	assert(I_OTHERS == getInstructionType(createTestInstrLine("mtcr")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("mul")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("muls")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("mul.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("mul.q")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("mul.u")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("muls.u")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("mulm.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("mulr.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("mulr.q")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("nand")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("nand.t")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("ne.a")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("ne")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("nez.a")));

	// is nop arithmetic??
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("nop")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("nor")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("nor.t")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("or")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("or.and.t")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("or.andn.t")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("or.nor.t")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("or.or.t")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("or.eq")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("or.ge")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("or.ge.u")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("or.lt")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("or.lt.u")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("or.ne")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("or.t")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("orn")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("orn.t")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("pack")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("parity")));

	assert(I_RETURN == getInstructionType(createTestInstrLine("ret ")));

	assert(I_OTHERS == getInstructionType(createTestInstrLine("rfe")));
	assert(I_OTHERS == getInstructionType(createTestInstrLine("rfm")));

	assert(I_LOAD == getInstructionType(createTestInstrLine("rslcx")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("rstv")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("rsub")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("rsubs")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("rsubs.u")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sat.b")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sat.bu")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sat.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sat.hu")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sel")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("seln")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sh")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sh.eq")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sh.ge")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sh.ge.u")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sh.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sh.lt")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sh.lt.u")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sh.ne")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sh.and.t")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sh.andn.t")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sh.nand.t")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sh.nor.t")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sh.or.t")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sh.orn.t")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sh.xnor.t")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sh.xor.t")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sha")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sha.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("shas")));
	
	assert(I_STORE == getInstructionType(createTestInstrLine("st.a")));
	assert(I_STORE == getInstructionType(createTestInstrLine("st.b")));
	assert(I_STORE == getInstructionType(createTestInstrLine("st.d")));
	assert(I_STORE == getInstructionType(createTestInstrLine("st.da")));
	assert(I_STORE == getInstructionType(createTestInstrLine("st.dw"))); // this instr is not in ISA spec
	assert(I_STORE == getInstructionType(createTestInstrLine("st.h")));
	assert(I_STORE == getInstructionType(createTestInstrLine("st.q")));
	assert(I_STORE == getInstructionType(createTestInstrLine("st.t")));
	assert(I_STORE == getInstructionType(createTestInstrLine("st.w")));
	assert(I_STORE == getInstructionType(createTestInstrLine("stlcx")));
	assert(I_STORE == getInstructionType(createTestInstrLine("stucx")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sub")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sub.a")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sub.b")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sub.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("subc")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("subs")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("subs.u")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("subs.h")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("subs.hu")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("subx")));

	assert(I_STORE == getInstructionType(createTestInstrLine("svlcx")));

	assert(I_SYNC == getInstructionType(createTestInstrLine("swap.w")));

	assert(I_OTHERS == getInstructionType(createTestInstrLine("syscall")));

	assert(I_OTHERS == getInstructionType(createTestInstrLine("trapsv")));
	assert(I_OTHERS == getInstructionType(createTestInstrLine("trapv")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("unpack")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("xnor")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("xnor.t")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("xor")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("xor.eq")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("xor.ge")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("xor.ge.u")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("xor.lt")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("xor.lt.u")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("xor.ne")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("xor.t")));


	// FPU instructions
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("add.f")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("cmp.f")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("div.f")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("ftoi")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("ftoq31")));
	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("ftou")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("itof")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("madd.f")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("msub.f")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("mul.f")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("q31tof")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("qseed.f")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("sub.f")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("updfl")));

	assert(I_ARITHMETIC == getInstructionType(createTestInstrLine("utof")));


	// MMU instructions
	assert(I_OTHERS == getInstructionType(createTestInstrLine("tlbdemap")));

	assert(I_OTHERS == getInstructionType(createTestInstrLine("tlbflush.a")));
	assert(I_OTHERS == getInstructionType(createTestInstrLine("tlbflush.b")));

	assert(I_OTHERS == getInstructionType(createTestInstrLine("tlbmap")));

	assert(I_OTHERS == getInstructionType(createTestInstrLine("tlbprobe.a")));
	assert(I_OTHERS == getInstructionType(createTestInstrLine("tlbprobe.i")));
}
string TriCoreDumpLineParser::createTestInstrLine(string shortname)
{
	string pre("a0000000:	00 00 00 00	");
	string post(" ");
	return pre+shortname+post;
}

void TriCoreDumpLineParser::initializeRegExs(void)
{
	// regular expression to determining valid code lines (identified by address: "axxxxx:")
	re_addr = regex("a"HEX_TOKEN"{7}:");
	// regular expression to determine jump/call labels
	re_label = regex("a"HEX_TOKEN"{7}[[:space:]]<[^>]*>:");
	// regular expression to find control flow affecting instructions like call, return and any jump instr (identified by instruction names followed by space)
	re_cf_change = regex("[[:space:]](ret|call[i|a]?|loop[u]?|(j[^>]*))[[:space:]]");
	// regular expression to find unconditional branch instructions: j, ja, ji, jl. Calls and returns are not considered.
	re_cf_change_uncond = regex("[[:space:]](j[l]?[ai]?)[[:space:]]");
	// regular expression to find a return
	re_cf_change_return = regex("[[:space:]]ret[[:space:]]");
	// regular expression to find an indirect jump: ji, jli
	re_cf_change_indirect = regex("[[:space:]]j[l]?i[[:space:]]");
	// regular expression to find a debug
	re_cf_change_debug = regex("[[:space:]]debug[[:space:]]");
	// regular expression to find a call
	re_cf_change_call = regex("[[:space:]]call[i|a]?[[:space:]]");
	//regular expression to find an indirect call: calli
	re_cf_change_call_indirect = regex("[[:space:]]calli[[:space:]]");
	// regular expression to find store instructions: st.a, st.b, st.d, st.da, st.h, st.q, st.w, stlcx and sthcx
	re_store = regex("[[:space:]](st[.period.]([abdhqtw]|da))|(st[l|u]cx)|(svlcx)[[:space:]]");
	// regular expression to find load instructions: ld.a, ld.b, ld.bu, ld.d, ld.da, ld.h, ld.q, ld.w, ldlcx and lducx
	re_load = regex("[[:space:]](ld[.period.]([abdhqtw]|da|hu|bu))|(ld[l|u]cx)|(rslcx)[[:space:]]");
	// regular expression to find synchronization instructions: swap.w and ldmst
	re_sync = regex("[[:space:]]swap[.period.]w|ldmst[[:space:]]");
	// regular expression to find other uncategorized instructions
	re_others = regex("[[:space:]](cache[a|i][.period.](i|w|wi))|(bisr)|(disable)|(enable)|([d|i]sync)|(rf[e|m])|(syscall)|(trap[s]?v)|(tlb((demap)|(flush[.period.][a|b])|(map)|(probe[.period.][a|i])))[[:space:]]");
	// regular expression to find core register instructions
	re_core_reg_instrs = regex("[[:space:]]m[f|t]cr[[:space:]]");
	// regular expression to find the 16 or 32 bit opcode in dump file
	re_opcode = regex("[[:space:]](["HEX_TOKEN"{2}[[:space:]])+");
	// regular expression to find memory holes inserted by compiler
	re_memory_hole = regex("^[[:space:]]+[.period.][.period.][.period.][[:space:]]*$");
	// regular expression to delete spaces in the opcode string
	pattern = regex("/ //gi", regex_constants::icase|regex_constants::perl);
}


