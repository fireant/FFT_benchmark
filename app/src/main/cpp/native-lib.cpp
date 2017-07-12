#include <jni.h>

#include <chrono>
#include <unistd.h>
#include <android/log.h>
#define  LOG_TAG    "fft_benchmark"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

#include "kiss_fftr.h"

using namespace std;

extern "C"
JNIEXPORT void JNICALL
Java_com_wavelethealth_fft_1benchmark_MainActivity_fftBenchmark(
        JNIEnv *env,
        jobject /* this */) {

    // repeat test to calculate average execution time
    const int repeats = 100;

    const int nfft = 16384;
    int samplingRate = 86; // Hz

    // generate sample input signal
    float testSignal[nfft];
    int testSignalFrq = 10; // Hz
    for (int i=0; i<nfft; i++) {
        testSignal[i] = (float)sin(2.0*3.141593*testSignalFrq*i/samplingRate);
    }

    // need to allocate memory for kissfft
    kiss_fftr_cfg cfg=kiss_fftr_alloc(nfft,
                                      0,     // inverse fft
                                      NULL,  // mem
                                      NULL); // lenmem

    // hold fft's output
    kiss_fft_cpx* fftBins = new kiss_fft_cpx[nfft / 2 + 1];

    auto maxTime = chrono::duration<double>(0);
    auto minTime = chrono::duration<double>(100);
    auto averageTime = chrono::duration<double>(0);

    for (int i=0; i<repeats; i++) {
        // record start time
        auto start = std::chrono::high_resolution_clock::now();

        // run real-input fft
        kiss_fftr(cfg, testSignal, fftBins);

        // record emd time and computed elapsed time
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds = end - start;
        if (elapsed_seconds > maxTime)
            maxTime = elapsed_seconds;
        if (elapsed_seconds < minTime)
            minTime = elapsed_seconds;
        averageTime += elapsed_seconds;
    }

    LOGD("elapsed average time: %f, max time: %f, min time: %f",
         averageTime.count() / repeats,
         maxTime.count(),
         minTime.count());

    // free kissfft resources
    free(cfg);

    // find the fft bin with the most power
    float maxPowerFrq = 0;
    float maxPower = 0;
    for (int i=0; i<(nfft/2+1); i++) {
        float power = (float)sqrt(pow(fftBins[i].i, 2) + pow(fftBins[i].r, 2)) / (nfft/2+1);
        if (power > maxPower) {
            maxPower = power;
            // convert bin index to frequency
            maxPowerFrq = float(i)/nfft * samplingRate;
        }
    }

    LOGD("maxPowerFrq: %f Hz, maxPower: %f", maxPowerFrq, maxPower);
}
