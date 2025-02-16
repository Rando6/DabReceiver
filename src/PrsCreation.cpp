#include "PrsCreation.h"
#include "DabConstants.h"
#include "FftCalculator.h"

#include <complex>

using namespace DabConstants;
using namespace std::complex_literals;

struct IndexTableEntry
{
    int k_prime;
    int i;
    int n;
};

// See table 23 of ETSI EN 300 401 V2.1.1.
static const IndexTableEntry INDEX_TABLE[] =
{
    {-768, 0, 1},
    {-736, 1, 2},
    {-704, 2, 0},
    {-672, 3, 1},
    {-640, 0, 3},
    {-608, 1, 2},
    {-576, 2, 2},
    {-544, 3, 3},
    {-512, 0, 2},
    {-480, 1, 1},
    {-448, 2, 2},
    {-416, 3, 3},
    {-384, 0, 1},
    {-352, 1, 2},
    {-320, 2, 3},
    {-288, 3, 3},
    {-256, 0, 2},
    {-224, 1, 2},
    {-192, 2, 2},
    {-160, 3, 1},
    {-128, 0, 1},
    {-96, 1, 3},
    {-64, 2, 1},
    {-32, 3, 2},
    {1, 0, 3},
    {33, 3, 1},
    {65, 2, 1},
    {97, 1, 1},
    {129, 0, 2},
    {161, 3, 2},
    {193, 2, 1},
    {225, 1, 0},
    {257, 0, 2},
    {289, 3, 2},
    {321, 2, 3},
    {353, 1, 3},
    {385, 0, 0},
    {417, 3, 2},
    {449, 2, 1},
    {481, 1, 3},
    {513, 0, 3},
    {545, 3, 3},
    {577, 2, 3},
    {609, 1, 0},
    {641, 0, 3},
    {673, 3, 0},
    {705, 2, 1},
    {737, 1, 1}
};

// See table 24 of ETSI EN 300 401 V2.1.1.
static const int H_TABLE[4][32] =
{
    {0, 2, 0, 0, 0, 0, 1, 1, 2, 0, 0, 0, 2, 2, 1, 1, 0, 2, 0, 0, 0, 0, 1, 1, 2, 0, 0, 0, 2, 2, 1, 1},
    {0, 3, 2, 3, 0, 1, 3, 0, 2, 1, 2, 3, 2, 3, 3, 0, 0, 3, 2, 3, 0, 1, 3, 0, 2, 1, 2, 3, 2, 3, 3, 0},
    {0, 0, 0, 2, 0, 2, 1, 3, 2, 2, 0, 2, 2, 0, 1, 3, 0, 0, 0, 2, 0, 2, 1, 3, 2, 2, 0, 2, 2, 0, 1, 3},
    {0, 1, 2, 1, 0, 3, 3, 2, 2, 3, 2, 1, 2, 1, 3, 2, 0, 1, 2, 1, 0, 3, 3, 2, 2, 3, 2, 1, 2, 1, 3, 2}
};

// See first formula of section 14.3.2 of ETSI EN 300 401 V2.1.1.
// Note: The implementation doesn't evaluate e^(i*phi_k) directly.
std::complex<float> calculate_z_1k(const int h, const int n)
{
    auto factor = (h + n) % 4;
    if (factor == 0)
    {
        return 1.0f;
    }
    else if (factor == 1)
    {
        return 1if;
    }
    else if (factor == 2)
    {
        return -1.0f;
    }
    else
    {
        return -1if;
    }
}

// Creates a reference PRS symbol as described in section 14.3.2 of ETSI EN 300 401 V2.1.1.
Eigen::VectorXcf PrsCreation::create(const FftCalculator& fft_calculator)
{
    auto prs_fd{ Eigen::VectorXcf(T_U) };

    // We start with the second half of the INDEX_TABLE which represents the positive frequencies.
    auto k = 1;
    auto current_table_index = 24;

    prs_fd[0] = 0; // The DC component is 0.
    for (int prs_fd_index = 1; prs_fd_index < T_U; prs_fd_index++)
    {
        if (prs_fd_index > K_MAX && prs_fd_index < T_U - K_MAX)
        {
            prs_fd[prs_fd_index] = 0;
            continue;
        }

        if (prs_fd_index == T_U - K_MAX)
        {
            current_table_index = 0;
            k = K_MIN;
        }

        if (k <= 737 && k >= INDEX_TABLE[current_table_index + 1].k_prime) // 737 == k_prime_max (see last row of table 23 of ETSI EN 300 401 V2.1.1)
        {
            current_table_index++;
        }

        auto current_index_row = INDEX_TABLE[current_table_index];

        auto k_prime = current_index_row.k_prime;
        auto i = current_index_row.i;
        auto n = current_index_row.n;
        auto h = H_TABLE[i][k - k_prime];
        auto z_1k = calculate_z_1k(h, n);
        prs_fd[prs_fd_index] = z_1k;

        k++;
    }

    auto prs_td_with_cp{ Eigen::VectorXcf(T_S) };
    auto prs_td_without_cp = prs_td_with_cp.tail<T_U>().data();
    fft_calculator.ifft(prs_fd.data(), prs_td_without_cp);

    prs_td_with_cp.head<T_G>() = prs_td_with_cp.tail<T_G>();

    return prs_td_with_cp;
}
