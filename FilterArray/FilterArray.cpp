#include "daisy_field.h"
#include "daisysp.h"
#include "FilterArray.h"
#include <algorithm>
#include <string>

#define LEFT_BORDER_WIDTH 1
#define COLUMN_WIDTH 6
#define COLUMN_MARGIN 2
#define KNOB_TOLERANCE .001f
#define KNOB_MIN .003f

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 60
#define HEADER_HEIGHT 10
#define PARAM_NAME_COL_WIDTH 70
#define ROW_HEIGHT 14
#define SKINNY_ROW_HEIGHT 13
#define DEFAULT_FONT Font_7x10
#define SMALL_FONT Font_6x8
#define CHECKBOX_SIZE 8
#define CHECKBOX_MARGIN 3
#define VALUE_BAR_HEIGHT 10
#define VALUE_BAR_WIDTH 55
#define VALUE_BAR_MARGIN 3
#define MAPPED_PARAM_CONTAINER_WIDTH 18
#define MAX_SCOPE_HEIGHT 60

using namespace daisy;
using namespace daisysp;

DaisyField hw;

float frequencies[16];
int bank;

enum {
    EVEN = 0,
    ODD  = 1,
};

inline const char * BankToLabel(int i)
{
    switch (i)
    {
        case 0:  return "Even";
        case 1:  return "Odd ";
        default: return "[Unknown]";
    }
}

bool transfer_odd_to_even  = false;
bool transfer_even_to_odd  = false;
int spectral_transfer_mode = 0;

void spectralTransferMode()
{
    if (spectral_transfer_mode == 0) { transfer_even_to_odd = false, transfer_odd_to_even = false; }
    if (spectral_transfer_mode == 1) { transfer_even_to_odd = true, transfer_odd_to_even = false; }
    if (spectral_transfer_mode == 2) { transfer_even_to_odd = false, transfer_odd_to_even = true; }
    if (spectral_transfer_mode == 3) { transfer_even_to_odd = true, transfer_odd_to_even = true ;}
}

inline const char * spectralTransferModeLabel(int modSettings)
{
    switch (modSettings)
    {
        case 0:  return "No Transfer";
        case 1:  return "Even -> Odd";
        case 2:  return "Odd -> Even";
        case 3:  return "E->O & O->E";
        default: return " [Unknown] ";
    }
}

enum
{
    BAND0ENVACTIVE = 0,
    BAND1ENVACTIVE = 1,
    BAND2ENVACTIVE = 2,
    BAND3ENVACTIVE = 3,
    BAND4ENVACTIVE = 4,
    BAND5ENVACTIVE = 5,
    BAND6ENVACTIVE = 6,
    BAND7ENVACTIVE = 7,
    BAND0MIDIACTIVE __attribute__((unused)) = 8,
    BAND1MIDIACTIVE __attribute__((unused)) = 9,
    BAND2MIDIACTIVE __attribute__((unused)) = 10,
    BAND3MIDIACTIVE __attribute__((unused)) = 11,
    BAND4MIDIACTIVE __attribute__((unused)) = 12,
    BAND5MIDIACTIVE __attribute__((unused)) = 13,
    BAND6MIDIACTIVE __attribute__((unused)) = 14,
    BAND7MIDIACTIVE __attribute__((unused)) = 15
};

int knob_leds[] = {
        DaisyField::LED_KNOB_1,
        DaisyField::LED_KNOB_2,
        DaisyField::LED_KNOB_3,
        DaisyField::LED_KNOB_4,
        DaisyField::LED_KNOB_5,
        DaisyField::LED_KNOB_6,
        DaisyField::LED_KNOB_7,
        DaisyField::LED_KNOB_8,
};
int keyboard_leds[] = {
        DaisyField::LED_KEY_A1,
        DaisyField::LED_KEY_A2,
        DaisyField::LED_KEY_A3,
        DaisyField::LED_KEY_A4,
        DaisyField::LED_KEY_A5,
        DaisyField::LED_KEY_A6,
        DaisyField::LED_KEY_A7,
        DaisyField::LED_KEY_A8,
        DaisyField::LED_KEY_B1,
        DaisyField::LED_KEY_B2,
        DaisyField::LED_KEY_B3,
        DaisyField::LED_KEY_B4,
        DaisyField::LED_KEY_B5,
        DaisyField::LED_KEY_B6,
        DaisyField::LED_KEY_B7,
        DaisyField::LED_KEY_B8,
};

struct ConditionalUpdate
{
    float oldVal = 0;

    bool Process(float newVal)
    {
        if(abs(newVal - oldVal) > KNOB_TOLERANCE)
        {
            oldVal = newVal;
            return true;
        }
        return false;
    }
};

ConditionalUpdate condUpdates[8];

class Filter
{
public:
    Filter() = default;
    ~Filter() = default;

