/*******************************************************************************
 * Copyright (C) 2013 Jörg Mische, University of Augsburg, Germany
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
#ifndef _INSTRSET_H_
#define _INSTRSET_H_

/**
	instrset.h - Instruction Set (Opcodes and Microcodes)
 */

#include "tricore_instrset_incl.h"

typedef struct { 
	string format;	//opcode format
	unsigned op2;
	unsigned op1;
	string type;	//combination of B(ranch),L(oad),M(icrocode),S(tore)
	string addrmode;
//	string mod;	//modifier
	string name;	//opcode name
} instruction_t;

// only for CInstruction::extractInstruction() in instruction.cpp
const instruction_t instructions[] = {
	// abs
	{"RR", 0x1c, 0x0b, "", "", "abs"},
	// absdif
	{"RR", 0x0e, 0x0b, "", "", "absdif"},
	{"RC", 0x0e, 0x8b, "", "", "absdif"},
	// add
	{"RR", 0x00, 0x0b, "", "", "add"},
	{"RC", 0x00, 0x8b, "", "", "add"},
	{"SRR", 0x00, 0x42, "", "", "add"},
	{"SRC", 0x00, 0xc2, "", "", "add"},
	{"SRR_d", 0x00, 0x1a, "", "", "add"},
	{"SRC_d", 0x00, 0x9a, "", "", "add"},
	{"SRR_a15b", 0x00, 0x12, "", "", "add"},
	{"SBC", 0x00, 0x92, "", "", "add"},
	// add.a
	{"RR_A", 0x01, 0x01, "", "", "add.a"},
	{"SRR", 0x00, 0x30, "", "", "add.a"},
	{"SRC_adda", 0x00, 0xb0, "", "", "add.a"},
	// addc
	{"RR", 0x05, 0x0b, "", "", "addc"},
	{"RC", 0x05, 0x8b, "", "", "addc"},
	// addi
	{"RLC_s", 0x00, 0x1b, "", "", "addi"},
	// addih
	{"RLC_h", 0x00, 0x9b, "", "", "addih"},
	// addih.a
	{"RLC_h_A", 0x00, 0x11, "", "", "addih.a"},
	// addsc.a
	{"SRO", 0x00, 0x10, "", "", "addsc.a"},
	{"SRO", 0x00, 0x50, "", "", "addsc.a"},
	{"SRO", 0x00, 0x90, "", "", "addsc.a"},
	{"SRO", 0x00, 0xd0, "", "", "addsc.a"},
	{"RR_A", 0x60, 0x01, "", "", "addsc.a"},
	// addsc.at
	{"RR_A", 0x62, 0x01, "", "", "addsc.at"},
	// addx
	{"RR", 0x04, 0x0b, "", "", "addx"},
	{"RC", 0x04, 0x8b, "", "", "addx"},
	// and
	{"RR", 0x08, 0x0f, "", "", "and"},
	{"RC_u", 0x08, 0x8f, "", "", "and"},
	{"SRR", 0x00, 0x26, "", "", "and"},
	{"SC_u", 0x00, 0x16, "", "", "and"},
	// and.and.t
	{"BIT", 0x00, 0x47, "", "", "and.and.t"},
	// and.andn.t
	{"BIT", 0x03, 0x47, "", "", "and.andn.t"},
	// and.nor.t
	{"BIT", 0x02, 0x47, "", "", "and.nor.t"},
	// and.or.t
	{"BIT", 0x01, 0x47, "", "", "and.or.t"},
	// and.eq
	{"RR", 0x20, 0x0b, "", "", "and.eq"},
	{"RC", 0x20, 0x8b, "", "", "and.eq"},
	// and.ge
	{"RR", 0x24, 0x0b, "", "", "and.ge"},
	{"RC", 0x24, 0x8b, "", "", "and.ge"},
	// and.ge.u
	{"RR", 0x25, 0x0b, "", "", "and.ge.u"},
	{"RC", 0x25, 0x8b, "", "", "and.ge.u"},
	// and.lt
	{"RR", 0x22, 0x0b, "", "", "and.lt"},
	{"RC", 0x22, 0x8b, "", "", "and.lt"},
	// and.lt.u
	{"RR", 0x23, 0x0b, "", "", "and.lt.u"},
	{"RC", 0x23, 0x8b, "", "", "and.lt.u"},
	// and.ne
	{"RR", 0x21, 0x0b, "", "", "and.ne"},
	{"RC", 0x21, 0x8b, "", "", "and.ne"},
	// and.t
	{"BIT", 0x00, 0x87, "", "", "and.t"},
	// andn
	{"RR", 0x0e, 0x0f, "", "", "andn"},
	{"RC_u", 0x0e, 0x8f, "", "", "andn"},
	// andn.t
	{"BIT", 0x03, 0x87, "", "", "andn.t"},
	// bmerge
	{"RR", 0x01, 0x4b, "", "", "bmerge"},
	// bsplit
	{"RR", 0x09, 0x4b, "", "", "bsplit"},
	// cadd
	{"RRR", 0x00, 0x2b, "", "", "cadd"},
	{"RCR", 0x00, 0xab, "", "", "cadd"},
	{"SRC", 0, 0x8a, "", "", "cadd"},
	// caddn
	{"RRR", 0x01, 0x2b, "", "", "caddn"},
	{"RCR", 0x01, 0xab, "", "", "caddn"},
	{"SRC", 0, 0xca, "", "", "caddn"},
	// call
	{"B", 0, 0x6d, "", "", "call"},
	{"SR_A", 0, 0x5c, "", "", "call"},
	// calla
	{"B", 0, 0xed, "", "", "calla"},
	// calli
	{"RR_A", 0x00, 0x2d, "", "", "calli"},
	// clo
	{"RR", 0x1c, 0x0f, "", "", "clo"},
	// cls
	{"RR", 0x1d, 0x0f, "", "", "cls"},
	// clz
	{"RR", 0x1b, 0x0f, "", "", "clz"},
	// cmov
	{"SRR", 0x00, 0x2a, "", "", "cmov"},
	{"SRC", 0x00, 0xaa, "", "", "cmov"},
	// cmovn
	{"SRR", 0x00, 0x6a, "", "", "cmovn"},
	{"SRC", 0x00, 0xea, "", "", "cmovn"},
	// csub
	{"RRR", 0x02, 0x2b, "", "", "csub"},
	// csubn
	{"RRR", 0x03, 0x2b, "", "", "csubn"},
	// dextr
	{"RRRR", 0x04, 0x17, "", "", "dextr"},
	{"RRPW", 0x00, 0x77, "", "", "dextr"},
	// disable
	{"BO", 0x0d, 0x0d, "", "", "disable"},
	// dsync
	{"BO", 0x12, 0x0d, "", "", "dsync"},
	// dvadj
	{"RRR", 0x0d, 0x6b, "", "", "dvadj"},
	// dvinit
	{"RR", 0x1a, 0x4b, "", "", "dvinit"},
	// dvinit.b
	{"RR", 0x5a, 0x4b, "", "", "dvinit.b"},
	// dvinit.bu
	{"RR", 0x4a, 0x4b, "", "", "dvinit.bu"},
	// dvinit.h
	{"RR", 0x3a, 0x4b, "", "", "dvinit.h"},
	// dvinit.hu
	{"RR", 0x2a, 0x4b, "", "", "dvinit.hu"},
	// dvinit.u
	{"RR", 0x0a, 0x4b, "", "", "dvinit.u"},
	// dvstep
	{"RRR", 0x0f, 0x6b, "", "", "dvstep"},
	// dvstep.u
	{"RRR", 0x0e, 0x6b, "", "", "dvstep.u"},
	// enable
	{"BO", 0x0c, 0x0d, "", "", "enable"},
	// eq
	{"RR", 0x10, 0x0b, "", "", "eq"},
	{"RC", 0x10, 0x8b, "", "", "eq"},
	{"SRR_d", 0x00, 0x3a, "", "", "eq"},
	{"SRC_d", 0x00, 0xba, "", "", "eq"},
	// eq.a
	{"RR_A", 0x40, 0x01, "", "", "eq.a"},
	// eqany.b
	{"RR", 0x56, 0x0b, "", "", "eqany.b"},
	{"RC", 0x56, 0x8b, "", "", "eqany.b"},
	// eqany.h
	{"RR", 0x76, 0x0b, "", "", "eqany.h"},
	{"RC", 0x76, 0x8b, "", "", "eqany.h"},
	// eqz.a
	{"RR_A", 0x48, 0x01, "", "", "eqz.a"},
	// extr
	{"RRRR", 0x02, 0x17, "", "", "extr"},
	{"RRRW", 0x02, 0x57, "", "", "extr"},
	{"RRPW", 0x02, 0x37, "", "", "extr"},
	// extr.u
	{"RRRR", 0x03, 0x17, "", "", "extr.u"},
	{"RRRW", 0x03, 0x57, "", "", "extr.u"},
	{"RRPW", 0x03, 0x37, "", "", "extr.u"},
	// ge
	{"RR", 0x14, 0x0b, "", "", "ge"},
	{"RC", 0x14, 0x8b, "", "", "ge"},
	// ge.a
	{"RR_A", 0x43, 0x01, "", "", "ge.a"},
	// ge.u
	{"RR", 0x15, 0x0b, "", "", "ge.u"},
	{"RC", 0x15, 0x8b, "", "", "ge.u"},
	// imask
	{"RRRW", 0x01, 0x57, "", "", "imask"},
	{"RRPW", 0x01, 0x37, "", "", "imask"},
	{"RCRW", 0x01, 0xd7, "", "", "imask"},
	{"RCPW", 0x01, 0xb7, "", "", "imask"},
	// ins.t
	{"BIT", 0x00, 0x67, "", "", "ins.t"},
	// insn.t
	{"BIT", 0x01, 0x67, "", "", "insn.t"},
	// insert
	{"RRRR", 0x00, 0x17, "", "", "insert"},
	{"RRRW", 0x00, 0x57, "", "", "insert"},
	{"RRPW", 0x00, 0x37, "", "", "insert"},
	{"RCRR", 0x00, 0x97, "", "", "insert"},
	{"RCRW", 0x00, 0xd7, "", "", "insert"},
	{"RCPW", 0x00, 0xb7, "", "", "insert"},
	// isync
	{"BO", 0x13, 0x0d, "", "", "isync"},
	// j
	{"B", 0, 0x1d, "B", "", "j"},
	{"SR_A", 0, 0x3c, "B", "", "j"},
	// ja
	{"B", 0, 0x9d, "B", "", "ja"},
	// jeq
	{"BRR", 0x00, 0x5f, "B", "", "jeq"},
	{"BRC", 0x00, 0xdf, "B", "", "jeq"},
	{"SBR", 0x00, 0x3e, "B", "", "jeq"},
	{"SBC", 0x00, 0x1e, "B", "", "jeq"},
	// jeq.a
	{"BRR_A", 0x00, 0x7d, "B", "", "jeq.a"},
	// jge
	{"BRR", 0x00, 0x7f, "B", "", "jge"},
	{"BRC", 0x00, 0xff, "B", "", "jge"},
	// jge.u
	{"BRR", 0x01, 0x7f, "B", "", "jge.u"},
	{"BRC", 0x01, 0xff, "B", "", "jge.u"},
	// jgez
	{"SBR", 0x00, 0xce, "B", "", "jgez"},
	// jgtz
	{"SBR", 0x00, 0x4e, "B", "", "jgtz"},
	// ji
	{"RR_A", 0x03, 0x2d, "B", "", "ji"},
	{"SR_A", 0x00, 0xdc, "B", "", "ji"},
	// jl
	{"B", 0, 0x5d, "B", "", "jl"},
	// jla
	{"B", 0, 0xdd, "B", "", "jla"},
	// jlez
	{"SBR", 0x00, 0x8e, "B", "", "jlez"},
	// jli
	{"RR_A", 0x02, 0x2d, "B", "", "jli"},
	// jlt
	{"BRR", 0x00, 0x3f, "B", "", "jlt"},
	{"BRC", 0x00, 0xbf, "B", "", "jlt"},
	// jlt.u
	{"BRR", 0x01, 0x3f, "B", "", "jlt.u"},
	{"BRC", 0x01, 0xbf, "B", "", "jlt.u"},
	// jltz
	{"SBR", 0, 0x0e, "B", "", "jltz"},
	// jne
	{"BRR", 0x01, 0x5f, "B", "", "jne"},
	{"BRC", 0x01, 0xdf, "B", "", "jne"},
	{"SBR", 0, 0x7e, "B", "", "jne"},
	{"SBC", 0, 0x5e, "B", "", "jne"},
	// jne.a
	{"BRR_A", 0x01, 0x7d, "B", "", "jne.a"},
	// jned
	{"BRR", 0x01, 0x1f, "B", "", "jned"},
	{"BRC", 0x01, 0x9f, "B", "", "jned"},
	// jnei
	{"BRR", 0x00, 0x1f, "B", "", "jnei"},
	{"BRC", 0x00, 0x9f, "B", "", "jnei"},
	// jnz
	{"SB", 0, 0xee, "B", "", "jnz"},
	{"SBR", 0, 0xf6, "B", "", "jnz"},
	// jnz.a
	{"BRR_A", 0x01, 0xbd, "B", "", "jnz.a"},
	{"SBR_A", 0, 0x7c, "B", "", "jnz.a"},
	// jnz.t
	{"BRR", 0x01, 0x6f, "B", "", "jnz.t"},
	{"BRR", 0x01, 0xef, "B", "", "jnz.t"},
	{"SBC", 0, 0xae, "B", "", "jnz.t"},
	// jz
	{"SB", 0, 0x6e, "B", "", "jz"},
	{"SBR", 0, 0x76, "B", "", "jz"},
	// jz.a
	{"BRR_A", 0x00, 0xbd, "B", "", "jz.a"},
	{"SBR_A", 0, 0xbc, "B", "", "jz.a"},
	// jz.t
	{"BRR", 0x00, 0x6f, "B", "", "jz.t"},
	{"BRR", 0x00, 0xef, "B", "", "jz.t"},
	{"SBC", 0, 0x2e, "B", "", "jz.t"},
	// ld.a
	{"BO", 0x16, 0x09, "L", "p", "ld.a"},
	{"BO", 0x06, 0x09, "L", "q", "ld.a"},
	{"BOL", 0, 0x99, "L", "", "ld.a"},
	{"BO", 0x26, 0x09, "L", "", "ld.a"},
	{"ABS", 0x02, 0x85, "L", "", "ld.a"},
	{"SLR", 0, 0xd4, "L", "", "ld.a"},
	{"SLRO", 0, 0xc8, "L", "", "ld.a"},
	{"SRO", 0, 0xcc, "L", "", "ld.a"},
	{"SpR", 0, 0xc4, "L", "q", "ld.a"},
	{"SCo", 0, 0xd8, "L", "", "ld.a"},
	// ld.b
	{"BO", 0x10, 0x09, "L", "p", "ld.b"},
	{"BO", 0x00, 0x09, "L", "q", "ld.b"},
	{"BO", 0x20, 0x09, "L", "", "ld.b"},
	{"ABS", 0x00, 0x05, "L", "", "ld.b"},
	// ld.bu
	{"BO", 0x11, 0x09, "L", "p", "ld.bu"},
	{"BO", 0x01, 0x09, "L", "q", "ld.bu"},
	{"BO", 0x21, 0x09, "L", "", "ld.bu"},
	{"ABS", 0x01, 0x05, "L", "", "ld.bu"},
	{"SLR", 0, 0x14, "L", "", "ld.bu"},
	{"SLRO", 0, 0x08, "L", "", "ld.bu"},
	{"SRO", 0, 0x0c, "L", "", "ld.bu"},
	{"SpR", 0, 0x04, "L", "q", "ld.bu"},
	// ld.d
	{"BO", 0x15, 0x09, "L", "p", "ld.d"},
	{"BO", 0x05, 0x09, "L", "q", "ld.d"},
	{"BO", 0x25, 0x09, "L", "", "ld.d"},
	{"ABS", 0x01, 0x85, "L", "", "ld.d"},
	// ld.da
	{"BO", 0x17, 0x09, "L", "p", "ld.da"},
	{"BO", 0x07, 0x09, "L", "q", "ld.da"},
	{"BO", 0x27, 0x09, "L", "", "ld.da"},
	{"ABS", 0x03, 0x85, "L", "", "ld.da"},
	// ld.h
	{"BO", 0x12, 0x09, "L", "p", "ld.h"},
	{"BO", 0x02, 0x09, "L", "q", "ld.h"},
	{"BO", 0x22, 0x09, "L", "", "ld.h"},
	{"ABS", 0x02, 0x05, "L", "", "ld.h"},
	{"SLR", 0, 0x94, "L", "", "ld.h"},
	{"SLRO", 0, 0x88, "L", "", "ld.h"},
	{"SRO", 0, 0x8c, "L", "", "ld.h"},
	{"SpR", 0, 0x84, "L", "q", "ld.h"},
	// ld.hu
	{"BO", 0x13, 0x09, "L", "p", "ld.hu"},
	{"BO", 0x03, 0x09, "L", "q", "ld.hu"},
	{"BO", 0x23, 0x09, "L", "", "ld.hu"},
	{"ABS", 0x03, 0x05, "L", "", "ld.hu"},
	// ld.q
	{"BO", 0x18, 0x09, "L", "p", "ld.q"},
	{"BO", 0x08, 0x09, "L", "q", "ld.q"},
	{"BO", 0x28, 0x09, "L", "", "ld.q"},
	{"ABS", 0x00, 0x45, "L", "", "ld.q"},
	// ld.w
	{"BO", 0x14, 0x09, "L", "p", "ld.w"},
	{"BO", 0x04, 0x09, "L", "q", "ld.w"},
	{"BOL", 0, 0x19, "L", "", "ld.w"},
	{"BO", 0x24, 0x09, "L", "", "ld.w"},
	{"ABS", 0x00, 0x85, "L", "", "ld.w"},
	{"SLR", 0, 0x54, "L", "", "ld.w"},
	{"SLRO", 0, 0x48, "L", "", "ld.w"},
	{"SRO", 0, 0x4c, "L", "", "ld.w"},
	{"SpR", 0, 0x44, "L", "q", "ld.w"},
	{"SCo", 0, 0x58, "L", "", "ld.w"},
	// ldmst
	{"BO", 0x21, 0x49, "L", "", "ldmst"},
	{"ABS", 0x01, 0xe5, "L", "", "ldmst"},
	// lea
	{"BOL", 0x00, 0xd9, "", "", "lea"},
	{"BO", 0x28, 0x49, "", "", "lea"},
	{"ABS", 0x00, 0xc5, "", "", "lea"},
	// loop
	{"BRR_A", 0x00, 0xfd, "B", "", "loop"},
	{"SBR_loop", 0, 0xfc, "B", "", "loop"},
	// loopu
	{"BRR_A", 0x01, 0xfd, "B", "", "loopu"},
	// lt
	{"RR", 0x12, 0x0b, "", "", "lt"},
	{"RC", 0x12, 0x8b, "", "", "lt"},
	{"SRR_d", 0, 0x7a, "", "", "lt"},
	{"SRC_d", 0, 0xfa, "", "", "lt"},
	// lt.u
	{"RR", 0x13, 0x0b, "", "", "lt.u"},
	{"RC", 0x13, 0x8b, "", "", "lt.u"},
	// lt.a
	{"RR_A", 0x42, 0x01, "", "", "lt.a"},
	// madd(32)
	{"RRR2", 0x0a, 0x03, "", "", "madd(32)"},
	{"RCR", 0x01, 0x13, "", "", "madd(32)"},
	// madd.u
	{"RRR2", 0x68, 0x03, "", "", "madd.u"},
	{"RCR", 0x02, 0x13, "", "", "madd.u"},
	// max
	{"RR", 0x1a, 0x0b, "", "", "max"},
	{"RC", 0x1a, 0x8b, "", "", "max"},
	// max.u
	{"RR", 0x1b, 0x0b, "", "", "max.u"},
	{"RC", 0x1b, 0x8b, "", "", "max.u"},
	// mfcr
	{"RLC_h_A", 0x00, 0x4d, "", "", "mfcr"},
	// min
	{"RR", 0x18, 0x0b, "", "", "min"},
	{"RC", 0x18, 0x8b, "", "", "min"},
	// min.u
	{"RR", 0x19, 0x0b, "", "", "min.u"},
	{"RC", 0x19, 0x8b, "", "", "min.u"},
	// mov
	{"RR", 0x1f, 0x0b, "", "", "mov"},
	{"RLC_s", 0x00, 0x3b, "", "", "mov"},
	{"SRR", 0x00, 0x02, "", "", "mov"},
	{"SRC", 0x00, 0x82, "", "", "mov"},
	{"SC_u", 0x00, 0xda, "", "", "mov"},
	// mov.a
	{"RR_A", 0x63, 0x01, "", "", "mov.a"},
	{"SBR_A", 0, 0x60, "", "", "mov.a"},
	{"SBR_A", 0, 0xa0, "", "", "mov.a"},
	// mov.aa
	{"RR_A", 0x00, 0x01, "", "", "mov.aa"},
	{"SR_A", 0, 0x40, "", "", "mov.aa"},
	// mov.d
	{"RR_A", 0x4c, 0x01, "", "", "mov.d"},
	{"SR_A", 0, 0x80, "", "", "mov.d"},
	// mov.u
	{"RLC_u", 0x00, 0xbb, "", "", "mov.u"},
	// movh
	{"RLC_h", 0x00, 0x7b, "", "", "movh"},
	// movh.a
	{"RLC_h_A", 0x00, 0x91, "", "", "movh.a"},
	// msub32
	{"RRR2", 0x0a, 0x23, "", "", "msub32"},
	{"RCR", 0x01, 0x33, "", "", "msub32"},
	// msub.u
	{"RRR2", 0x68, 0x23, "", "", "msub.u"},
	{"RCR", 0x02, 0x33, "", "", "msub.u"},
	// mtcr
	{"RLC_h_A", 0x00, 0xcd, "", "", "mtcr"},
	// mul
	{"SRR", 0x00, 0xe2, "", "", "mul"},
	{"RR2", 0x0a, 0x73, "", "", "mul"},
	{"RR2", 0x6a, 0x73, "", "", "mul"},
	{"RC", 0x01, 0x53, "", "", "mul"},
	{"RC", 0x03, 0x53, "", "", "mul"},
	// mul.u
	{"RR2", 0x68, 0x73, "", "", "mul.u"},
	{"RC", 0x02, 0x53, "", "", "mul.u"},
	// nand
	{"RR", 0x09, 0x0f, "", "", "nand"},
	{"RC_u", 0x09, 0x8f, "", "", "nand"},
	// nand.t
	{"BIT", 0x00, 0x07, "", "", "nand.t"},
	// ne
	{"RR", 0x11, 0x0b, "", "", "ne"},
	{"RC", 0x11, 0x8b, "", "", "ne"},
	// ne.a
	{"RR_A", 0x41, 0x01, "", "", "ne.a"},
	// nez.a
	{"RR_A", 0x49, 0x01, "", "", "nez.a"},
	// nop
	{"BO", 0x00, 0x0d, "", "", "nop"},
	{"SR_A", 0x00, 0x00, "", "", "nop"},
	// nor
	{"RR", 0x0b, 0x0f, "", "", "nor"},
	{"RC_u", 0x0b, 0x8f, "", "", "nor"},
	// nor.t
	{"BIT", 0x02, 0x87, "", "", "nor.t"},
	// not
	{"SR", 0x00, 0x46, "", "", "not"},
	// or
	{"RR", 0x0a, 0x0f, "", "", "or"},
	{"RC_u", 0x0a, 0x8f, "", "", "or"},
	{"SRR", 0, 0xa6, "", "", "or"},
	{"SC_u", 0, 0x96, "", "", "or"},
	// or.and.t
	{"BIT", 0x00, 0xc7, "", "", "or.and.t"},
	// or.andn.t
	{"BIT", 0x03, 0xc7, "", "", "or.andn.t"},
	// or.nor.t
	{"BIT", 0x02, 0xc7, "", "", "or.nor.t"},
	// or.or.t
	{"BIT", 0x01, 0xc7, "", "", "or.or.t"},
	// or.eq
	{"RR", 0x27, 0x0b, "", "", "or.eq"},
	{"RC", 0x27, 0x8b, "", "", "or.eq"},
	// or.ge
	{"RR", 0x2b, 0x0b, "", "", "or.ge"},
	{"RC", 0x2b, 0x8b, "", "", "or.ge"},
	// or.ge.u
	{"RR", 0x2c, 0x0b, "", "", "or.ge.u"},
	{"RC", 0x2c, 0x8b, "", "", "or.ge.u"},
	// or.lt
	{"RR", 0x29, 0x0b, "", "", "or.lt"},
	{"RC", 0x29, 0x8b, "", "", "or.lt"},
	// or.lt.u
	{"RR", 0x2a, 0x0b, "", "", "or.lt.u"},
	{"RC", 0x2a, 0x8b, "", "", "or.lt.u"},
	// or.ne
	{"RR", 0x28, 0x0b, "", "", "or.ne"},
	{"RC", 0x28, 0x8b, "", "", "or.ne"},
	// or.t
	{"BIT", 0x01, 0x87, "", "", "or.t"},
	// orn
	{"RR", 0x0f, 0x0f, "", "", "orn"},
	{"RC_u", 0x0f, 0x8f, "", "", "orn"},
	// orn.t
	{"BIT", 0x01, 0x07, "", "", "orn.t"},
	// pack
	{"RRR", 0x00, 0x6b, "", "", "pack"},
	// parity
	{"RR", 0x02, 0x4b, "", "", "parity"},
	// ret
	{"SR_A", 0x09, 0x00, "", "", "ret"},
	{"BO", 0x06, 0x0d, "", "", "ret"},
	// rsub
	{"SR", 0x05, 0x32, "", "", "rsub"},
	{"RC", 0x08, 0x8b, "", "", "rsub"},
	// sel
	{"RRR", 0x04, 0x2b, "", "", "sel"},
	{"RCR", 0x04, 0xab, "", "", "sel"},
	// seln
	{"RRR", 0x05, 0x2b, "", "", "seln"},
	{"RCR", 0x05, 0xab, "", "", "seln"},
	// sh
	{"RR", 0x00, 0x0f, "", "", "sh"},
	{"RC_u", 0x00, 0x8f, "", "", "sh"},
	{"SRC", 0x00, 0x06, "", "", "sh"},
	// sh.eq
	{"RR", 0x37, 0x0b, "", "", "sh.eq"},
	{"RC", 0x37, 0x8b, "", "", "sh.eq"},
	// sh.ge
	{"RR", 0x3b, 0x0b, "", "", "sh.ge"},
	{"RC", 0x3b, 0x8b, "", "", "sh.ge"},
	// sh.ge.u
	{"RR", 0x3c, 0x0b, "", "", "sh.ge.u"},
	{"RC", 0x3c, 0x8b, "", "", "sh.ge.u"},
	// sh.lt
	{"RR", 0x39, 0x0b, "", "", "sh.lt"},
	{"RC", 0x39, 0x8b, "", "", "sh.lt"},
	// sh.lt.u
	{"RR", 0x3a, 0x0b, "", "", "sh.lt.u"},
	{"RC", 0x3a, 0x8b, "", "", "sh.lt.u"},
	// sh.ne
	{"RR", 0x38, 0x0b, "", "", "sh.ne"},
	{"RC", 0x38, 0x8b, "", "", "sh.ne"},
	// sh.and.t
	{"BIT", 0x00, 0x27, "", "", "sh.and.t"},
	// sh.andn.t
	{"BIT", 0x03, 0x27, "", "", "sh.andn.t"},
	// sh.nand.t
	{"BIT", 0x00, 0xa7, "", "", "sh.nand.t"},
	// sh.nor.t
	{"BIT", 0x02, 0x27, "", "", "sh.nor.t"},
	// sh.or.t
	{"BIT", 0x01, 0x27, "", "", "sh.or.t"},
	// sh.orn.t
	{"BIT", 0x01, 0xa7, "", "", "sh.orn.t"},
	// sh.xnor.t
	{"BIT", 0x02, 0xa7, "", "", "sh.xnor.t"},
	// sh.xor.t
	{"BIT", 0x03, 0xa7, "", "", "sh.xor.t"},
	// sha
	{"RR", 0x01, 0x0f, "", "", "sha"},
	{"RC_u", 0x01, 0x8f, "", "", "sha"},
	{"SRC", 0x00, 0x86, "", "", "sha"},
	// st.a
	{"BO", 0x16, 0x89, "S", "p", "st.a"},
	{"BO", 0x06, 0x89, "S", "q", "st.a"},
	{"BO", 0x26, 0x89, "S", "", "st.a"},
	{"ABS", 0x02, 0xa5, "S", "", "st.a"},
	{"SLR", 0, 0xf4, "S", "", "st.a"},
	{"SLRO", 0, 0xe8, "S", "", "st.a"},
	{"SRO", 0, 0xec, "S", "", "st.a"},
	{"SpR", 0, 0xe4, "S", "q", "st.a"},
	{"SCo", 0, 0xf8, "S", "", "st.a"},
	// st.b
	{"BO", 0x10, 0x89, "S", "p", "st.b"},
	{"BO", 0x00, 0x89, "S", "q", "st.b"},
	{"BO", 0x20, 0x89, "S", "", "st.b"},
	{"ABS", 0x00, 0x25, "S", "", "st.b"},
	{"SLR", 0, 0x34, "S", "", "st.b"},
	{"SLRO", 0, 0x28, "S", "", "st.b"},
	{"SRO", 0, 0x2c, "S", "", "st.b"},
	{"SpR", 0, 0x24, "S", "q", "st.b"},
	// st.d
	{"BO", 0x15, 0x89, "S", "p", "st.d"},
	{"BO", 0x05, 0x89, "S", "q", "st.d"},
	{"BO", 0x25, 0x89, "S", "", "st.d"},
	{"ABS", 0x01, 0xa5, "S", "", "st.d"},
	// st.da
	{"BO", 0x17, 0x89, "", "", "st.da"},
	{"BO", 0x07, 0x89, "", "", "st.da"},
	{"BO", 0x27, 0x89, "", "", "st.da"},
	{"ABS", 0x03, 0xa5, "", "", "st.da"},
	// st.h
	{"BO", 0x12, 0x89, "S", "p", "st.h"},
	{"BO", 0x02, 0x89, "S", "q", "st.h"},
	{"BO", 0x22, 0x89, "S", "", "st.h"},
	{"ABS", 0x02, 0x25, "S", "", "st.h"},
	{"SLR", 0, 0xb4, "S", "", "st.h"},
	{"SLRO", 0, 0xa8, "S", "", "st.h"},
	{"SRO", 0, 0xac, "S", "", "st.h"},
	{"SpR", 0, 0xa4, "S", "q", "st.h"},
	// st.q
	{"BO", 0x18, 0x89, "S", "p", "st.q"},
	{"BO", 0x08, 0x89, "S", "q", "st.q"},
	{"BO", 0x28, 0x89, "S", "", "st.q"},
	{"ABS", 0x00, 0x65, "S", "", "st.q"},
	// st.t
	{"ABS", 0x00, 0xd5, "L", "", "st.t"},
	// st.w
	{"BO", 0x14, 0x89, "S", "p", "st.w"},
	{"BO", 0x04, 0x89, "S", "q", "st.w"},
	{"BOL", 0x00, 0x59, "S", "", "st.w"},
	{"BO", 0x24, 0x89, "S", "", "st.w"},
	{"ABS", 0x00, 0xa5, "S", "", "st.w"},
	{"SLR", 0, 0x74, "S", "", "st.w"},
	{"SLRO", 0, 0x68, "S", "", "st.w"},
	{"SRO", 0, 0x6c, "S", "", "st.w"},
	{"SpR", 0, 0x64, "S", "q", "st.w"},
	{"SCo", 0, 0x78, "S", "", "st.w"},
	// sub
	{"RR", 0x08, 0x0b, "", "", "sub"},
	{"SRR", 0x00, 0xa2, "", "", "sub"},
	{"SRR_a15b", 0x00, 0x52, "", "", "sub"},
	{"SRR_d", 0x00, 0x5a, "", "", "sub"},
	// sub.a
	{"RR_A", 0x02, 0x01, "", "", "sub.a"},
	{"SC_suba", 0x00, 0x20, "", "", "sub.a"},
	// subc
	{"RR", 0x0d, 0x0b, "", "", "subc"},
	// subx
	{"RR", 0x0c, 0x0b, "", "", "subx"},
	// swap.w
	{"BO", 0x20, 0x49, "L", "", "swap.w"},
	{"ABS", 0x00, 0xe5, "L", "", "swap.w"},
	// unpack
	{"RR", 0x08, 0x4b, "", "", "unpack"},
	// xnor
	{"RR", 0x0d, 0x0f, "", "", "xnor"},
	{"RC_u", 0x0d, 0x8f, "", "", "xnor"},
	// xnor.t
	{"BIT", 0x02, 0x07, "", "", "xnor.t"},
	// xor
	{"RR", 0x0c, 0x0f, "", "", "xor"},
	{"RC_u", 0x0c, 0x8f, "", "", "xor"},
	{"SRR", 0, 0xc6, "", "", "xor"},
	// xor.eq
	{"RR", 0x2f, 0x0b, "", "", "xor.eq"},
	{"RC", 0x2f, 0x8b, "", "", "xor.eq"},
	// xor.ge
	{"RR", 0x33, 0x0b, "", "", "xor.ge"},
	{"RC", 0x33, 0x8b, "", "", "xor.ge"},
	// xor.ge.u
	{"RR", 0x34, 0x0b, "", "", "xor.ge.u"},
	{"RC", 0x34, 0x8b, "", "", "xor.ge.u"},
	// xor.lt
	{"RR", 0x31, 0x0b, "", "", "xor.lt"},
	{"RC", 0x31, 0x8b, "", "", "xor.lt"},
	// xor.lt.u
	{"RR", 0x32, 0x0b, "", "", "xor.lt.u"},
	{"RC", 0x32, 0x8b, "", "", "xor.lt.u"},
	// xor.ne
	{"RR", 0x30, 0x0b, "", "", "xor.ne"},
	{"RC", 0x30, 0x8b, "", "", "xor.ne"},
	// xor.t
	{"BIT", 0x03, 0x07, "", "", "xor.t"},
	// yield
	{"BO", 0x11, 0x0d, "", "", "yield"},
	// swapout
	{"BO", 0x20, 0x0d, "", "", "swapout"},
	// swapin
	{"BO", 0x21, 0x0d, "", "", "swapin"},
	// add.f
	{"RRR", 0x02, 0x6b, "", "", "add.f"},
	// cmp.f
	{"RR", 0x00, 0x4b, "", "", "cmp.f"},
	// div.f
	{"RR", 0x05, 0x4b, "", "", "div.f"},
	// ftoi
	{"RR", 0x10, 0x4b, "", "", "ftoi"},
	// ftoiz
	{"RR", 0x13, 0x4b, "", "", "ftoiz"},
	// ftoq31
	{"RR", 0x11, 0x4b, "", "", "ftoq31"},
	// ftoq31z
	{"RR", 0x18, 0x4b, "", "", "ftoq31z"},
	// ftou
	{"RR", 0x12, 0x4b, "", "", "ftou"},
	// ftouz
	{"RR", 0x17, 0x4b, "", "", "ftouz"},
	// itof
	{"RR", 0x14, 0x4b, "", "", "itof"},
	// madd.f
	{"RRR", 0x06, 0x6b, "", "", "madd.f"},
	// msub.f
	{"RRR", 0x07, 0x6b, "", "", "msub.f"},
	// mul.f
	{"RR", 0x04, 0x4b, "", "", "mul.f"},
	// q31tof
	{"RR", 0x15, 0x4b, "", "", "q31tof"},
	// qseed.f
	{"RR", 0x19, 0x4b, "", "", "qseed.f"},
	// sub.f
	{"RRR", 0x03, 0x6b, "", "", "sub.f"},
	// updfl
	{"RR", 0x0c, 0x4b, "", "", "updfl"},
	// utof
	{"RR", 0x16, 0x4b, "", "", "utof"},
};

