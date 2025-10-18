//
// Created by August Pemberton on 15/10/2025.
//

#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

#include "imagiro_processor/src/dsp/filter/AWeightingFilter.h"

struct BufferUtils {
    static float getAWeightedRMS(const juce::AudioSampleBuffer &buffer, int startSample,
                                 int numSamples, double sampleRate) {
        if (numSamples <= 0) numSamples = buffer.getNumSamples() - startSample;

        // Create a copy of the region to filter
        juce::AudioBuffer<float> region(buffer.getNumChannels(), numSamples);
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            region.copyFrom(ch, 0, buffer, ch, startSample, numSamples);
        }

        juce::dsp::ProcessSpec spec{
            sampleRate, static_cast<uint32_t>(numSamples), static_cast<uint16_t>(buffer.getNumChannels())
        };
        // Apply A-weighting
        AWeightingFilter aFilter;
        aFilter.prepare(spec);
        aFilter.processBlock(region);

        // Calculate RMS on filtered signal
        float rms = 0.f;
        for (auto c = 0; c < region.getNumChannels(); c++) {
            rms += region.getRMSLevel(c, 0, numSamples);
        }
        rms /= (float) region.getNumChannels();

        return rms;
    }

    static float getPerceivedLoudness(const juce::AudioSampleBuffer &buffer, int startSample,
                                      int numSamples, double sampleRate,
                                      float attackWeight = 0.9f, float sustainWeight = 0.1f) {
        if (numSamples <= 0) numSamples = buffer.getNumSamples() - startSample;

        // Define attack window (30ms is typical for piano)
        constexpr float attackWindowSeconds = 0.1f;
        int attackSamples = std::min((int) (attackWindowSeconds * sampleRate), numSamples);
        int sustainSamples = numSamples - attackSamples;

        // Get A-weighted RMS for attack region
        float attackRMS = 0.f;
        if (attackSamples > 0) {
            attackRMS = getAWeightedRMS(buffer, startSample, attackSamples, sampleRate);
        }

        // Get A-weighted RMS for sustain region
        float sustainRMS = 0.f;
        if (sustainSamples > 0) {
            sustainRMS = getAWeightedRMS(buffer, startSample + attackSamples,
                                         sustainSamples, sampleRate);
        }

        // Combine with weighting
        // Default weights: 0.6 for attack (transient impact), 0.4 for sustain (body)
        // You can tune these values to taste
        float perceived = attackWeight * attackRMS + sustainWeight * sustainRMS;

        return perceived;
    }
};
