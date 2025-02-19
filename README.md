MOS step ratios

```
1:1 Equal
5:4 
4:3 
3:2  Soft
5:3 
2:1 Basic
5:2 
3:1  Hard
4:1 
5:1 
Collapsed
```

scale: 
period and degrees in cents

kbm: 
anchor & its freq, 
root & scale pattern

result: 
ordered map of included notes. 
define as period#. + degree#.
convert 0.0

layout def: 
hex where the root is.
map steps R then DL.
gaps too big then will skip some entries

given an edo, assign tiers 
meantone rule
figure out exceptions and set
tier equals number of sharps (can ve negative

if edo is a weird one, manually set 
else, get size of fifth
anchor note is scale zero
color of Do is scale X
count until 
tier 0
F C G D A E B
tier 1, but what if its enharmoni

?
count by 4s. 101 means from flat 11p
 F C G D   A
size of fifth:
if edo is a weird one


color palettes:
1-12 different primary hues
0 white
-1 +1 black
then different levels of same hue

kite style: find patent val of each prime, map that hue to that tier

!! add a quick transpose up down and input size of trasp the qose step in cents

store
base color
gradient object info

assign color to scale.
1) get okLCH of each RGB maximum.
2) calculate euclid difference
3) assign to spread am
euclid difference
calculatcolor difference

always need a period size / octave.

types of tunings

anything in SCALA and KBM

periodic = equave becomes period

regardless of step size,

LAYOUT by step axis (member of struct)
can attempt to guess step axis in certain scenarios but best to just let it be defined as a rule.

SCALE is permitted (SCALA superset, KBM subset)

PALETTE default to gradient (rainbow, kite) over scale pitch in relation to equave
PALETTE option to do tiers. MOS scales are tiered by sub and superscale. EDOs are tiered against meantone.

collect full MIDI map, select notes from tuning table, likely will have unisons. allow off range to be null notes.

×××××××
non periodic = equave is not a period{
JI = ratio axis
cents wise = cent axis
support 2 axes is trivial.
3 axes means only one step in the 3d direction.

LAYOUT by cents/ratio lattice
SCALE not used
PALETTE is kite if JI
PALETTE is gradient over equave if not JI
PALETTE can be tiered radially by distance.

MOS scale generator?
period 1200
generator 699
steps 7
--
step A1: make.
0 699 1398 2097 2796 3495 4194
step A2: sort.
0 198 396 594 699 897 1095
198 198 198 105 198 198 105
step A3: assert valid via algorithm
step A4: calculate daughter scale
substitute 198 with 93 + 105
send to SCL

step A5: expand tuning table to include all pitches of daughter scale, this is the pitch universe

layout MOS wicki hayden style

step B1: more common step is +1 hex
step B2: calculate Hex software style angle, use that to determine vector of less common step
step B3: pick pitches to fill out layout
step B4: tuning table = this minus outliers to make at most 128

scale MOS super sub set default

step C1: MOS is in-scale, sharps are out

palette depends

step D2: color by rainbow step, dim sharps? or two tone.

layout assumes C = 1.
mode means shift the

2d lattice cents:
input period, X and Y cent. will be isomorphic but if coprime then enharmonics will exist
* creates tuning table
* creates layout (lattice)
* does not create scale unless MOS
* default color by rainbow interval, kite avail if period is 3 limit

2d lattice by JI:
input JI fraction X and Y (uint/uint) and equave (period)
* creates tuning table
* creates layout lattice
* does not create scale -- all set to 1
* special color kite by JI

write tuning table that covers full grid
then octave equivalence

MOS scale
input generator and period and set size. optional superset (not isomorphic) or infinite (calc large and small and pass to 2d lattice)
* creates tuning table, if superset then limit to superset degrees
* creates layout wicki/hex app style
BAAA
AAA
or a bosanquet style
A A A
     B A A ...
if using superset, by scale degrees, else by cents and permit looping
* creates scale
* if superset then can color by tier, rainbow, dim sharp flat, etc
otherwise color like 2d lattice

from SCL and KBM file, gives period and step definition
* creates tuning
* does not create layout, invent a default
* createa scale (assume x = out of scale)
* does not create coloring or tiers, default to rainbow or kite

if you save a terpstra app layout it will give you the step vectors and the color schematic.


disambiguate
https://en.xen.wiki/w/Scale
scale is the subset.
tuning is the superset.
scale setting mode,
highlight just one set of keys in one period (maybe define in setup)
can cycle the mode, or let user toggle on and off what note is in key, then save.

palette cycle mode
cycle through palettes if tiers available. give extra options for 12 and 7 note scales (ie Orff colors)

tuning mode
select a note, it displays offset from concert, you retune it, save, the rest recalcs.

layout mode
1) recenter -- select a new anchor (key), brings layout with you in
2) nudge screen 1 hex
3) rotate 60
4) rotate OLED

set anchor = offset from concert, and CC midi

transpose is done by 12edo semitone, internally it changes anchor +cents without changing layout

MTS tuning table
1. get the set of pitches
2. sort by pitch
3. count uniques
4. disable the lowest or highest in order of distance from middle 64 (E4) until table is 128 in size
5. construct the MIDI message
6. make it so that the noteOn plays the assoc midi note

MIDI note is a one to one with frequency

scala -> period and steps in cents
kbm->

size of map
1st midi note to tune
last midi note to tune
anchor note (scale degree zero)
anchor note (global tuning)
Hz of that note
formal octave
vector of mapping
