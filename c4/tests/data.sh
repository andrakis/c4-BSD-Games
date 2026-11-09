#!/bin/sh
# data.sh DIR -- the games' data in DIR, under the 8.3 names the bare machine and
# C4DOS can have. A game finds them with bsdc_path (c4/c4bsd.c). From c4's root.
BSD=games/bsdgames
D=$1
cp $BSD/fish/fish.instr "$D/fish.ins"
cp $BSD/wump/wump.info "$D/wump.inf"
cp $BSD/cribbage/cribbage.n "$D/cribbage.ins"
cp $BSD/boggle/boggle/helpfile "$D/boghelp"
# quiz: each subject as qz and six letters of its name, which are unique, and
# the index that names them
for f in $BSD/quiz/datfiles/*; do
    n=$(basename "$f")
    case "$n" in Makefrag|index.in) continue ;; esac
    cp "$f" "$D/qz$(echo "$n" | cut -c1-6)"
done
sed 's|^@quiz_dir@/\([^:]\{1,6\}\)[^:]*:|qz\1:|' $BSD/quiz/datfiles/index.in > "$D/qzindex"
# atc's games: a_ and six letters of the name (c4/atc/grammar.c)
for f in $BSD/atc/games/*; do
    n=$(basename "$f")
    case "$n" in
        Makefrag) ;;
        Game_List) cp "$f" "$D/a_list" ;;
        *) cp "$f" "$D/a_$(echo "$n" | tr A-Z a-z | cut -c1-6)" ;;
    esac
done
# phantasia's monsters, which it makes into its own file on its first run
cp $BSD/phantasia/monsters.asc "$D/phmonst.txt"
# hack's: the help, the symbols, and the rumours
cp $BSD/hack/help "$D/hackhelp"
cp $BSD/hack/hh "$D/hackhh"
cp $BSD/hack/data "$D/hackdata"
cp $BSD/hack/rumors "$D/hackrumo"
# hangman's and boggle's words: helios and C4 have no /usr/share/dict, so the
# lower-case words in the games' own text
cat $BSD/fish/fish.instr $BSD/wump/wump.info $BSD/adventure/glorkz $BSD/*/*.6 $BSD/quiz/datfiles/[a-z]* |
    tr -c 'a-zA-Z' '\n' | grep -E '^[a-z]{4,}$' | LC_ALL=C sort -u > "$D/words.txt"
# boggle's dictionary, from those words, by its own tools
T=$(mktemp -d)
gcc -w -D_DEFAULT_SOURCE -I $BSD/c4/boggle -I $BSD/boggle/boggle -o "$T/mkdict" $BSD/boggle/mkdict/mkdict.c
gcc -w -D_DEFAULT_SOURCE -I $BSD/c4/boggle -I $BSD/boggle/boggle -o "$T/mkindex" $BSD/boggle/mkindex/mkindex.c
"$T/mkdict" < "$D/words.txt" > "$D/bogdict" 2> /dev/null
"$T/mkindex" < "$D/bogdict" > "$D/bogindex"
# monop's cards, by its own tool, in 32 bits as the game reads them
gcc -m32 -w -D_DEFAULT_SOURCE -I $BSD/monop -o "$T/initdeck" $BSD/monop/initdeck.c
"$T/initdeck" $BSD/monop/cards.inp "$D/cards.pck" > /dev/null
rm -rf "$T"
