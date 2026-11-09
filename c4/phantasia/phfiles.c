// phfiles.c -- phantasia's files, made where it runs when they aren't there yet:
// what its setup tool did when it was installed (phantasia/setup.c), so that a
// first run needs nothing but the monsters' text. Its characters, scoreboard and
// the rest are kept in the current directory, which must be one that can be
// written: in C4IX, /tmp. The bare machine and C4DOS have no such disk, so the
// game is not on theirs.
//
// The monsters are made into the game's binary file here, by the build that
// reads them, so the file is laid out as that build's struct monster is.
#include "include.h"

static void ph_cant(const char *name) {
    fprintf(stderr, "phantasia: can't make %s here. It keeps its characters in the\n"
        "current directory, which must be one it can write to, such as /tmp.\n", name);
    exit(1);
}

// name, empty, unless it is there
static void ph_make(const char *name) {
    FILE *f;
    f = fopen(name, "r");
    if (f) { fclose(f); return; }
    f = fopen(name, "w");
    if (!f) ph_cant(name);
    fclose(f);
}

void ph_files(void) {
    FILE *f;
    FILE *out;
    ph_make(_PATH_PEOPLE);
    ph_make(_PATH_MESS);
    ph_make(_PATH_LASTDEAD);
    ph_make(_PATH_MOTD);
    ph_make(_PATH_GOLD);
    ph_make(_PATH_VOID);
    ph_make(_PATH_SCORE);

    f = fopen(_PATH_MONST, "r");
    if (f) { fclose(f); return; }
    f = fopen(bsdc_path("phmonst.txt"), "r");
    if (!f) {
        fprintf(stderr, "phantasia: no phmonst.txt, its monsters\n");
        exit(1);
    }
    out = fopen(_PATH_MONST, "w");
    if (!out) ph_cant(_PATH_MONST);
    // as setup.c does it
    Curmonster.m_o_strength = 0.0;
    Curmonster.m_o_speed = 0.0;
    Curmonster.m_maxspeed = 0.0;
    Curmonster.m_o_energy = 0.0;
    Curmonster.m_melee = 0.0;
    Curmonster.m_skirmish = 0.0;
    while (fgets(Databuf, SZ_DATABUF, f) != NULL) {
        sscanf(&Databuf[24], "%lf%lf%lf%lf%lf%d%d%lf",
            &Curmonster.m_strength, &Curmonster.m_brains,
            &Curmonster.m_speed, &Curmonster.m_energy,
            &Curmonster.m_experience, &Curmonster.m_treasuretype,
            &Curmonster.m_type, &Curmonster.m_flock);
        Databuf[24] = '\0';
        strcpy(Curmonster.m_name, Databuf);
        fwrite((char *)&Curmonster, SZ_MONSTERSTRUCT, 1, out);
    }
    fclose(f);
    if (fclose(out) != 0) ph_cant(_PATH_MONST);
}
