/* grammar.c -- atc's grammar.y and lex.l by hand, for a build with no yacc or lex.
   The same language and the same checks, as one recursive descent: a game file is
   its definitions (update, newplane, width, height) and then its lines (beacon,
   exit, line, airport). c4/atc/... has the game's file names too. */
#include <sys/cdefs.h>
#include "include.h"
#include <setjmp.h>

FILE	*yyin;
int	errors = 0;
int	line = 1;

static int tok;                         /* the token: a character, or one of these */
static int tokval;
enum { TK_END = 256, TK_CONST, TK_DIR, TK_UPDATE, TK_NEWPLANE, TK_WIDTH, TK_HEIGHT,
       TK_AIRPORT, TK_LINE, TK_EXIT, TK_BEACON };

int yyerror(const char *s) {
	fprintf(stderr, "\"%s\": line %d: %s\n", file, line, s);
	errors++;
	return (errors);
}

static int same(const char *a, const char *b) { return strcmp(a, b) == 0; }

static void next_tok(void) {
	static char word[32];
	int c, n;

	for (;;) {
		c = getc(yyin);
		if (c == EOF) { tok = TK_END; return; }
		if (c == '\n') { line++; continue; }
		if (c == ' ' || c == '\t') continue;
		if (c == '#') {
			while ((c = getc(yyin)) != EOF && c != '\n') ;
			line++;
			continue;
		}
		break;
	}
	if (c >= '0' && c <= '9') {
		tokval = 0;
		while (c >= '0' && c <= '9') {
			tokval = tokval * 10 + c - '0';
			c = getc(yyin);
		}
		if (c != EOF) ungetc(c, yyin);
		tok = TK_CONST;
		return;
	}
	if (c >= 'a' && c <= 'z') {
		n = 0;
		while (c >= 'a' && c <= 'z' && n < 31) {
			word[n++] = (char)c;
			c = getc(yyin);
		}
		word[n] = 0;
		if (c != EOF) ungetc(c, yyin);
		if (same(word, "height")) tok = TK_HEIGHT;
		else if (same(word, "width")) tok = TK_WIDTH;
		else if (same(word, "newplane")) tok = TK_NEWPLANE;
		else if (same(word, "update")) tok = TK_UPDATE;
		else if (same(word, "airport")) tok = TK_AIRPORT;
		else if (same(word, "line")) tok = TK_LINE;
		else if (same(word, "exit")) tok = TK_EXIT;
		else if (same(word, "beacon")) tok = TK_BEACON;
		else if (n == 1 && strchr("wedcxzaq", word[0])) { tok = TK_DIR; tokval = word[0]; }
		else tok = word[0];
		return;
	}
	tok = c;
}

/* A syntax error ends the parse, as yacc's does */
static jmp_buf bad;

static void want(int t) {
	if (tok != t) {
		yyerror("syntax error");
		longjmp(bad, 1);
	}
	next_tok();
}

static int a_number(void) {
	int v;
	v = tokval;
	want(TK_CONST);
	return v;
}

static int direction(void) {
	int v;
	v = tokval;
	want(TK_DIR);
	return v;
}

void check_edge(int x, int y) {
	if (!(x == 0) && !(x == sp->width - 1) &&
	    !(y == 0) && !(y == sp->height - 1))
		yyerror("edge value not on edge.");
}

void check_point(int x, int y) {
	if (x < 1 || x >= sp->width - 1)
		yyerror("X value out of range.");
	if (y < 1 || y >= sp->height - 1)
		yyerror("Y value out of range.");
}

void check_linepoint(int x, int y) {
	if (x < 0 || x >= sp->width)
		yyerror("X value out of range.");
	if (y < 0 || y >= sp->height)
		yyerror("Y value out of range.");
}

void check_line(int x1, int y1, int x2, int y2) {
	int d1, d2;

	check_linepoint(x1, y1);
	check_linepoint(x2, y2);
	d1 = ABS(x2 - x1);
	d2 = ABS(y2 - y1);
	if (!(d1 == d2) && !(d1 == 0) && !(d2 == 0))
		yyerror("Bad line endpoints.");
}

void check_edir(int x, int y, int dir) {
	int bad_dir = 0;

	if (x == sp->width - 1)
		x = 2;
	else if (x != 0)
		x = 1;
	if (y == sp->height - 1)
		y = 2;
	else if (y != 0)
		y = 1;

	switch (x * 10 + y) {
	case 0: if (dir != 3) bad_dir++; break;
	case 1: if (dir < 1 || dir > 3) bad_dir++; break;
	case 2: if (dir != 1) bad_dir++; break;
	case 10: if (dir < 3 || dir > 5) bad_dir++; break;
	case 11: break;
	case 12: if (dir > 1 && dir < 7) bad_dir++; break;
	case 20: if (dir != 5) bad_dir++; break;
	case 21: if (dir < 5) bad_dir++; break;
	case 22: if (dir != 7) bad_dir++; break;
	default:
		yyerror("Unknown value in checkdir!  Get help!");
		break;
	}
	if (bad_dir)
		yyerror("Bad direction for entrance at exit.");
}

