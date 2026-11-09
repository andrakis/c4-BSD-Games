# c4.mk -- the BSD games' part of c4's Makefile. c4 includes it from
# games/bsdgames/c4.mk, its submodule, and runs it from c4's root, with c4's
# tools and variables (games/README.md in c4). Without it, c4 builds everything
# else, and test-bsdgames says it was skipped.

# Each game is its units, with c4lc, and c4/c4bsd.c: the same objects for C4IX,
# the bare machine and C4DOS. They get a game's generated pathnames.h from
# c4/NAME. BSD_TEXT is the games that use no curses, BSD_CURSES those on PDCurses
# (games/pdcurses). Upstream's own header files are for glibc, so its include/
# is not used. BSD_U_NAME is a game's units, from the repository's root.
BSD        := games/bsdgames
BSD_TEXT   := fish wump pig quiz adventure backgammon trek monop
BSD_CURSES :=
ifneq ($(wildcard games/pdcurses/c4.mk),)
BSD_CURSES := hangman rain worm gomoku snake canfield robots tetris cribbage mille boggle battlestar atc sail phantasia hack
endif
# Of those, the ones that write files as they play (c4/tests/writes) need a disk
# that can be written, which only C4IX's /tmp is: they are on no other disk
# (c4-images.sh), and their test is on C4IX alone.
BSD_GAMES  := $(BSD_TEXT) $(BSD_CURSES)

BSD_U_fish      := fish/fish
BSD_U_wump      := wump/wump
BSD_U_pig       := pig/pig
BSD_U_quiz      := quiz/quiz quiz/rxp
BSD_U_adventure := $(addprefix adventure/,main init done save subr vocab wizard io crc)
BSD_U_hangman   := $(addprefix hangman/,endgame extern getguess getword main playgame prdata prman prword setup)
BSD_U_rain      := rain/rain
BSD_U_worm      := worm/worm
BSD_U_gomoku    := $(addprefix gomoku/,bdinit bdisp main makemove pickmove stoc)
BSD_U_snake     := snake/snake/snake
BSD_U_canfield  := canfield/canfield/canfield
BSD_U_robots    := $(addprefix robots/,auto extern init_field main make_level move move_robs play_level query rnd_pos score flush_in)
BSD_U_tetris    := tetris/input tetris/shapes tetris/scores tetris/tetris c4/tetris/cscreen
BSD_I_tetris    := -I $(BSD)/tetris
BSD_U_cribbage  := $(addprefix cribbage/,extern crib cards instr io score support)
BSD_U_mille     := $(addprefix mille/,comp end extern init mille misc move print roll save types varpush)
BSD_U_boggle    := $(addprefix boggle/boggle/,bog help mach prtable timer word)
BSD_U_trek      := $(addprefix trek/,abandon attack autover capture cgetc check_out checkcond compkl computer damage damaged dcrept destruct dock dumpgame dumpme dumpssradio events externs getcodi getpar help impulse initquad kill klmove lose lrscan main move nova out phaser play ram ranf rest schedule score setup setwarp shield snova srscan systemname torped visual warp win)
BSD_U_monop     := $(addprefix monop/,monop cards execute getinp houses jail misc morg print prop rent roll spec trade)
BSD_U_battlestar := $(addprefix battlestar/,battlestar command1 command2 command3 command4 command5 command6 command7 cypher dayfile dayobjs fly getcom globals init misc nightfile nightobjs parse room save words)
BSD_U_atc       := $(addprefix atc/,extern graphics input list log main tunable update) c4/atc/grammar
BSD_I_atc       := -I $(BSD)/atc
BSD_U_backgammon := $(addprefix backgammon/common_source/,allow board check fancy init odds one save subs table)                     $(addprefix backgammon/backgammon/,extra main move text version)
BSD_I_backgammon := -I $(BSD)/backgammon/common_source -D V7=1
BSD_U_sail      := $(addprefix sail/,assorted dr_1 dr_2 dr_3 dr_4 dr_5 dr_main game globals lo_main main misc parties pl_1 pl_2 pl_3 pl_4 pl_5 pl_6 pl_7 pl_main sync version)
BSD_U_phantasia := $(addprefix phantasia/,fight gamesupport interplayer io main misc phantglobs) c4/phantasia/phfiles
BSD_I_phantasia := -I $(BSD)/phantasia
BSD_U_hack      := $(addprefix hack/,alloc hack.Decl hack.apply hack.bones hack hack.cmd hack.do hack.do_name hack.do_wear hack.dog hack.eat hack.end hack.engrave hack.fight hack.invent hack.ioctl hack.lev hack.main hack.makemon hack.mhitu hack.mklev hack.mkmaze hack.mkobj hack.mkshop hack.mon hack.monst hack.o_init hack.objnam hack.options hack.pager hack.potion hack.pri hack.read hack.rip hack.rumors hack.save hack.search hack.shk hack.shknam hack.steal hack.termcap hack.timeout hack.topl hack.track hack.trap hack.tty hack.u_init hack.unix hack.vault hack.version hack.wield hack.wizard hack.worm hack.worn hack.zap rnd) c4/hack/c4hack
BSD_I_hack      := -I .bsd/hack

