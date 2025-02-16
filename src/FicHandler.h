#pragma once

#include "Viterbi.h";

#include "Eigen/Dense"

class FicHandler final
{
public:
    static FicHandler create();

    void update_fib_blocks(Eigen::MatrixX<uint8_t>& hard_bits);

private:
    FicHandler(const Viterbi& viterbi, const Eigen::VectorX<uint8_t>& puncturing_mask);

    static std::shared_ptr<ConvolutionalCodeConfig> get_convolutional_code_config();
    static void depuncture(const Eigen::VectorX<uint8_t>& raw_bits, Eigen::VectorX<uint8_t>& depunctured_bits);

    void update_raw_hard_bits_per_fic_block(Eigen::MatrixX<uint8_t>& hard_bits);

    std::vector<Eigen::VectorX<uint8_t>> m_raw_hard_bits_per_fic_block;
    std::vector<Eigen::VectorX<uint8_t>> m_decoded_hard_bits_per_fic_block;
    Eigen::VectorX<uint8_t> m_puncturing_mask;
    Eigen::VectorX<uint8_t> m_depunctured_hard_bits_buffer;
    Viterbi m_viterbi;
};