const unsigned INSTRUCTION_COUNT = 451;


/* Opcode format */
typedef enum 
{
    OF_UNKNOWN=0, OF_SpR=1, OF_SBR_A=2, OF_SBR_loop=3, OF_SC_suba=4, OF_SCo=5, OF_SLR=6, OF_SLRO=7, OF_SR_A=8, OF_SRC_adda=9, OF_SRO=10, OF_ABS=11, OF_B=12, OF_BO=13, OF_BOL=14, OF_BRR_A=15, OF_RLC_h_A=16, OF_RR_A=17, OF_SB=18, OF_SBC=19, OF_SBR=20, OF_SC_u=21, OF_SR=22, OF_SRC=23, OF_SRC_d=24, OF_SRR=25, OF_SRR_d=26, OF_SRR_a15b=27, OF_BIT=28, OF_BRC=29, OF_BRR=30, OF_RC=31, OF_RC_u=32, OF_RCR=33, OF_RLC_s=34, OF_RLC_u=35, OF_RLC_h=36, OF_RR=37, OF_RR2=38, OF_RRR=39, OF_RRR2=40, OF_RCPW=41, OF_RCRR=42, OF_RCRW=43, OF_RRPW=44, OF_RRRR=45, OF_RRRW=46
} opformat_t;