BSD_HDRS  := $(wildcard $(BSD)/c4/*.h $(BSD)/c4/*/*.h) \
             $(wildcard src/c4ix/libc/include/*.h src/c4ix/libc/include/sys/*.h)
BSD_CC    := ./c4sp32 -c 400000 src/c4sp/lisp/c4lc.lisp -O $(IMG_G) -mfuse -fno-builtin -c -conforming \
             -D C4BSD_GUEST=1 -I $(BSD)/c4 -I src/c4ix/libc/include -I games/pdcurses

# bsd_unit GAME UNIT
define bsd_unit
.bsd/$(1)/$(notdir $(2)).c4o: $(BSD)/$(2).c $$(wildcard $(BSD)/$(dir $(2))*.h) $$(BSD_HDRS) $$(C4LC_LISP) .img_g | c4sp32
	@mkdir -p .bsd/$(1)
	$$(BSD_CC) $(if $(filter $(1),$(BSD_CURSES)),-D BSD_CURSES_GAME=1) -I $(BSD)/c4/$(1) -I $(BSD)/$(dir $(2)) $(BSD_I_$(1)) $$< $$@ > /dev/null
bsdgames-objs: .bsd/$(1)/$(notdir $(2)).c4o
endef
$(foreach g,$(BSD_GAMES),$(foreach u,$(BSD_U_$(g)),$(eval $(call bsd_unit,$(g),$(u)))))

bsdgames-objs: .bsd/c4bsd.c4o .bsd/c4bsd-jmp.c4o
.bsd/c4bsd%.c4o: $(BSD)/c4/c4bsd%.c $(BSD_HDRS) $(C4LC_LISP) .img_g | c4sp32
	@mkdir -p .bsd
	$(BSD_CC) $< $@ > /dev/null
.bsd/c4bsd.c4o: $(BSD)/c4/c4bsd.c $(BSD_HDRS) $(C4LC_LISP) .img_g | c4sp32
	@mkdir -p .bsd
	$(BSD_CC) $< $@ > /dev/null
ifneq ($(BSD_CURSES),)
bsdgames-objs: .bsd/c4bsd-curses.c4o pdcurses-objs
endif

# adventure's text is in its program, lightly encrypted by a tool of its own,
# which must use the game's random(): c4bsd.c's
bsdgames-objs: .bsd/adventure/data.c4o
.bsd/adventure/data.c: $(BSD)/adventure/setup.c $(BSD)/adventure/glorkz $(BSD)/c4/c4bsd.c $(BSD_HDRS)
	@mkdir -p .bsd/adventure
	gcc -w -D_DEFAULT_SOURCE -include $(BSD)/c4/c4bsd.h -idirafter $(BSD)/c4 -I $(BSD)/adventure -o .bsd/adventure/setup $(BSD)/adventure/setup.c $(BSD)/c4/c4bsd.c
	.bsd/adventure/setup $(BSD)/adventure/glorkz > $@
.bsd/adventure/data.c4o: .bsd/adventure/data.c $(C4LC_LISP) .img_g | c4sp32
	$(BSD_CC) $< $@ > /dev/null

# hack's objects' names, by its own tool, for both builds
.bsd/hack/hack.onames.h: $(BSD)/hack/makedefs.c $(BSD)/hack/def.objects.h
	@mkdir -p .bsd/hack
	gcc -w -o .bsd/hack/makedefs $(BSD)/hack/makedefs.c
	.bsd/hack/makedefs $(BSD)/hack/def.objects.h > $@
$(foreach u,$(BSD_U_hack),.bsd/hack/$(notdir $(u)).c4o) .bsd/host/hack: .bsd/hack/hack.onames.h

GAME_OBJS += bsdgames-objs
PHONY += bsdgames-objs

# ... and by gcc for the host, the reference test-bsdgames holds them to.
# 32-bit, as c4 is. glibc's headers win over c4/'s, with -idirafter.
BSD_HOST_CC := gcc -m32 -O1 -w -std=gnu99 -fcommon -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -D_DEFAULT_SOURCE -D_GNU_SOURCE -include $(BSD)/c4/c4bsd.h -idirafter $(BSD)/c4
# bsd_host GAME EXTRA-SOURCES
define bsd_host
.bsd/host/$(1): $(BSD_U_$(1):%=$(BSD)/%.c) $(BSD)/c4/c4bsd.c $$(BSD_HDRS) $(2)
	@mkdir -p .bsd/host
	$$(BSD_HOST_CC) -I $(BSD)/c4/$(1) $(sort $(foreach u,$(BSD_U_$(1)),-I$(BSD)/$(dir $(u)))) $(BSD_I_$(1)) -o $$@ $(BSD_U_$(1):%=$(BSD)/%.c) $(BSD)/c4/c4bsd.c $(2) -lm
bsdgames-host: .bsd/host/$(1)
endef
define bsd_host_curses
.bsd/host/$(1): $(BSD_U_$(1):%=$(BSD)/%.c) $(BSD)/c4/c4bsd.c $(BSD)/c4/c4bsd-curses.c $$(BSD_HDRS) | pdcurses-host
	@mkdir -p .bsd/host
	$$(BSD_HOST_CC) -DBSD_CURSES_GAME=1 -I games/pdcurses -I $(BSD)/c4/$(1) $(sort $(foreach u,$(BSD_U_$(1)),-I$(BSD)/$(dir $(u)))) $(BSD_I_$(1)) -o $$@ $(BSD_U_$(1):%=$(BSD)/%.c) $(BSD)/c4/c4bsd.c $(BSD)/c4/c4bsd-curses.c $$(PDC_HOST_OBJS) -lm
bsdgames-host: .bsd/host/$(1)
endef
$(foreach g,$(filter-out adventure,$(BSD_TEXT)),$(eval $(call bsd_host,$(g),)))
$(eval $(call bsd_host,adventure,.bsd/adventure/data.c))
$(foreach g,$(BSD_CURSES),$(eval $(call bsd_host_curses,$(g))))
PHONY += bsdgames-host

# test-bsdgames: c4/tests/run.sh. Each game plays a script from the same seed
# on gcc's build, the bare machine, C4DOS and C4IX, and must say the same.
test-bsdgames: bsdgames-host c4bb-images
	sh $(BSD)/c4/tests/run.sh $(BSD)/c4/tests .bsd/host .bsd_t $(BSD_GAMES)
GAME_TESTS += test-bsdgames
PHONY += test-bsdgames