    EnvelopeFollower  follower;
    Svf               filter;
    float             knob_amp {};
    float             follower_amp {};
    float             frontend_follower_amp {};
    int               split_signal_band_index {};
    int               paired_band_index {};
    bool              is_odd {};
    bool isOdd() const {
        return is_odd;
    }

    void Init(float sample_rate,
              float frequency,
              int global_band_index)
    {
        follower.Init(sample_rate, 1, 20);
        filter.Init(sample_rate);
        filter.SetRes(0.6);
        filter.SetDrive(0.002);
        filter.SetFreq(frequency);
        knob_amp = 1.0f;
        follower_amp = 1.0f;
        frontend_follower_amp = 1.0f;
        split_signal_band_index = floor(global_band_index / 2);
        is_odd = global_band_index % 2;
        paired_band_index = (is_odd) ? (global_band_index - 1) : (global_band_index + 1);
    }

    // Get filtered signal from bank, respecting the knob amplitude attenuation but ignoring envelope follower
    float PreProcess(float in)
    {
        filter.Process(in);
        return filter.Band() * knob_amp;
    }

    // Get filtered signal from bank, applying both envelope follower and the attenuation from the knobs
    float Process(float in)
    {
        filt.Process(in);
        return filt.Band() * follower_amp * knob_amp;
    }
};

Filter filters[16];

static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)
{
    for(size_t i = 0; i < size; i++) {
        for(int j = 0; j < 16; j++) {
            int pairedBandIndex = filters[j].paired_band_index; // get paired band index

            // get input signals, filter them accordingly
            float unfilteredInput = (filters[j].isOdd()) ? in[ODD][i] : in[EVEN][i];
            float filteredInput = filters[j].PreProcess(unfilteredInput);
            float pairedUnfilteredInput = (filters[pairedBandIndex].isOdd()) ? in[ODD][i] : in[EVEN][i];
            float pairedFilteredInput = filters[pairedBandIndex].PreProcess(pairedUnfilteredInput);

            // Update the display envelope amp using the filtered input signal where it is active
            if (filters[j].follower.isActive()) {
                filters[j].frontend_follower_amp = float(filters[j].follower.Process(filteredInput, true));
            }
            else {
                filters[j].frontend_follower_amp = 1.0f;
            }

            // if the paired band is transferring to the current band, use the paired follower amp...
            if (filters[pairedBandIndex].follower.isActive()) {
                if (transfer_odd_to_even && !filters[j].isOdd()) {
                    filters[j].follower_amp = float(filters[pairedBandIndex].follower.Process(pairedFilteredInput, true));
                }
                else if (transfer_even_to_odd && filters[j].isOdd()) {
                    filters[j].follower_amp = float(filters[pairedBandIndex].follower.Process(pairedFilteredInput, true));
                }
            }
            // If its not being transferred to, but it is active, we update the current follower amp
            else if (filters[j].follower.isActive()) {
                filters[j].follower_amp = float(filters[j].follower.Process(filteredInput, true));
            }
        }

        float even_output = 0.f;
        float odd_output = 0.f;

        // Then pass input signals through the filters and scale across 16 bands for outputting
        for(int j = 0; j < 16; j++) {
            if (!filters[j].isOdd()) {
                even_output += filters[j].Process(in[EVEN][i]);
            }
            if (filters[j].isOdd()) {
                odd_output += filters[j].Process(in[ODD][i]);
            }
        }
        // Scaling factors for 16 filter bands
        even_output *= .06;
        odd_output *= .06;

        // send output to outs
        out[EVEN][i] = even_output;
        out[ODD][i] = odd_output;
    }
}

void InitFrequencies()
{
    frequencies[0]  = 50.f;
    frequencies[1]  = 75.f;
    frequencies[2]  = 110.f;
    frequencies[3]  = 150.f;
    frequencies[4]  = 220.f;
    frequencies[5]  = 350.f;
    frequencies[6]  = 500.f;
    frequencies[7]  = 750.f;
    frequencies[8]  = 1100.f;
    frequencies[9]  = 1600.f;
    frequencies[10] = 2200.f;
    frequencies[11] = 3600.f;
    frequencies[12] = 5200.f;
    frequencies[13] = 7500.f;
    frequencies[14] = 11000.f;
    frequencies[15] = 15000.f;
}

void InitFilters(float sample_rate)
{
    for(int i = 0; i < 16; i++)
    {
        filters[i].Init(sample_rate,frequencies[i],i);
    }
}

