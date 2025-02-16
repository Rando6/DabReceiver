#pragma once

#include "MainController.h"
#include "RawFileHandler.h"

#include "Eigen/Dense"

#include <string>

class MainController final
{
public:
    MainController();

    void run(const std::string& file_path);

private:
    Eigen::VectorXcf m_signal_buffer;
    Eigen::VectorXcf m_frame_buffer;
    Eigen::MatrixX<uint8_t> m_hard_bits;

    void MainController::update_signal_buffer(RawFileHandler& raw_file_handler, int prs_start_index);
    void MainController::update_frame_buffer(RawFileHandler& raw_file_handler, int prs_start_index);
};
