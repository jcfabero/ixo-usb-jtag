/*
 * Copyright (C) 2007 Kolja Waschk, ixo.de
 * Copyright (C) 2017 Alexandru Gagniuc <mr.nuke.me@gmail.com>
 * Copyright (C) 2021 Ein Terakawa <applause@elfmimi.jp>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <fx2regs.h>
#include "hardware.h"

#define TMS_PIN 0
#define TDI_PIN 2
#define TDO_PIN 4
#define TCK_PIN 6

#define PORT_B_OUTPINS ((1 << TCK_PIN) | (1 << TDI_PIN) | (1 << TMS_PIN))
#define PORT_B_INPINS  (1 << TDO_PIN)

/* JTAG pins accessed through bit-addressable SFR for Port B */
sbit at (0x90 | TMS_PIN) TMS; /* Port B.0 */
sbit at (0x90 | TDI_PIN) TDI; /* Port B.2 */
sbit at (0x90 | TDO_PIN) TDO; /* Port B.4 */
sbit at (0x90 | TCK_PIN) TCK; /* Port B.6 */

/* Unused. Purpose unknown. */
void ProgIO_Poll(void) {}

void ProgIO_Enable(void)
{
	OEB &= ~PORT_B_INPINS;
	OEB |= PORT_B_OUTPINS;
}

void ProgIO_Init(void)
{
	/* Set the CPU clock to 48MHz */
	CPUCS = bmCLKOE | bmCLKSPD1;

	/* Use internal 48 MHz, enable output, use "Port" mode for all pins */
	IFCONFIG = bmIFCLKSRC | bm3048MHZ | bmIFCLKOE;

	/* Reset to its default value */
	AUTOPTRSETUP = 0x06;
}

/*
 * Set state of output pins:
 * - d.0 => TCK
 * - d.1 => TMS
 * - d.4 => TDI
 */
void ProgIO_Set_State(unsigned char d)
{
	TCK = d & (1 << 0) ? 1 : 0;
	TMS = d & (1 << 1) ? 1 : 0;
	TDI = d & (1 << 4) ? 1 : 0;
}

unsigned char ProgIO_Set_Get_State(unsigned char d)
{
	ProgIO_Set_State(d);
	return TDO;
}

/*
 * Shift out byte 'c':
 * - Output least significant bit on TDI
 * - Raise TCK
 * - Shift c right
 * - Lower TCK
 * Repeat for all 8 bits
 */
void ProgIO_ShiftOut(unsigned char c)
{
	(void)c; /* argument passed in DPL */

	_asm
	MOV  A,DPL
	;; Bit0
	RRC  A
	MOV  _TDI,C
	SETB _TCK
	;; Bit1
	RRC  A
	CLR  _TCK
	MOV  _TDI,C
	SETB _TCK
	;; Bit2
	RRC  A
	CLR  _TCK
	MOV  _TDI,C
	SETB _TCK
	;; Bit3
	RRC  A
	CLR  _TCK
	MOV  _TDI,C
	SETB _TCK
	;; Bit4
	RRC  A
	CLR  _TCK
	MOV  _TDI,C
	SETB _TCK
	;; Bit5
	RRC  A
	CLR  _TCK
	MOV  _TDI,C
	SETB _TCK
	;; Bit6
	RRC  A
	CLR  _TCK
	MOV  _TDI,C
	SETB _TCK
	;; Bit7
	RRC  A
	CLR  _TCK
	MOV  _TDI,C
	SETB _TCK
	NOP
	CLR  _TCK
	ret
	_endasm;
}

/*
 * Shift out byte C, shift in from TDO:
 * - Read carry from TDO
 * - Output least significant bit on TDI
 * - Raise TCK
 * - Shift c right, append carry (TDO) at left
 * - Lower TCK
 * Repeat for all 8 bits.
 */
unsigned char ProgIO_ShiftInOut(unsigned char c)
{
	(void)c; /* argument passed in DPL */

	_asm
	MOV  A,DPL

	;; Bit0
	MOV  C,_TDO
	RRC  A
	MOV  _TDI,C
	SETB _TCK
	CLR  _TCK
	;; Bit1
	MOV  C,_TDO
	RRC  A
	MOV  _TDI,C
	SETB _TCK
	CLR  _TCK
	;; Bit2
	MOV  C,_TDO
	RRC  A
	MOV  _TDI,C
	SETB _TCK
	CLR  _TCK
	;; Bit3
	MOV  C,_TDO
	RRC  A
	MOV  _TDI,C
	SETB _TCK
	CLR  _TCK
	;; Bit4
	MOV  C,_TDO
	RRC  A
	MOV  _TDI,C
	SETB _TCK
	CLR  _TCK
	;; Bit5
	MOV  C,_TDO
	RRC  A
	MOV  _TDI,C
	SETB _TCK
	CLR  _TCK
	;; Bit6
	MOV  C,_TDO
	RRC  A
	MOV  _TDI,C
	SETB _TCK
	CLR  _TCK
	;; Bit7
	MOV  C,_TDO
	RRC  A
	MOV  _TDI,C
	SETB _TCK
	CLR  _TCK

	MOV  DPL,A
	ret
	_endasm;

	/* return value in DPL */
	return c;
}
