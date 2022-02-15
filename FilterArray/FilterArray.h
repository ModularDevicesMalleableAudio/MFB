#include "daisy.h"
#include "daisysp.h"
#include <cmath>

using namespace daisy;

// taken from https://kferg.dev/posts/2020/audio-reactive-programming-envelope-followers/
//public:
//EnvelopeFollower(double attack, double decay) :
//        attack_(attack), decay_(decay), output_(0) {}
// maybe swap back to double and remove fs
class EnvelopeFollower {
public:
    EnvelopeFollower() = default;
    ~EnvelopeFollower() = default;

    bool active = false;

    bool isActive() const {
        return active;
    }

    void setActive(bool b) {
        EnvelopeFollower::active = b;
    }

    void Init(float sample_rate, float attack_half_life_ms, float decay_half_life_ms)
    {
        double attack_samples = (sample_rate * (attack_half_life_ms / 1000.0));
        double decay_samples = (sample_rate * (decay_half_life_ms / 1000.0));
        attack_ = exp(log(0.5) / attack_samples);
        decay_  = pow(0.5, decay_samples);
    }

    // helper to translate
    static double LinearTransform(double input, int old_max, int old_min, int new_max, int new_min){
        int old_range = (old_max - old_min);
        int new_range = (new_max - new_min);
        return  (((input - old_min) * new_range) / old_range) + new_min;
    }

    double Process(double input, bool rescale) {
        if (rescale) {
            // convert input from -1 to 1 into 0 to 1 range
            double* input_pointer = &input;
            *input_pointer = LinearTransform(input, 1, -1, 1, 0);
        }

        const auto abs_input = abs(input);
        if (abs_input > output_) {
            output_ = attack_ * output_ + (1 - attack_) * abs_input;
        } else {
            output_ = decay_ * output_ + (1 - decay_) * abs_input;
        }
        return output_;
    }

private:
    double attack_{}, decay_{}, output_{};
};