typedef struct 
{
    alu_func_t		func;
    cond_t		cond;
    opformat_t		format;
    bool		xrd;
    unsigned		inswr;
    unsigned		wr;
    bool		csfrwr;
    bool		signext;
    unsigned		memwidth;
} func_mode_t;




#define OFF_SHIFT(o) (((u&0x40)!=0) ? (o) << 2 :  ( ((u&0x80)!=0) ? (o) << 1 : (o) ) )
#define OPERAND_UNDEFINED 0

typedef struct
{
    unsigned op2;
    bool sel0, sel1, sel2, sel3;
    word_t operand0, operand1, operand2, operand3;
    reg_t dest;
    const char *disasm;
} instrset_operands_t;


inline void instrset_map_operands(instrset_operands_t *r, opformat_t of, uint32_t u, uint32_t pc, bool signext)
{
    r->op2 = 0;
    switch (of)
    {
	case OF_SpR:
	    // no op2
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=1; r->operand1 = BITRANGE(u, 15, 12); // b
	    r->sel2=0; r->operand2 = OFF_SHIFT(1); // shimm1
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = BITRANGE(u, 15, 12); // b
	    r->disasm = "%s1%, [%s2%]";
	    break;
	case OF_SBR_A:
	    // no op2
	    r->sel0=1; r->operand0 = BITRANGE(u, 15, 12); // b
	    r->sel1=0; r->operand1 = BITRANGE(u, 15, 12); // const4u
	    r->sel2=0; r->operand2 = BITRANGE(u, 11, 8)*2; // disp4
	    r->sel3=0; r->operand3 = pc; // pc
	    r->dest = BITRANGE(u, 11, 8); // a
	    r->disasm = "%s2%, %disp4%";
	    break;
	case OF_SBR_loop:
	    // no op2
	    r->sel0=0; r->operand0 = OPERAND_UNDEFINED; // 
	    r->sel1=1; r->operand1 = BITRANGE(u, 15, 12); // b
	    r->sel2=0; r->operand2 = BITRANGE(u, 11, 8)*2 | 0xffffffe0; // disp4n
	    r->sel3=0; r->operand3 = pc; // pc
	    r->dest = BITRANGE(u, 15, 12); // b
	    r->disasm = "%s2%, %disp4n%";
	    break;
	case OF_SC_suba:
	    // no op2
	    r->sel0=1; r->operand0 = 10; // 10
	    r->sel1=0; r->operand1 = BITRANGE(u, 15, 8); // const8u
	    r->sel2=0; r->operand2 = OPERAND_UNDEFINED; // 
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = 10; // 10
	    r->disasm = "%%sp, %const8u%";
	    break;
	case OF_SCo:
	    // no op2
	    r->sel0=1; r->operand0 = 15; // 15
	    r->sel1=1; r->operand1 = 10; // 10
	    r->sel2=0; r->operand2 = OFF_SHIFT(BITRANGE(u, 15, 8)); // shift8
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = OPERAND_UNDEFINED; // 
	    r->disasm = "%15%, [%%sp]%shift8%";
	    break;
	case OF_SLR:
	    // no op2
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=1; r->operand1 = BITRANGE(u, 15, 12); // b
	    r->sel2=0; r->operand2 = 0; // imm0
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = BITRANGE(u, 15, 12); // b
	    r->disasm = "%s1%, [%s2%]";
	    break;
	case OF_SLRO:
	    // no op2
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=1; r->operand1 = 15; // 15
	    r->sel2=0; r->operand2 = OFF_SHIFT(BITRANGE(u, 15, 12)); // shift4hi
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = OPERAND_UNDEFINED; // 
	    r->disasm = "%s1%, [%15%]%shift4hi%";
	    break;
	case OF_SR_A:
	    r->op2 = BITRANGE(u, 15, 12);
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=1; r->operand1 = BITRANGE(u, 15, 12); // b
	    r->sel2=0; r->operand2 = SIGN_EXTEND_32(BITRANGE(u, 15, 8), 8)*2; // disp8
	    r->sel3=0; r->operand3 = pc; // pc
	    r->dest = BITRANGE(u, 11, 8); // a
	    r->disasm = "%s1%, 0x00";
	    break;
	case OF_SRC_adda:
	    // no op2
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=0; r->operand1 = SIGN_EXTEND_32(BITRANGE(u, 15, 12), 4); // const4s
	    r->sel2=0; r->operand2 = OPERAND_UNDEFINED; // 
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = BITRANGE(u, 11, 8); // a
	    r->disasm = "%s1%, %const4s%";
	    break;
	case OF_SRO:
	    // no op2
	    r->sel0=1; r->operand0 = 15; // 15
	    r->sel1=1; r->operand1 = BITRANGE(u, 15, 12); // b
	    r->sel2=0; r->operand2 = OFF_SHIFT(BITRANGE(u, 11, 8)); // shift4lo
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = BITRANGE(u, 11, 8); // a
	    r->disasm = "%15%, %s2%, %shift4lo%";
	    break;
	case OF_ABS:
	    r->op2 = BITRANGE(u, 27, 26);
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=0; r->operand1 = 0; // imm0
	    r->sel2=0; r->operand2 = (BITRANGE(u, 15, 12)<<28) + (BITRANGE(u, 25, 22)<<10) + (BITRANGE(u, 31, 28)<<6) + BITRANGE(u, 21, 16); // off18
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = BITRANGE(u, 11, 8); // a
	    r->disasm = "%s1%, %off18% (,%bpos%)";
	    break;
	case OF_B:
	    // no op2
	    r->sel0=0; r->operand0 = OPERAND_UNDEFINED; // 
	    r->sel1=0; r->operand1 = OPERAND_UNDEFINED; // 
	    r->sel2=0; r->operand2 = (SIGN_EXTEND_32(BITRANGE(u, 15, 8), 8)<<17) + (BITRANGE(u, 31, 16)<<1); // disp24
	    r->sel3=0; r->operand3 = pc; // pc
	    r->dest = 11; // 11
	    r->disasm = "%disp24%";
	    break;
	case OF_BO:
	    r->op2 = BITRANGE(u, 27, 22);
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=1; r->operand1 = BITRANGE(u, 15, 12); // b
	    r->sel2=0; r->operand2 = (SIGN_EXTEND_32(BITRANGE(u, 31, 28), 4)<<6) + BITRANGE(u, 21, 16); // off10
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = BITRANGE(u, 15, 12); // b
	    r->disasm = "%s1%, [%addr%]%off10%";
	    break;
	case OF_BOL:
	    // no op2
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=1; r->operand1 = BITRANGE(u, 15, 12); // b
	    r->sel2=0; r->operand2 = (SIGN_EXTEND_32(BITRANGE(u, 27, 22), 6)<<10) + (BITRANGE(u, 31, 28)<<6) + BITRANGE(u, 21, 16); // off16
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = BITRANGE(u, 11, 8); // a
	    r->disasm = "%s1%, [%addr%]%off16%";
	    break;
	case OF_BRR_A:
	    r->op2 = BITRANGE(u, 31, 31);
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=1; r->operand1 = BITRANGE(u, 15, 12); // b
	    r->sel2=0; r->operand2 = SIGN_EXTEND_32(BITRANGE(u, 30, 16), 15)*2; // disp15
	    r->sel3=0; r->operand3 = pc; // pc
	    r->dest = BITRANGE(u, 15, 12); // b
	    r->disasm = "%s1%, (%s2%,) %disp15%";
	    break;
	case OF_RLC_h_A:
	    // no op2
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=0; r->operand1 = BITRANGE(u, 27, 12) << 16; // const16h
	    r->sel2=0; r->operand2 = OPERAND_UNDEFINED; // 
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = BITRANGE(u, 31, 28); // c
	    r->disasm = "%d%, (%s1%,) %const16u%";
	    break;
	case OF_RR_A:
	    r->op2 = BITRANGE(u, 27, 20);
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=1; r->operand1 = BITRANGE(u, 15, 12); // b
	    r->sel2=0; r->operand2 = OPERAND_UNDEFINED; // 
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = BITRANGE(u, 31, 28); // c
	    r->disasm = "%d%, %s1%, %s2%, %n%";
	    break;
	case OF_SB:
	    // no op2
	    r->sel0=1; r->operand0 = 15; // 15
	    r->sel1=0; r->operand1 = OPERAND_UNDEFINED; // 
	    r->sel2=0; r->operand2 = SIGN_EXTEND_32(BITRANGE(u, 15, 8), 8)*2; // disp8
	    r->sel3=0; r->operand3 = pc; // pc
	    r->dest = OPERAND_UNDEFINED; // 
	    r->disasm = "%15%, %disp8%";
	    break;
	case OF_SBC:
	    // no op2
	    r->sel0=1; r->operand0 = 15; // 15
	    r->sel1=0; r->operand1 = SIGN_EXTEND_32(BITRANGE(u, 15, 12), 4); // const4s
	    r->sel2=0; r->operand2 = BITRANGE(u, 11, 8)*2; // disp4
	    r->sel3=0; r->operand3 = pc; // pc
	    r->dest = BITRANGE(u, 11, 8); // a
	    r->disasm = "%15%, %const4s%, %disp4%";
	    break;
	case OF_SBR:
	    // no op2
	    r->sel0=1; r->operand0 = BITRANGE(u, 15, 12); // b
	    r->sel1=1; r->operand1 = 15; // 15
	    r->sel2=0; r->operand2 = BITRANGE(u, 11, 8)*2; // disp4
	    r->sel3=0; r->operand3 = pc; // pc
	    r->dest = OPERAND_UNDEFINED; // 
	    r->disasm = "%s2%, %disp4%";
	    break;
	case OF_SC_u:
	    // no op2
	    r->sel0=1; r->operand0 = 15; // 15
	    r->sel1=0; r->operand1 = BITRANGE(u, 15, 8); // const8u
	    r->sel2=0; r->operand2 = OPERAND_UNDEFINED; // 
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = 15; // 15
	    r->disasm = "%15%, %const8u%";
	    break;
	case OF_SR:
	    r->op2 = BITRANGE(u, 15, 12);
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=0; r->operand1 = 0; // imm0
	    r->sel2=0; r->operand2 = OPERAND_UNDEFINED; // 
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = BITRANGE(u, 11, 8); // a
	    r->disasm = "%s1%";
	    break;
	case OF_SRC:
	    // no op2
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=0; r->operand1 = SIGN_EXTEND_32(BITRANGE(u, 15, 12), 4); // const4s
	    r->sel2=1; r->operand2 = 15; // 15
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = BITRANGE(u, 11, 8); // a
	    r->disasm = "%s1%, %const4s%";
	    break;
	case OF_SRC_d:
	    // no op2
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=0; r->operand1 = SIGN_EXTEND_32(BITRANGE(u, 15, 12), 4); // const4s
	    r->sel2=0; r->operand2 = OPERAND_UNDEFINED; // 
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = 15; // 15
	    r->disasm = "%15%, %s1%, %const4s%";
	    break;
	case OF_SRR:
	    // no op2
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=1; r->operand1 = BITRANGE(u, 15, 12); // b
	    r->sel2=1; r->operand2 = 15; // 15
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = BITRANGE(u, 11, 8); // a
	    r->disasm = "%s1%, %s2%";
	    break;
	case OF_SRR_d:
	    // no op2
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=1; r->operand1 = BITRANGE(u, 15, 12); // b
	    r->sel2=0; r->operand2 = OPERAND_UNDEFINED; // 
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = 15; // 15
	    r->disasm = "%15%, %s1%, %s2%";
	    break;
	case OF_SRR_a15b:
	    // no op2
	    r->sel0=1; r->operand0 = 15; // 15
	    r->sel1=1; r->operand1 = BITRANGE(u, 15, 12); // b
	    r->sel2=0; r->operand2 = OPERAND_UNDEFINED; // 
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = BITRANGE(u, 11, 8); // a
	    r->disasm = "%s1%, %%d15, %s2%";
	    break;
	case OF_BIT:
	    r->op2 = BITRANGE(u, 22, 21);
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=1; r->operand1 = BITRANGE(u, 15, 12); // b
	    r->sel2=1; r->operand2 = BITRANGE(u, 27, 24); // d
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = BITRANGE(u, 31, 28); // c
	    r->disasm = "%d%, %s1%, %w%, %s2%, %p%";
	    break;
	case OF_BRC:
	    r->op2 = BITRANGE(u, 31, 31);
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=0; r->operand1 = (signext
		? SIGN_EXTEND_32(BITRANGE(u, 15, 12), 4)
		: BITRANGE(u, 15, 12)); // const4
	    r->sel2=0; r->operand2 = SIGN_EXTEND_32(BITRANGE(u, 30, 16), 15)*2; // disp15
	    r->sel3=0; r->operand3 = pc; // pc
	    r->dest = BITRANGE(u, 11, 8); // a
	    r->disasm = "%s1%, %const4%, %disp15%";
	    break;
	case OF_BRR:
	    r->op2 = BITRANGE(u, 31, 31);
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=1; r->operand1 = BITRANGE(u, 15, 12); // b
	    r->sel2=0; r->operand2 = SIGN_EXTEND_32(BITRANGE(u, 30, 16), 15)*2; // disp15
	    r->sel3=0; r->operand3 = pc; // pc
	    r->dest = BITRANGE(u, 11, 8); // a
	    r->disasm = "%s1%, %s2%, %disp15%";
	    break;
	case OF_RC:
	    r->op2 = BITRANGE(u, 27, 21);
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=0; r->operand1 = (signext
		? SIGN_EXTEND_32(BITRANGE(u, 20, 12), 9)
		: BITRANGE(u, 20, 12));; // const9
	    r->sel2=1; r->operand2 = BITRANGE(u, 31, 28); // c
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = BITRANGE(u, 31, 28); // c
	    r->disasm = "%d%, %s1%, %const9s%";
	    break;
	case OF_RC_u:
	    r->op2 = BITRANGE(u, 27, 21);
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=0; r->operand1 = BITRANGE(u, 20, 12); // const9u
	    r->sel2=0; r->operand2 = OPERAND_UNDEFINED; // 
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = BITRANGE(u, 31, 28); // c
	    r->disasm = "%d%, %s1%, %const9u%";
	    break;
	case OF_RCR:
	    r->op2 = BITRANGE(u, 23, 21);
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=0; r->operand1 = SIGN_EXTEND_32(BITRANGE(u, 20, 12), 9); // const9s
	    r->sel2=1; r->operand2 = BITRANGE(u, 27, 24); // d
	    r->sel3=1; r->operand3 = (BITRANGE(u, 27, 24)|1); // dodd
	    r->dest = BITRANGE(u, 31, 28); // c
	    r->disasm = "%d%, %s3%, %s1%, %const9s%";
	    break;
	case OF_RLC_s:
	    // no op2
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=0; r->operand1 = SIGN_EXTEND_32(BITRANGE(u, 27, 12), 16); // const16s
	    r->sel2=0; r->operand2 = OPERAND_UNDEFINED; // 
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = BITRANGE(u, 31, 28); // c
	    r->disasm = "%d%, (%s1%,) %const16s%";
	    break;
	case OF_RLC_u:
	    // no op2
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=0; r->operand1 = BITRANGE(u, 27, 12); // const16u
	    r->sel2=0; r->operand2 = OPERAND_UNDEFINED; // 
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = BITRANGE(u, 31, 28); // c
	    r->disasm = "%d%, (%s1%,) %const16u%";
	    break;
	case OF_RLC_h:
	    // no op2
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=0; r->operand1 = BITRANGE(u, 27, 12) << 16; // const16h
	    r->sel2=0; r->operand2 = OPERAND_UNDEFINED; // 
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = BITRANGE(u, 31, 28); // c
	    r->disasm = "%d%, (%s1%,) %const16u%";
	    break;
	case OF_RR:
	    r->op2 = BITRANGE(u, 27, 20);
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=1; r->operand1 = BITRANGE(u, 15, 12); // b
	    r->sel2=1; r->operand2 = BITRANGE(u, 31, 28); // c
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = BITRANGE(u, 31, 28); // c
	    r->disasm = "%d%, %s1%, %s2%, %n%";
	    break;
	case OF_RR2:
	    r->op2 = BITRANGE(u, 27, 16);
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=1; r->operand1 = BITRANGE(u, 15, 12); // b
	    r->sel2=0; r->operand2 = OPERAND_UNDEFINED; // 
	    r->sel3=0; r->operand3 = OPERAND_UNDEFINED; // 
	    r->dest = BITRANGE(u, 31, 28); // c
	    r->disasm = "%d%, %s1%, %s2%";
	    break;
	case OF_RRR:
	    r->op2 = BITRANGE(u, 23, 20);
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=1; r->operand1 = BITRANGE(u, 15, 12); // b
	    r->sel2=1; r->operand2 = BITRANGE(u, 27, 24); // d
	    r->sel3=1; r->operand3 = (BITRANGE(u, 27, 24)|1); // dodd
	    r->dest = BITRANGE(u, 31, 28); // c
	    r->disasm = "%d%, %s3%, %s1%, %s2%";
	    break;
	case OF_RRR2:
	    r->op2 = BITRANGE(u, 23, 16);
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=1; r->operand1 = BITRANGE(u, 15, 12); // b
	    r->sel2=1; r->operand2 = BITRANGE(u, 27, 24); // d
	    r->sel3=1; r->operand3 = (BITRANGE(u, 27, 24)|1); // dodd
	    r->dest = BITRANGE(u, 31, 28); // c
	    r->disasm = "%d%, %s1%, %s2%, %s3%";
	    break;
	case OF_RCPW:
	    r->op2 = BITRANGE(u, 22, 21);
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=0; r->operand1 = BITRANGE(u, 15, 12); // const4u
	    r->sel2=0; r->operand2 = BITRANGE(u, 27, 23); // p
	    r->sel3=0; r->operand3 = BITRANGE(u, 20, 16); // w
	    r->dest = BITRANGE(u, 31, 28); // c
	    r->disasm = "%d%, (%s1%,) %const4u%, %p%, %w%";
	    break;
	case OF_RCRR:
	    r->op2 = BITRANGE(u, 23, 21);
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=0; r->operand1 = BITRANGE(u, 15, 12); // const4u
	    r->sel2=1; r->operand2 = BITRANGE(u, 27, 24); // d
	    r->sel3=1; r->operand3 = (BITRANGE(u, 27, 24)|1); // dodd
	    r->dest = BITRANGE(u, 31, 28); // c
	    r->disasm = "%d%, (%s1%,) %const4u%, e%s3%";
	    break;
	case OF_RCRW:
	    r->op2 = BITRANGE(u, 23, 21);
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=0; r->operand1 = BITRANGE(u, 15, 12); // const4u
	    r->sel2=1; r->operand2 = BITRANGE(u, 27, 24); // d
	    r->sel3=0; r->operand3 = BITRANGE(u, 20, 16); // w
	    r->dest = BITRANGE(u, 31, 28); // c
	    r->disasm = "%d%, (%s1%,) %const4u%, %s3%, %w%";
	    break;
	case OF_RRPW:
	    r->op2 = BITRANGE(u, 22, 21);
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=1; r->operand1 = BITRANGE(u, 15, 12); // b
	    r->sel2=0; r->operand2 = BITRANGE(u, 27, 23); // p
	    r->sel3=0; r->operand3 = BITRANGE(u, 20, 16); // w
	    r->dest = BITRANGE(u, 31, 28); // c
	    r->disasm = "%d%, %s1%, %p%, %w%";
	    break;
	case OF_RRRR:
	    r->op2 = BITRANGE(u, 23, 21);
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=1; r->operand1 = BITRANGE(u, 15, 12); // b
	    r->sel2=1; r->operand2 = BITRANGE(u, 27, 24); // d
	    r->sel3=1; r->operand3 = (BITRANGE(u, 27, 24)|1); // dodd
	    r->dest = BITRANGE(u, 31, 28); // c
	    r->disasm = "%d%, %s1%, (%s2%,) e%s3%";
	    break;
	case OF_RRRW:
	    r->op2 = BITRANGE(u, 23, 21);
	    r->sel0=1; r->operand0 = BITRANGE(u, 11, 8); // a
	    r->sel1=1; r->operand1 = BITRANGE(u, 15, 12); // b
	    r->sel2=1; r->operand2 = BITRANGE(u, 27, 24); // d
	    r->sel3=0; r->operand3 = BITRANGE(u, 20, 16); // w
	    r->dest = BITRANGE(u, 31, 28); // c
	    r->disasm = "%d%, %s1%, (%s2%,) %s3%, %w%";
	    break;
	default:
	    break;
    }
}



