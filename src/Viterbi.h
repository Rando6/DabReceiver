#pragma once

#include "Eigen/Dense";

#include <map>

struct ConvolutionalCodeConfig
{
    ConvolutionalCodeConfig(
        int n_states,
        Eigen::MatrixX<uint8_t> bits_per_state,
        std::map<uint8_t, std::vector<uint8_t>> previous_states_by_state,
        Eigen::MatrixX<Eigen::VectorX<uint8_t>> output_by_state_transition,
        Eigen::MatrixX<uint8_t> input_by_state_transition,
        Eigen::MatrixX<uint8_t> next_state_by_state_and_input,
        int n_conv_output);

    // Number of states of the convolutional encoder.
    int n_states;

    // Each row represents a state of the convolutional encoder.
    // Each column represents the taps of the convolutional encoder.
    Eigen::MatrixX<uint8_t> bits_per_state;

    // Here, each state is represented by its row index in bits_per_state.
    std::map<uint8_t, std::vector<uint8_t>> previous_states_by_state;

    // The row index represents a state index and
    // the column index represents the next state index.
    // The content of each cell represents
    // the output of such a state transition.
    Eigen::MatrixX<Eigen::VectorX<uint8_t>> output_by_state_transition;

    // Similar to output_by_state_transition but
    // with the input bit stored in each cell.
    Eigen::MatrixX<uint8_t> input_by_state_transition;

    // The row index represents a state index and
    // the column index represents an input bit.
    // The content of each cell represents the next state.
    Eigen::MatrixX<uint8_t> next_state_by_state_and_input;

    // The number of bits which the convolutional encoder outputs per step.
    int n_conv_output;
};

class Viterbi final
{
public:
    Viterbi(const std::shared_ptr<ConvolutionalCodeConfig>& convolutional_code_config, int l_conv_codeword);

    // Determines the decoded_bits and returns the number of error bits
    // found in the last time step of the Viterbi algorithm.
    int run(
        const Eigen::VectorX<uint8_t>& depunctured_received_bits,
        const Eigen::VectorX<uint8_t>& puncturing_mask,
        Eigen::VectorX<uint8_t>& decoded_bits);

private:
    // The convolutional code config.
    std::shared_ptr<ConvolutionalCodeConfig> m_convolutional_code_config;

    // The length of the convolutional codeword.
    int m_l_conv_codeword;

    // The length of the input vector to the convolutional encoder.
    // Correponds directly to m_l_conv_codeword.
    int m_time_length;

    // Stores the number of error bits in each cell.
    // The rows correspond to the states of the convoluational encoder and
    // the columns correspond to the length of the input vector to the convolutional encoder.
    Eigen::MatrixX<int> m_viterbi_matrix;

    // The chosen states per time step.
    // The length of this vector is equal to m_time_length.
    Eigen::VectorX<uint8_t> m_best_states;
};
