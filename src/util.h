//
// Created by August Pemberton on 12/09/2022.
//

#pragma once
#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_dsp/juce_dsp.h"
#include "fastapprox.h"

namespace imagiro
{
    template <typename T>
    [[maybe_unused]] int ifloor (T val) {
        return (int)val;
    }
    template <typename T>
    [[maybe_unused]] int iceil(T val) {
        return ifloor(val) + 1;
    }

    template <typename T>
    struct LoopRange {
        T start;
        T end;
        T loopStart;
        T loopEnd;

        bool looped = false;

        [[maybe_unused]] bool contains(LoopRange& other) {
            if (!looped) return juce::Range<T>(start, end).contains({other.start, other.end});

            return contains({other.start, std::min(other.end, loopEnd)}) &&
                   contains({std::max(other.start, loopStart), other.end});
        }

        [[maybe_unused]] bool contains(juce::Range<T> other) {
            return juce::Range<T>(start, std::min(end, loopEnd)).contains(other) ||
                   juce::Range<T>(std::max(start, loopStart), end).contains(other);
        }

    };

    template <typename T>
    [[maybe_unused]] juce::String formatNumber (T v)
    {
        if (v == 0)
            return "0";

        int dec = 0;
        if (std::abs (v) < 10)   dec = 1;
        if (std::abs (v) < 1)    dec = 2;
        if (std::abs (v) < 0.1)  dec = 3;

        if (dec == 0)
            return juce::String (juce::roundToInt (v));

        return juce::String (v, dec);
    }

    [[maybe_unused]] static int prevPowerOfTwo(float v) {
        return (int)floor(log2f(v) + 0.5f);
    }

    [[maybe_unused]] static inline double lim_p2(uint32_t i, uint32_t size) {
        return i & (size-1);
    }

    template <typename T>
    [[maybe_unused]] static inline T wrapWithinRangePow2(T i, int min, int max) {
        auto size = max - min;
        jassert(juce::isPowerOfTwo(size));

        return lim_p2(static_cast<uint32_t>(i - min), static_cast<uint32_t>(size)) + min;
    }

    [[maybe_unused]] static inline juce::int64 wrapWithinRange(juce::int64 i, juce::int64 min, juce::int64 max, bool pow2 = false) {
        if (i>=min && i<max) return i;
        if (pow2 && i > 0) return (int) wrapWithinRangePow2(i, min, max);
        auto n = max - min;
        if (n <= 0) return min;
        return (((i-min % n) + n) % n) + min;
    }

    [[maybe_unused]] static inline int wrapWithinRange(int i, int min, int max, bool pow2 = false) {
        if (i>=min && i<max) return i;
        if (pow2 && i > 0) return (int) wrapWithinRangePow2(i, min, max);
        auto n = max - min;
        if (n <= 0) return min;
        return (((i-min % n) + n) % n) + min;
    }

    template <typename T>
    [[maybe_unused]] static inline T wrapWithinRange(T i, T min, T max, bool pow2 = false) {
        if (pow2 && i > 0) return wrapWithinRangePow2(i, min, max);
        if (i>=min && i<max) return i;
        auto n = max - min;
        return fmod(fmod(i-min, n) + n, n) + min;
    }
    [[maybe_unused]] static std::optional<juce::AudioSampleBuffer> loadFileIntoBuffer(const juce::File& file) {
        juce::AudioFormatManager afm;
        afm.registerBasicFormats();
        std::unique_ptr<juce::AudioFormatReader> reader (afm.createReaderFor(file));
        if (!reader) return {};

        juce::AudioSampleBuffer b (reader->numChannels, reader->lengthInSamples);
        reader->read(
                b.getArrayOfWritePointers(),
                reader->numChannels,
                0,
                reader->lengthInSamples
                );

        return b;
    }

    [[maybe_unused]] static void writeBufferToFile(const juce::File& file, juce::AudioSampleBuffer& buffer, double sampleRate = 48000) {
        if (file.exists()) file.deleteFile();
        juce::WavAudioFormat format;
        std::unique_ptr<juce::AudioFormatWriter> writer;
        writer.reset (format.createWriterFor (new juce::FileOutputStream (file),
                                              sampleRate,
                                              buffer.getNumChannels(),
                                              24,
                                              {},
                                              0));
        if (writer != nullptr)
            writer->writeFromAudioSampleBuffer (buffer, 0, buffer.getNumSamples());

    }

    [[maybe_unused]] static float midiNoteToFreq(float midiNote) {
        return powf(2, (midiNote-69)/12) * 440;
    }

    [[maybe_unused]] static float freqToMidiNote(float freq) {
        return log(freq/440.0)/log(2) * 12 + 69;
    }

    [[maybe_unused]] static float rand01() { return juce::Random::getSystemRandom().nextFloat(); }
    [[maybe_unused]] static float randGain() { return rand01() * 2 - 1; }

    //==============================================================================

    [[maybe_unused]] static inline float lerp(float a, float b, float t) {
        return a + (b-a) * t;
    }

    [[maybe_unused]] static double lerp(double a, double b, double t) {
        return a + (b-a) * t;
    }

    [[maybe_unused]] static double nearestMultiple(double x, double m) {
        return round(x / m) * m;
    }

    template <typename T>
    [[maybe_unused]] inline bool almostEqual (T a, T b, T precision = T (0.00001))
    {
        return std::abs (a - b) < precision;
    }

    [[maybe_unused]] static inline juce::NormalisableRange<float> getFreqRange() {
        return {20, 20000, 0.25};
    }

    [[maybe_unused]] static inline juce::NormalisableRange<float> getNormalisableRangeExp(float min, float max, float step = 0.001f)
    {
        jassert(min > 0.0f);
        jassert(max > 0.0f);
        jassert(min < max);

        float logmin = std::log(min);
        float logmax = std::log(max);
        float logrange = (logmax - logmin);

        jassert(logrange > 0.0f);

        return {min, max,
                [logmin,logrange](float start, float end, float normalized)
                {
                    normalized = std::max(0.0f, std::min(1.0f, normalized));
                    float value = std::exp((normalized * logrange) + logmin);
                    return std::max(start, std::min(end, value));
                },
                [logmin,logrange](float start, float end, float value)
                {
                    value = std::max(start, std::min(end, value));
                    float logvalue = fastlog(value);
                    return juce::jlimit(0.f, 1.f, (logvalue - logmin) / logrange);
                },
                [step](float start, float end, float value)-> float
                {
                    if (step == 0) return value;
                    return start + step * std::floor ((value - start) / step + 0.5f);
                }};
    }

    static auto logMin = std::log(20);
    static auto logMax = std::log(20000);
    static auto logRange = logMax - logMin;

    static float normToFreq(float norm) {
        float v = fastexp((norm * logRange) + logMin);
        return juce::jlimit(20.f, 20000.f, v);
    }

    static float freqToNorm(float freq) {
        float logvalue = fastlog(freq);
        return (logvalue - logMin) / logRange;
    }

}
