/*	$NetBSD: sync.c,v 1.23 2004/09/07 13:20:39 jrf Exp $	*/

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
static char sccsid[] = "@(#)sync.c	8.2 (Berkeley) 4/28/95";
#else
__RCSID("$NetBSD: sync.c,v 1.23 2004/09/07 13:20:39 jrf Exp $");
#endif
#endif /* not lint */

#include <sys/file.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "extern.h"
#include "pathnames.h"

#define BUFSIZE 4096

static int	sync_update(int, struct ship *, const char *, long, long, long, long);

static char sync_buf[BUFSIZE];
static char *sync_bp = sync_buf;

void
fmtship(char *buf, size_t len, const char *fmt, struct ship *ship)
{
	while (*fmt) {
		if (len-- == 0) {
			*buf = '\0';
			return;
		}
		if (*fmt == '$' && fmt[1] == '$') {
			size_t l = snprintf(buf, len, "%s (%c%c)",
			    ship->shipname, colours(ship), sterncolour(ship));
			buf += l;
			len -= l - 1;
			fmt += 2;
		}
		else
			*buf++ = *fmt++;
	}

	if (len > 0)
		*buf = '\0';
}


/*VARARGS3*/
void
makesignal(struct ship *from, const char *fmt, struct ship *ship, ...)
{
	char message[BUFSIZ];
	char format[BUFSIZ];
	va_list ap;

	va_start(ap, ship);
	fmtship(format, sizeof(format), fmt, ship);
	vsprintf(message, format, ap);
	va_end(ap);
	Writestr(W_SIGNAL, from, message);
}

/*VARARGS2*/
void
makemsg(struct ship *from, const char *fmt, ...)
{
	char message[BUFSIZ];
	va_list ap;

	va_start(ap, fmt);
	vsprintf(message, fmt, ap);
	va_end(ap);
	Writestr(W_SIGNAL, from, message);
}

/*
 * c4: one program plays the game, the player and the driver both (pl_main.c),
 * so there is no sync file for others to read and nobody else's records to
 * read from it. Write and Writestr apply each record as they make it, which is
 * all a record does in the program that made it.
 */
int
sync_exists(int game __attribute__((__unused__)))
{
	return 0;
}

int
sync_open(void)
{
	sync_bp = sync_buf;
	return 0;
}

void
sync_close(int remove __attribute__((__unused__)))
{
}

void
Write(int type, struct ship *ship, long a, long b, long c, long d)
{

	sprintf(sync_bp, "%d %d 0 %ld %ld %ld %ld\n",
		       type, ship->file->index, a, b, c, d);
	while (*sync_bp++)
		;
	sync_bp--;
	if (sync_bp >= &sync_buf[sizeof sync_buf])
		abort();
	sync_update(type, ship, NULL, a, b, c, d);
}

void
Writestr(int type, struct ship *ship, const char *a)
{
	sprintf(sync_bp, "%d %d 1 %s\n", type, ship->file->index, a);
	while (*sync_bp++)
		;
	sync_bp--;
	if (sync_bp >= &sync_buf[sizeof sync_buf])
		abort();
	sync_update(type, ship, a, 0, 0, 0, 0);
}

int
Sync(void)
{
	sync_bp = sync_buf;		/* c4: applied already, and nobody to send them to */
	return 0;
}

