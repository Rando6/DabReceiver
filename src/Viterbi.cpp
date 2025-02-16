#include "Viterbi.h";

#include <iostream>;

ConvolutionalCodeConfig::ConvolutionalCodeConfig(
    int n_states,
    Eigen::MatrixX<uint8_t> bits_per_state,
    std::map<uint8_t, std::vector<uint8_t>> previous_states_by_state,
    Eigen::MatrixX<Eigen::VectorX<uint8_t>> output_by_state_transition,
    Eigen::MatrixX<uint8_t> input_by_state_transition,
    Eigen::MatrixX<uint8_t> next_state_by_state_and_input,
    int n_conv_output) :
    n_states(n_states),
    bits_per_state(bits_per_state),
    previous_states_by_state(previous_states_by_state),
    output_by_state_transition(output_by_state_transition),
    input_by_state_transition(input_by_state_transition),
    next_state_by_state_and_input(next_state_by_state_and_input),
    n_conv_output(n_conv_output)
{

}

Viterbi::Viterbi(const std::shared_ptr<ConvolutionalCodeConfig>& convolutional_code_config, int l_conv_codeword) :
    m_convolutional_code_config(convolutional_code_config),
    m_l_conv_codeword(l_conv_codeword),
    m_time_length((l_conv_codeword / convolutional_code_config->n_conv_output) + 1),
    m_viterbi_matrix(convolutional_code_config->n_states, m_time_length),
    m_best_states(m_time_length)
{

}

int Viterbi::run(
    const Eigen::VectorX<uint8_t>& depunctured_received_bits,
    const Eigen::VectorX<uint8_t>& puncturing_mask,
    Eigen::VectorX<uint8_t>& decoded_bits)
{
    auto length_of_received_bits = static_cast<int>(depunctured_received_bits.size());
    assert(m_l_conv_codeword == length_of_received_bits);
    auto end_time = m_time_length - 1;

    // Reset the viterbi matrix buffer.
    // We know that the state at time 0 is 0.
    // Hence, the number of error bits of the state 0 is set to 0.
    // The number of the other states at time 0 is set to the maximum possible integer.
    m_viterbi_matrix = Eigen::MatrixX<int>::Zero(m_convolutional_code_config->n_states, m_time_length);
    for (int row = 0; row < m_convolutional_code_config->n_states; row++)
    {
        m_viterbi_matrix(row, 0) = INT_MAX;
    }
    m_viterbi_matrix(0, 0) = 0;

    // In the forward direction we determine the minimum number of error bits per state and time step.
    for (int time = 1; time < m_time_length; time++)
    {
        auto received_bits_group =
            depunctured_received_bits.segment(m_convolutional_code_config->n_conv_output * (time - 1), m_convolutional_code_config->n_conv_output).eval();
        auto puncturing_group =
            puncturing_mask.segment(m_convolutional_code_config->n_conv_output * (time - 1), m_convolutional_code_config->n_conv_output).eval();

        for (int state_index = 0; state_index < m_convolutional_code_config->n_states; state_index++)
        {
            auto distance = INT_MAX;
            auto possible_previous_states = m_convolutional_code_config->previous_states_by_state[state_index];

            for (auto previous_state_index : possible_previous_states)
            {
                auto possible_previous_distance = m_viterbi_matrix(previous_state_index, time - 1);
                auto possible_output = m_convolutional_code_config->output_by_state_transition(previous_state_index, state_index);

                // Determine the Hamming distance.
                auto hamming_distance = 0;
                for (int i = 0; i < m_convolutional_code_config->n_conv_output; i++)
                {
                    if (puncturing_group[i] == 1)
                    {
                        if (received_bits_group[i] != possible_output[i])
                        {
                            hamming_distance += 1;
                        }
                    }
                }

                auto possible_distance = INT_MAX;
                if (possible_previous_distance < INT_MAX)
                {
                    possible_distance = possible_previous_distance + hamming_distance;
                }

                if (possible_distance < distance)
                {
                    distance = possible_distance;
                }
            }

            m_viterbi_matrix(state_index, time) = distance;
        }
    }

    // In the backward direction we select the state with the minimum number of error bits.

    // First we reset the selected states from a previous run.
    m_best_states = Eigen::VectorX<uint8_t>::Zero(m_time_length);

    // Find the best state at the end time.
    auto best_value = m_viterbi_matrix(0, end_time);
    auto chosen_state_index = 0;
    for (int state_index = 0; state_index < m_convolutional_code_config->n_states; state_index++)
    {
        auto value = m_viterbi_matrix(state_index, end_time);
        if (value < best_value)
        {
            chosen_state_index = state_index;
        }
    }
    m_best_states[end_time] = chosen_state_index;

    // Go back in time and select the state with the minimum number of error bits.
    for (auto time = end_time; time > 0; time--)
    {
        auto possible_previous_states = m_convolutional_code_config->previous_states_by_state[m_best_states[time]];
        best_value = INT_MAX;
        for (auto previous_state_index : possible_previous_states)
        {
            auto value = m_viterbi_matrix(previous_state_index, time - 1);
            if (value < best_value)
            {
                m_best_states[time - 1] = previous_state_index;
                best_value = value;
            }
        }
    }

    // Determine the decoded_bits by going again forward in time and using the best state per time step.
    decoded_bits = Eigen::VectorX<uint8_t>::Zero(m_time_length - 1);
    for (int time = 0; time < m_time_length - 1; time++)
    {
        auto decoded_bit = m_convolutional_code_config->input_by_state_transition(m_best_states[time], m_best_states[time + 1]);
        decoded_bits[time] = decoded_bit;
    }

    return m_viterbi_matrix(m_best_states[end_time], end_time);
}


