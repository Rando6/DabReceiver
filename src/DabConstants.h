#pragma once

namespace DabConstants
{
    // The number of useful carriers of an OFDM symbol.
    constexpr int N_CARRIERS = 1'536;

    // Minimum discrete frequency.
    constexpr int K_MIN = -768;

    // Maximum discrete frequency.
    constexpr int K_MAX = 768;

    // Number of OFDM symbols.
    constexpr int N_OFDM_SYMBOLS = 76;

    // Number of symbols carrying data.
    // It is 1 smaller than N_OFDM_SYMBOLS because
    // the PRS symbol doesn't carry data.
    constexpr int N_DATA_SYMBOLS = N_OFDM_SYMBOLS - 1;

    // Length of a DAB frame.
    constexpr int T_F = 196'608;

    // Minimum length comprising one DAB frame and one symbol
    // which is equal to a power of 2.
    // It is used for the FFT to determine
    // the cross correlation between an ideal PRS symbol
    // and the received signal.
    constexpr int T_F_FFT = 262'144;

    // Length of the Null symbol.
    constexpr int T_NULL = 2'656;

    // Length of a DAB frame without the Null symbol.
    constexpr int T_F_U = T_F - T_NULL;

    // Useful length of an OFDM symbol.
    constexpr int T_U = 2'048;

    // Length of the guard interval.
    constexpr int T_G = 504;

    // Length of one of the OFDM symbols 1-76.
    constexpr int T_S = T_U + T_G;

    // Number of OFDM symbols used for the fast information channel (FIC).
    constexpr int N_FIC_SYMBOLS = 3;

    // There are 2 binary digits, 0 and 1.
    constexpr int BINARY = 2;

    // Number of raw bits per FIC symbol.
    constexpr int N_RAW_FIC_SYMBOL_BITS = N_CARRIERS * BINARY;

    // Number of common interleaved frames (CIFs).
    constexpr int N_CIFS = 4;

    // Number of raw bits in the FIC per CIF.
    constexpr int N_RAW_FIC_BLOCK_BITS = (N_FIC_SYMBOLS * N_RAW_FIC_SYMBOL_BITS) / N_CIFS;

    // Number of taps of the convolutional encoder.
    constexpr int N_TAPS = 6;

    // Number of states the convoluational encoder can have
    // (which is 2 to the power of N_TAPS).
    constexpr int N_STATES = 64;

    // Number of bits which the convoluational encoder produces per step.
    constexpr int N_CONV_OUTPUT = 4;

    // Number of data bits per fast information block (FIB).
    constexpr int N_FIB_BITS = 768;

    // Length of the convolutional codeword.
    constexpr int L_CONV_CODEWORD = N_CONV_OUTPUT * (N_FIB_BITS + N_TAPS);
}
