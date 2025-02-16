#include "FicHandler.h"
#include "DabConstants.h"

#include "fmt/printf.h"

#include <iostream>

using namespace DabConstants;

FicHandler::FicHandler(const Viterbi& viterbi, const Eigen::VectorX<uint8_t>& puncturing_mask) :
    m_raw_hard_bits_per_fic_block(N_CIFS),
    m_decoded_hard_bits_per_fic_block(N_CIFS),
    m_puncturing_mask(puncturing_mask),
    m_depunctured_hard_bits_buffer(L_CONV_CODEWORD),
    m_viterbi(viterbi)
{
    for (int i = 0; i < N_CIFS; i++)
    {
        m_raw_hard_bits_per_fic_block[i] = Eigen::VectorX<uint8_t>(N_RAW_FIC_BLOCK_BITS);
        m_decoded_hard_bits_per_fic_block[i] = Eigen::VectorX<uint8_t>(N_FIB_BITS);
    }
}

FicHandler FicHandler::create()
{
    auto convolutional_code_config = get_convolutional_code_config();
    Eigen::VectorX<uint8_t> ones = Eigen::VectorX<uint8_t>::Ones(N_RAW_FIC_BLOCK_BITS);
    Eigen::VectorX<uint8_t> puncturing_mask = Eigen::VectorX<uint8_t>::Zero(L_CONV_CODEWORD);
    depuncture(ones, puncturing_mask);
    auto viterbi{ Viterbi(convolutional_code_config, L_CONV_CODEWORD) };

    return FicHandler(viterbi, puncturing_mask);
}

std::shared_ptr<ConvolutionalCodeConfig> FicHandler::get_convolutional_code_config()
{
    auto bits_per_state{ Eigen::MatrixX<uint8_t>(N_STATES, N_TAPS) };
    auto previous_states_by_state{ std::map<uint8_t, std::vector<uint8_t>>() };

    for (uint8_t state_index = 0; state_index < N_STATES; state_index++)
    {
        previous_states_by_state.insert({ state_index, std::vector<uint8_t>() });

        for (uint8_t tap_index = 0; tap_index < N_TAPS; tap_index++)
        {
            bits_per_state(state_index, tap_index) = (state_index >> (N_TAPS - (tap_index + 1))) & 1;
        }
    }

    auto output_by_state_transition{ Eigen::MatrixX<Eigen::VectorX<uint8_t>>(N_STATES, N_STATES) };
    auto input_by_state_transition{ Eigen::MatrixX<uint8_t>(N_STATES, N_STATES) };
    auto next_state_by_state_and_input{ Eigen::MatrixX<uint8_t>(N_STATES, BINARY) };
    int n_conv_output = N_CONV_OUTPUT;

    for (uint8_t state_index = 0; state_index < N_STATES; state_index++)
    {
        for (uint8_t bit = 0; bit < BINARY; bit++)
        {
            uint8_t next_state = bit * ((uint8_t)pow(2, N_TAPS - 1)) + (state_index >> 1);
            next_state_by_state_and_input(state_index, bit) = next_state;
            previous_states_by_state[next_state].push_back(state_index);

            // See first formula of section 11.1.1 of ETSI EN 300 401 V2.1.1.
            auto output{ Eigen::VectorX<uint8_t>(N_CONV_OUTPUT) };
            output[0] = bit ^ bits_per_state(state_index, 1) ^ bits_per_state(state_index, 2) ^ bits_per_state(state_index, 4) ^ bits_per_state(state_index, 5);
            output[1] = bit ^ bits_per_state(state_index, 0) ^ bits_per_state(state_index, 1) ^ bits_per_state(state_index, 2) ^ bits_per_state(state_index, 5);
            output[2] = bit ^ bits_per_state(state_index, 0) ^ bits_per_state(state_index, 3) ^ bits_per_state(state_index, 5);
            output[3] = bit ^ bits_per_state(state_index, 1) ^ bits_per_state(state_index, 2) ^ bits_per_state(state_index, 4) ^ bits_per_state(state_index, 5);

            output_by_state_transition(state_index, next_state) = output;
            input_by_state_transition(state_index, next_state) = bit;
        }
    }

    auto convoluational_code_config =
        std::make_shared<ConvolutionalCodeConfig>(
            N_STATES,
            bits_per_state,
            previous_states_by_state,
            output_by_state_transition,
            input_by_state_transition,
            next_state_by_state_and_input,
            n_conv_output);
    return convoluational_code_config;
}

