//
// Created by August Pemberton on 12/09/2022.
//

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <fastexp/fastexp.h>

#include <melatonin_perfetto/melatonin_perfetto.h>


static inline float
fastlog2 (float x)
{
    union { float f; uint32_t i; } vx = { x };
    union { uint32_t i; float f; } mx = { (vx.i & 0x007FFFFF) | 0x3f000000 };
    float y = vx.i;
    y *= 1.1920928955078125e-7f;

    return y - 124.22551499f
           - 1.498030302f * mx.f
           - 1.72587999f / (0.3520887068f + mx.f);
}

static inline float
fastlog (float x)
{
    return 0.69314718f * fastlog2 (x);
}

static double fastExp(double x) {
    return fastexp::exp(x);
}

static inline float
fasterlog2 (float x)
{
    union { float f; uint32_t i; } vx = { x };
    float y = vx.i;
    y *= 1.1920928955078125e-7f;
    return y - 126.94269504f;
}

static inline float fasterlog (float x)
{
//  return 0.69314718f * fasterlog2 (x);

    union { float f; uint32_t i; } vx = { x };
    float y = vx.i;
    y *= 8.2629582881927490e-8f;
    return y - 87.989971088f;
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


static void writeBufferToFile(juce::File file, juce::AudioSampleBuffer& buffer) {
    if (file.exists()) file.deleteFile();
    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatWriter> writer;
    writer.reset (format.createWriterFor (new juce::FileOutputStream (file),
                                          48000.0,
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

static int prevPowerOfTwo(float v) {
    return floor(log2f(v) + 0.5f);
}

static float rand01() { return juce::Random::getSystemRandom().nextFloat(); }
static float randGain() { return rand01() * 2 - 1; }

static double getSnappedPitch(double p, std::vector<double>& pitches) {
    if (p < 0) {
        p = fmod((p + 12), 12);
    }
    if (pitches.empty()) return p;
    if (pitches.size() == 1) return pitches.front();

    auto closest = std::lower_bound(pitches.begin(), pitches.end(), p);
    if (closest == pitches.end()) return *(closest-1);
    if (closest == pitches.begin()) return *closest;
    if (abs(*(closest-1) - p) <= abs(*closest - p)) return *(closest-1);

    return *closest;
}
//==============================================================================
static float lerp(float a, float b, float t) {
    return a + (b-a) * t;
}

static double lerp(double a, double b, double t) {
    return a + (b-a) * t;
}

/** Find nearest multiple of a number */
static double nearestMultiple(double x, double m) {
    return round(x / m) * m;
}

/** Check a bool, it's set, clear and return true */
inline bool compareAndReset (bool& flag)
{
    if (flag)
    {
        flag = false;
        return true;
    }
    return false;
}

/** Get RMS */
inline float calculateRMS (const float* values, int n)
{
    float rms = 0;

    for (int i = 0; i < n; i++)
        rms += values[i] * values[i];

    return std::sqrt ((1.0f / float ( n )) * rms);
}

/** Get average */
inline float calculateMedian (const float* values, int n)
{
    juce::Array<float> f;
    f.insertArray (0, values, n);
    f.sort();

    if (f.size() % 2 == 0)
        return (f[f.size() / 2] + f[f.size() / 2 - 1]) / 2.0f;

    return f[f.size() / 2];
}

//==============================================================================
/** Fisher-Yates Shuffle for juce::Array
 */
template <typename T>
void shuffleArray (juce::Random& r, T array)
{
    const int n = array.size();
    for (int i = n - 1; i >= 1; i--)
    {
        int j = r.nextInt (i + 1);
        array.swap (i, j);
    }
}

//==============================================================================
/**
 Perlin noise - realistic looking noise
 Based on reference implementation of Perlin Noise by Ken Perlin
 http://mrl.nyu.edu/~perlin/paper445.pdf
 */
template <class T>
class PerlinNoise
{
public:
    PerlinNoise()
    {
        p = { 151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
              8,99,37,240,21,10,23,190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
              35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,
              134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
              55,46,245,40,244,102,143,54, 65,25,63,161,1,216,80,73,209,76,132,187,208, 89,
              18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,
              250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
              189,28,42,223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167,
              43,172,9,129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,
              97,228,251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,
              107,49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
              138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180 };
    }

    PerlinNoise (unsigned int seed)
    {
        juce::Random r (seed);

        for (int i = 0; i <= 255; i++)
            p.add (i);

        shuffleArray (r, p);

        p.addArray (p);
    }

    T noise (T x, T y = 0, T z = 0)
    {
        int X = (int) std::floor (x) & 255;
        int Y = (int) std::floor (y) & 255;
        int Z = (int) std::floor (z) & 255;

        x -= std::floor (x);
        y -= std::floor (y);
        z -= std::floor (z);

        T u = fade (x);
        T v = fade (y);
        T w = fade (z);

        int A  = p[X] + Y;
        int AA = p[A] + Z;
        int AB = p[A + 1] + Z;
        int B  = p[X + 1] + Y;
        int BA = p[B] + Z;
        int BB = p[B + 1] + Z;

        // Add blended results from 8 corners of cube
        T res = lerp (w,
                      lerp (v, lerp (u, grad(p[AA], x, y, z),
                                     grad (p[BA], x-1, y, z)),
                            lerp (u, grad (p[AB], x, y-1, z),
                                  grad (p[BB], x-1, y-1, z))),
                      lerp (v, lerp (u, grad (p[AA+1], x, y, z-1),
                                     grad (p[BA+1], x-1, y, z-1)),
                            lerp (u, grad (p[AB+1], x, y-1, z-1),
                                  grad (p[BB+1], x-1, y-1, z-1))));

        return T ((res + 1.0) / 2.0);
    }

private:
    T fade (T t)
    {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    T lerp (T t, T a, T b)
    {
        return a + t * (b - a);
    }

    T grad (int hash, T x, T y, T z)
    {
        int h = hash & 15;
        T u = h < 8 ? x : y, v = h < 4 ? y : h == 12 || h == 14 ? x : z;
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }

    juce::Array<int> p;
};

//==============================================================================
/** Keeps a rolling average of a series of numbers
 */
class RollingAverage
{
public:
    RollingAverage (int numVals_)
            : numVals (numVals_)
    {
    }

    double average (double nextValue)
    {
        return (nextValue + numVals * currAvg) / (double)(numVals + 1);
    }

    double getAverage()
    {
        return currAvg;
    }

    void setAverage (double avg)
    {
        currAvg = avg;
    }

private:
    int numVals = 0;
    double currAvg = 0.0;
};

//==============================================================================
/** Time Profiler -- get a quick idea how long something takes
  */
class TimeProfiler
{
public:
    TimeProfiler (const juce::String& name_) :
            name (name_), start (juce::Time::getMillisecondCounterHiRes()) {}

    ~TimeProfiler()
    {
        DBG (name + juce::String::formatted (" %.2fs", (juce::Time::getMillisecondCounterHiRes() - start) / 1000.0));
    }

private:
    juce::String name;
    double start;
};

//==============================================================================
/** Are two floats pretty close? */
template <typename T>
inline bool almostEqual (T a, T b, T precision = T (0.00001))
{
    return std::abs (a - b) < precision;
}

int versionStringToInt (const juce::String& versionString);

//==============================================================================
/** Do a lambda, a bit later */
void delayedLambda (std::function<void ()> callback, int delayMS);

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

static inline juce::NormalisableRange<float> getFreqRangeExp()
{
    static const auto min = 20.f;
    static const auto max = 20000.f;
    static const auto logmin = std::log(min);
    static const auto logmax = std::log(max);
    static const auto logrange = (logmax - logmin);

    static const juce::dsp::LookupTableTransform<float> from0To1([](float normalized) {
        normalized = std::max(0.0f, std::min(1.0f, normalized));
        auto value = exp((normalized * logrange) + logmin);
        return std::max(min, std::min(max, value));
    }, 0, 1, 20000);

    static juce::dsp::LookupTableTransform<float> to0To1([](float value) {
        value = std::max(min, std::min(max, value));
        float logvalue = std::log(value);
        return (logvalue - logmin) / logrange;
    }, min, max, 20000);

    return {min, max,
            [](float start, float end, float normalized)
            {
                return from0To1(normalized);
            },
            [](float start, float end, float value)
            {
                return to0To1(value);
            },
            [](float start, float end, float value)
            {
                return std::max(start, std::min(end, value));
            }};
}

static juce::Rectangle<int> getSquareBounds(juce::Rectangle<int> b) {
    auto size = std::min(b.getWidth(), b.getHeight());
    auto bounds = juce::Rectangle<int>(size, size)
            .withCentre(b.getCentre());
    return bounds;
}

static juce::Rectangle<int> getSquareBoundsBottom(juce::Rectangle<int> b) {
    auto size = std::min(b.getWidth(), b.getHeight());
    auto bounds = juce::Rectangle<int>(size, size)
            .withCentre(b.getCentre()).withBottomY(b.getBottom());
    return bounds;
}

// https://forum.juce.com/t/best-way-of-rotating-components/17750/2
static void setVerticalRotatedWithBounds(juce::Component * component,
                                         bool clockWiseRotation,
                                         juce::Rectangle<int> verticalBounds)
{
    auto angle = juce::MathConstants<float>::pi / 2.0f;

    if (! clockWiseRotation)
        angle *= -1.0f;

    component->setTransform({});
    component->setSize(verticalBounds.getHeight(), verticalBounds.getWidth());  // this line is a bit ugly... and could just look like a typo (it's not, honest)
    component->setCentrePosition(0, 0);
    component->setTransform(juce::AffineTransform::rotation(angle)
                                    .translated(verticalBounds.getCentreX(), verticalBounds.getCentreY()));
}



static auto logMin = std::log(20);
static auto logMax = std::log(20000);
static auto logRange = logMax - logMin;

static float normToFreq(float norm) {
    float v = fastExp((norm * logRange) + logMin);
    return juce::jlimit(20.f, 20000.f, v);
}

static float freqToNorm(float freq) {
    float logvalue = fastlog(freq);
    return (logvalue - logMin) / logRange;
}