int checkdefs(void) {
	int err = 0;

	if (sp->width == 0) {
		yyerror("'width' undefined.");
		err++;
	}
	if (sp->height == 0) {
		yyerror("'height' undefined.");
		err++;
	}
	if (sp->update_secs == 0) {
		yyerror("'update' undefined.");
		err++;
	}
	if (sp->newplane_time == 0) {
		yyerror("'newplane' undefined.");
		err++;
	}
	return err ? -1 : 0;
}

/* update = 5; and the others. Returns 0 when the next token isn't one. */
static int definition(void) {
	int which, v;

	which = tok;
	if (which != TK_UPDATE && which != TK_NEWPLANE && which != TK_WIDTH && which != TK_HEIGHT)
		return 0;
	next_tok();
	want('=');
	v = a_number();
	want(';');
	switch (which) {
	case TK_UPDATE:
		if (sp->update_secs != 0) yyerror("Redefinition of 'update'.");
		else if (v < 1) yyerror("'update' is too small.");
		else sp->update_secs = v;
		break;
	case TK_NEWPLANE:
		if (sp->newplane_time != 0) yyerror("Redefinition of 'newplane'.");
		else if (v < 1) yyerror("'newplane' is too small.");
		else sp->newplane_time = v;
		break;
	case TK_WIDTH:
		if (sp->width != 0) yyerror("Redefinition of 'width'.");
		else if (v < 3) yyerror("'width' is too small.");
		else sp->width = v;
		break;
	case TK_HEIGHT:
		if (sp->height != 0) yyerror("Redefinition of 'height'.");
		else if (v < 3) yyerror("'height' is too small.");
		else sp->height = v;
		break;
	}
	return 1;
}

static void *grow(void *p, int n, int size) {
	if (n % REALLOC) return p;
	return p ? realloc(p, (n + REALLOC) * size) : malloc((n + REALLOC) * size);
}

static void a_beacon(void) {
	int x, y;

	want('(');
	x = a_number();
	y = a_number();
	want(')');
	sp->beacon = (BEACON *)grow(sp->beacon, sp->num_beacons, sizeof(BEACON));
	sp->beacon[sp->num_beacons].x = x;
	sp->beacon[sp->num_beacons].y = y;
	check_point(x, y);
	sp->num_beacons++;
}

static void exit_point(void) {
	int x, y, dir;

	want('(');
	x = a_number();
	y = a_number();
	dir = dir_no(direction());
	want(')');
	sp->exit = (EXIT *)grow(sp->exit, sp->num_exits, sizeof(EXIT));
	sp->exit[sp->num_exits].x = x;
	sp->exit[sp->num_exits].y = y;
	sp->exit[sp->num_exits].dir = dir;
	check_edge(x, y);
	check_edir(x, y, dir);
	sp->num_exits++;
}

static void a_airport(void) {
	int x, y, dir;

	want('(');
	x = a_number();
	y = a_number();
	dir = dir_no(direction());
	want(')');
	sp->airport = (AIRPORT *)grow(sp->airport, sp->num_airports, sizeof(AIRPORT));
	sp->airport[sp->num_airports].x = x;
	sp->airport[sp->num_airports].y = y;
	sp->airport[sp->num_airports].dir = dir;
	check_point(x, y);
	sp->num_airports++;
}

static void a_line(void) {
	int x1, y1, x2, y2;

	want('[');
	want('(');
	x1 = a_number();
	y1 = a_number();
	want(')');
	want('(');
	x2 = a_number();
	y2 = a_number();
	want(')');
	want(']');
	sp->line = (LINE *)grow(sp->line, sp->num_lines, sizeof(LINE));
	sp->line[sp->num_lines].p1.x = x1;
	sp->line[sp->num_lines].p1.y = y1;
	sp->line[sp->num_lines].p2.x = x2;
	sp->line[sp->num_lines].p2.y = y2;
	check_line(x1, y1, x2, y2);
	sp->num_lines++;
}

int yyparse(void) {
	int k;

	errors = 0;
	line = 1;
	if (setjmp(bad))
		return errors;
	next_tok();
	if (!definition()) {
		yyerror("syntax error");
		return errors;
	}
	while (definition()) ;
	if (checkdefs() < 0)
		return errors;
	do {
		k = tok;
		if (k != TK_BEACON && k != TK_EXIT && k != TK_LINE && k != TK_AIRPORT) {
			yyerror("syntax error");
			return errors;
		}
		next_tok();
		want(':');
		do {
			if (k == TK_BEACON) a_beacon();
			else if (k == TK_EXIT) exit_point();
			else if (k == TK_LINE) a_line();
			else a_airport();
		} while (tok != ';');
		want(';');
	} while (tok != TK_END);
	if (sp->num_exits + sp->num_airports < 2)
		yyerror("Need at least 2 airports and/or exits.");
	return errors;
}

/* A game's file: on the flat disks a_ and six letters of its name, in lower case
   (c4/tests/data.sh), after any directory, or a_list for the list of them. */
const char *atc_path(const char *s) {
	static char name[12];
	const char *p;
	int i;

	p = strrchr(s, '/');
	p = p ? p + 1 : s;
	if (same(p, GAMES))
		return bsdc_path("a_list");
	name[0] = 'a';
	name[1] = '_';
	for (i = 0; i < 6 && p[i]; i++)
		name[2 + i] = (char)((p[i] >= 'A' && p[i] <= 'Z') ? p[i] + 32 : p[i]);
	name[2 + i] = 0;
	return bsdc_path(name);
}
