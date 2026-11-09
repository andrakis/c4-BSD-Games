/*	$NetBSD: hack.termcap.c,v 1.12 2003/04/02 18:36:40 jsm Exp $	*/
/* For Linux: still using old termcap interface from version 1.9.  */

/*
 * Copyright (c) 1985, Stichting Centrum voor Wiskunde en Informatica,
 * Amsterdam
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of the Stichting Centrum voor Wiskunde en
 * Informatica, nor the names of its contributors may be used to endorse or
 * promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright (c) 1982 Jay Fenlason <hack@gnu.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#ifndef lint
__RCSID("$NetBSD: hack.termcap.c,v 1.12 2003/04/02 18:36:40 jsm Exp $");
#endif				/* not lint */

#include <string.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include "hack.h"
#include "extern.h"
#include "def.flag.h"		/* for flags.nonull */

/*
 * c4: there is no termcap here. Hack's terminal is curses (PDCurses), and these
 * move and clear with it: c4/hack/c4hack.c. The rest of hack writes to stdout,
 * which c4hack.c puts on the screen as a terminal would.
 */
static char     cap[] = "";
char           *HO, *CL, *CE, *UP, *CM, *ND, *XD, *BC, *SO, *SE, *TI, *TE;
char            PC = '\0';
char           *CD;		/* tested in pri.c: docorner() */
int             CO, LI;		/* used in pri.c and whatis.c */

void
startup()
{
	hk_start();
	/* what it can do: everything, by curses */
	HO = CL = CE = UP = CM = ND = XD = BC = SO = SE = CD = cap;
	TI = TE = 0;
	CO = 80;
	LI = 24;
	set_whole_screen();	/* uses LI and CD */
}

void
start_screen()
{
}

void
end_screen()
{
	hk_end();
}

/* Cursor movements */
void
curs(x, y)
	int             x, y;	/* not xchar: perhaps xchar is unsigned and
				 * curx-x would be unsigned as well */
{
	cmov(x, y);
}

void
nocmov(x, y)
	int x, y;
{
	cmov(x, y);
}

void
cmov(x, y)
	int x, y;
{
	hk_move(x - 1, y - 1);
	cury = y;
	curx = x;
}

int
xputc(c)
	char            c;
{
	return (putchar(c));
}

void
xputs(s)
	char           *s;
{
	fputs(s, stdout);
}

void
cl_end()
{
	hk_clear(1);
}

void
clear_screen()
{
	hk_clear(0);
	hk_move(0, 0);
	curx = cury = 1;
}

void
home()
{
	hk_move(0, 0);
	curx = cury = 1;
}

void
standoutbeg()
{
	hk_standout(1);
}

void
standoutend()
{
	hk_standout(0);
}

void
backsp()
{
	putchar('\b');
	curx--;
}

void
bell()
{
	hk_bell();	/* curx does not change */
}

void
delay_output()
{
	/* delay 50 ms, with what has been drawn on the screen */
	(void) fflush(stdout);
	usleep(50000);
}

void
cl_eos()
{				/* free after Robert Viduya *//* must only be
				 * called with curx = 1 */
	hk_clear(2);
}
