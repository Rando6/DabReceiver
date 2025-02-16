#pragma once

#include "Eigen/Dense"

#include <fstream>
#include <vector>

class RawFileHandler final
{
public:
    RawFileHandler(const std::string& file_path);
    ~RawFileHandler();

    void read(Eigen::VectorXcf& output, int start_index, int stop_index);

    bool get_file_end_reached();

private:
    const int BUFFER_SIZE = 65536;

    std::ifstream m_ifstream;
    int m_buffer_index;
    std::vector<uint8_t> m_raw_iq_buffer;

    bool m_file_end_reached;
};