inline opformat_t opcode_format(unsigned op1)
{
    switch (op1)
    {
	case 0x00: return OF_SR_A;
	case 0x01: return OF_RR_A;
	case 0x02: return OF_SRR;
	case 0x03: return OF_RRR2;
	case 0x04: return OF_SpR;
	case 0x05: return OF_ABS;
	case 0x06: return OF_SRC;
	case 0x07: return OF_BIT;
	case 0x08: return OF_SLRO;
	case 0x09: return OF_BO;
	case 0x0b: return OF_RR;
	case 0x0c: return OF_SRO;
	case 0x0d: return OF_BO;
	case 0x0e: return OF_SBR;
	case 0x0f: return OF_RR;
	case 0x10: return OF_SRO;
	case 0x11: return OF_RLC_h_A;
	case 0x12: return OF_SRR_a15b;
	case 0x13: return OF_RCR;
	case 0x14: return OF_SLR;
	case 0x16: return OF_SC_u;
	case 0x17: return OF_RRRR;
	case 0x19: return OF_BOL;
	case 0x1a: return OF_SRR_d;
	case 0x1b: return OF_RLC_s;
	case 0x1d: return OF_B;
	case 0x1e: return OF_SBC;
	case 0x1f: return OF_BRR;
	case 0x20: return OF_SC_suba;
	case 0x23: return OF_RRR2;
	case 0x24: return OF_SpR;
	case 0x25: return OF_ABS;
	case 0x26: return OF_SRR;
	case 0x27: return OF_BIT;
	case 0x28: return OF_SLRO;
	case 0x2a: return OF_SRR;
	case 0x2b: return OF_RRR;
	case 0x2c: return OF_SRO;
	case 0x2d: return OF_RR_A;
	case 0x2e: return OF_SBC;
	case 0x30: return OF_SRR;
	case 0x32: return OF_SR;
	case 0x33: return OF_RCR;
	case 0x34: return OF_SLR;
	case 0x37: return OF_RRPW;
	case 0x3a: return OF_SRR_d;
	case 0x3b: return OF_RLC_s;
	case 0x3c: return OF_SR_A;
	case 0x3e: return OF_SBR;
	case 0x3f: return OF_BRR;
	case 0x40: return OF_SR_A;
	case 0x42: return OF_SRR;
	case 0x44: return OF_SpR;
	case 0x45: return OF_ABS;
	case 0x46: return OF_SR;
	case 0x47: return OF_BIT;
	case 0x48: return OF_SLRO;
	case 0x49: return OF_BO;
	case 0x4b: return OF_RR;
	case 0x4c: return OF_SRO;
	case 0x4d: return OF_RLC_h_A;
	case 0x4e: return OF_SBR;
	case 0x50: return OF_SRO;
	case 0x52: return OF_SRR_a15b;
	case 0x53: return OF_RC;
	case 0x54: return OF_SLR;
	case 0x57: return OF_RRRW;
	case 0x58: return OF_SCo;
	case 0x59: return OF_BOL;
	case 0x5a: return OF_SRR_d;
	case 0x5c: return OF_SR_A;
	case 0x5d: return OF_B;
	case 0x5e: return OF_SBC;
	case 0x5f: return OF_BRR;
	case 0x60: return OF_SBR_A;
	case 0x64: return OF_SpR;
	case 0x65: return OF_ABS;
	case 0x67: return OF_BIT;
	case 0x68: return OF_SLRO;
	case 0x6a: return OF_SRR;
	case 0x6b: return OF_RRR;
	case 0x6c: return OF_SRO;
	case 0x6d: return OF_B;
	case 0x6e: return OF_SB;
	case 0x6f: return OF_BRR;
	case 0x73: return OF_RR2;
	case 0x74: return OF_SLR;
	case 0x76: return OF_SBR;
	case 0x77: return OF_RRPW;
	case 0x78: return OF_SCo;
	case 0x7a: return OF_SRR_d;
	case 0x7b: return OF_RLC_h;
	case 0x7c: return OF_SBR_A;
	case 0x7d: return OF_BRR_A;
	case 0x7e: return OF_SBR;
	case 0x7f: return OF_BRR;
	case 0x80: return OF_SR_A;
	case 0x82: return OF_SRC;
	case 0x84: return OF_SpR;
	case 0x85: return OF_ABS;
	case 0x86: return OF_SRC;
	case 0x87: return OF_BIT;
	case 0x88: return OF_SLRO;
	case 0x89: return OF_BO;
	case 0x8a: return OF_SRC;
	case 0x8b: return OF_RC;
	case 0x8c: return OF_SRO;
	case 0x8e: return OF_SBR;
	case 0x8f: return OF_RC_u;
	case 0x90: return OF_SRO;
	case 0x91: return OF_RLC_h_A;
	case 0x92: return OF_SBC;
	case 0x94: return OF_SLR;
	case 0x96: return OF_SC_u;
	case 0x97: return OF_RCRR;
	case 0x99: return OF_BOL;
	case 0x9a: return OF_SRC_d;
	case 0x9b: return OF_RLC_h;
	case 0x9d: return OF_B;
	case 0x9f: return OF_BRC;
	case 0xa0: return OF_SBR_A;
	case 0xa2: return OF_SRR;
	case 0xa4: return OF_SpR;
	case 0xa5: return OF_ABS;
	case 0xa6: return OF_SRR;
	case 0xa7: return OF_BIT;
	case 0xa8: return OF_SLRO;
	case 0xaa: return OF_SRC;
	case 0xab: return OF_RCR;
	case 0xac: return OF_SRO;
	case 0xae: return OF_SBC;
	case 0xb0: return OF_SRC_adda;
	case 0xb4: return OF_SLR;
	case 0xb7: return OF_RCPW;
	case 0xba: return OF_SRC_d;
	case 0xbb: return OF_RLC_u;
	case 0xbc: return OF_SBR_A;
	case 0xbd: return OF_BRR_A;
	case 0xbf: return OF_BRC;
	case 0xc2: return OF_SRC;
	case 0xc4: return OF_SpR;
	case 0xc5: return OF_ABS;
	case 0xc6: return OF_SRR;
	case 0xc7: return OF_BIT;
	case 0xc8: return OF_SLRO;
	case 0xca: return OF_SRC;
	case 0xcc: return OF_SRO;
	case 0xcd: return OF_RLC_h_A;
	case 0xce: return OF_SBR;
	case 0xd0: return OF_SRO;
	case 0xd4: return OF_SLR;
	case 0xd5: return OF_ABS;
	case 0xd7: return OF_RCRW;
	case 0xd8: return OF_SCo;
	case 0xd9: return OF_BOL;
	case 0xda: return OF_SC_u;
	case 0xdc: return OF_SR_A;
	case 0xdd: return OF_B;
	case 0xdf: return OF_BRC;
	case 0xe2: return OF_SRR;
	case 0xe4: return OF_SpR;
	case 0xe5: return OF_ABS;
	case 0xe8: return OF_SLRO;
	case 0xea: return OF_SRC;
	case 0xec: return OF_SRO;
	case 0xed: return OF_B;
	case 0xee: return OF_SB;
	case 0xef: return OF_BRR;
	case 0xf4: return OF_SLR;
	case 0xf6: return OF_SBR;
	case 0xf8: return OF_SCo;
	case 0xfa: return OF_SRC_d;
	case 0xfc: return OF_SBR_loop;
	case 0xfd: return OF_BRR_A;
	case 0xff: return OF_BRC;

	default: return OF_UNKNOWN;
    }
}

// Numbers of the microcodes
// Cannot be enums, as they are aliases to functional codes for the alus
// the first is reserved for mcNONE
#define MC_NONE	0
#define MC_CALL_L	(alu_func_t)1
#define MC_CALL_S	(alu_func_t)2
#define MC_CALLA	(alu_func_t)3
#define MC_CALLI	(alu_func_t)4
#define MC_LDMST	(alu_func_t)5
#define MC_LDMST_ABS	(alu_func_t)6
#define MC_RET	(alu_func_t)7
#define MC_STDA_PRE	(alu_func_t)8
#define MC_STDA_POST	(alu_func_t)9
#define MC_STDA_NORM	(alu_func_t)10
#define MC_STDA_ABS	(alu_func_t)11
#define MC_STT	(alu_func_t)12


inline int pd_pipeline(uint32_t instr) { return ((instr >> 1) & 1); }
inline int pd_width(uint32_t instr) { return (((instr << 1)&2)+2); }

inline bool pd_branch(uint32_t instr)
{
    switch (instr & 0xff)
    {
	case 0x0e:
	case 0x1d:
	case 0x1e:
	case 0x1f:
	case 0x2e:
	case 0x3c:
	case 0x3e:
	case 0x3f:
	case 0x4e:
	case 0x5d:
	case 0x5e:
	case 0x5f:
	case 0x6e:
	case 0x6f:
	case 0x76:
	case 0x7c:
	case 0x7d:
	case 0x7e:
	case 0x7f:
	case 0x8e:
	case 0x9d:
	case 0x9f:
	case 0xae:
	case 0xbc:
	case 0xbd:
	case 0xbf:
	case 0xce:
	case 0xdc:
	case 0xdd:
	case 0xdf:
	case 0xee:
	case 0xef:
	case 0xf6:
	case 0xfc:
	case 0xfd:
	case 0xff:
	    return true;
    }
    return false;
}

inline bool pd_load(uint32_t instr)
{
    /* dirty workaround for superflous lea aX, aY, off10 
       that only the Tasking compiler generates */
    if ((instr & 0x0fc000ff) == 0x0a000049) return false;

    switch (instr & 0xff)
    {
	case 0x04:
	case 0x05:
	case 0x08:
	case 0x09:
	case 0x0c:
	case 0x14:
	case 0x19:
	case 0x44:
	case 0x45:
	case 0x48:
	case 0x49:
	case 0x4c:
	case 0x54:
	case 0x58:
	case 0x84:
	case 0x85:
	case 0x88:
	case 0x8c:
	case 0x94:
	case 0x99:
	case 0xc4:
	case 0xc8:
	case 0xcc:
	case 0xd4:
	case 0xd5:
	case 0xd8:
	case 0xe5:
	    return true;
    }
    return false;
}

inline bool pd_store(uint32_t instr)
{
    switch (instr & 0xff)
    {
	case 0x24:
	case 0x25:
	case 0x28:
	case 0x2c:
	case 0x34:
	case 0x59:
	case 0x64:
	case 0x65:
	case 0x68:
	case 0x6c:
	case 0x74:
	case 0x78:
	case 0x89:
	case 0xa4:
	case 0xa5:
	case 0xa8:
	case 0xac:
	case 0xb4:
	case 0xe4:
	case 0xe8:
	case 0xec:
	case 0xf4:
	case 0xf8:
	    return true;
    }
    return false;
}

inline bool pd_lms(uint32_t instr)
{
    return (((instr & 0xff)==0xd5) /* st.t (ABS) */
//        || ((instr & 0xff)==0xe5) /* ldmst / swap.w (ABS) */
//        || ((instr & 0x038000ff)==0x00000049)); /* ldmst / swap.w (BO) */
        || ((instr & 0x0c0000ff)==0x040000e5) /* ldmst, NOT swap.w (ABS) */
        || ((instr & 0x03c000ff)==0x00400049)); /* ldmst, NOT swap.w (BO) */
}


#define POS_MC_CALL_L (1)
#define END_MC_CALL_L (18)
#define POS_MC_CALL_S (19)
#define END_MC_CALL_S (36)
#define POS_MC_CALLA (37)
#define END_MC_CALLA (54)
#define POS_MC_CALLI (55)
#define END_MC_CALLI (72)
#define POS_MC_LDMST (73)
#define END_MC_LDMST (75)
#define POS_MC_LDMST_ABS (76)
#define END_MC_LDMST_ABS (78)
#define POS_MC_RET (79)
#define END_MC_RET (100)
#define POS_MC_STDA_PRE (101)
#define END_MC_STDA_PRE (102)
#define POS_MC_STDA_POST (103)
#define END_MC_STDA_POST (104)
#define POS_MC_STDA_NORM (105)
#define END_MC_STDA_NORM (106)
#define POS_MC_STDA_ABS (107)
#define END_MC_STDA_ABS (108)
#define POS_MC_STT (109)
#define END_MC_STT (111)


inline unsigned pd_microcode(uint32_t instr)
{
    unsigned op1 = instr & 0xff;
    if (op1==0x6d) return POS_MC_CALL_L; /* MC_CALL_L */
    else if (op1==0x5c) return POS_MC_CALL_S; /* MC_CALL_S */
    else if (op1==0xed) return POS_MC_CALLA; /* MC_CALLA */
    else if (op1==0x2d && (BITRANGE(instr, 27, 20) == 0x00)) return POS_MC_CALLI; /* MC_CALLI */
    else if (op1==0x49 && (BITRANGE(instr, 27, 22) == 0x21)) return POS_MC_LDMST; /* MC_LDMST */
    else if (op1==0xe5 && (BITRANGE(instr, 27, 26) == 0x01)) return POS_MC_LDMST_ABS; /* MC_LDMST_ABS */
    else if (op1==0x00 && (BITRANGE(instr, 15, 12) == 0x09)) return POS_MC_RET; /* MC_RET */
    else if (op1==0x0d && (BITRANGE(instr, 27, 22) == 0x06)) return POS_MC_RET; /* MC_RET */
    else if (op1==0x89 && (BITRANGE(instr, 27, 22) == 0x17)) return POS_MC_STDA_PRE; /* MC_STDA_PRE */
    else if (op1==0x89 && (BITRANGE(instr, 27, 22) == 0x07)) return POS_MC_STDA_POST; /* MC_STDA_POST */
    else if (op1==0x89 && (BITRANGE(instr, 27, 22) == 0x27)) return POS_MC_STDA_NORM; /* MC_STDA_NORM */
    else if (op1==0xa5 && (BITRANGE(instr, 27, 26) == 0x03)) return POS_MC_STDA_ABS; /* MC_STDA_ABS */
    else if (op1==0xd5 && (BITRANGE(instr, 27, 26) == 0x00)) return POS_MC_STT; /* MC_STT */
    else return 0;
}



