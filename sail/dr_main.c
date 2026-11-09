/*	$NetBSD: dr_main.c,v 1.12 2003/08/07 09:37:42 agc Exp $	*/

/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#ifndef lint
#if 0
static char sccsid[] = "@(#)dr_main.c	8.2 (Berkeley) 4/16/94";
#else
__RCSID("$NetBSD: dr_main.c,v 1.12 2003/08/07 09:37:42 agc Exp $");
#endif
#endif /* not lint */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "extern.h"
#include "driver.h"

/*
 * c4: the driver is not a process of its own, which needs fork. pl_main.c calls
 * dr_init where it would have forked one, and newturn calls dr_turn every turn,
 * as the driver's loop did after its sleep(7). The driver's own records are
 * applied as it writes them (sync.c), in the same memory as the player's.
 */
int
dr_init(void)
{
	struct ship *sp;
	int nat[NNATION];
	int n;

	if (game < 0 || game >= NSCENE) {
		fprintf(stderr, "DRIVER: Bad game number %d\n", game);
		exit(1);
	}
	cc = &scene[game];
	ls = SHIP(cc->vessels);
	for (n = 0; n < NNATION; n++)
		nat[n] = 0;
	foreachship(sp) {
		if (sp->file == NULL &&
		    (sp->file = (struct File *)calloc(1, sizeof (struct File))) == NULL) {
			fprintf(stderr, "DRIVER: Out of memory.\n");
			exit(1);
		}
		sp->file->index = sp - SHIP(0);
		sp->file->loadL = L_ROUND;
		sp->file->loadR = L_ROUND;
		sp->file->readyR = R_LOADED|R_INITIAL;
		sp->file->readyL = R_LOADED|R_INITIAL;
		sp->file->stern = nat[sp->nationality]++;
		sp->file->dir = sp->shipdir;
		sp->file->row = sp->shiprow;
		sp->file->col = sp->shipcol;
	}
	windspeed = cc->windspeed;
	winddir = cc->winddir;
	return 0;
}

/* One turn of the driver's: 0, or -1 when the game is over */
int
dr_turn(void)
{
	if (Sync() < 0)
		return -1;
	if (next() < 0)
		return -1;
	unfoul();
	checkup();
	prizecheck();
	moveall();
	thinkofgrapples();
	boardcomp();
	compcombat();
	resolve();
	reload();
	checksails();
	if (Sync() < 0)
		return -1;
	return 0;
}

int
dr_main(void)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	dr_init();
	people = 0;
	for (;;) {
		sleep(7);
		if (dr_turn() < 0)
			break;
	}
	sync_close(1);
	return 0;
}
