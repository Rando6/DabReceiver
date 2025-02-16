#pragma once

#include "FftCalculator.h"

#include "Eigen/Dense";

class OfdmDemodulator final
{
public:
    OfdmDemodulator();

    void update_hard_bits(Eigen::VectorXcf& frame_buffer, Eigen::MatrixX<uint8_t>& hard_bits);

private:
    // Initializes the member variable m_k_by_n.
    void initialize_k_by_n();

    void correct_frequency_offset(Eigen::VectorXcf& frame_buffer);
    void demodulate_ofdm_symbol(Eigen::VectorXcf& frame_buffer);
    void correct_phase();
    void deinterleave_frequencies();
    void demap_qpsk_symobls(Eigen::MatrixX<uint8_t>& hard_bits);

    Eigen::VectorXcf m_time_buffer;
    FftCalculator m_fft_calculator;
    Eigen::VectorXcf m_symbol_without_cp_td;
    Eigen::VectorXcf m_symbol_without_cp_fd;
    Eigen::MatrixXcf m_carrier_values;
    Eigen::MatrixXcf m_phase_corrected_carrier_values;

    // Represents the mapping between n and k as described in the table 25 of section 14.6 of ETSI EN 300 401 V2.1.1.
    // Here, k ranges from 0 to 1535.
    std::vector<int> m_k_by_n;
    Eigen::MatrixXcf m_frequency_deinterleaved_values;
};