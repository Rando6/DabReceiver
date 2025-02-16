#include "RawFileHandler.h"

#include <fstream>

RawFileHandler::RawFileHandler(const std::string& file_path) :
    m_ifstream(std::ifstream(file_path, std::ios_base::binary)),
    m_buffer_index(BUFFER_SIZE),
    m_raw_iq_buffer(BUFFER_SIZE),
    m_file_end_reached(false)
{

}

RawFileHandler::~RawFileHandler()
{
    m_ifstream.close();
}

void RawFileHandler::read(Eigen::VectorXcf& output, int start_index, int stop_index)
{
    if (start_index > stop_index)
    {
        return;
    }

    if (m_file_end_reached)
    {
        return;
    }

    for (int i = start_index; i <= stop_index; i++)
    {
        if (m_buffer_index >= BUFFER_SIZE)
        {
            m_buffer_index = 0;

            auto position_before = m_ifstream.tellg();
            m_ifstream.read(reinterpret_cast<char*>(m_raw_iq_buffer.data()), BUFFER_SIZE);
            auto position_after = m_ifstream.tellg();
            auto read_bytes = position_after - position_before;
            if (read_bytes < BUFFER_SIZE)
            {
                m_file_end_reached = true;
                return;
            }
        }

        auto real = m_raw_iq_buffer[m_buffer_index++];
        auto imag = m_raw_iq_buffer[m_buffer_index++];

        // The IQ data is expected to be of the type uint8_t.
        // So we remove the DC and scale it.
        output[i] = std::complex<float>((real - 128.0f) / 128.0f, (imag - 128.0f) / 128.0f);
    }
}

bool RawFileHandler::get_file_end_reached()
{
    return m_file_end_reached;
}
