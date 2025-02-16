#pragma once

#include "FftCalculator.h"

#include "Eigen/Dense"

class TimeSynchronizer final
{
public:
    static TimeSynchronizer create();

    int get_prs_start_index(const Eigen::VectorXcf& signal_td);

private:
    TimeSynchronizer(const FftCalculator& fft_calculator, const Eigen::VectorXcf& prepared_prs_symbol_fd);

    static Eigen::VectorXcf create_prepared_prs_symbol_fd(const Eigen::VectorXcf& prs_symbol, const FftCalculator& fft_calculator);

    FftCalculator m_fft_calculator;
    Eigen::VectorXcf m_prepared_prs_symbol_fd;
    Eigen::VectorXcf m_signal_fd;
    Eigen::VectorXcf m_product_fd;
    Eigen::VectorXcf m_correlation_result;
};