void RenderBars() {
    for(int i = 0; i < 16; i++)
    {
        // refactor to have a thin envelope bar inside the fat knob value?
        int amplitude = 0;
        amplitude += fmin(
                floor((filters[i].knob_amp * filters[i].frontend_follower_amp) * 45),
                45
                );
        hw.display.DrawRect(LEFT_BORDER_WIDTH + (i * (COLUMN_WIDTH + COLUMN_MARGIN)),
                            60 - amplitude,
                            LEFT_BORDER_WIDTH + ((i + 1) * (COLUMN_WIDTH)) + (i * COLUMN_MARGIN),
                            60,
                            true,
                            filters[i].isOdd() != bank % 2);

        hw.display.DrawLine(LEFT_BORDER_WIDTH + (i * (COLUMN_WIDTH + COLUMN_MARGIN)),
                            63,
                            LEFT_BORDER_WIDTH + + ((i + 1) * (COLUMN_WIDTH)) + (i * COLUMN_MARGIN),
                            63,
                            filters[i].isOdd() == bank % 2);
    }
}

void UpdateLeds()
{
    for(int i = 0; i < 8; i++)
    {
        int x = 0;
        // get relevant filter index for the selected bank (odd or even)
        if (bank == ODD) { x = (i * 2) + 1; }
        if (bank == EVEN) { x = (i * 2); }
        hw.led_driver.SetLed(
                knob_leds[i],
                float((filters[x].knob_amp * filters[x].frontend_follower_amp))
                );
        hw.led_driver.SetLed(keyboard_leds[i + 8], float(filters[x].follower.isActive()));
    }
    hw.led_driver.SwapBuffersAndTransmit();
}

void UpdateOled()
{
    hw.display.Fill(false);
    RenderBars();
    hw.display.SetCursor(LEFT_BORDER_WIDTH, 0);
    hw.display.WriteString(BankToLabel(bank), SMALL_FONT, true);
    hw.display.SetCursor(LEFT_BORDER_WIDTH + 25, 0);
    hw.display.WriteString("|", SMALL_FONT, true);
    hw.display.SetCursor(DISPLAY_WIDTH - 65, 0);
    hw.display.WriteString("|", SMALL_FONT, true);
    hw.display.SetCursor(DISPLAY_WIDTH - 67, 0);
    hw.display.WriteString(spectralTransferModeLabel(spectral_transfer_mode), SMALL_FONT, true);
    hw.display.Update();
}

void UpdateControls() {
    hw.ProcessAnalogControls();
    hw.ProcessDigitalControls();

    // Using switches to shift through banks
    bank += hw.sw[0].RisingEdge() ? 1 : 0;
//    bank = (bank % 2 + 2) % 2;
    bank = bank % 2;
    // Using switches to shift through spectral transfer modulation
    spectral_transfer_mode += hw.sw[1].RisingEdge() ? 1 : 0;
    spectral_transfer_mode = (spectral_transfer_mode % 4 + 4) % 4;

    float odd_vals[8];
    float even_vals[8];

    // get even or odd knob values according to the selected bank
    for (int i = 0; i < 8; i++) {
        if (bank == EVEN) {
            float even_val = hw.knob[i].Process();
            if (even_val < KNOB_MIN) {
                even_vals[i] = 0.f;
            }
            else {
                even_vals[i] = hw.knob[i].Process();
            }
        }
        if (bank == ODD) {
            float odd_val = hw.knob[i].Process();
            if (odd_val < KNOB_MIN) {
                odd_vals[i] = 0.f;
            }
            else {
                odd_vals[i] = hw.knob[i].Process();
            }
        }
    }

    // for each band, apply odd or even knob values to the filter amplitude
    for (int i = 0; i < 16; i++) {
        int k = floor(i / 2);
        // Even bands
        if (bank == EVEN && !filters[i].isOdd()) {
            if (condUpdates[k].Process(even_vals[k])) { filters[i].knob_amp = even_vals[k]; }
        }
        // Odd bands
        if (bank == ODD && filters[i].isOdd()) {
            if (condUpdates[k].Process(odd_vals[k])) { filters[i].knob_amp = odd_vals[k]; }
        }
    }

    // check the top row of buttons
    for(int i = 8; i < 16; i++)
    {
        int x = 0;
        if (hw.KeyboardRisingEdge(i)) {
            // get relevant filter index for the selected bank (odd or even)
            if (bank == ODD) { x = ((i - 8) * 2) + 1; }
            if (bank == EVEN) { x = ((i - 8) * 2); }
            filters[x].follower.setActive(!filters[x].follower.isActive());
        }
    }
}

//if (filters[i].follower.isActive()) {
//amplitude += fmin(floor((filters[i].knob_amp * filters[i].follower_amp) * 45), 45);
//}
//else {
//amplitude += fmin((filters[i].knob_amp * 45), 45);
//}
int main()
{
    float sample_rate;
    hw.Init(); // init hardware
    sample_rate = hw.AudioSampleRate();

    InitFrequencies();
    InitFilters(sample_rate);
    bank = EVEN;

    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    while(true)
    {
        UpdateOled();
        UpdateLeds();
        System::Delay(1);
        UpdateControls();
    }
}
