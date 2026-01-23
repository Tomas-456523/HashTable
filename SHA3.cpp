//implementation file for the SHA-3 encryption

#include <cstdint> //this is a very low-level hash algorithm for efficiency, which is why we use special variable types (these include those types)
#include <cstddef>
#include <string>
using namespace std;

namespace SHA3 {
    //bitwise rotation left by "shift" spaces for the given 64-bit integer
    uint64_t rotl64(uint64_t bits, int shift) {
        //moves all the bits to the left by amount "shift". This discards the leftmost bits, so we also move the bits 64-shift to the right, therefore discarding everything but the leftmost ones which are now pushed all the way to the right
        //after ORing the two, they're effectively combined since there's no overlap
        return (bits << shift) | (bits >> (64-shift));
    }
    //gets the amount to rotl each column by during the rho step. Each number was meticulously and specifically chosen by the makers of SHA-3 to ensure maximum scrambled-ness
    int GetRhotation(int pos) {
        int rhotations[25] = {
            0,  1,  62, 28, 27,
            36, 44, 6,  55, 20,
            3,  10, 43, 25, 39,
            41, 45, 15, 21, 8,
            18, 2,  61, 56, 14
        };
        return rhotations[pos];
    }
    //keccak the thing (pronounced ketch-ak), done in 5 greek letter steps: THETA, RHO, PI, CHI, and IOTA, and we do that 24 times
    void KeccakIt(uint64_t state[25]) {
        //THETA STEP, diffuse each column with its adjacent columns
        uint64_t ColCol[5]; //stands for Collapsed Column
        for (int c = 0; c < 5; c++) { //collapse each column into one 64-bit int by xoring all the rows
            ColCol[c] = state[c]^state[c+5]^state[c+10]^state[c+15]^state[c+20];
        }
        //diffuse each column with its neighbors by xoring it with the left ColCol xor'd with (the right ColCol rotled by 1)
        for (int c = 0; c < 5; c++) { //for each column, figure out the mixture (described above) of the adjacent columns (looping around the edges)
            //we have to +4 instead of -1 because C++ modulo can't handle negatives
            uint64_t lr = ColCol[(c+4)%5] ^ rotl64(ColCol[(c+1)%5], 1);
            for (int r = 0; r < 5; r++) { //for each row in the column, xor it with the neighbors
                state[c + 5*r] ^= lr;
            }
        }
        //rho
        //pi
        //chi
        //iota
    }
    void Absorb(uint64_t state[25], const uint8_t* data, size_t dataSize) {
        
    }
    void Squeeze(uint64_t state[25], uint8_t* output, size_t outputSize) {

    }
    size_t Hash(const string& input) {
        return size_t(0);
    }
}