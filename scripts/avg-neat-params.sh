#!/usr/bin/sh

mean="$(realpath "$MEAN")"

for DIR in multi_move-normal-normal\ 75-60-0-?; do (
	cd "$DIR/p/"
	python $mean 3 > scores.avg
	cd "../h/"
	python $mean 3 > scores.avg
) done
