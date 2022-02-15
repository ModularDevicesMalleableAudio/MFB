# Filter Bank

## Author

Original [Source Code](https://github.com/electro-smith/DaisyExamples/tree/master/patch/FilterBank) by Ben Sergentanis. 
Ported to Field Hardware, additional features by Kris Graham.	

## Description
Fixed resonant peak filterbank with amplitude control per filter. Try running it with whitenoise, oscillators, or any 
sound source you please!

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

| Control | Description | Comment |
| --- | --- | --- |
| Switch Up/Down | Control Mode | Selects which filters ctrls map to |
| Knobs 1-4 | Filter Amplitude | Volume per filter |
| Audio In 1 | Filter bank Input | |
| Audio Out 1-4 | Filter Bank Output | |

## Diagram
<img src="https://raw.githubusercontent.com/electro-smith/DaisyExamples/master/patch/FilterBank/resources/FilterBank.png" alt="FilterBank.png" style="width: 100%;"/>
