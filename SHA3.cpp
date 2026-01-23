//implementation file for the SHA-3 encryption

#include <cstdint> //this is a very low-level hash algorithm for efficiency, which is why we use special variable types (these include those types)
#include <cstddef>
#include <string>

namespace SHA3 {
    //keccak the thing
    void KeccakIt(uint64_t state[25]) {
        //theta
        //rho
        //pi
        //chi
        //iota
    }
    void Absorb(uint64_t state[25], const uint8_t* data, size_t dataSize) {
        
    }
    void Squeeze(uint64_t state[25], uint8_t* output, size_t outputSize) {

    }
    size_t Hash(const std::string& input) {
        return size_t(0);
    }
}