inline func_mode_t instrset_decode(uint32_t instr)
{
    func_mode_t f;
    f.format = OF_UNKNOWN;
    f.func = FUNC_UNKNOWN;
    f.cond = COND_1;
    f.xrd = 0;
    f.inswr = 0;
    f.wr = 1;
    f.csfrwr = 0;
    f.signext = false;
    f.memwidth = 0;

    switch (instr & 0xff)
    {
	case 0x00:
	    f.format = OF_SR_A;
	    switch (BITRANGE(instr, 15, 12))
	    {
		case 0x00: f.func = FUNC_NOP; f.inswr = 0; f.wr = 0; break;
		case 0x09: f.func = MC_RET; f.inswr = 0; f.wr = 1; break;
	    }
	    break;
	case 0x01:
	    f.format = OF_RR_A;
	    switch (BITRANGE(instr, 27, 20))
	    {
		case 0x00: f.func = A_SRC1; f.inswr = 0; f.wr = 1; break;
		case 0x01: f.func = A_ADD; f.inswr = 0; f.wr = 1; break;
		case 0x02: f.func = A_SUB; f.inswr = 0; f.wr = 1; break;
		case 0x40: f.func = A_SETC; f.cond = COND_EQ; f.inswr = 0; f.wr = 4; break;
		case 0x41: f.func = A_SETC; f.cond = COND_NE; f.inswr = 0; f.wr = 4; break;
		case 0x42: f.func = A_SETC; f.cond = COND_LT; f.inswr = 0; f.wr = 4; break;
		case 0x43: f.func = A_SETC; f.cond = COND_GE; f.inswr = 0; f.wr = 4; break;
		case 0x48: f.func = A_SETC; f.cond = COND_EQZ; f.inswr = 0; f.wr = 4; break;
		case 0x49: f.func = A_SETC; f.cond = COND_NEZ; f.inswr = 0; f.wr = 4; break;
		case 0x4c: f.func = A_SRC1; f.inswr = 0; f.wr = 4; break;
		case 0x60: f.func = A_ADDSC_AL; f.xrd = 1; f.inswr = 0; f.wr = 1; break;
		case 0x62: f.func = A_ADDSC_AT; f.xrd = 1; f.inswr = 0; f.wr = 1; break;
		case 0x63: f.func = A_XSRC; f.xrd = 1; f.inswr = 0; f.wr = 1; break;
	    }
	    break;
	case 0x02:
	    f.format = OF_SRR; f.func = D_SRC1; f.inswr = 0; f.wr = 1; break;
	case 0x03:
	    f.format = OF_RRR2;
	    switch (BITRANGE(instr, 23, 16))
	    {
		case 0x0a: f.func = D_MADD32; f.inswr = 0; f.wr = 1; break;
		case 0x68: f.func = D_MADDU; f.inswr = 0; f.wr = 3; break;
	    }
	    break;
	case 0x04:
	    f.format = OF_SpR; f.func = A_POST; f.inswr = 4; f.wr = 1; f.memwidth = MW_8u; break;
	case 0x05:
	    f.format = OF_ABS;
	    switch (BITRANGE(instr, 27, 26))
	    {
		case 0x00: f.func = A_PRE; f.inswr = 4; f.wr = 0; f.memwidth = MW_8s; break;
		case 0x01: f.func = A_PRE; f.inswr = 4; f.wr = 0; f.memwidth = MW_8u; break;
		case 0x02: f.func = A_PRE; f.inswr = 4; f.wr = 0; f.memwidth = MW_16s; break;
		case 0x03: f.func = A_PRE; f.inswr = 4; f.wr = 0; f.memwidth = MW_16u; break;
	    }
	    break;
	case 0x06:
	    f.format = OF_SRC; f.func = D_SH; f.inswr = 0; f.wr = 1; break;
	case 0x07:
	    f.format = OF_BIT;
	    switch (BITRANGE(instr, 22, 21))
	    {
		case 0x00: f.func = D_SETC; f.cond = COND_NAND; f.inswr = 0; f.wr = 1; break;
		case 0x01: f.func = D_SETC; f.cond = COND_ORN; f.inswr = 0; f.wr = 1; break;
		case 0x02: f.func = D_SETC; f.cond = COND_XNOR; f.inswr = 0; f.wr = 1; break;
		case 0x03: f.func = D_SETC; f.cond = COND_XOR; f.inswr = 0; f.wr = 1; break;
	    }
	    break;
	case 0x08:
	    f.format = OF_SLRO; f.func = A_PRE; f.inswr = 4; f.wr = 0; f.memwidth = MW_8u; break;
	case 0x09:
	    f.format = OF_BO;
	    switch (BITRANGE(instr, 27, 22))
	    {
		case 0x00: f.func = A_POST; f.inswr = 4; f.wr = 1; f.memwidth = MW_8s; break;
		case 0x01: f.func = A_POST; f.inswr = 4; f.wr = 1; f.memwidth = MW_8u; break;
		case 0x02: f.func = A_POST; f.inswr = 4; f.wr = 1; f.memwidth = MW_16s; break;
		case 0x03: f.func = A_POST; f.inswr = 4; f.wr = 1; f.memwidth = MW_16u; break;
		case 0x04: f.func = A_POST; f.inswr = 4; f.wr = 1; f.memwidth = MW_32; break;
		case 0x05: f.func = A_POST; f.inswr = 12; f.wr = 1; f.memwidth = MW_64; break;
		case 0x06: f.func = A_POST; f.inswr = 1; f.wr = 1; f.memwidth = MW_32; break;
		case 0x07: f.func = A_POST; f.inswr = 3; f.wr = 1; f.memwidth = MW_64; break;
		case 0x08: f.func = A_POST; f.inswr = 4; f.wr = 1; f.memwidth = MW_16h; break;
		case 0x10: f.func = A_PRE; f.inswr = 4; f.wr = 1; f.memwidth = MW_8s; break;
		case 0x11: f.func = A_PRE; f.inswr = 4; f.wr = 1; f.memwidth = MW_8u; break;
		case 0x12: f.func = A_PRE; f.inswr = 4; f.wr = 1; f.memwidth = MW_16s; break;
		case 0x13: f.func = A_PRE; f.inswr = 4; f.wr = 1; f.memwidth = MW_16u; break;
		case 0x14: f.func = A_PRE; f.inswr = 4; f.wr = 1; f.memwidth = MW_32; break;
		case 0x15: f.func = A_PRE; f.inswr = 12; f.wr = 1; f.memwidth = MW_64; break;
		case 0x16: f.func = A_PRE; f.inswr = 1; f.wr = 1; f.memwidth = MW_32; break;
		case 0x17: f.func = A_PRE; f.inswr = 3; f.wr = 1; f.memwidth = MW_64; break;
		case 0x18: f.func = A_PRE; f.inswr = 4; f.wr = 1; f.memwidth = MW_16h; break;
		case 0x20: f.func = A_PRE; f.inswr = 4; f.wr = 0; f.memwidth = MW_8s; break;
		case 0x21: f.func = A_PRE; f.inswr = 4; f.wr = 0; f.memwidth = MW_8u; break;
		case 0x22: f.func = A_PRE; f.inswr = 4; f.wr = 0; f.memwidth = MW_16s; break;
		case 0x23: f.func = A_PRE; f.inswr = 4; f.wr = 0; f.memwidth = MW_16u; break;
		case 0x24: f.func = A_PRE; f.inswr = 4; f.wr = 0; f.memwidth = MW_32; break;
		case 0x25: f.func = A_PRE; f.inswr = 12; f.wr = 0; f.memwidth = MW_64; break;
		case 0x26: f.func = A_PRE; f.inswr = 1; f.wr = 0; f.memwidth = MW_32; break;
		case 0x27: f.func = A_PRE; f.inswr = 3; f.wr = 0; f.memwidth = MW_64; break;
		case 0x28: f.func = A_PRE; f.inswr = 4; f.wr = 0; f.memwidth = MW_16h; break;
	    }
	    break;
	case 0x0b:
	    f.format = OF_RR;
	    switch (BITRANGE(instr, 27, 20))
	    {
		case 0x00: f.func = D_ADD; f.inswr = 0; f.wr = 1; break;
		case 0x04: f.func = D_ADDX; f.inswr = 0; f.wr = 1; break;
		case 0x05: f.func = D_ADDC; f.inswr = 0; f.wr = 1; break;
		case 0x08: f.func = D_SUB; f.inswr = 0; f.wr = 1; break;
		case 0x0c: f.func = D_SUBX; f.inswr = 0; f.wr = 1; break;
		case 0x0d: f.func = D_SUBC; f.inswr = 0; f.wr = 1; break;
		case 0x0e: f.func = D_ABSDIF; f.cond = COND_LT; f.inswr = 0; f.wr = 1; break;
		case 0x10: f.func = D_SETC; f.cond = COND_EQ; f.inswr = 0; f.wr = 1; break;
		case 0x11: f.func = D_SETC; f.cond = COND_NE; f.inswr = 0; f.wr = 1; break;
		case 0x12: f.func = D_SETC; f.cond = COND_LT; f.inswr = 0; f.wr = 1; break;
		case 0x13: f.func = D_SETC; f.cond = COND_LTU; f.inswr = 0; f.wr = 1; break;
		case 0x14: f.func = D_SETC; f.cond = COND_GE; f.inswr = 0; f.wr = 1; break;
		case 0x15: f.func = D_SETC; f.cond = COND_GEU; f.inswr = 0; f.wr = 1; break;
		case 0x18: f.func = D_AORB; f.cond = COND_GE; f.inswr = 0; f.wr = 1; break;
		case 0x19: f.func = D_AORB; f.cond = COND_GEU; f.inswr = 0; f.wr = 1; break;
		case 0x1a: f.func = D_AORB; f.cond = COND_LT; f.inswr = 0; f.wr = 1; break;
		case 0x1b: f.func = D_AORB; f.cond = COND_LTU; f.inswr = 0; f.wr = 1; break;
		case 0x1c: f.func = D_ABS_W; f.inswr = 0; f.wr = 1; break;
		case 0x1f: f.func = D_SRC1; f.inswr = 0; f.wr = 1; break;
		case 0x20: f.func = D_ANDC; f.cond = COND_EQ; f.inswr = 0; f.wr = 1; break;
		case 0x21: f.func = D_ANDC; f.cond = COND_NE; f.inswr = 0; f.wr = 1; break;
		case 0x22: f.func = D_ANDC; f.cond = COND_LT; f.inswr = 0; f.wr = 1; break;
		case 0x23: f.func = D_ANDC; f.cond = COND_LTU; f.inswr = 0; f.wr = 1; break;
		case 0x24: f.func = D_ANDC; f.cond = COND_GE; f.inswr = 0; f.wr = 1; break;
		case 0x25: f.func = D_ANDC; f.cond = COND_GEU; f.inswr = 0; f.wr = 1; break;
		case 0x27: f.func = D_ORC; f.cond = COND_EQ; f.inswr = 0; f.wr = 1; break;
		case 0x28: f.func = D_ORC; f.cond = COND_NE; f.inswr = 0; f.wr = 1; break;
		case 0x29: f.func = D_ORC; f.cond = COND_LT; f.inswr = 0; f.wr = 1; break;
		case 0x2a: f.func = D_ORC; f.cond = COND_LTU; f.inswr = 0; f.wr = 1; break;
		case 0x2b: f.func = D_ORC; f.cond = COND_GE; f.inswr = 0; f.wr = 1; break;
		case 0x2c: f.func = D_ORC; f.cond = COND_GEU; f.inswr = 0; f.wr = 1; break;
		case 0x2f: f.func = D_XORC; f.cond = COND_EQ; f.inswr = 0; f.wr = 1; break;
		case 0x30: f.func = D_XORC; f.cond = COND_NE; f.inswr = 0; f.wr = 1; break;
		case 0x31: f.func = D_XORC; f.cond = COND_LT; f.inswr = 0; f.wr = 1; break;
		case 0x32: f.func = D_XORC; f.cond = COND_LTU; f.inswr = 0; f.wr = 1; break;
		case 0x33: f.func = D_XORC; f.cond = COND_GE; f.inswr = 0; f.wr = 1; break;
		case 0x34: f.func = D_XORC; f.cond = COND_GEU; f.inswr = 0; f.wr = 1; break;
		case 0x37: f.func = D_SHC; f.cond = COND_EQ; f.inswr = 0; f.wr = 1; break;
		case 0x38: f.func = D_SHC; f.cond = COND_NE; f.inswr = 0; f.wr = 1; break;
		case 0x39: f.func = D_SHC; f.cond = COND_LT; f.inswr = 0; f.wr = 1; break;
		case 0x3a: f.func = D_SHC; f.cond = COND_LTU; f.inswr = 0; f.wr = 1; break;
		case 0x3b: f.func = D_SHC; f.cond = COND_GE; f.inswr = 0; f.wr = 1; break;
		case 0x3c: f.func = D_SHC; f.cond = COND_GEU; f.inswr = 0; f.wr = 1; break;
		case 0x56: f.func = D_EQANY_B; f.inswr = 0; f.wr = 1; break;
		case 0x76: f.func = D_EQANY_H; f.inswr = 0; f.wr = 1; break;
	    }
	    break;
	case 0x0c:
	    f.format = OF_SRO; f.func = A_PRE; f.inswr = 4; f.wr = 0; f.memwidth = MW_8u; break;
	case 0x0d:
	    f.format = OF_BO;
	    switch (BITRANGE(instr, 27, 22))
	    {
		case 0x00: f.func = FUNC_NOP; f.inswr = 0; f.wr = 0; break;
		case 0x06: f.func = MC_RET; f.inswr = 0; f.wr = 1; break;
		case 0x0c: f.func = FUNC_NOP; f.inswr = 0; f.wr = 0; break;
		case 0x0d: f.func = FUNC_NOP; f.inswr = 0; f.wr = 0; break;
		case 0x11: f.func = FUNC_NOP; f.inswr = 0; f.wr = 0; break;
		case 0x12: f.func = FUNC_NOP; f.inswr = 0; f.wr = 0; break;
		case 0x13: f.func = FUNC_NOP; f.inswr = 0; f.wr = 0; break;
		case 0x20: f.func = C_SWAPOUT; f.inswr = 0; f.wr = 1; break;
		case 0x21: f.func = C_SWAPIN; f.inswr = 0; f.wr = 1; break;
	    }
	    break;
	case 0x0e:
	    f.format = OF_SBR; f.func = D_J; f.cond = COND_LTZ; f.inswr = 0; f.wr = 0; break;
	case 0x0f:
	    f.format = OF_RR;
	    switch (BITRANGE(instr, 27, 20))
	    {
		case 0x00: f.func = D_SH; f.inswr = 0; f.wr = 1; break;
		case 0x01: f.func = D_SHA; f.inswr = 0; f.wr = 1; break;
		case 0x08: f.func = D_AND; f.inswr = 0; f.wr = 1; break;
		case 0x09: f.func = D_NAND; f.inswr = 0; f.wr = 1; break;
		case 0x0a: f.func = D_OR; f.inswr = 0; f.wr = 1; break;
		case 0x0b: f.func = D_NOR; f.inswr = 0; f.wr = 1; break;
		case 0x0c: f.func = D_XOR; f.inswr = 0; f.wr = 1; break;
		case 0x0d: f.func = D_XNOR; f.inswr = 0; f.wr = 1; break;
		case 0x0e: f.func = D_ANDN; f.inswr = 0; f.wr = 1; break;
		case 0x0f: f.func = D_ORN; f.inswr = 0; f.wr = 1; break;
		case 0x1b: f.func = D_CLZ; f.inswr = 0; f.wr = 1; break;
		case 0x1c: f.func = D_CLO; f.inswr = 0; f.wr = 1; break;
		case 0x1d: f.func = D_CLS; f.inswr = 0; f.wr = 1; break;
	    }
	    break;
	case 0x10:
	    f.format = OF_SRO; f.func = A_ADDSC_A; f.xrd = 1; f.inswr = 0; f.wr = 1; break;
	case 0x11:
	    f.format = OF_RLC_h_A; f.func = A_ADD; f.inswr = 0; f.wr = 1; break;
	case 0x12:
	    f.format = OF_SRR_a15b; f.func = D_ADD; f.inswr = 0; f.wr = 1; break;
	case 0x13:
	    f.format = OF_RCR;
	    switch (BITRANGE(instr, 23, 21))
	    {
		case 0x01: f.func = D_MADD32; f.inswr = 0; f.wr = 1; break;
		case 0x02: f.func = D_MADDU; f.inswr = 0; f.wr = 3; break;
	    }
	    break;
	case 0x14:
	    f.format = OF_SLR; f.func = A_PRE; f.inswr = 4; f.wr = 0; f.memwidth = MW_8u; break;
	case 0x16:
	    f.format = OF_SC_u; f.func = D_AND; f.inswr = 0; f.wr = 1; break;
	case 0x17:
	    f.format = OF_RRRR;
	    switch (BITRANGE(instr, 23, 21))
	    {
		case 0x00: f.func = D_INSERT; f.inswr = 0; f.wr = 1; break;
		case 0x02: f.func = D_EXTR; f.inswr = 0; f.wr = 1; break;
		case 0x03: f.func = D_EXTR_U; f.inswr = 0; f.wr = 1; break;
		case 0x04: f.func = D_DEXTR; f.inswr = 0; f.wr = 1; break;
	    }
	    break;
	case 0x19:
	    f.format = OF_BOL; f.func = A_PRE; f.inswr = 4; f.wr = 0; f.memwidth = MW_32; break;
	case 0x1a:
	    f.format = OF_SRR_d; f.func = D_ADD; f.inswr = 0; f.wr = 1; break;
	case 0x1b:
	    f.format = OF_RLC_s; f.func = D_ADD; f.inswr = 0; f.wr = 1; break;
	case 0x1d:
	    f.format = OF_B; f.func = A_J4; f.cond = COND_1; f.inswr = 0; f.wr = 0; break;
	case 0x1e:
	    f.format = OF_SBC; f.func = D_J; f.cond = COND_EQ; f.inswr = 0; f.wr = 0; break;
	case 0x1f:
	    f.format = OF_BRR;
	    switch (BITRANGE(instr, 31, 31))
	    {
		case 0x00: f.func = D_JNEI; f.cond = COND_NE; f.inswr = 0; f.wr = 1; break;
		case 0x01: f.func = D_JNED; f.cond = COND_NE; f.inswr = 0; f.wr = 1; break;
	    }
	    break;
	case 0x20:
	    f.format = OF_SC_suba; f.func = A_SUB; f.inswr = 0; f.wr = 1; break;
	case 0x23:
	    f.format = OF_RRR2;
	    switch (BITRANGE(instr, 23, 16))
	    {
		case 0x0a: f.func = D_MSUB32; f.inswr = 0; f.wr = 1; break;
		case 0x68: f.func = D_MSUBU; f.inswr = 0; f.wr = 3; break;
	    }
	    break;
	case 0x24:
	    f.format = OF_SpR; f.func = A_POST; f.xrd = 1; f.inswr = 0; f.wr = 1; f.memwidth = MW_8u; break;
	case 0x25:
	    f.format = OF_ABS;
	    switch (BITRANGE(instr, 27, 26))
	    {
		case 0x00: f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 0; f.memwidth = MW_8u; break;
		case 0x02: f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 0; f.memwidth = MW_16u; break;
	    }
	    break;
	case 0x26:
	    f.format = OF_SRR; f.func = D_AND; f.inswr = 0; f.wr = 1; break;
	case 0x27:
	    f.format = OF_BIT;
	    switch (BITRANGE(instr, 22, 21))
	    {
		case 0x00: f.func = D_SHC; f.cond = COND_AND; f.inswr = 0; f.wr = 1; break;
		case 0x01: f.func = D_SHC; f.cond = COND_OR; f.inswr = 0; f.wr = 1; break;
		case 0x02: f.func = D_SHC; f.cond = COND_NOR; f.inswr = 0; f.wr = 1; break;
		case 0x03: f.func = D_SHC; f.cond = COND_ANDN; f.inswr = 0; f.wr = 1; break;
	    }
	    break;
	case 0x28:
	    f.format = OF_SLRO; f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 0; f.memwidth = MW_8u; break;
	case 0x2a:
	    f.format = OF_SRR; f.func = D_SELN; f.inswr = 0; f.wr = 1; break;
	case 0x2b:
	    f.format = OF_RRR;
	    switch (BITRANGE(instr, 23, 20))
	    {
		case 0x00: f.func = D_CADD; f.inswr = 0; f.wr = 1; break;
		case 0x01: f.func = D_CADDN; f.inswr = 0; f.wr = 1; break;
		case 0x02: f.func = D_CSUB; f.inswr = 0; f.wr = 1; break;
		case 0x03: f.func = D_CSUBN; f.inswr = 0; f.wr = 1; break;
		case 0x04: f.func = D_SEL; f.inswr = 0; f.wr = 1; break;
		case 0x05: f.func = D_SELN; f.inswr = 0; f.wr = 1; break;
	    }
	    break;
	case 0x2c:
	    f.format = OF_SRO; f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 0; f.memwidth = MW_8u; break;
	case 0x2d:
	    f.format = OF_RR_A;
	    switch (BITRANGE(instr, 27, 20))
	    {
		case 0x00: f.func = MC_CALLI; f.inswr = 0; f.wr = 1; break;
		case 0x02: f.func = A_JI; f.cond = COND_1; f.inswr = 0; f.wr = 1; break;
		case 0x03: f.func = A_JI; f.cond = COND_1; f.inswr = 0; f.wr = 0; break;
	    }
	    break;
	case 0x2e:
	    f.format = OF_SBC; f.func = D_J; f.cond = COND_SJZT; f.inswr = 0; f.wr = 0; break;
	case 0x30:
	    f.format = OF_SRR; f.func = A_ADD; f.inswr = 0; f.wr = 1; break;
	case 0x32:
	    f.format = OF_SR; f.func = D_RSUB; f.inswr = 0; f.wr = 1; break;
	case 0x33:
	    f.format = OF_RCR;
	    switch (BITRANGE(instr, 23, 21))
	    {
		case 0x01: f.func = D_MSUB32; f.inswr = 0; f.wr = 1; break;
		case 0x02: f.func = D_MSUBU; f.inswr = 0; f.wr = 3; break;
	    }
	    break;
	case 0x34:
	    f.format = OF_SLR; f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 0; f.memwidth = MW_8u; break;
	case 0x37:
	    f.format = OF_RRPW;
	    switch (BITRANGE(instr, 22, 21))
	    {
		case 0x00: f.func = D_INSERT; f.inswr = 0; f.wr = 1; break;
		case 0x01: f.func = D_IMASK; f.inswr = 0; f.wr = 3; break;
		case 0x02: f.func = D_EXTR; f.inswr = 0; f.wr = 1; break;
		case 0x03: f.func = D_EXTR_U; f.inswr = 0; f.wr = 1; break;
	    }
	    break;
	case 0x3a:
	    f.format = OF_SRR_d; f.func = D_SETC; f.cond = COND_EQ; f.inswr = 0; f.wr = 1; break;
	case 0x3b:
	    f.format = OF_RLC_s; f.func = D_SRC1; f.inswr = 0; f.wr = 1; break;
	case 0x3c:
	    f.format = OF_SR_A; f.func = A_J2; f.cond = COND_1; f.inswr = 0; f.wr = 0; break;
	case 0x3e:
	    f.format = OF_SBR; f.func = D_J; f.cond = COND_EQ; f.inswr = 0; f.wr = 0; break;
	case 0x3f:
	    f.format = OF_BRR;
	    switch (BITRANGE(instr, 31, 31))
	    {
		case 0x00: f.func = D_J; f.cond = COND_LT; f.inswr = 0; f.wr = 0; break;
		case 0x01: f.func = D_J; f.cond = COND_LTU; f.inswr = 0; f.wr = 0; break;
	    }
	    break;
	case 0x40:
	    f.format = OF_SR_A; f.func = A_SRC1; f.inswr = 0; f.wr = 1; break;
	case 0x42:
	    f.format = OF_SRR; f.func = D_ADD; f.inswr = 0; f.wr = 1; break;
	case 0x44:
	    f.format = OF_SpR; f.func = A_POST; f.inswr = 4; f.wr = 1; f.memwidth = MW_32; break;
	case 0x45:
	    f.format = OF_ABS; f.func = A_PRE; f.inswr = 4; f.wr = 0; f.memwidth = MW_16h; break;
	case 0x46:
	    f.format = OF_SR; f.func = D_NOR; f.inswr = 0; f.wr = 1; break;
	case 0x47:
	    f.format = OF_BIT;
	    switch (BITRANGE(instr, 22, 21))
	    {
		case 0x00: f.func = D_ANDC; f.cond = COND_AND; f.inswr = 0; f.wr = 1; break;
		case 0x01: f.func = D_ANDC; f.cond = COND_OR; f.inswr = 0; f.wr = 1; break;
		case 0x02: f.func = D_ANDC; f.cond = COND_NOR; f.inswr = 0; f.wr = 1; break;
		case 0x03: f.func = D_ANDC; f.cond = COND_ANDN; f.inswr = 0; f.wr = 1; break;
	    }
	    break;
	case 0x48:
	    f.format = OF_SLRO; f.func = A_PRE; f.inswr = 4; f.wr = 0; f.memwidth = MW_32; break;
	case 0x49:
	    f.format = OF_BO;
	    switch (BITRANGE(instr, 27, 22))
	    {
		case 0x20: f.func = A_PRE; f.xrd = 1; f.inswr = 4; f.wr = 0; f.memwidth = MW_32; break;
		case 0x21: f.func = MC_LDMST; f.inswr = 0; f.wr = 1; break;
		case 0x28: f.func = A_PRE; f.inswr = 0; f.wr = 1; break;
	    }
	    break;
	case 0x4b:
	    f.format = OF_RR;
	    switch (BITRANGE(instr, 27, 20))
	    {
		case 0x00: f.func = F_CMP; f.inswr = 0; f.wr = 1; break;
		case 0x01: f.func = D_BMERGE; f.inswr = 0; f.wr = 1; break;
		case 0x02: f.func = D_PARITY; f.inswr = 0; f.wr = 1; break;
		case 0x04: f.func = F_MUL; f.inswr = 0; f.wr = 1; break;
		case 0x05: f.func = F_DIV; f.inswr = 0; f.wr = 1; break;
		case 0x08: f.func = D_UNPACK; f.inswr = 0; f.wr = 3; break;
		case 0x09: f.func = D_BSPLIT; f.inswr = 0; f.wr = 3; break;
		case 0x0a: f.func = D_DVINIT_U; f.inswr = 0; f.wr = 3; break;
		case 0x0c: f.func = F_UPDFL; f.inswr = 0; f.wr = 1; break;
		case 0x10: f.func = F_FTOI; f.inswr = 0; f.wr = 1; break;
		case 0x11: f.func = F_FTOQ31; f.inswr = 0; f.wr = 1; break;
		case 0x12: f.func = F_FTOU; f.inswr = 0; f.wr = 1; break;
		case 0x13: f.func = F_FTOIZ; f.inswr = 0; f.wr = 1; break;
		case 0x14: f.func = F_ITOF; f.inswr = 0; f.wr = 1; break;
		case 0x15: f.func = F_Q31TOF; f.inswr = 0; f.wr = 1; break;
		case 0x16: f.func = F_UTOF; f.inswr = 0; f.wr = 1; break;
		case 0x17: f.func = F_FTOUZ; f.inswr = 0; f.wr = 1; break;
		case 0x18: f.func = F_FTOQ31Z; f.inswr = 0; f.wr = 1; break;
		case 0x19: f.func = F_QSEED; f.inswr = 0; f.wr = 1; break;
		case 0x1a: f.func = D_DVINIT; f.inswr = 0; f.wr = 3; break;
		case 0x2a: f.func = D_DVINIT_HU; f.inswr = 0; f.wr = 3; break;
		case 0x3a: f.func = D_DVINIT_H; f.inswr = 0; f.wr = 3; break;
		case 0x4a: f.func = D_DVINIT_BU; f.inswr = 0; f.wr = 3; break;
		case 0x5a: f.func = D_DVINIT_B; f.inswr = 0; f.wr = 3; break;
	    }
	    break;
	case 0x4c:
	    f.format = OF_SRO; f.func = A_PRE; f.inswr = 4; f.wr = 0; f.memwidth = MW_32; break;
	case 0x4d:
	    f.format = OF_RLC_h_A; f.func = A_MFCR; f.inswr = 0; f.wr = 4; break;
	case 0x4e:
	    f.format = OF_SBR; f.func = D_J; f.cond = COND_GTZ; f.inswr = 0; f.wr = 0; break;
	case 0x50:
	    f.format = OF_SRO; f.func = A_ADDSC_A; f.xrd = 1; f.inswr = 0; f.wr = 1; break;
	case 0x52:
	    f.format = OF_SRR_a15b; f.func = D_SUB; f.inswr = 0; f.wr = 1; break;
	case 0x53:
	    f.format = OF_RC;
	    switch (BITRANGE(instr, 27, 21))
	    {
		case 0x01: f.func = D_MUL; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x02: f.func = D_MUL_U; f.inswr = 0; f.wr = 3; f.signext = false; break;
		case 0x03: f.func = D_MUL; f.inswr = 0; f.wr = 3; f.signext = true; break;
	    }
	    break;
	case 0x54:
	    f.format = OF_SLR; f.func = A_PRE; f.inswr = 4; f.wr = 0; f.memwidth = MW_32; break;
	case 0x57:
	    f.format = OF_RRRW;
	    switch (BITRANGE(instr, 23, 21))
	    {
		case 0x00: f.func = D_INSERT; f.inswr = 0; f.wr = 1; break;
		case 0x01: f.func = D_IMASK; f.inswr = 0; f.wr = 3; break;
		case 0x02: f.func = D_EXTR; f.inswr = 0; f.wr = 1; break;
		case 0x03: f.func = D_EXTR_U; f.inswr = 0; f.wr = 1; break;
	    }
	    break;
	case 0x58:
	    f.format = OF_SCo; f.func = A_PRE; f.inswr = 4; f.wr = 0; f.memwidth = MW_32; break;
	case 0x59:
	    f.format = OF_BOL; f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 0; f.memwidth = MW_32; break;
	case 0x5a:
	    f.format = OF_SRR_d; f.func = D_SUB; f.inswr = 0; f.wr = 1; break;
	case 0x5c:
	    f.format = OF_SR_A; f.func = MC_CALL_S; f.inswr = 0; f.wr = 1; break;
	case 0x5d:
	    f.format = OF_B; f.func = A_J4; f.cond = COND_1; f.inswr = 0; f.wr = 1; break;
	case 0x5e:
	    f.format = OF_SBC; f.func = D_J; f.cond = COND_NE; f.inswr = 0; f.wr = 0; break;
	case 0x5f:
	    f.format = OF_BRR;
	    switch (BITRANGE(instr, 31, 31))
	    {
		case 0x00: f.func = D_J; f.cond = COND_EQ; f.inswr = 0; f.wr = 0; break;
		case 0x01: f.func = D_J; f.cond = COND_NE; f.inswr = 0; f.wr = 0; break;
	    }
	    break;
	case 0x60:
	    f.format = OF_SBR_A; f.func = A_XSRC; f.xrd = 1; f.inswr = 0; f.wr = 1; break;
	case 0x64:
	    f.format = OF_SpR; f.func = A_POST; f.xrd = 1; f.inswr = 0; f.wr = 1; f.memwidth = MW_32; break;
	case 0x65:
	    f.format = OF_ABS; f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 0; f.memwidth = MW_16u; break;
	case 0x67:
	    f.format = OF_BIT;
	    switch (BITRANGE(instr, 22, 21))
	    {
		case 0x00: f.func = D_INSC; f.cond = COND_XOR; f.inswr = 0; f.wr = 1; break;
		case 0x01: f.func = D_INSC; f.cond = COND_XNOR; f.inswr = 0; f.wr = 1; break;
	    }
	    break;
	case 0x68:
	    f.format = OF_SLRO; f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 0; f.memwidth = MW_32; break;
	case 0x6a:
	    f.format = OF_SRR; f.func = D_SEL; f.inswr = 0; f.wr = 1; break;
	case 0x6b:
	    f.format = OF_RRR;
	    switch (BITRANGE(instr, 23, 20))
	    {
		case 0x00: f.func = D_PACK; f.inswr = 0; f.wr = 1; break;
		case 0x02: f.func = F_ADD; f.inswr = 0; f.wr = 1; break;
		case 0x03: f.func = F_SUB; f.inswr = 0; f.wr = 1; break;
		case 0x06: f.func = F_MADD; f.inswr = 0; f.wr = 1; break;
		case 0x07: f.func = F_MSUB; f.inswr = 0; f.wr = 1; break;
		case 0x0d: f.func = D_DVADJ; f.inswr = 0; f.wr = 3; break;
		case 0x0e: f.func = D_DVSTEP_U; f.inswr = 0; f.wr = 3; break;
		case 0x0f: f.func = D_DVSTEP; f.inswr = 0; f.wr = 3; break;
	    }
	    break;
	case 0x6c:
	    f.format = OF_SRO; f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 0; f.memwidth = MW_32; break;
	case 0x6d:
	    f.format = OF_B; f.func = MC_CALL_L; f.inswr = 0; f.wr = 1; break;
	case 0x6e:
	    f.format = OF_SB; f.func = D_J; f.cond = COND_EQZ; f.inswr = 0; f.wr = 0; break;
	case 0x6f:
	    f.format = OF_BRR;
	    switch (BITRANGE(instr, 31, 31))
	    {
		case 0x00: f.func = D_J; f.cond = COND_JZT; f.inswr = 0; f.wr = 0; break;
		case 0x01: f.func = D_J; f.cond = COND_JNZT; f.inswr = 0; f.wr = 0; break;
	    }
	    break;
	case 0x73:
	    f.format = OF_RR2;
	    switch (BITRANGE(instr, 27, 16))
	    {
		case 0x0a: f.func = D_MUL; f.inswr = 0; f.wr = 1; break;
		case 0x68: f.func = D_MUL_U; f.inswr = 0; f.wr = 3; break;
		case 0x6a: f.func = D_MUL; f.inswr = 0; f.wr = 3; break;
	    }
	    break;
	case 0x74:
	    f.format = OF_SLR; f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 0; f.memwidth = MW_32; break;
	case 0x76:
	    f.format = OF_SBR; f.func = D_J; f.cond = COND_EQZ; f.inswr = 0; f.wr = 0; break;
	case 0x77:
	    f.format = OF_RRPW; f.func = D_DEXTR; f.inswr = 0; f.wr = 1; break;
	case 0x78:
	    f.format = OF_SCo; f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 0; f.memwidth = MW_32; break;
	case 0x7a:
	    f.format = OF_SRR_d; f.func = D_SETC; f.cond = COND_LT; f.inswr = 0; f.wr = 1; break;
	case 0x7b:
	    f.format = OF_RLC_h; f.func = D_SRC1; f.inswr = 0; f.wr = 1; break;
	case 0x7c:
	    f.format = OF_SBR_A; f.func = A_J4; f.cond = COND_NEZ; f.inswr = 0; f.wr = 0; break;
	case 0x7d:
	    f.format = OF_BRR_A;
	    switch (BITRANGE(instr, 31, 31))
	    {
		case 0x00: f.func = A_J4; f.cond = COND_EQ; f.inswr = 0; f.wr = 0; break;
		case 0x01: f.func = A_J4; f.cond = COND_NE; f.inswr = 0; f.wr = 0; break;
	    }
	    break;
	case 0x7e:
	    f.format = OF_SBR; f.func = D_J; f.cond = COND_NE; f.inswr = 0; f.wr = 0; break;
	case 0x7f:
	    f.format = OF_BRR;
	    switch (BITRANGE(instr, 31, 31))
	    {
		case 0x00: f.func = D_J; f.cond = COND_GE; f.inswr = 0; f.wr = 0; break;
		case 0x01: f.func = D_J; f.cond = COND_GEU; f.inswr = 0; f.wr = 0; break;
	    }
	    break;
	case 0x80:
	    f.format = OF_SR_A; f.func = A_SRC1; f.inswr = 0; f.wr = 4; break;
	case 0x82:
	    f.format = OF_SRC; f.func = D_SRC1; f.inswr = 0; f.wr = 1; break;
	case 0x84:
	    f.format = OF_SpR; f.func = A_POST; f.inswr = 4; f.wr = 1; f.memwidth = MW_16s; break;
	case 0x85:
	    f.format = OF_ABS;
	    switch (BITRANGE(instr, 27, 26))
	    {
		case 0x00: f.func = A_PRE; f.inswr = 4; f.wr = 0; f.memwidth = MW_32; break;
		case 0x01: f.func = A_PRE; f.inswr = 12; f.wr = 0; f.memwidth = MW_64; break;
		case 0x02: f.func = A_PRE; f.inswr = 1; f.wr = 0; f.memwidth = MW_32; break;
		case 0x03: f.func = A_PRE; f.inswr = 3; f.wr = 0; f.memwidth = MW_64; break;
	    }
	    break;
	case 0x86:
	    f.format = OF_SRC; f.func = D_SHA; f.inswr = 0; f.wr = 1; break;
	case 0x87:
	    f.format = OF_BIT;
	    switch (BITRANGE(instr, 22, 21))
	    {
		case 0x00: f.func = D_SETC; f.cond = COND_AND; f.inswr = 0; f.wr = 1; break;
		case 0x01: f.func = D_SETC; f.cond = COND_OR; f.inswr = 0; f.wr = 1; break;
		case 0x02: f.func = D_SETC; f.cond = COND_NOR; f.inswr = 0; f.wr = 1; break;
		case 0x03: f.func = D_SETC; f.cond = COND_ANDN; f.inswr = 0; f.wr = 1; break;
	    }
	    break;
	case 0x88:
	    f.format = OF_SLRO; f.func = A_PRE; f.inswr = 4; f.wr = 0; f.memwidth = MW_16s; break;
	case 0x89:
	    f.format = OF_BO;
	    switch (BITRANGE(instr, 27, 22))
	    {
		case 0x00: f.func = A_POST; f.xrd = 1; f.inswr = 0; f.wr = 1; f.memwidth = MW_8u; break;
		case 0x02: f.func = A_POST; f.xrd = 1; f.inswr = 0; f.wr = 1; f.memwidth = MW_16u; break;
		case 0x04: f.func = A_POST; f.xrd = 1; f.inswr = 0; f.wr = 1; f.memwidth = MW_32; break;
		case 0x05: f.func = A_POST; f.xrd = 1; f.inswr = 0; f.wr = 1; f.memwidth = MW_64; break;
		case 0x06: f.func = A_POST; f.inswr = 0; f.wr = 1; f.memwidth = MW_32; break;
		case 0x07: f.func = MC_STDA_POST; f.inswr = 0; f.wr = 1; break;
		case 0x08: f.func = A_POST; f.xrd = 1; f.inswr = 0; f.wr = 1; f.memwidth = MW_16u; break;
		case 0x10: f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 1; f.memwidth = MW_8u; break;
		case 0x12: f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 1; f.memwidth = MW_16u; break;
		case 0x14: f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 1; f.memwidth = MW_32; break;
		case 0x15: f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 1; f.memwidth = MW_64; break;
		case 0x16: f.func = A_PRE; f.inswr = 0; f.wr = 1; f.memwidth = MW_32; break;
		case 0x17: f.func = MC_STDA_PRE; f.inswr = 0; f.wr = 1; break;
		case 0x18: f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 1; f.memwidth = MW_16u; break;
		case 0x20: f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 0; f.memwidth = MW_8u; break;
		case 0x22: f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 0; f.memwidth = MW_16u; break;
		case 0x24: f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 0; f.memwidth = MW_32; break;
		case 0x25: f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 0; f.memwidth = MW_64; break;
		case 0x26: f.func = A_PRE; f.inswr = 0; f.wr = 0; f.memwidth = MW_32; break;
		case 0x27: f.func = MC_STDA_NORM; f.inswr = 0; f.wr = 1; break;
		case 0x28: f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 0; f.memwidth = MW_16u; break;
	    }
	    break;
	case 0x8a:
	    f.format = OF_SRC; f.func = D_CADD; f.inswr = 0; f.wr = 1; break;
	case 0x8b:
	    f.format = OF_RC;
	    switch (BITRANGE(instr, 27, 21))
	    {
		case 0x00: f.func = D_ADD; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x04: f.func = D_ADDX; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x05: f.func = D_ADDC; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x08: f.func = D_RSUB; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x0e: f.func = D_ABSDIF; f.cond = COND_LT; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x10: f.func = D_SETC; f.cond = COND_EQ; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x11: f.func = D_SETC; f.cond = COND_NE; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x12: f.func = D_SETC; f.cond = COND_LT; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x13: f.func = D_SETC; f.cond = COND_LTU; f.inswr = 0; f.wr = 1; f.signext = false; break;
		case 0x14: f.func = D_SETC; f.cond = COND_GE; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x15: f.func = D_SETC; f.cond = COND_GEU; f.inswr = 0; f.wr = 1; f.signext = false; break;
		case 0x18: f.func = D_AORB; f.cond = COND_GE; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x19: f.func = D_AORB; f.cond = COND_GEU; f.inswr = 0; f.wr = 1; f.signext = false; break;
		case 0x1a: f.func = D_AORB; f.cond = COND_LT; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x1b: f.func = D_AORB; f.cond = COND_LTU; f.inswr = 0; f.wr = 1; f.signext = false; break;
		case 0x20: f.func = D_ANDC; f.cond = COND_EQ; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x21: f.func = D_ANDC; f.cond = COND_NE; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x22: f.func = D_ANDC; f.cond = COND_LT; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x23: f.func = D_ANDC; f.cond = COND_LTU; f.inswr = 0; f.wr = 1; f.signext = false; break;
		case 0x24: f.func = D_ANDC; f.cond = COND_GE; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x25: f.func = D_ANDC; f.cond = COND_GEU; f.inswr = 0; f.wr = 1; f.signext = false; break;
		case 0x27: f.func = D_ORC; f.cond = COND_EQ; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x28: f.func = D_ORC; f.cond = COND_NE; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x29: f.func = D_ORC; f.cond = COND_LT; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x2a: f.func = D_ORC; f.cond = COND_LTU; f.inswr = 0; f.wr = 1; f.signext = false; break;
		case 0x2b: f.func = D_ORC; f.cond = COND_GE; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x2c: f.func = D_ORC; f.cond = COND_GEU; f.inswr = 0; f.wr = 1; f.signext = false; break;
		case 0x2f: f.func = D_XORC; f.cond = COND_EQ; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x30: f.func = D_XORC; f.cond = COND_NE; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x31: f.func = D_XORC; f.cond = COND_LT; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x32: f.func = D_XORC; f.cond = COND_LTU; f.inswr = 0; f.wr = 1; f.signext = false; break;
		case 0x33: f.func = D_XORC; f.cond = COND_GE; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x34: f.func = D_XORC; f.cond = COND_GEU; f.inswr = 0; f.wr = 1; f.signext = false; break;
		case 0x37: f.func = D_SHC; f.cond = COND_EQ; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x38: f.func = D_SHC; f.cond = COND_NE; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x39: f.func = D_SHC; f.cond = COND_LT; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x3a: f.func = D_SHC; f.cond = COND_LTU; f.inswr = 0; f.wr = 1; f.signext = false; break;
		case 0x3b: f.func = D_SHC; f.cond = COND_GE; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x3c: f.func = D_SHC; f.cond = COND_GEU; f.inswr = 0; f.wr = 1; f.signext = false; break;
		case 0x56: f.func = D_EQANY_B; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x76: f.func = D_EQANY_H; f.inswr = 0; f.wr = 1; f.signext = true; break;
	    }
	    break;
	case 0x8c:
	    f.format = OF_SRO; f.func = A_PRE; f.inswr = 4; f.wr = 0; f.memwidth = MW_16s; break;
	case 0x8e:
	    f.format = OF_SBR; f.func = D_J; f.cond = COND_LEZ; f.inswr = 0; f.wr = 0; break;
	case 0x8f:
	    f.format = OF_RC_u;
	    switch (BITRANGE(instr, 27, 21))
	    {
		case 0x00: f.func = D_SH; f.inswr = 0; f.wr = 1; break;
		case 0x01: f.func = D_SHA; f.inswr = 0; f.wr = 1; break;
		case 0x08: f.func = D_AND; f.inswr = 0; f.wr = 1; break;
		case 0x09: f.func = D_NAND; f.inswr = 0; f.wr = 1; break;
		case 0x0a: f.func = D_OR; f.inswr = 0; f.wr = 1; break;
		case 0x0b: f.func = D_NOR; f.inswr = 0; f.wr = 1; break;
		case 0x0c: f.func = D_XOR; f.inswr = 0; f.wr = 1; break;
		case 0x0d: f.func = D_XNOR; f.inswr = 0; f.wr = 1; break;
		case 0x0e: f.func = D_ANDN; f.inswr = 0; f.wr = 1; break;
		case 0x0f: f.func = D_ORN; f.inswr = 0; f.wr = 1; break;
	    }
	    break;
	case 0x90:
	    f.format = OF_SRO; f.func = A_ADDSC_A; f.xrd = 1; f.inswr = 0; f.wr = 1; break;
	case 0x91:
	    f.format = OF_RLC_h_A; f.func = A_SRC1; f.inswr = 0; f.wr = 1; break;
	case 0x92:
	    f.format = OF_SBC; f.func = D_ADD; f.inswr = 0; f.wr = 1; break;
	case 0x94:
	    f.format = OF_SLR; f.func = A_PRE; f.inswr = 4; f.wr = 0; f.memwidth = MW_16s; break;
	case 0x96:
	    f.format = OF_SC_u; f.func = D_OR; f.inswr = 0; f.wr = 1; break;
	case 0x97:
	    f.format = OF_RCRR; f.func = D_INSERT; f.inswr = 0; f.wr = 1; break;
	case 0x99:
	    f.format = OF_BOL; f.func = A_PRE; f.inswr = 1; f.wr = 0; f.memwidth = MW_32; break;
	case 0x9a:
	    f.format = OF_SRC_d; f.func = D_ADD; f.inswr = 0; f.wr = 1; break;
	case 0x9b:
	    f.format = OF_RLC_h; f.func = D_ADD; f.inswr = 0; f.wr = 1; break;
	case 0x9d:
	    f.format = OF_B; f.func = A_JA; f.cond = COND_1; f.inswr = 0; f.wr = 0; break;
	case 0x9f:
	    f.format = OF_BRC;
	    switch (BITRANGE(instr, 31, 31))
	    {
		case 0x00: f.func = D_JNEI; f.cond = COND_NE; f.inswr = 0; f.wr = 1; f.signext = true; break;
		case 0x01: f.func = D_JNED; f.cond = COND_NE; f.inswr = 0; f.wr = 1; f.signext = true; break;
	    }
	    break;
	case 0xa0:
	    f.format = OF_SBR_A; f.func = A_SRC1; f.inswr = 0; f.wr = 1; break;
	case 0xa2:
	    f.format = OF_SRR; f.func = D_SUB; f.inswr = 0; f.wr = 1; break;
	case 0xa4:
	    f.format = OF_SpR; f.func = A_POST; f.xrd = 1; f.inswr = 0; f.wr = 1; f.memwidth = MW_16u; break;
	case 0xa5:
	    f.format = OF_ABS;
	    switch (BITRANGE(instr, 27, 26))
	    {
		case 0x00: f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 0; f.memwidth = MW_32; break;
		case 0x01: f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 0; f.memwidth = MW_64; break;
		case 0x02: f.func = A_PRE; f.inswr = 0; f.wr = 0; f.memwidth = MW_32; break;
		case 0x03: f.func = MC_STDA_ABS; f.inswr = 0; f.wr = 1; break;
	    }
	    break;
	case 0xa6:
	    f.format = OF_SRR; f.func = D_OR; f.inswr = 0; f.wr = 1; break;
	case 0xa7:
	    f.format = OF_BIT;
	    switch (BITRANGE(instr, 22, 21))
	    {
		case 0x00: f.func = D_SHC; f.cond = COND_NAND; f.inswr = 0; f.wr = 1; break;
		case 0x01: f.func = D_SHC; f.cond = COND_ORN; f.inswr = 0; f.wr = 1; break;
		case 0x02: f.func = D_SHC; f.cond = COND_XNOR; f.inswr = 0; f.wr = 1; break;
		case 0x03: f.func = D_SHC; f.cond = COND_XOR; f.inswr = 0; f.wr = 1; break;
	    }
	    break;
	case 0xa8:
	    f.format = OF_SLRO; f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 0; f.memwidth = MW_16u; break;
	case 0xaa:
	    f.format = OF_SRC; f.func = D_SELN; f.inswr = 0; f.wr = 1; break;
	case 0xab:
	    f.format = OF_RCR;
	    switch (BITRANGE(instr, 23, 21))
	    {
		case 0x00: f.func = D_CADD; f.inswr = 0; f.wr = 1; break;
		case 0x01: f.func = D_CADDN; f.inswr = 0; f.wr = 1; break;
		case 0x04: f.func = D_SEL; f.inswr = 0; f.wr = 1; break;
		case 0x05: f.func = D_SELN; f.inswr = 0; f.wr = 1; break;
	    }
	    break;
	case 0xac:
	    f.format = OF_SRO; f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 0; f.memwidth = MW_16u; break;
	case 0xae:
	    f.format = OF_SBC; f.func = D_J; f.cond = COND_SJNZT; f.inswr = 0; f.wr = 0; break;
	case 0xb0:
	    f.format = OF_SRC_adda; f.func = A_ADD; f.inswr = 0; f.wr = 1; break;
	case 0xb4:
	    f.format = OF_SLR; f.func = A_PRE; f.xrd = 1; f.inswr = 0; f.wr = 0; f.memwidth = MW_16u; break;
	case 0xb7:
	    f.format = OF_RCPW;
	    switch (BITRANGE(instr, 22, 21))
	    {
		case 0x00: f.func = D_INSERT; f.inswr = 0; f.wr = 1; break;
		case 0x01: f.func = D_IMASK; f.inswr = 0; f.wr = 3; break;
	    }
	    break;
	case 0xba:
	    f.format = OF_SRC_d; f.func = D_SETC; f.cond = COND_EQ; f.inswr = 0; f.wr = 1; break;
	case 0xbb:
	    f.format = OF_RLC_u; f.func = D_SRC1; f.inswr = 0; f.wr = 1; break;
	case 0xbc:
	    f.format = OF_SBR_A; f.func = A_J4; f.cond = COND_EQZ; f.inswr = 0; f.wr = 0; break;
	case 0xbd:
	    f.format = OF_BRR_A;
	    switch (BITRANGE(instr, 31, 31))
	    {
		case 0x00: f.func = A_J4; f.cond = COND_EQZ; f.inswr = 0; f.wr = 0; break;
		case 0x01: f.func = A_J4; f.cond = COND_NEZ; f.inswr = 0; f.wr = 0; break;
	    }
	    break;
	case 0xbf:
	    f.format = OF_BRC;
	    switch (BITRANGE(instr, 31, 31))
	    {
		case 0x00: f.func = D_J; f.cond = COND_LT; f.inswr = 0; f.wr = 0; f.signext = true; break;
		case 0x01: f.func = D_J; f.cond = COND_LTU; f.inswr = 0; f.wr = 0; f.signext = false; break;
	    }
	    break;
	case 0xc2:
	    f.format = OF_SRC; f.func = D_ADD; f.inswr = 0; f.wr = 1; break;
	case 0xc4:
	    f.format = OF_SpR; f.func = A_POST; f.inswr = 1; f.wr = 1; f.memwidth = MW_32; break;
	case 0xc5:
	    f.format = OF_ABS; f.func = A_PRE; f.inswr = 0; f.wr = 1; break;
	case 0xc6:
	    f.format = OF_SRR; f.func = D_XOR; f.inswr = 0; f.wr = 1; break;
	case 0xc7:
	    f.format = OF_BIT;
	    switch (BITRANGE(instr, 22, 21))
	    {
		case 0x00: f.func = D_ORC; f.cond = COND_AND; f.inswr = 0; f.wr = 1; break;
		case 0x01: f.func = D_ORC; f.cond = COND_OR; f.inswr = 0; f.wr = 1; break;
		case 0x02: f.func = D_ORC; f.cond = COND_NOR; f.inswr = 0; f.wr = 1; break;
		case 0x03: f.func = D_ORC; f.cond = COND_ANDN; f.inswr = 0; f.wr = 1; break;
	    }
	    break;
	case 0xc8:
	    f.format = OF_SLRO; f.func = A_PRE; f.inswr = 1; f.wr = 0; f.memwidth = MW_32; break;
	case 0xca:
	    f.format = OF_SRC; f.func = D_CADDN; f.inswr = 0; f.wr = 1; break;
	case 0xcc:
	    f.format = OF_SRO; f.func = A_PRE; f.inswr = 1; f.wr = 0; f.memwidth = MW_32; break;
	case 0xcd:
	    f.format = OF_RLC_h_A; f.func = FUNC_NOP; f.xrd = 1; f.csfrwr = 1; f.inswr = 0; f.wr = 0; break;
	case 0xce:
	    f.format = OF_SBR; f.func = D_J; f.cond = COND_GEZ; f.inswr = 0; f.wr = 0; break;
	case 0xd0:
	    f.format = OF_SRO; f.func = A_ADDSC_A; f.xrd = 1; f.inswr = 0; f.wr = 1; break;
	case 0xd4:
	    f.format = OF_SLR; f.func = A_PRE; f.inswr = 1; f.wr = 0; f.memwidth = MW_32; break;
	case 0xd5:
	    f.format = OF_ABS; f.func = MC_STT; f.inswr = 0; f.wr = 1; break;
	case 0xd7:
	    f.format = OF_RCRW;
	    switch (BITRANGE(instr, 23, 21))
	    {
		case 0x00: f.func = D_INSERT; f.inswr = 0; f.wr = 1; break;
		case 0x01: f.func = D_IMASK; f.inswr = 0; f.wr = 3; break;
	    }
	    break;
	case 0xd8:
	    f.format = OF_SCo; f.func = A_PRE; f.inswr = 1; f.wr = 0; f.memwidth = MW_32; break;
	case 0xd9:
	    f.format = OF_BOL; f.func = A_PRE; f.inswr = 0; f.wr = 1; break;
	case 0xda:
	    f.format = OF_SC_u; f.func = D_SRC1; f.inswr = 0; f.wr = 1; break;
	case 0xdc:
	    f.format = OF_SR_A; f.func = A_JI; f.cond = COND_1; f.inswr = 0; f.wr = 0; break;
	case 0xdd:
	    f.format = OF_B; f.func = A_JA; f.cond = COND_1; f.inswr = 0; f.wr = 1; break;
	case 0xdf:
	    f.format = OF_BRC;
	    switch (BITRANGE(instr, 31, 31))
	    {
		case 0x00: f.func = D_J; f.cond = COND_EQ; f.inswr = 0; f.wr = 0; f.signext = true; break;
		case 0x01: f.func = D_J; f.cond = COND_NE; f.inswr = 0; f.wr = 0; f.signext = true; break;
	    }
	    break;
	case 0xe2:
	    f.format = OF_SRR; f.func = D_MUL; f.inswr = 0; f.wr = 1; break;
	case 0xe4:
	    f.format = OF_SpR; f.func = A_POST; f.inswr = 0; f.wr = 1; f.memwidth = MW_32; break;
	case 0xe5:
	    f.format = OF_ABS;
	    switch (BITRANGE(instr, 27, 26))
	    {
		case 0x00: f.func = A_PRE; f.xrd = 1; f.inswr = 4; f.wr = 0; f.memwidth = MW_32; break;
		case 0x01: f.func = MC_LDMST_ABS; f.inswr = 0; f.wr = 1; break;
	    }
	    break;
	case 0xe8:
	    f.format = OF_SLRO; f.func = A_PRE; f.inswr = 0; f.wr = 0; f.memwidth = MW_32; break;
	case 0xea:
	    f.format = OF_SRC; f.func = D_SEL; f.inswr = 0; f.wr = 1; break;
	case 0xec:
	    f.format = OF_SRO; f.func = A_PRE; f.inswr = 0; f.wr = 0; f.memwidth = MW_32; break;
	case 0xed:
	    f.format = OF_B; f.func = MC_CALLA; f.inswr = 0; f.wr = 1; break;
	case 0xee:
	    f.format = OF_SB; f.func = D_J; f.cond = COND_NEZ; f.inswr = 0; f.wr = 0; break;
	case 0xef:
	    f.format = OF_BRR;
	    switch (BITRANGE(instr, 31, 31))
	    {
		case 0x00: f.func = D_J; f.cond = COND_JZT; f.inswr = 0; f.wr = 0; break;
		case 0x01: f.func = D_J; f.cond = COND_JNZT; f.inswr = 0; f.wr = 0; break;
	    }
	    break;
	case 0xf4:
	    f.format = OF_SLR; f.func = A_PRE; f.inswr = 0; f.wr = 0; f.memwidth = MW_32; break;
	case 0xf6:
	    f.format = OF_SBR; f.func = D_J; f.cond = COND_NEZ; f.inswr = 0; f.wr = 0; break;
	case 0xf8:
	    f.format = OF_SCo; f.func = A_PRE; f.inswr = 0; f.wr = 0; f.memwidth = MW_32; break;
	case 0xfa:
	    f.format = OF_SRC_d; f.func = D_SETC; f.cond = COND_LT; f.inswr = 0; f.wr = 1; break;
	case 0xfc:
	    f.format = OF_SBR_loop; f.func = A_LOOP; f.cond = COND_NEZ; f.inswr = 0; f.wr = 1; break;
	case 0xfd:
	    f.format = OF_BRR_A;
	    switch (BITRANGE(instr, 31, 31))
	    {
		case 0x00: f.func = A_LOOP; f.inswr = 0; f.wr = 1; break;
		case 0x01: f.func = A_J4; f.cond = COND_1; f.inswr = 0; f.wr = 0; break;
	    }
	    break;
	case 0xff:
	    f.format = OF_BRC;
	    switch (BITRANGE(instr, 31, 31))
	    {
		case 0x00: f.func = D_J; f.cond = COND_GE; f.inswr = 0; f.wr = 0; f.signext = true; break;
		case 0x01: f.func = D_J; f.cond = COND_GEU; f.inswr = 0; f.wr = 0; f.signext = false; break;
	    }
	    break;

	}
	return f;
}