static int
sync_update(int type, struct ship *ship, const char *astr, long a, long b, long c, long d)
{
	switch (type) {
	case W_DBP: {
		struct BP *p = &ship->file->DBP[a];
		p->turnsent = b;
		p->toship = SHIP(c);
		p->mensent = d;
		break;
		}
	case W_OBP: {
		struct BP *p = &ship->file->OBP[a];
		p->turnsent = b;
		p->toship = SHIP(c);
		p->mensent = d;
		break;
		}
	case W_FOUL: {
		struct snag *p = &ship->file->foul[a];
		if (SHIP(a)->file->dir == 0)
			break;
		if (p->sn_count++ == 0)
			p->sn_turn = turn;
		ship->file->nfoul++;
		break;
		}
	case W_GRAP: {
		struct snag *p = &ship->file->grap[a];
		if (SHIP(a)->file->dir == 0)
			break;
		if (p->sn_count++ == 0)
			p->sn_turn = turn;
		ship->file->ngrap++;
		break;
		}
	case W_UNFOUL: {
		struct snag *p = &ship->file->foul[a];
		if (p->sn_count > 0) {
			if (b) {
				ship->file->nfoul -= p->sn_count;
				p->sn_count = 0;
			} else {
				ship->file->nfoul--;
				p->sn_count--;
			}
		}
		break;
		}
	case W_UNGRAP: {
		struct snag *p = &ship->file->grap[a];
		if (p->sn_count > 0) {
			if (b) {
				ship->file->ngrap -= p->sn_count;
				p->sn_count = 0;
			} else {
				ship->file->ngrap--;
				p->sn_count--;
			}
		}
		break;
		}
	case W_SIGNAL:
		if (mode == MODE_PLAYER) {
			if (nobells)
				Signal("$$: %s", ship, astr);
			else
				Signal("\7$$: %s", ship, astr);
		}
		break;
	case W_CREW: {
		struct shipspecs *s = ship->specs;
		s->crew1 = a;
		s->crew2 = b;
		s->crew3 = c;
		break;
		}
	case W_CAPTAIN:
		strlcpy(ship->file->captain, astr,
			sizeof ship->file->captain);
		break;
	case W_CAPTURED:
		if (a < 0)
			ship->file->captured = 0;
		else
			ship->file->captured = SHIP(a);
		break;
	case W_CLASS:
		ship->specs->class = a;
		break;
	case W_DRIFT:
		ship->file->drift = a;
		break;
	case W_EXPLODE:
		if ((ship->file->explode = a) == 2)
			ship->file->dir = 0;
		break;
	case W_FS:
		ship->file->FS = a;
		break;
	case W_GUNL: {
		struct shipspecs *s = ship->specs;
		s->gunL = a;
		s->carL = b;
		break;
		}
	case W_GUNR: {
		struct shipspecs *s = ship->specs;
		s->gunR = a;
		s->carR = b;
		break;
		}
	case W_HULL:
		ship->specs->hull = a;
		break;
	case W_MOVE:
		strlcpy(ship->file->movebuf, astr,
			sizeof ship->file->movebuf);
		break;
	case W_PCREW:
		ship->file->pcrew = a;
		break;
	case W_POINTS:
		ship->file->points = a;
		break;
	case W_QUAL:
		ship->specs->qual = a;
		break;
	case W_RIGG: {
		struct shipspecs *s = ship->specs;
		s->rig1 = a;
		s->rig2 = b;
		s->rig3 = c;
		s->rig4 = d;
		break;
		}
	case W_RIG1:
		ship->specs->rig1 = a;
		break;
	case W_RIG2:
		ship->specs->rig2 = a;
		break;
	case W_RIG3:
		ship->specs->rig3 = a;
		break;
	case W_RIG4:
		ship->specs->rig4 = a;
		break;
	case W_COL:
		ship->file->col = a;
		break;
	case W_DIR:
		ship->file->dir = a;
		break;
	case W_ROW:
		ship->file->row = a;
		break;
	case W_SINK:
		if ((ship->file->sink = a) == 2)
			ship->file->dir = 0;
		break;
	case W_STRUCK:
		ship->file->struck = a;
		break;
	case W_TA:
		ship->specs->ta = a;
		break;
	case W_ALIVE:
		alive = 1;
		break;
	case W_TURN:
		turn = a;
		break;
	case W_WIND:
		winddir = a;
		windspeed = b;
		break;
	case W_BEGIN:
		strcpy(ship->file->captain, "begin");
		people++;
		break;
	case W_END:
		*ship->file->captain = 0;
		ship->file->points = 0;
		people--;
		break;
	case W_DDEAD:
		hasdriver = 0;
		break;
	default:
		fprintf(stderr, "sync_update: unknown type %d\r\n", type);
		return -1;
	}
	return 0;
}
