#include "TimeSynchronizer.h"
#include "DabConstants.h"
#include "PrsCreation.h"

TimeSynchronizer::TimeSynchronizer(const FftCalculator& fft_calculator, const Eigen::VectorXcf& prepared_prs_symbol_fd) :
    m_fft_calculator(fft_calculator),
    m_prepared_prs_symbol_fd(prepared_prs_symbol_fd),
    m_signal_fd(DabConstants::T_F_FFT),
    m_product_fd(DabConstants::T_F_FFT),
    m_correlation_result(DabConstants::T_F_FFT)
{

}

TimeSynchronizer TimeSynchronizer::create()
{
    auto symbol_fft_calculator{ FftCalculator(DabConstants::T_U) };
    auto frame_fft_calculator{ FftCalculator(DabConstants::T_F_FFT) };

    auto prs_symbol = PrsCreation::create(symbol_fft_calculator);
    auto prepared_prs_symbol_fd = create_prepared_prs_symbol_fd(prs_symbol, frame_fft_calculator);
    auto time_synchronizer{ TimeSynchronizer(frame_fft_calculator, prepared_prs_symbol_fd) };

    return time_synchronizer;
}

Eigen::VectorXcf TimeSynchronizer::create_prepared_prs_symbol_fd(const Eigen::VectorXcf& prs_symbol, const FftCalculator& fft_calculator)
{
    Eigen::VectorXcf prepared_prs_symbol_td = Eigen::VectorXcf::Zero(DabConstants::T_F_FFT);
    Eigen::VectorXcf prepared_prs_symbol_fd = Eigen::VectorXcf::Zero(DabConstants::T_F_FFT);

    prepared_prs_symbol_td.head<DabConstants::T_S>() = prs_symbol.reverse().conjugate();
    fft_calculator.fft(prepared_prs_symbol_td.data(), prepared_prs_symbol_fd.data());

    return prepared_prs_symbol_fd;
}

// Calculates the cross correlation between the signal and the PRS symbol to find the start of the PRS symbol.
// The cross correlation is calculated using the "fourier transformation trick".
int TimeSynchronizer::get_prs_start_index(const Eigen::VectorXcf& signal_td)
{
    m_fft_calculator.fft(signal_td.data(), m_signal_fd.data());
    m_product_fd = m_signal_fd.array() * m_prepared_prs_symbol_fd.array();
    m_fft_calculator.ifft(m_product_fd.data(), m_correlation_result.data());

    auto argmax = 0;
    auto max = 0.0f;
    for (int i = 0; i < signal_td.size(); i++)
    {
        auto current_max = std::norm(m_correlation_result[i]);
        if (current_max > max)
        {
            argmax = i;
            max = current_max;
        }
    }

    argmax = argmax - DabConstants::T_S + 1; // Offset corrected by the length of the PRS symbol.

    if (argmax < 0)
    {
        throw std::logic_error("How to handle the case when argmax is less than 0?");
    }
    else if (argmax > DabConstants::T_F)
    {
        return argmax - DabConstants::T_F;
    }
    else
    {
        return argmax;
    }
}