struct micro_op_t {
	alu_func_t	func;
	cond_t		cond;
	
	reg_t	dest1;
	reg_t	reg1;
	reg_t	reg2;
	reg_t	reg3;

	bool	xrd;
	bool	dest_wr;
	bool	dest_wr2;
	bool	dest_Xwr;
	bool	dest_Xwr2;

	bool	mem_read;
	bool	mem_write;
	uint3_t	mem_width;
};
typedef micro_op_t const *microcode_t;

//-------------------------------------------------------------



static const int MICROCODE_COUNT = 12;
typedef func_t micro_func_t;



/* Microcode table */
const micro_op_t mc_table[] = {
	{END_OF_MC,COND_0,	0,	0,0,0,	0,0,0,0,0,	0,0,0},

	/* microsequence for CALL_L */
	{A_SRC1,	COND_1,	19,	0,REG_SHADOW_FCX,OM_disp24,false,	1,0,0,0,	0,0,0},
	{A_CSA,	COND_1,	18,	0,REG_SHADOW_FCX,0,false,	1,0,0,0,	0,0,0},
	{A_POST,	COND_1,	0,	0,18,0,false,	0,0,0,0,	1,0,MW_32},
	{A_INSLD,	COND_1,	REG_SHADOW_FCX,	0,0,0,false,	1,0,0,0,	0,0,0},
	{A_POST,	COND_1,	18,	REG_SHADOW_PCXI,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	REG_SHADOW_PSW,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_CPCF,	COND_1,	REG_SHADOW_PCXI,	REG_SHADOW_PCXI,19,0,false,	1,0,0,0,	0,0,0},
	{A_POST,	COND_1,	18,	10,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	11,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	8,18,OM_imm8,1,	1,0,0,0,	0,1,MW_64},
	{A_POST,	COND_1,	18,	10,18,OM_imm8,1,	1,0,0,0,	0,1,MW_64},
	{A_POST,	COND_1,	18,	12,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	13,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	14,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	15,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_J4,	COND_1,	11,	0,0,OM_disp24,false,	1,0,0,0,	0,0,0},
	{A_POST,	COND_1,	18,	12,18,OM_imm8,1,	1,0,0,0,	0,1,MW_64},
	{A_POST,	COND_1,	18,	14,18,OM_imm8,1,	1,0,0,0,	0,1,MW_64},

	/* microsequence for CALL_S */
	{A_SRC1,	COND_1,	19,	0,REG_SHADOW_FCX,OM_disp8,false,	1,0,0,0,	0,0,0},
	{A_CSA,	COND_1,	18,	0,REG_SHADOW_FCX,0,false,	1,0,0,0,	0,0,0},
	{A_POST,	COND_1,	0,	0,18,0,false,	0,0,0,0,	1,0,MW_32},
	{A_INSLD,	COND_1,	REG_SHADOW_FCX,	0,0,0,false,	1,0,0,0,	0,0,0},
	{A_POST,	COND_1,	18,	REG_SHADOW_PCXI,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	REG_SHADOW_PSW,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_CPCF,	COND_1,	REG_SHADOW_PCXI,	REG_SHADOW_PCXI,19,0,false,	1,0,0,0,	0,0,0},
	{A_POST,	COND_1,	18,	10,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	11,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	8,18,OM_imm8,1,	1,0,0,0,	0,1,MW_64},
	{A_POST,	COND_1,	18,	10,18,OM_imm8,1,	1,0,0,0,	0,1,MW_64},
	{A_POST,	COND_1,	18,	12,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	13,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	14,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	15,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_J2,	COND_1,	11,	0,0,OM_disp8,false,	1,0,0,0,	0,0,0},
	{A_POST,	COND_1,	18,	12,18,OM_imm8,1,	1,0,0,0,	0,1,MW_64},
	{A_POST,	COND_1,	18,	14,18,OM_imm8,1,	1,0,0,0,	0,1,MW_64},

	/* microsequence for CALLA */
	{A_SRC1,	COND_1,	19,	0,REG_SHADOW_FCX,OM_disp24,false,	1,0,0,0,	0,0,0},
	{A_CSA,	COND_1,	18,	0,REG_SHADOW_FCX,0,false,	1,0,0,0,	0,0,0},
	{A_POST,	COND_1,	0,	0,18,0,false,	0,0,0,0,	1,0,MW_32},
	{A_INSLD,	COND_1,	REG_SHADOW_FCX,	0,0,0,false,	1,0,0,0,	0,0,0},
	{A_POST,	COND_1,	18,	REG_SHADOW_PCXI,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	REG_SHADOW_PSW,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_CPCF,	COND_1,	REG_SHADOW_PCXI,	REG_SHADOW_PCXI,19,0,false,	1,0,0,0,	0,0,0},
	{A_POST,	COND_1,	18,	10,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	11,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	8,18,OM_imm8,1,	1,0,0,0,	0,1,MW_64},
	{A_POST,	COND_1,	18,	10,18,OM_imm8,1,	1,0,0,0,	0,1,MW_64},
	{A_POST,	COND_1,	18,	12,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	13,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	14,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	15,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_JA,	COND_1,	11,	0,0,OM_disp24,false,	1,0,0,0,	0,0,0},
	{A_POST,	COND_1,	18,	12,18,OM_imm8,1,	1,0,0,0,	0,1,MW_64},
	{A_POST,	COND_1,	18,	14,18,OM_imm8,1,	1,0,0,0,	0,1,MW_64},

	/* microsequence for CALLI */
	{A_SRC1,	COND_1,	19,	OM_a,REG_SHADOW_FCX,0,false,	1,0,0,0,	0,0,0},
	{A_CSA,	COND_1,	18,	0,REG_SHADOW_FCX,0,false,	1,0,0,0,	0,0,0},
	{A_POST,	COND_1,	0,	0,18,0,false,	0,0,0,0,	1,0,MW_32},
	{A_INSLD,	COND_1,	REG_SHADOW_FCX,	0,0,0,false,	1,0,0,0,	0,0,0},
	{A_POST,	COND_1,	18,	REG_SHADOW_PCXI,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	REG_SHADOW_PSW,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_CPCF,	COND_1,	REG_SHADOW_PCXI,	REG_SHADOW_PCXI,19,0,false,	1,0,0,0,	0,0,0},
	{A_POST,	COND_1,	18,	10,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	11,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	8,18,OM_imm8,1,	1,0,0,0,	0,1,MW_64},
	{A_POST,	COND_1,	18,	10,18,OM_imm8,1,	1,0,0,0,	0,1,MW_64},
	{A_POST,	COND_1,	18,	12,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	13,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	14,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	15,18,OM_imm4,false,	1,0,0,0,	0,1,MW_32},
	{A_JI,	COND_1,	11,	OM_a,0,0,false,	1,0,0,0,	0,0,0},
	{A_POST,	COND_1,	18,	12,18,OM_imm8,1,	1,0,0,0,	0,1,MW_64},
	{A_POST,	COND_1,	18,	14,18,OM_imm8,1,	1,0,0,0,	0,1,MW_64},

	/* microsequence for LDMST */
	{A_PRE,	COND_1,	18,	0,OM_b,OM_off10,false,	0,0,0,0,	1,0,MW_32},
	{A_LDMST,	COND_1,	18,	OM_a,18,0,1,	1,0,0,0,	0,0,0},
	{A_PRE,	COND_1,	0,	18,OM_b,OM_off10,false,	0,0,0,0,	0,1,MW_32},

	/* microsequence for LDMST_ABS */
	{A_PRE,	COND_1,	18,	0,OM_imm0,OM_off18,false,	0,0,0,0,	1,0,MW_32},
	{A_LDMST,	COND_1,	18,	OM_a,18,0,1,	1,0,0,0,	0,0,0},
	{A_PRE,	COND_1,	0,	18,OM_imm0,OM_off18,false,	0,0,0,0,	0,1,MW_32},

	/* microsequence for RET */
	{A_SRC1,	COND_1,	19,	0,REG_SHADOW_PCXI,0,false,	1,0,0,0,	0,0,0},
	{A_CSA,	COND_1,	18,	0,REG_SHADOW_PCXI,0,false,	1,0,0,0,	0,0,0},
	{A_POST,	COND_1,	0,	0,18,OM_imm0,false,	0,0,0,0,	1,0,MW_32},
	{A_INSLD,	COND_1,	REG_SHADOW_PCXI,	0,0,0,false,	1,1,0,0,	0,0,0},
	{A_POST,	COND_1,	18,	REG_SHADOW_FCX,18,OM_imm8,false,	1,0,0,0,	0,1,MW_32},
	{A_CLCF,	COND_1,	REG_SHADOW_FCX,	19,0,0,false,	1,0,0,0,	0,0,0},
	{A_SRC1,	COND_1,	19,	0,11,0,false,	1,0,0,0,	0,0,0},
	{A_POST,	COND_1,	18,	0,18,OM_imm8,false,	1,0,0,0,	1,0,MW_64},
	{A_INSLD,	COND_1,	10,	0,0,0,false,	1,1,0,0,	0,0,0},
	{A_POST,	COND_1,	18,	0,18,OM_imm8,false,	1,0,0,0,	1,0,MW_64},
	{A_INSLD,	COND_1,	8,	0,0,0,false,	0,0,1,1,	0,0,0},
	{A_POST,	COND_1,	18,	0,18,OM_imm8,false,	1,0,0,0,	1,0,MW_64},
	{A_INSLD,	COND_1,	10,	0,0,0,false,	0,0,1,1,	0,0,0},
	{A_POST,	COND_1,	18,	0,18,OM_imm8,false,	1,0,0,0,	1,0,MW_64},
	{A_INSLD,	COND_1,	12,	0,0,0,false,	1,1,0,0,	0,0,0},
	{A_POST,	COND_1,	18,	0,18,OM_imm8,false,	1,0,0,0,	1,0,MW_64},
	{A_INSLD,	COND_1,	14,	0,0,0,false,	1,1,0,0,	0,0,0},
	{A_POST,	COND_1,	18,	0,18,OM_imm8,false,	1,0,0,0,	1,0,MW_64},
	{A_INSLD,	COND_1,	12,	0,0,0,false,	0,0,1,1,	0,0,0},
	{A_JI,	COND_1,	0,	19,0,0,false,	0,0,0,0,	0,0,0},
	{A_POST,	COND_1,	18,	0,18,OM_imm8,false,	1,0,0,0,	1,0,MW_64},
	{A_INSLD,	COND_1,	14,	0,0,0,false,	0,0,1,1,	0,0,0},

	/* microsequence for STDA_PRE */
	{A_PRE,	COND_1,	OM_b,	OM_a,OM_b,OM_off10,false,	1,0,0,0,	0,1,MW_32},
	{A_PRE,	COND_1,	0,	OM_a1,OM_b,OM_imm4,false,	0,0,0,0,	0,1,MW_32},

	/* microsequence for STDA_POST */
	{A_PRE,	COND_1,	0,	OM_a1,OM_a,OM_imm4,false,	0,0,0,0,	0,1,MW_32},
	{A_POST,	COND_1,	18,	OM_a,OM_b,OM_off10,false,	1,0,0,0,	0,1,MW_32},

	/* microsequence for STDA_NORM */
	{A_PRE,	COND_1,	18,	OM_a,OM_b,OM_off10,false,	1,0,0,0,	0,1,MW_32},
	{A_PRE,	COND_1,	0,	OM_a1,18,OM_imm4,false,	0,0,0,0,	0,1,MW_32},

	/* microsequence for STDA_ABS */
	{A_PRE,	COND_1,	18,	OM_a,OM_imm0,OM_off18,false,	1,0,0,0,	0,1,MW_32},
	{A_PRE,	COND_1,	0,	OM_a1,18,OM_imm4,false,	0,0,0,0,	0,1,MW_32},

	/* microsequence for STT */
	{A_PRE,	COND_1,	18,	0,OM_imm0,OM_off18a,false,	1,0,0,0,	1,0,MW_32},
	{A_STT,	COND_1,	18,	18,0,0,false,	1,0,0,0,	0,0,0},
	{A_PRE,	COND_1,	0,	18,OM_imm0,OM_off18a,false,	0,0,0,0,	0,1,MW_32},
};



