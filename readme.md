# Meebleeps Mutant Mozzi Synth

My first Arduino project started with the plan to push lfo/envelope voltages to my Volca Modular, but thanks to inspiration from Hagiwo and the Mozzi Library it turned into a cool little generative synth that I can let run all day, built to sync & fit with my Volca collection.

https://www.youtube.com/watch?v=d2Lml1Z9uWc

Source released under Creative Commons ttribution-NonCommercial-ShareAlike 4.0

***
__Important build-note:__ my builds have used normally-closed switches, so the code assumes this.  A couple of people building this have had issues because they have normally-open switches, so to adjust for this edit the code in updateButtonControls() to invert the results of each call to digitalRead(). 
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


## Controls

### Button Controls

| Button Index 	| Pin	| Name		| Function					| Alternative Function	|
| --- 		| --- 	| --- 		| --- 						| --- 			|
| 0		| D4	| Func		| Access alternative control function 		| n/a | 
| 1		| D5	| Start		| Starts/stops the sequencer			| Tap-tempo | 
| 2		| D6	| Scales	| Cycle through available musical scales	| Cycle through available mutation algorithms | 
| 3		| D3	| Root		| Increase the tonic note			| Decrease the tonic note | 
| 4		| D10	| Rec		| Hold to record knob movements			| Hold while moving a knob to clear the recorded value  | 
| 5		| D12	| Clone		| Retrigger the current step


### Analog Controls

| Input Index 	| Pin	| Name		| Function						| Alternative Function	|
| --- 		| --- 	| --- 		| --- 							| --- 			|
| 0		| A0	| Species		| Oscillator 2 detune					| Amount of sidechain/ducking effect	|
| 1		| A1	| Lifespan	| Note length						| n/a	|
| 2		| A2	| Mutation	| Probabilty that sequence will change over time	| Probability of a note playing on any step	|
| 3		| A3	| Shape		| Shape of the filter envelope				| n/a	|
| 4		| A4	| Colour	| Base filter value					| Filter resonance	|
| 5		| A5	| Population	| Number of steps in the sequence 1-16			| n/a	|

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