static const int PI_16_TABLE[32] = { 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0 };
static const int PI_15_TABLE[32] = { 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0 };
static const int PI_X_TABLE[24] = { 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0 };

// See 11.1.2 and 11.2.1 of ETSI EN 300 401 V2.1.1.
void FicHandler::depuncture(const Eigen::VectorX<uint8_t>& raw_bits, Eigen::VectorX<uint8_t>& depunctured_bits)
{
    auto raw_bits_index = 0;
    auto filled_bits_index = 0;

    // First we must reset the depunctured_bits to zeros.
    for (int i = 0; i < depunctured_bits.size(); i++)
    {
        depunctured_bits[i] = 0;
    }

    // The first 21 blocks are punctured as defined in clause 11.1.2, according to the puncturing index PI = 16.
    for (int block = 0; block < 21; block++)
    {
        for (int subblock = 0; subblock < 4; subblock++)
        {
            for (int bit_index = 0; bit_index < 32; bit_index++)
            {
                if (PI_16_TABLE[bit_index] == 1)
                {
                    depunctured_bits[filled_bits_index] = raw_bits[raw_bits_index];
                    raw_bits_index++;
                }
                filled_bits_index++;
            }
        }
    }

    // The remaining 3 blocks are punctured as defined in clause 11.1.2, according to the puncturing index PI = 15.
    for (int block = 0; block < 3; block++)
    {
        for (int subblock = 0; subblock < 4; subblock++)
        {
            for (int bit_index = 0; bit_index < 32; bit_index++)
            {
                if (PI_15_TABLE[bit_index] == 1)
                {
                    depunctured_bits[filled_bits_index] = raw_bits[raw_bits_index];
                    raw_bits_index++;
                }
                filled_bits_index++;
            }
        }
    }

    // Finally, the last 24 bits of the serial mother codeword are punctured as defined in clause 11.1.2.
    for (int bit_index = 0; bit_index < 24; bit_index++)
    {
        if (PI_X_TABLE[bit_index] == 1)
        {
            depunctured_bits[filled_bits_index] = raw_bits[raw_bits_index];
            raw_bits_index++;
        }
        filled_bits_index++;
    }
}

// TODO: Not complete. After running Viterbi, the data bits must be processed.
void FicHandler::update_fib_blocks(Eigen::MatrixX<uint8_t>& hard_bits)
{
    update_raw_hard_bits_per_fic_block(hard_bits);
    for (int i = 0; i < N_CIFS; i++)
    {
        depuncture(m_raw_hard_bits_per_fic_block[i], m_depunctured_hard_bits_buffer);
        int number_of_error_bits = m_viterbi.run(m_depunctured_hard_bits_buffer, m_puncturing_mask, m_decoded_hard_bits_per_fic_block[i]);
        fmt::println("The number of error bits is {} for the FIC block {}.", number_of_error_bits, i);
    }
}

void FicHandler::update_raw_hard_bits_per_fic_block(Eigen::MatrixX<uint8_t>& hard_bits)
{
    auto reshaped_hard_bits = hard_bits.topRows(N_FIC_SYMBOLS).reshaped<Eigen::StorageOptions::RowMajor>(N_CIFS, N_RAW_FIC_BLOCK_BITS);
    for (int row = 0; row < N_CIFS; row++)
    {
        for (int col = 0; col < N_RAW_FIC_BLOCK_BITS; col++)
        {
            m_raw_hard_bits_per_fic_block[row][col] = reshaped_hard_bits(row, col);
        }
    }
}
