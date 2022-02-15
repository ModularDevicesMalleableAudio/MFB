# Filter Bank
A Fixed bank of 16 Band Pass filters with amplitude control per filter, and envelope followers to allow for controlling 
the amplitude of frequency bands using the amplitude of another signal (spectral transfer).

### To do

#### External Controls 
- midi note control over amplitude triggering
- midi CC control over resonance
- CV control
- Field sequencer buttons increment or decrement amplitude per filter

#### Input / Outputs
- Audio in L/R two possible (configurable) inputs
- Audio out L/R two possible outputs from configurable filters from the  array.

#### Filter Array setup
- routing between Odd and Even banks (296e)
- routing left/right audio inputs to specific filters
- envelope followers from each filter
- routing left/right as carrier/modulator signal
- envelope controls

## Controls

| Control        | Description                 | Comment |
|----------------|-----------------------------| --- |
| Switch Up/Down | Control Mode                | Selects which filters ctrls map to |
| Knobs 1-8      | Filter Amplitude            | Volume per filter |
| Audio In 1-2   | Filter bank Input Even/Odd  | |
| Audio Out 1-2  | Filter Bank Output Even/Odd | |

Original [Source Code](https://github.com/electro-smith/DaisyExamples/tree/master/patch/FilterBank) by Ben Sergentanis.