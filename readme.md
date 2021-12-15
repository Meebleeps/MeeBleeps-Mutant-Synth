# Meebleeps Mutant Mozzi Synth

My first Arduino project started with the plan to push lfo/envelope voltages to my Volca Modular, but thanks to inspiration from Hagiwo and the Mozzi Library it turned into a cool little generative synth that I can let run all day, built to sync & fit with my Volca collection.

https://www.youtube.com/watch?v=d2Lml1Z9uWc

Source released under Creative Commons ttribution-NonCommercial-ShareAlike 4.0

***

## Synth Voice Features

-    2 saw oscillators
-    Multiple tuning modes for 2nd oscillator - off, detune, fifths & octaves up/down
-    Digital low pass filter with variable cutoff and resonance
-    Variable level ducking/sidechain effect

## Sequencer

-    Generative sequencer mutates/evolves at user-defined rate
-    Variable sequence mutation probability & note-density
-    Variable sequencer length (1-16 steps)
-    Sync input & output (Korg Volca compatible)
-    Selectable tonic note
-    Selectable scale quantisation (Major, Minor, Pentatonic, Phrygian (GOA!), Octaves, Fifths)
-    16-step parameter-lock recording of synth parameters (the Elektron way!)
-    Retrig (clone) button for fills
-    Tap-tempo control

## Hardware

-    Arduino Nano (Elegoo)
-    Simple Mozzi output circuit
-    CV inputs (volca modular levels) for synth and sequencer parameters
-    Designed to fit into Volca form factor
-    Laser-cut metalisized acrylic faceplate
-    Laser-cut wooden box
-    9V DC Power input
-    Access to nano's USB port for firmware upgrades

## Problems (mostly hardware due to this being my first project!)

-    some sort of overflow bug linked to the low pass filter which causes the unit to enter a distorted output state until the cutoff is set to zero
-    no voltage protection on inputs
-    no buffering of inputs or outputs
-    no volume control
-    probably more unknown unknowns 😂

## Thanks

-    Inspired by Hagiwo's modular builds: https://www.youtube.com/channel/UCxEr...
-    Mozzi audio library: https://sensorium.github.io/Mozzi/
-    Inkscape for vector graphics: https://inkscape.org/
-    Fritzing for circuit design: https://fritzing.org/
-    Great Scott for awesome electronics tips! https://www.youtube.com/channel/UC6mI...
