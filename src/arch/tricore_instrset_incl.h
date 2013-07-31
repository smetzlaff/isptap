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
#ifndef _INSTRSET_INCL_H_
#define _INSTRSET_INCL_H_

// just to hint the really needed number of bits (for VHDL)
typedef unsigned	uint2_t;
typedef unsigned	uint3_t;
typedef unsigned	uint4_t;
typedef unsigned	uint5_t;
typedef unsigned	uint6_t;
typedef unsigned	uint7_t;
typedef unsigned	uint20_t;

typedef int32_t		word_t;		//signed
typedef int64_t		dword_t;	//signed
typedef uint32_t	uword_t;	//unsigned
typedef uint64_t	udword_t;	//unsigned
typedef uint32_t	address_t;
typedef uint32_t	pc_t;
typedef uint64_t	fetch_t;
typedef uint32_t	instr_t;
typedef uint7_t		reg_t;
typedef uint16_t	sfr_addr_t;
typedef uint32_t	func_t;
typedef uint8_t		cond_t;
typedef int32_t		period_t;	//signed!
typedef uint20_t	thrid_t;
typedef uint6_t		thr_t;
typedef uint6_t		core_t;

typedef uint8_t		mcpc_t;
typedef uint8_t		issue_state_t;	// 3 bit
typedef uint8_t		stall_counter_t;// 3 bit

typedef uint8_t		mem_width_t; // 8 bit for byte enable (1111 1111 = 64bit, 1111 0000 = upper 32bit and so on, for signed and unsigned 16bit/8bit accesses, MSB could be used to indicate)

// -----------------------------------------------------------------------------
//
// Constants visible to user (executed program)
//
// Synchronize a change of the following constants with user programs
// Most values are specified by the TriCore manuals
//
// -----------------------------------------------------------------------------



// -----------------------------------------------------------------------------
// CSFR register numbers
// -----------------------------------------------------------------------------

// from TriCore
#define CSFR_PCXI 			0xfe00
//#define CSFR_SYSCON			0xfe14
//#define CSFR_BIV			0xfe20
//#define CSFR_BTV			0xfe24
//#define CSFR_ISP			0xfe28
//#define CSFR_ICR			0xfe2c
#define CSFR_FCX			0xfe38
#define CSFR_LCX			0xfe3c
#define CSFR_PSW			0xfe04

// for evaluation
#define CSFR_BENCHMARK_COUNTDOWN	0x77f0	///< reset stats and stop simulation after fixed number of cycles (passed by CSFR value)
#define CSFR_BENCHMARK_PRINT		0x77f4	///< print stats but continmue with imulation
#define CSFR_BENCHMARK_BEGIN		0x77f8	///< reset stats
#define CSFR_BENCHMARK_END		0x77fc	///< print stats and stop simulation
#define CSFR_BENCHMARK_BEGIN_CORE	0x77e0	///< reset stats for the initiating core only
#define CSFR_BENCHMARK_BEGIN_SLOT	0x77e4	///< reset stats for the initiating slot only

#ifdef RAPITIME
// for rapitime RPT_Ipoint
#define CSFR_RPT    0x7800  
#endif

// Global Scheduling Data in TCB 0
#define SCB_MY_CORE			0x0000
#define SCB_MY_SLOT			0x0004
#define SCB_MY_TCB			0x0008
#define SCB_ROUND			0x0010	///< periodic round lenght
#define SCB_DTSL			0x0014	///< First thrid in dominant time slicing list
#define SCB_FPL				0x0018	///< First thrid in fixed priority list
#define SCB_PIQL			0x001c	///< First thrid in lingering guaranteed instruction list
#define SCB_RRIQL			0x0020	///< First thrid in round robin list
#define SCB_UNSAT_THRID			0x00e0	///< Thrid if a PIQ thread was not satisfied
#define SCB_UNSAT_QUANTUM		0x00e4	///< Quantum of the PIQ thread that was not satisfied
#define SCB_EARLY_SATURATION		0x00ec	///< how many cycles where left when last gi thread was saturated in the last round





// -----------------------------------------------------------------------------
// Memory addresses
// -----------------------------------------------------------------------------


// TriCore memory segments
#define MEMSEG_MASK		0xf0000000
#define MEMSEG_TCB		0x90000000
#define MEMSEG_TEXT		0xa0000000
#define MEMSEG_SYS		0xf0000000

#define CTX_AREA_SIZE		0x100		// 4 context blocks with 16 registers, each 32 bits
#define CTX_AREA_START		0x90000000	// start of the memory area for the contexts
#define TCB_SIZE		0x100		// 4 context blocks with 16 registers, each 32 bits
#define TCB_AREA_START		0x90000000	// start of the memory area for the contexts


