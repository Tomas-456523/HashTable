//header file for the SHA-3 hash algorithm

#ifndef SHA_3
#define SHA_3

#include <cstdint> //this is a very low-level hash algorithm for efficiency, which is why we use special variable types (these include those types)
#include <cstddef>
#include <string>

namespace SHA3 {
    void KeccakIt(uint64_t state[25]); //scrambles the given state 24 times in a 5x5 array, but a 25 array is used instead cause its slightly more efficient
    //SHA-3 uses a "sponge" so we use sponge terminology
    void Absorb(uint64_t state[25], const uint8_t* data, size_t dataSize); //handles the given data in blocks, which it XORs into state, then scrambles it
    void Squeeze(uint64_t state[25], uint8_t* output, size_t outputSize); //extracts the processed data from state
    size_t Hash(const std::string& input); //uses SHA-3 to do stuff to the given string and return a spot in the hash table based on that
}

#endif