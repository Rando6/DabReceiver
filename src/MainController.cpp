#include "MainController.h"
#include "DabConstants.h"
#include "RawFileHandler.h"
#include "TimeSynchronizer.h"
#include "OfdmDemodulator.h"
#include "FicHandler.h"

#include "fmt/printf.h"

MainController::MainController() :
    m_signal_buffer(DabConstants::T_F_FFT),
    m_frame_buffer(DabConstants::T_F_U),
    m_hard_bits(DabConstants::N_DATA_SYMBOLS, DabConstants::N_RAW_FIC_SYMBOL_BITS)
{

}

void MainController::run(const std::string& file_path)
{
    auto raw_file_handler{ RawFileHandler(file_path) };
    auto time_synchronizer = TimeSynchronizer::create();
    auto ofdm_demodulator{ OfdmDemodulator() };
    auto fic_handler = FicHandler::create();

    raw_file_handler.read(m_signal_buffer, 0, m_signal_buffer.size() - 1);
    if (raw_file_handler.get_file_end_reached())
    {
        fmt::println("File doesn't contain enough data.");
        return;
    }

    auto global_prs_start_index = -DabConstants::T_F_U;
    while (true)
    {
        auto prs_start_index = time_synchronizer.get_prs_start_index(m_signal_buffer);
        global_prs_start_index = global_prs_start_index + DabConstants::T_F_U + prs_start_index;
        fmt::println("PRS start index found at sample {}.", global_prs_start_index);

        update_frame_buffer(raw_file_handler, prs_start_index);
        if (raw_file_handler.get_file_end_reached())
        {
            break;
        }

        update_signal_buffer(raw_file_handler, prs_start_index);
        if (raw_file_handler.get_file_end_reached())
        {
            break;
        }

        ofdm_demodulator.update_hard_bits(m_frame_buffer, m_hard_bits);

        fic_handler.update_fib_blocks(m_hard_bits);
    }

    fmt::println("File ended.");
}

void MainController::update_frame_buffer(RawFileHandler& raw_file_handler, int prs_start_index)
{
    auto current_signal_index = prs_start_index;
    for (int i = 0; i < m_frame_buffer.size(); i++)
    {
        m_frame_buffer[i] = m_signal_buffer[current_signal_index++];

        if (current_signal_index >= m_signal_buffer.size())
        {
            raw_file_handler.read(m_frame_buffer, i + 1, m_frame_buffer.size() - 1);
            break;
        }
    }
}

void MainController::update_signal_buffer(RawFileHandler& raw_file_handler, int prs_start_index)
{
    auto number_of_left_points = m_signal_buffer.size() - (prs_start_index + DabConstants::T_F_U);
    if (number_of_left_points > 0)
    {
        m_signal_buffer.head(number_of_left_points) = m_signal_buffer.tail(number_of_left_points);
        raw_file_handler.read(m_signal_buffer, number_of_left_points, m_signal_buffer.size() - 1);
    }
    else
    {
        raw_file_handler.read(m_signal_buffer, 0, m_signal_buffer.size() - 1);
    }
}