// Check if address lies within the Code-RAM area 0xa0000000 - 0xa007ffff
#define WITHIN_INSTR_MEM(a)	(((a)&0xfff80000)==0xa0000000)

// Check if address lies within the Context Memory Areas (Segments 0x9 and 0xd)
#define WITHIN_CONTEXT_MEM(a)	(((a)&0xb0000000)==0x90000000)


#define MEM_CLOCK		0xf0000210	// memory adress of the cycle counter





// -----------------------------------------------------------------------------
// offsets in TCB
// -----------------------------------------------------------------------------

#define CTX_OFS_SCHED_FLAGS		0x50
#define CTX_OFS_SCHED_NEXT		0x54
#define CTX_OFS_SCHED_PREV		0x58
#define CTX_OFS_SCHED_QUANTUM		0x5c
#define CTX_OFS_A0			0x60
#define CTX_OFS_A8			0x68
#define CTX_OFS_FCX			0x78
#define CTX_OFS_PC			0x80
#define CTX_OFS_A2			0x88
#define CTX_OFS_D0			0x90
#define CTX_OFS_D2			0x98
#define CTX_OFS_A4			0xa0
#define CTX_OFS_A6			0xa8
#define CTX_OFS_D4			0xb0
#define CTX_OFS_D6			0xb8
#define CTX_OFS_PSW			0xc0
#define CTX_OFS_A10			0xc8
#define CTX_OFS_D8			0xd0
#define CTX_OFS_D10			0xd8
#define CTX_OFS_A12			0xe0
#define CTX_OFS_A14			0xe8
#define CTX_OFS_D12			0xf0
#define CTX_OFS_D14			0xf8


// -----------------------------------------------------------------------------
// Scheduling flags
// -----------------------------------------------------------------------------



// bits 0-7: flags that enforce scheduler action
#define SF_SUSPENDED	0x00000002	///< thrid is in scheduling list, but should currently not be swapped in
					///< if in a slot: this slot is valid, but not ready
#define SF_TERMINATE	0x00000004	///< terminate this thread slot, remove this tcb from list

// bits 8-23: configuration flags 
#define SF_RESIDENT	0x00000100	///< if thrid is not in a slot, swap it in and if it is in one, don't swap out
					///< This flag is independent of the slot state ie if the slot is running
#define SF_DMAA		0x00000200	///< Enable DMAA if this slot has priority 0
#define SF_DONT_STALL	0x00000400	///< don't stall if scheduler reaches a RRIQ thread that still is in a slot

// bits 24-31: scheduling algorithm
#define SF_POLICY_MASK	0xff000000
#define POLICY_FROM_FLAGS(flags)	(((flags)>>24)&0x07)
#define SF_DTS		0x01000000	///< Dominant Time Sharing
#define SF_FP		0x02000000	///< Fixed priority
#define SF_PIQ		0x03000000	///< Periodical Instruction Quantum
#define SF_RRIQ		0x04000000	///< Round Robin by Instruction Quantum

#define MAX_PRIORITY	MAX_SLOTS


// PSW bits
#define PSW_RESET_VALUE	0x00000b80
#define PSW_C		31
#define PSW_V		30
#define PSW_SV		29
#define PSW_AV		28
#define PSW_SAV		27

// Instruction Bits
#define INSTR_MASK_LONG	1	// bit 0 is set if long instruction
#define INSTR_MASK_DATA	2	// bit 1 is set if instruction for data pipeline

#define PIPELINE_A	0
#define PIPELINE_D	1








// -----------------------------------------------------------------------------
//
// Constants for internal decoding
//
// Do NOT change the following constants (unless you know what you do)
// These constants are not visible to an executed binary
//
// -----------------------------------------------------------------------------



#define PC_WIDTH	32	///< the program counter is 32bit wide
#define INSTRUC_WIDTH	32	///< width of instruction, same as width of a data word
#define WORD_WIDTH   	32	///< width of a data word, same as width of instruction word
#define MEM_WORD_WIDTH 	64	///< maximum width of a word to be read/written from/to memory
#define BYTE_WIDTH	8	///< width of a byte, the smallest addressable unit
#define WORD_SIZE	4	///< number of bytes a word consists of

#define REG_COUNT	32	///< maximum register count per pipeline
#define REG_A_COUNT	26	///< address + shadow regs
#define REG_D_COUNT	16	///< data regs

#define SFR_WIDTH	16	///< width of address of a core special function register
#define OP1_WIDTH	8	///< width of opcode1, bit 7:0
#define OP2_WIDTH	10	///< max width of opcode2, variable bit position
#define FUNC_WIDTH	32	///< width of the func signal to the alu, contains many reserved bits
#define PERIOD_WIDTH	32	///<  width of counters for cycles (signed), must hold one period
#define MAX_PERIOD	(((uint32_t)1 << (PERIOD_WIDTH-1))-2)	///<  It is very hard to generate this value without a warning

