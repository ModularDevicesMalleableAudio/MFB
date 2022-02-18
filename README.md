# MFB Filter Bank
A Fixed bank of 16 Band Pass filters with amplitude control per filter, and envelope followers to allow for controlling 
the amplitude of frequency bands using the amplitude of another signal (spectral transfer).

### I/O
- Stereo i/o - 16 fixed frequency filters, outputting to 8 even numbered filter banks (left) and 8 odd numbered banks
(right).
- Equally, the filter banks can be used as 2 sets of separate mono filter banks. Although because of the even/odd 
frequency split right now, this will cut frequencies even with all the filters fully open.

### Features
 - 2 banks of even (0,2,4,...,12,14,16) & odd (1,3,5,...11,13,15) fixed frequency filters, with amplitudes controlled by 
knobs for each bank (switches scroll between banks).
 - Each filter has its own enevelop follower that can be used to modulate the amplitude of it's neighbouring filter (ie.
filter 0 envelope follower modulates the amplitude of filter 1). This allows spectral transfer between the odd and even 
inputs (odd->even / even->odd / both)
 
### To do
- Use MIDI notes to trigger amplitude per filter
  - MIDI note to trigger (controllable) ADSR amplitude envelope for a given filter.
- Use MIDI CC to control amplitdude - direct control via CC.
- CV control of other (selectable) parameters (resonance, drive etc)
- Macro level filter sweep (with ADSR) - pingable (via CV/MIDI note) end-of-the-chain filter sweep.

### Controls

| Control        | Description                 | Comment                                                                   |
|----------------|-----------------------------|---------------------------------------------------------------------------|
| Switch Up/Down | Control Mode                | Selects which filters is controllable by buttons/knobs                    |
| Buttons 1-8    | Envelope follower toggle    | Enables/Disables amplitude to follow envelope from modulator (per filter) |
| Buttons 9-16   | TBC - CV control toggle     | Enables/Disables direct CV control of filter amplitude (per filter)       |
| Knobs 1-8      | Filter Amplitude            | Overall amplitude per filter                                              |
| Audio In 1-2   | Filter bank Input Even/Odd  | Even / Odd carrier signal                                                 |
| Audio Out 1-2  | Filter Bank Output Even/Odd | Filtered Even / Odd output signal                                         |

### Credits
Original [Source Code](https://github.com/electro-smith/DaisyExamples/tree/master/patch/FilterBank) by Ben Sergentanis.