#pragma once

#include "FftCalculator.h"

#include "Eigen/Dense"

namespace PrsCreation
{
    Eigen::VectorXcf create(const FftCalculator& fft_calculator);
}