#define MEM_UNIT	8			//each cell is 8 bit




// Index in mc_pd_table
#define MC_PD_LOAD	0
#define MC_PD_STORE	1
#define MC_PD_BRANCH	2
#define MC_PD_ENDOFMC	3

#define BITRANGE(v,h,l)		(((v)>>(l))&((1<<((h)-(l)+1))-1))
#define BITRANGE_S(v,h,l)	((((int32_t)(v))<<(32-h))>>(32-h+l)) 
#define SIGN_EXTEND_32(v,b)	(((int32_t)((int32_t)(v)<<(32-b)))>>(32-b))
#define SIGN_EXTEND_64(v,b)	(((int64_t)((int64_t)(v)<<(64-b)))>>(64-b))


// issue states
#define IS_INACTIVE	0
#define IS_ACTIVE	1
#define IS_INSERT	2
#define IS_MC		3

#define IS_SUSPENDED	4




// -----------------------------------------------------------------------------
// internal decode of instruction arguments
// -----------------------------------------------------------------------------

#define MIN_OM			REG_COUNT
#define OM_imm0			(MIN_OM + 1)
#define OM_imm4			(MIN_OM + 2)
#define OM_imm8			(MIN_OM + 3)
#define OM_a			(MIN_OM + 4)
#define OM_a1			(MIN_OM + 5)
#define OM_b			(MIN_OM + 6)
#define OM_pc			(MIN_OM + 7)
#define OM_disp8		(MIN_OM + 8)
#define OM_disp24		(MIN_OM + 9)
#define OM_off10		(MIN_OM + 10)
#define OM_off18		(MIN_OM + 11)
#define OM_off18a		(MIN_OM + 12)



#define REG_SHADOW_PCXI		16
#define REG_SHADOW_PSW		(REG_SHADOW_PCXI+1)	/* important for ret microcode */
// 18, 19 are used internally for call and return microcodes
#define REG_SHADOW_FCX		20	// 30
#define REG_SHADOW_LCX		21	// 31
#define REG_SHADOW_PC		22	/* not a real shadow register but marker for PC read/write */



// -----------------------------------------------------------------------------
// width of memory access
// -----------------------------------------------------------------------------
static const unsigned MW_8s		= 1;
static const unsigned MW_8u		= 2;
static const unsigned MW_16s		= 3;
static const unsigned MW_16u		= 4;
static const unsigned MW_16h		= 5;
static const unsigned MW_32		= 6;
static const unsigned MW_64		= 7;



// -----------------------------------------------------------------------------
// constants for conditions
// -----------------------------------------------------------------------------

// constants for conditions
#define COND_MASK_CLASS	48	// 11 0000	00=compare 01=logical op between 2 bits 10=one bit for jnz.t and jz.t
#define COND_MASK_NOT	1	// 00 0001
#define COND_MASK_ZERO	2	// 00 0010
#define COND_MASK_FUNC	12	// 00 1100

// classes, masked with COND_MASK_CLASS
#define COND_CLASS_CMP	0	// 00 xxxx	compare a and b
#define COND_CLASS_BITOP 16	// 01 xxxx	logical operation on bits specified by a and b
#define COND_CLASS_JT	32	// 10 xxxx	check of a bit in a specified by the instruction word

// functions wihin the classes, after masking with COND_MASK_FUNC
#define COND_ULESS	0	// (00) 00xx
#define COND_EQUAL	4	// (00) 01xx
#define COND_LESS	8	// (00) 10xx
#define COND_LESSEQUAL	12	// (00) 11xx

#define COND_SBRN	0	// (10) 00xx	jnz.t, jz.t (16 bit)
#define COND_BRN	4	// (10) 01xx	jnz.t, jz.t (32 bit)

// complete conditional codes for comparing
#define COND_0		0	// 00 0000	never
#define COND_1		1	// 00 0001	always
#define COND_LTU	2	// 00 0010	unsigned a<b
#define COND_GEU	3	// 00 0011	unsigned a>=b
#define COND_EQZ	4	// 00 0100	a==0
#define COND_NEZ	5	// 00 0101	a!=0
#define COND_EQ		6	// 00 0110	a==b
#define COND_NE		7	// 00 0111	a!=b
#define COND_LTZ	8	// 00 1000	a<0
#define COND_GEZ	9	// 00 1001	a>=0
#define COND_LT		10	// 00 1010	a<b
#define COND_GE		11	// 00 1011	a>=b
#define COND_LEZ	12	// 00 1100	a<=0
#define COND_GTZ	13	// 00 1101	a>0
#define COND_LE		14	// 00 1110	a<=b
#define COND_GT		15	// 00 1111	a>b

