cmake_minimum_required(VERSION 3.10)

set (CMAKE_CXX_STANDARD 17)

project(DabReceiver)

find_package(fmt CONFIG REQUIRED)
find_package(eigen3 CONFIG REQUIRED)
find_package(fftw3f CONFIG REQUIRED)

add_executable(DabReceiver 
    "src/main.cpp"
    "src/DabConstants.h"
    "src/MainController.h"
    "src/MainController.cpp"
    "src/RawFileHandler.h"
    "src/RawFileHandler.cpp"
    "src/FftCalculator.h"
    "src/FftCalculator.cpp"
    "src/PrsCreation.h"
    "src/PrsCreation.cpp"
    "src/TimeSynchronizer.h"
    "src/TimeSynchronizer.cpp"
    "src/OfdmDemodulator.h"
    "src/OfdmDemodulator.cpp"
    "src/FicHandler.h"
    "src/FicHandler.cpp"
    "src/Viterbi.h"
    "src/Viterbi.cpp")

target_link_libraries(DabReceiver PRIVATE fmt::fmt FFTW3::fftw3f)