/* Microcode table for predecoding */
const unsigned char mc_pd_table[][4] = {
	{0,0,0,0},

	/* predecode signals for CALL_L */
	{0,0,0,0},
	{0,0,0,0},
	{1,0,0,0},
	{0,0,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,0,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,0,1,0},
	{0,1,0,0},
	{0,1,0,1},

	/* predecode signals for CALL_S */
	{0,0,0,0},
	{0,0,0,0},
	{1,0,0,0},
	{0,0,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,0,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,0,1,0},
	{0,1,0,0},
	{0,1,0,1},

	/* predecode signals for CALLA */
	{0,0,0,0},
	{0,0,0,0},
	{1,0,0,0},
	{0,0,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,0,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,0,1,0},
	{0,1,0,0},
	{0,1,0,1},

	/* predecode signals for CALLI */
	{0,0,0,0},
	{0,0,0,0},
	{1,0,0,0},
	{0,0,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,0,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,0,1,0},
	{0,1,0,0},
	{0,1,0,1},

	/* predecode signals for LDMST */
	{1,0,0,0},
	{0,0,0,0},
	{0,1,0,1},

	/* predecode signals for LDMST_ABS */
	{1,0,0,0},
	{0,0,0,0},
	{0,1,0,1},

	/* predecode signals for RET */
	{0,0,0,0},
	{0,0,0,0},
	{1,0,0,0},
	{0,0,0,0},
	{0,1,0,0},
	{0,0,0,0},
	{0,0,0,0},
	{1,0,0,0},
	{0,0,0,0},
	{1,0,0,0},
	{0,0,0,0},
	{1,0,0,0},
	{0,0,0,0},
	{1,0,0,0},
	{0,0,0,0},
	{1,0,0,0},
	{0,0,0,0},
	{1,0,0,0},
	{0,0,0,0},
	{0,0,1,0},
	{1,0,0,0},
	{0,0,0,1},

	/* predecode signals for STDA_PRE */
	{0,1,0,0},
	{0,1,0,1},

	/* predecode signals for STDA_POST */
	{0,1,0,0},
	{0,1,0,1},

	/* predecode signals for STDA_NORM */
	{0,1,0,0},
	{0,1,0,1},

	/* predecode signals for STDA_ABS */
	{0,1,0,0},
	{0,1,0,1},

	/* predecode signals for STT */
	{1,0,0,0},
	{0,0,0,0},
	{0,1,0,1},
};