// complete conditional codes for bit operations
// encoded by a	simple LUT (ba):      11 10 01 00
#define COND_AND	24	// 01 1000
#define COND_ANDN	18	// 01 0010
#define COND_NAND	23	// 01 0111
#define COND_OR		30	// 01 1110
#define COND_ORN	27	// 01 1011
#define COND_NOR	17	// 01 0001
#define COND_XOR	22	// 01 0110
#define COND_XNOR	25	// 01 1001

#define COND_SJNZT	33	// 10 0001
#define COND_SJZT	32	// 10 0000
#define COND_JNZT	37	// 10 0101
#define COND_JZT	36	// 10 0100

#define COND_WIDTH	6



// -----------------------------------------------------------------------------
// opcodes and alu functions
// -----------------------------------------------------------------------------
typedef enum 
	{
		//TriCore instruction mnemonics
		FUNC_UNKNOWN = 0,
		FUNC_NOP = 1, 

		// mop:  only in microcode sequences

		// instructions for address alu
		A_ADD, 	
		A_ADDSC_A,
		A_ADDSC_AL,
		A_ADDSC_AT,
		A_CLCF,		//mop
		A_CPCF,		//mop
		A_CSA,		//mop
		A_INSLD,	//mop
		A_J2,
		A_J4,
		A_JI,
		A_JA,
		A_LDMST,	//mop
		A_LOOP,
		A_MFCR,
		A_POST,
		A_PRE,		
		A_SETC,			// *** not in base version
		A_SRC1,
		A_STT,		//mop
		A_SUB,
		A_SWAP,		//mop
		A_XSRC,

		// instructions only for data alu
		D_ABS_W,
		D_ABSDIF,		// *** not in base version
		D_ADD,
		D_ADDC,		
		D_ADDX, 
		D_AND,
		D_ANDC,			// *** not in base version
		D_ANDN,
		D_AORB,		// for min and max // *** not in base version
		D_CADD,			// *** not in base version
		D_CADDN,			// *** not in base version
		D_CLO,			// *** not in base version
		D_CLZ,			// *** not in base version
		D_CSUB,			// *** not in base version
		D_DEXTR,
		D_DVADJ, D_DVINIT, D_DVINIT_U, D_DVINIT_B, D_DVINIT_BU, D_DVINIT_H, D_DVINIT_HU, D_DVSTEP, D_DVSTEP_U, //ID raus
		D_EQANY_B,		// *** not in base version
		D_EQANY_H,		// *** not in base version 
 		D_EXTR,
		D_EXTR_U,
		D_IMASK,		// *** not in base version
		D_INSC,			// *** not in base version
		D_INSERT,		// *** not in base version
		D_J,
		D_JNED,
		D_JNEI,
		D_MADD32,
		D_MADDU,		// *** not in base version
		D_MSUB32,
		D_MSUBU,		// *** not in base version
		D_MUL,
		D_MUL_U,	// for sqrt.c
		D_NAND,
		D_NOR,
		D_OR,
		D_ORC,			// *** not in base version
		D_ORN,
		D_RSUB, 
		D_SEL,
		D_SELN,
		D_SETC,
		D_SH, 
		D_SHA, 
		D_SHC,			// *** not in base version
		D_SRC1,
		D_SUB,
		D_SUBC,			// *** not in base version
		D_SUBX, 		// *** not in base version
		D_XNOR,
		D_XOR,
		D_XORC,			// *** not in base version

		D_BMERGE,		// *** not in 2nd version
		D_BSPLIT,		// *** not in 2nd version
		D_CLS,			// *** not in 2nd version
		D_CSUBN,		// *** not in 2nd version
		D_PACK,			// *** not in 2nd version
		D_PARITY,		// *** not in 2nd version
		D_UNPACK,		// *** not in 2nd version
		
		// floating point instructions
		F_ADD,
		F_CMP,
		F_DIV,
		F_FTOI,
		F_FTOIZ,
		F_FTOQ31,
		F_FTOQ31Z,
		F_FTOU,
		F_FTOUZ,
		F_ITOF,
		F_MADD,
		F_MSUB,
		F_MUL,
		F_Q31TOF,
		F_QSEED,
		F_SUB,
		F_UPDFL,
		F_UTOF,

		// branch and memory instructions (not implemented in new way)
		C_SWAPIN,
		C_SWAPOUT,
		C_RELOAD
		
	} alu_func_t;

static const alu_func_t END_OF_MC = FUNC_UNKNOWN;


#endif /* _INSTRSET_INCL_H_ */
