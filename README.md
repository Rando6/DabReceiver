# DabReceiver

This project was created for educational purposes.
It demonstrates how the time synchronization works for DAB signals,
how the OFDM symbols are demodulated and
how the Viterbi algorithm is used to decode the received bits.

The DAB specification ETSI EN 300 401 V2.1.1 was the basis for this work.

The project is by far not finished.
Currently, the output bits of the Viterbi algorithm are not further processed.
Furthermore, the implementation of decoding the main service channel is not even started.


# How to Use it?

Just start the application with a command line argument which is the file path of a raw IQ file.
(Such a raw IQ file coulde also be captured by the help of an SDR antenna.)
It should then print the found start indices of the PRS symbols and
the number of error bits per decoded FIC block found by the Viterbi algorithm.
The number of error bits for the accompanied data\test.iq is not greater than 3 for any FIC block.


# Architecure of the Project

The heart of the application is the MainController, especially its run method.
First, raw IQ data is read from an IQ file (an example file can be found in the data folder).
Then, the time synchronizer determines the start of the PRS symbol using correlation
between an ideal PRS symbol and the read data.
After that, the OFDM demodulator demodulates the OFDM symbols.
For that, it needs to determine the fine frequency offset,
correct the phase, deinterleave the frequencies and demap the QPSK symbols.
Finally, the Viterbi algorithm decodes the bits for the FIC handler.


## How to Build the Project?

For this project was Visual Studio 2022 used on a Windows 11 machine.
Make sure that the CMake tools and the vcpkg manager is installed by the Visual Studio Installer.
Open the project by right clicking in the root directory of this repository and select open in Visual Studio 2022.
Then, build and run the project.
That should have been it.


## To Dos

* Unit tests.
* Error handling.
* Coarse frequency offset determination.
* Improvement of the performance of the Viterbi algorithm.


## Used Abbreviations

* FFT = Fast Fourier Transformation
* IFFT = Inverse Fast Fourier Transformation
* FD = Frequency Domain
* TD = Time Domain
* PRS = Phase Reference Symbol
* CP = Cyclic Prefix
* prev = previous
