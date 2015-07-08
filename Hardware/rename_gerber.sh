#!/bin/bash
rm *pnp.txt
for i in Brew*.*; do mv "$i" "BrewDuino.${i##*.}"; done
mv BrewDuino.gm1 BrewDuino.gko
mv BrewDuino.txt BrewDuino.xln
zip BrewDuino.zip Brew*.g*
zip BrewDuino.zip Brew*.x*
