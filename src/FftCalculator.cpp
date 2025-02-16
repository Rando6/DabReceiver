#include "FftCalculator.h"

#include "fftw3.h"

FftCalculator::FftCalculator(int size) :
    m_reference_count(nullptr),
    m_size(size),
    m_input_buffer((fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)* size)),
    m_output_buffer((fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)* size)),
    m_fft_plan(fftwf_plan_dft_1d(size, m_input_buffer, m_output_buffer, FFTW_FORWARD, FFTW_ESTIMATE)),
    m_ifft_plan(fftwf_plan_dft_1d(size, m_input_buffer, m_output_buffer, FFTW_BACKWARD, FFTW_ESTIMATE))
{
    m_reference_count = new int;
    *m_reference_count = 1;
}

FftCalculator::FftCalculator(const FftCalculator& obj) :
    m_reference_count(obj.m_reference_count),
    m_size(obj.m_size),
    m_input_buffer(obj.m_input_buffer),
    m_output_buffer(obj.m_output_buffer),
    m_fft_plan(obj.m_fft_plan),
    m_ifft_plan(obj.m_ifft_plan)
{
    (*m_reference_count) = (*m_reference_count) + 1;
}

FftCalculator::~FftCalculator()
{
    (*m_reference_count) = (*m_reference_count) - 1;

    if ((*m_reference_count) <= 0)
    {
        delete m_reference_count;
        fftwf_destroy_plan(m_ifft_plan);
        fftwf_destroy_plan(m_fft_plan);
        fftwf_free(m_output_buffer);
        fftwf_free(m_input_buffer);
    }
}

void FftCalculator::fft(const std::complex<float> input[], std::complex<float> output[]) const
{
    memcpy(m_input_buffer, input, sizeof(fftwf_complex) * m_size);
    fftwf_execute(m_fft_plan);
    memcpy(output, m_output_buffer, sizeof(fftwf_complex) * m_size);
}

void FftCalculator::ifft(const std::complex<float> input[], std::complex<float> output[]) const
{
    memcpy(m_input_buffer, input, sizeof(fftwf_complex) * m_size);
    fftwf_execute(m_ifft_plan);
    memcpy(output, m_output_buffer, sizeof(fftwf_complex) * m_size);

    auto scaling = static_cast<std::complex<float>>(m_size);
    for (int i = 0; i < m_size; i++)
    {
        output[i] = output[i] / scaling;
    }
}
