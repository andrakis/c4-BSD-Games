# c4-images.sh c4ix|bare -- the BSD games' part of c4's build-images.sh, which
# sources it from c4's root (games/README.md in c4). Their data goes under 8.3
# names (c4/tests/data.sh); a game with a longer name is cut to 8 characters on
# the games disk. The curses games bring PDCurses (games/pdcurses), and are only
# built when it is here. build-images.sh sources this in its own shell, inside its
# loop over the games, so the names here are bsd_: its g is its own.
BSD_TEXT_GAMES="fish wump pig quiz adventure backgammon trek monop"
BSD_CURSES_GAMES="hangman rain worm gomoku snake canfield robots tetris cribbage mille boggle battlestar atc sail"
# and those that write files as they play: only C4IX has a disk for them (its /tmp)
BSD_WRITES_GAMES="phantasia hack"
# A game's name on the bare machine's and C4DOS's disks: cut to 8 characters,
# and canfield is canfld, since The Ace of Penguins (games/ace) has a Canfield
# too and the games disk is one directory (as C4IX's /usr/games/bin is:
# c4ix.vfs.txt names it canfld there). c4/tests/run.sh says the same.
bsd_short() { case "$1" in canfield) echo canfld ;; *) echo "$1" | cut -c1-8 ;; esac; }
case "$1" in
c4ix)
    rm -rf $DISK/bsdgames
    if [ -f .bsd/c4bsd.c4o ] && [ -f $PB/lib/libc.c4s ]; then
        say "linking the BSD games for C4IX"
        mkdir -p $DISK/bsdgames
        sh games/bsdgames/c4/tests/data.sh $DISK/bsdgames
        for bsd_g in $BSD_TEXT_GAMES; do
            ./c4rlink32 .bsd/$bsd_g/*.c4o .bsd/c4bsd.c4o .bsd/c4bsd-jmp.c4o -l $PB/lib/libc.c4s -o $DISK/c4ix-bsdgames-$bsd_g.c4r > /dev/null
        done
        for bsd_g in $BSD_CURSES_GAMES $BSD_WRITES_GAMES; do
            if [ -f .pdcurses/plat-c4ix.c4o ]; then
                ./c4rlink32 .bsd/$bsd_g/*.c4o .bsd/c4bsd.c4o .bsd/c4bsd-jmp.c4o .bsd/c4bsd-curses.c4o .pdcurses/plat-c4ix.c4o --start-lib .pdcurses/lib/*.c4o -l $PB/lib/libc.c4s -o $DISK/c4ix-bsdgames-$bsd_g.c4r > /dev/null
            fi
        done
    fi ;;
bare)
    if [ -f .bsd/c4bsd.c4o ]; then
        sh games/bsdgames/c4/tests/data.sh $GAMESDISK
        for bsd_g in $BSD_TEXT_GAMES; do
            bsd_s=$(bsd_short $bsd_g)
            item $bsd_s
            ./c4rlink32 .bsd/$bsd_g/*.c4o .bsd/c4bsd.c4o .bsd/c4bsd-jmp.c4o .libc-bare/libc-*.c4o -o $GAMESDISK/$bsd_s.c4r > /dev/null
            GAMES_ON_DISK="$GAMES_ON_DISK $(echo $bsd_s | tr a-z A-Z)"
        done
        for bsd_g in $BSD_CURSES_GAMES; do
            if [ -f .pdcurses/plat-bare.c4o ]; then
                bsd_s=$(bsd_short $bsd_g)
                item $bsd_s
                ./c4rlink32 .bsd/$bsd_g/*.c4o .bsd/c4bsd.c4o .bsd/c4bsd-jmp.c4o .bsd/c4bsd-curses.c4o .pdcurses/plat-bare.c4o --start-lib .pdcurses/lib/*.c4o .libc-bare/libc-*.c4o -o $GAMESDISK/$bsd_s.c4r > /dev/null
                GAMES_ON_DISK="$GAMES_ON_DISK $(echo $bsd_s | tr a-z A-Z)"
            fi
        done
    fi ;;
esac
