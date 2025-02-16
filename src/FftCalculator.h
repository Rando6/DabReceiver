#pragma once

#include "fftw3.h"

#include <complex>
#include <vector>

class FftCalculator final
{
public:
    FftCalculator(int size);
    FftCalculator(const FftCalculator& obj);
    FftCalculator& operator=(const FftCalculator&) = delete;
    FftCalculator(FftCalculator&&) = delete;
    FftCalculator& operator=(FftCalculator&&) = delete;
    ~FftCalculator();

    void fft(const std::complex<float> input[], std::complex<float> output[]) const;
    void ifft(const std::complex<float> input[], std::complex<float> output[]) const;

private:
    int* m_reference_count;
    int m_size;
    fftwf_complex* m_input_buffer;
    fftwf_complex* m_output_buffer;
    fftwf_plan m_fft_plan;
    fftwf_plan m_ifft_plan;
};