//
// Created by August Pemberton on 12/09/2022.
//

#pragma once
#include "juce_audio_processors/juce_audio_processors.h"

template <typename T>
struct LoopRange {
    T start;
    T end;
    T loopStart;
    T loopEnd;

    bool looped = false;

    bool contains(LoopRange& other) {
        if (!looped) return juce::Range<T>(start, end).contains({other.start, other.end});

        return contains({other.start, std::min(other.end, loopEnd)}) &&
               contains({std::max(other.start, loopStart), other.end});
    }

    bool contains(juce::Range<T> other) {
        return juce::Range<T>(start, std::min(end, loopEnd)).contains(other) ||
               juce::Range<T>(std::max(start, loopStart), end).contains(other);
    }

};
#include "juce_dsp/juce_dsp.h"
#include "fastapprox.h"

template <typename T>
juce::String formatNumber (T v)
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

static int prevPowerOfTwo(float v) {
    return (int)floor(log2f(v) + 0.5f);
}

static inline double lim_p2(uint32_t i, uint32_t size) {
    return i & (size-1);
}

static inline double wrapWithinRangePow2(double i, int min, int max) {
    auto size = max - min;
    jassert(juce::isPowerOfTwo(size));

    return lim_p2(static_cast<uint32_t>(i - min), static_cast<uint32_t>(size)) + min;
}

static inline double wrapWithinRange(double i, int min, int max, bool pow2 = false) {
    if (pow2 && i > 0) return wrapWithinRangePow2(i, min, max);
    if (i>=min && i<max) return i;
    auto n = max - min;
    return fmod(fmod(i-min, n) + n, n) + min;
}

static inline int wrapWithinRange(int i, int min, int max, bool pow2 = false) {
    if (i>=min && i<max) return i;
    if (pow2 && i > 0) return (int) wrapWithinRangePow2(i, min, max);
    auto n = max - min;
    if (n <= 0) return min;
    return (((i-min % n) + n) % n) + min;
}

static std::optional<juce::AudioSampleBuffer> loadFileIntoBuffer(const juce::File& file) {
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

static void writeBufferToFile(const juce::File& file, juce::AudioSampleBuffer& buffer, double sampleRate = 48000) {
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

static float midiNoteToFreq(float midiNote) {
    return powf(2, (midiNote-69)/12) * 440;
}

static float freqToMidiNote(float freq) {
    return log(freq/440.0)/log(2) * 12 + 69;
}

static float rand01() { return juce::Random::getSystemRandom().nextFloat(); }
static float randGain() { return rand01() * 2 - 1; }

//==============================================================================
static float lerp(float a, float b, float t) {
    return a + (b-a) * t;
}

static double lerp(double a, double b, double t) {
    return a + (b-a) * t;
}

static double nearestMultiple(double x, double m) {
    return round(x / m) * m;
}

template <typename T>
inline bool almostEqual (T a, T b, T precision = T (0.00001))
{
    return std::abs (a - b) < precision;
}

static inline juce::NormalisableRange<float> getFreqRange() {
    return {20, 20000, 0.25};
}

static inline juce::NormalisableRange<float> getNormalisableRangeExp(float min, float max, float step = 0.001f)
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
