#include "OfdmDemodulator.h"
#include "DabConstants.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <complex>
#include <iostream>

using namespace DabConstants;
using namespace std::complex_literals;

OfdmDemodulator::OfdmDemodulator() :
    m_time_buffer(T_F_U),
    m_fft_calculator(T_U),
    m_symbol_without_cp_td(T_U),
    m_symbol_without_cp_fd(T_U),
    m_carrier_values(N_OFDM_SYMBOLS, N_CARRIERS),
    m_phase_corrected_carrier_values(N_DATA_SYMBOLS, N_CARRIERS),
    m_k_by_n(N_CARRIERS),
    m_frequency_deinterleaved_values(N_DATA_SYMBOLS, N_CARRIERS)
{
    m_time_buffer.setLinSpaced(0, T_F_U - 1);
    initialize_k_by_n();
}

void OfdmDemodulator::initialize_k_by_n()
{
    auto prev_pi = 0;
    auto n = 0;

    for (int i = 0; i < T_U; i++)
    {
        auto pi = (13 * prev_pi + 511) % 2048; // See first formula of 14.6.1 of ETSI EN 300 401 V2.1.1.

        if (256 <= pi && pi <= 1792 && pi != 1024)
        {
            auto k = pi - 1024;
            if (k > 0)
            {
                k = k - 1;
            }

            m_k_by_n[n] = k + 768; // This shifts k to positive numbers which can be used for indexing.
            n = n + 1;
        }

        prev_pi = pi;
    }
}

void OfdmDemodulator::update_hard_bits(Eigen::VectorXcf& frame_buffer, Eigen::MatrixX<uint8_t>& hard_bits)
{
    correct_frequency_offset(frame_buffer);
    demodulate_ofdm_symbol(frame_buffer);
    correct_phase();
    deinterleave_frequencies();
    demap_qpsk_symobls(hard_bits);
}

void OfdmDemodulator::correct_frequency_offset(Eigen::VectorXcf& frame_buffer)
{
    for (int i = 0; i < N_OFDM_SYMBOLS; i++)
    {
        // Determine an estimator for beta for the current symbol.
        auto symbol_start_index = i * T_S;

        auto cyclic_prefix = frame_buffer.segment(symbol_start_index, T_G);
        auto symbol_tail = frame_buffer.segment(symbol_start_index + T_U, T_G);
        auto dot_product = cyclic_prefix.dot(symbol_tail);

        float pi = M_PI;
        auto beta_estimator = (1 / (2 * pi)) * std::arg(dot_product);

        // Apply frequency correction.
        auto phase_vector = -1if * 2.0f * pi * (beta_estimator / T_U) * m_time_buffer.segment(symbol_start_index, T_S);
        auto symbol = frame_buffer.segment(symbol_start_index, T_S).array();
        symbol = symbol.array() * phase_vector.array().exp();
    }
}

void OfdmDemodulator::demodulate_ofdm_symbol(Eigen::VectorXcf& frame_buffer)
{
    for (int i = 0; i < N_OFDM_SYMBOLS; i++)
    {
        m_symbol_without_cp_td = frame_buffer.segment(i * T_S + T_G, T_U);
        m_fft_calculator.fft(m_symbol_without_cp_td.data(), m_symbol_without_cp_fd.data());

        m_carrier_values.row(i).head(N_CARRIERS / 2) = m_symbol_without_cp_fd.tail(N_CARRIERS / 2);
        m_carrier_values.row(i).tail(N_CARRIERS / 2) = m_symbol_without_cp_fd.segment(1, N_CARRIERS / 2);
    }
}

void OfdmDemodulator::correct_phase()
{
    for (int i = 1; i < N_OFDM_SYMBOLS; i++)
    {
        m_phase_corrected_carrier_values.row(i - 1) = m_carrier_values.row(i - 1).conjugate().array() * m_carrier_values.row(i).array();
    }
}

void OfdmDemodulator::deinterleave_frequencies()
{
    for (int n = 0; n < N_CARRIERS; n++)
    {
        m_frequency_deinterleaved_values.col(n) = m_phase_corrected_carrier_values.col(m_k_by_n[n]);
    }
}

void OfdmDemodulator::demap_qpsk_symobls(Eigen::MatrixX<uint8_t>& hard_bits)
{
    for (int symbol_index = 0; symbol_index < N_OFDM_SYMBOLS - 1; symbol_index++)
    {
        for (int carrier_index = 0; carrier_index < N_CARRIERS; carrier_index++)
        {
            auto complex_value = m_frequency_deinterleaved_values(symbol_index, carrier_index);
            hard_bits(symbol_index, carrier_index) = complex_value.real() >= 0 ? 0 : 1;
            hard_bits(symbol_index, carrier_index + N_CARRIERS) = complex_value.imag() >= 0 ? 0 : 1;
        }
    }
}