struct ctx_op_t {
	uint8_t	ofs;		///< offset in TCB (lowset 3 bits always 0, ignore in VHDL)
	uint5_t	idx;		///< index within register set (0..15 for data reg, 0..15 for addr reg, 16..31 for shadow reg)

	bool	read_d;		///< read from data regset
	bool	read_a;		///< read from addr regset
	
	bool	load;		///< load from memory
	bool	store;		///< store to memory
	
	bool	write_d;	///< write to data regset
	bool	write_a;	///< write to addr regset
	bool	writepc;	///< write pc in select stage
	bool	writeman;	///< write scheduling data in manager
	
	bool	last;
};


#define CMC_SWAPOUT 1;
#define CMC_SWAPIN 20;





/* Seqences for Context Pipeline */
const ctx_op_t ctx_microcode[] = {
	{0,0,0,0,0,0,0,0,0,0,0},

	/* context sequence signals for SWAPOUT */
	{CTX_OFS_A0,	0,	0,1,0,1,0,0,0,0,0},
	{CTX_OFS_A8,	8,	0,1,0,1,0,0,0,0,0},
	{CTX_OFS_FCX,	REG_SHADOW_FCX,	0,1,0,1,0,0,0,0,0},
	{CTX_OFS_PC,	REG_SHADOW_PC,	0,1,0,1,0,0,0,0,0},
	{CTX_OFS_A2,	2,	0,1,0,1,0,0,0,0,0},
	{CTX_OFS_D0,	0,1,0,0,1,0,0,0,0,0},
	{CTX_OFS_D2,	2,1,0,0,1,0,0,0,0,0},
	{CTX_OFS_A4,	4,	0,1,0,1,0,0,0,0,0},
	{CTX_OFS_A6,	6,	0,1,0,1,0,0,0,0,0},
	{CTX_OFS_D4,	4,1,0,0,1,0,0,0,0,0},
	{CTX_OFS_D6,	6,1,0,0,1,0,0,0,0,0},
	{CTX_OFS_PSW,	REG_SHADOW_PCXI,	0,1,0,1,0,0,0,0,0},
	{CTX_OFS_A10,	10,	0,1,0,1,0,0,0,0,0},
	{CTX_OFS_D8,	8,1,0,0,1,0,0,0,0,0},
	{CTX_OFS_D10,	10,1,0,0,1,0,0,0,0,0},
	{CTX_OFS_A12,	12,	0,1,0,1,0,0,0,0,0},
	{CTX_OFS_A14,	14,	0,1,0,1,0,0,0,0,0},
	{CTX_OFS_D12,	12,1,0,0,1,0,0,0,0,0},
	{CTX_OFS_D14,	14,1,0,0,1,0,0,0,0,1},

	/* context sequence signals for SWAPIN */
	{CTX_OFS_SCHED_FLAGS,	0,	0,0,1,0,0,0,0,1,0},
	{CTX_OFS_SCHED_PREV,	2,	0,0,1,0,0,0,0,1,0},
	{CTX_OFS_A0,	0,	0,0,1,0,0,1,0,0,0},
	{CTX_OFS_A8,	8,	0,0,1,0,0,1,0,0,0},
	{CTX_OFS_FCX,	REG_SHADOW_FCX,	0,0,1,0,0,1,0,0,0},
	{CTX_OFS_PC,	REG_SHADOW_PC,	0,0,1,0,0,1,1,0,0},
	{CTX_OFS_A2,	2,	0,0,1,0,0,1,0,0,0},
	{CTX_OFS_D0,	0,	0,0,1,0,1,0,0,0,0},
	{CTX_OFS_D2,	2,	0,0,1,0,1,0,0,0,0},
	{CTX_OFS_A4,	4,	0,0,1,0,0,1,0,0,0},
	{CTX_OFS_A6,	6,	0,0,1,0,0,1,0,0,0},
	{CTX_OFS_D4,	4,	0,0,1,0,1,0,0,0,0},
	{CTX_OFS_D6,	6,	0,0,1,0,1,0,0,0,0},
	{CTX_OFS_PSW,	REG_SHADOW_PCXI,	0,0,1,0,0,1,0,0,0},
	{CTX_OFS_A10,	10,	0,0,1,0,0,1,0,0,0},
	{CTX_OFS_D8,	8,	0,0,1,0,1,0,0,0,0},
	{CTX_OFS_D10,	10,	0,0,1,0,1,0,0,0,0},
	{CTX_OFS_A12,	12,	0,0,1,0,0,1,0,0,0},
	{CTX_OFS_A14,	14,	0,0,1,0,0,1,0,0,0},
	{CTX_OFS_D12,	12,	0,0,1,0,1,0,0,0,0},
	{CTX_OFS_D14,	14,	0,0,1,0,1,0,0,0,1},
};


#endif /* _INSTRSET_H_ */
