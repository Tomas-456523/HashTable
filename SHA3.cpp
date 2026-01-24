//implementation file for the SHA-3 hash algorithm, featuring Keccak-f[1600]

#include <cstdint> //this is a very low-level hash algorithm for efficiency, which is why we use special variable types (these include those types)
#include <cstddef>
#include <cstring>
#include <string>
using namespace std;

namespace SHA3 {
    //bitwise rotation left by "shift" spaces for the given 64-bit integer
    uint64_t rotl64(uint64_t bits, int shift) {
        if (!shift) { //if there's 0 shift we just return the bits unchanged, we have to specifically check for this because uint64_t >> 64 is apparently undefined behavior
            return bits;
        }//moves all the bits to the left by amount "shift". This discards the leftmost bits, so we also move the bits 64-shift to the right, therefore discarding everything but the leftmost ones which are now pushed all the way to the right
        //after ORing the two, they're effectively combined since there's no overlap
        return (bits << shift) | (bits >> (64-shift));
    }
    //used during the rho step to get the amount to rotl each column by. The numbers were chosen by the makers of SHA-3 to ensure maximum scrambled-ness
    static const int rhotations[25] = {
        0,  1,  62, 28, 27,
        36, 44, 6,  55, 20,
        3,  10, 43, 25, 39,
        41, 45, 15, 21, 8,
        18, 2,  61, 56, 14
    };
    //used during the pi step to more efficiently compute r +(2*c+3*r)%5*5, which is the position that the current item in state gets moved to. This formula was chosen similarly to and alongside the above rhotations to ensure maximum scrambled-ness
    static const int piShuffle[25] = {
        0,  10, 20, 5,  15,
        16, 1,  11, 21, 6,
        7,  17, 2,  12, 22,
        23, 8,  18, 3,  13,
        14, 24, 9,  19, 4
    };
    //used during the iota step, each keccak round uses a different one, which is xor'd into the first item in state. The constants were generated to look random and they're standardized so they're what every SHA-3 uses 
    static const uint64_t iotaConstants[24] = {
        0x0000000000000001ULL, 0x0000000000008082ULL, //ULL means Unsigned Long Long
        0x800000000000808AULL, 0x8000000080008000ULL,
        0x000000000000808BULL, 0x0000000080000001ULL,
        0x8000000080008081ULL, 0x8000000000008009ULL,
        0x000000000000008AULL, 0x0000000000000088ULL,
        0x0000000080008009ULL, 0x000000008000000AULL,
        0x000000008000808BULL, 0x800000000000008BULL,
        0x8000000000008089ULL, 0x8000000000008003ULL,
        0x8000000000008002ULL, 0x8000000000000080ULL,
        0x000000000000800AULL, 0x800000008000000AULL,
        0x8000000080008081ULL, 0x8000000000008080ULL,
        0x0000000080000001ULL, 0x8000000080008008ULL
    };
    //the rate used by SHA-3 256 is standardized as 136. If I ever wanted to change SHA-3s I need to change the rate here
    static constexpr size_t RATE = 136;
    //keccak the given state array (pronounced ketch-ak), done in 5 greek letter steps: THETA, RHO, PI, CHI, and IOTA, and we do that 24 times
    void KeccakIt(uint64_t state[25]) {
        for (int round = 0; round < 24; round++) {
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
            //RHO STEP, rotl each column according to the hardcoded rhotations
            uint64_t copy[25];
            for (int i = 0; i < 25; i++) {
                copy[i] = rotl64(state[i], rhotations[i]);
            }
            //PI STEP, shuffle all the 64-bit ints according to the hardcoded piShuffles, which is based on r +(2*c+3*r)%5*5
            for (int i = 0; i < 25; i++) {
                state[piShuffle[i]] = copy[i];
            }
            //CHI STEP, xors each item with its ((inverted following neighbor) anded with the following following neighbor), in order to make it unpredictable with linear algebra
            memcpy(copy, state, sizeof(copy)); //copies the state as of the pi step so we can use that while still modifying state
            for (int c = 0; c < 5; c++) {
                for (int r = 0; r < 5; r++) {
                    state[c+5*r] ^= (~copy[(c+1)%5+5*r] & copy[(c+2)%5+5*r]);
                }
            }
            //IOTA STEP, just xor the current round's iotaConstant into the first item in state. This is done to prevent edge cases where the result of the above four steps was just the original (for example, an input of all 0s)
            state[0] ^= iotaConstants[round];
        }
    }
    //absorbs the input into state in RATE-sized blocks and runs each block through Keccak
    void Absorb(uint64_t state[25], const uint8_t* input, size_t inputSize) {
        //reinterpret state as a bytes object so we can modify the bits of state
        uint8_t* stateBytes = reinterpret_cast<uint8_t*>(state);
        //while the input is able to be divided into RATE-sized blocks without padding
        while (inputSize >= RATE) { //technically this program just skips this loop since I just input 4-byte ints but I included this check in case I ever wanted to use it for something else
            for (size_t i = 0; i < RATE; i++) { //xor the next RATE bytes of input into state
                stateBytes[i] ^= input[i];
            } //do the whole Keccak process with the newly updated state
            KeccakIt(state);
            input += RATE; //moves the pointer forward because we don't need to read the data again
            inputSize -= RATE; //removes RATE amount from the input size so at the end we know how much input is left to process
        } //the last block of data left to process, we put it into a RATE-sized block of 0s
        uint8_t end[RATE] = {0};
        memcpy(end, input, inputSize); //put the remaining input into the end block
        //standardized padding; 6 after the remaining input and 128 at the end
        end[inputSize] = 6;
        end[RATE-1] |= 128; //we or it into end to account for the edge case where inputSize == RATE - 1, so the 128 doesn't just overwrite the 6
        for (size_t i = 0; i < RATE; i++) { //xor the last block into state
            stateBytes[i] ^= end[i];
        } //normally there would be a final KeccakIt(state) here but I moved it to Squeeze
    }
    //squeezes the output from state
    void Squeeze(uint64_t state[25], uint8_t* output, size_t outputSize) {
        //reinterpret state as a bytes object so we can deal with the specific bits rather than pesky 64-bit 8-byte integers
        uint8_t* stateBytes = reinterpret_cast<uint8_t*>(state);

        //while we still need to write more output
        while (outputSize > 0) {
            KeccakIt(state); //Keccak the last block from Absorb on the first go, or to extract different bytes from the sponge on the subsequent goes
            //if we don't have at least RATE bytes left to output, we only check the remaining outputSize bytes. Otherwise, we keep adding to the output in RATE-sized blocks
            size_t blockSize = (outputSize < RATE) ? outputSize : RATE;
            //copy the next blockSize bytes into the output from state
            memcpy(output, stateBytes, blockSize);
            outputSize -= blockSize; //subtract from outputSize so we know how many bytes we still need to output
            output += blockSize; //moves the output pointer forward so we keep writing output in the next loop where we left off in this loop
        }
    }
    //creates a 256-bit 32-byte hash stored as a string using the inputted integer
    string Hash(const int input) {
        uint64_t state[25] = {0}; //creates the state array which will be used to process the input
        //absorb the state into the "sponge", using the data from the input which is byte-ified
        Absorb(state, reinterpret_cast<const uint8_t*>(&input), sizeof(input));
        string hash(32, '\0'); //creates the container for the hash bytes, SHA-3 256 creates a 32-byte hash so we make sure the string is that length
        //extract the hash from the sponge into the hash string (which is byte-ified to allow for the modification of the individual bits)
        Squeeze(state, reinterpret_cast<uint8_t*>(&hash[0]), hash.size());
        return hash; //return the generated hash! lets goooooooooooooooooooooooooooooo
    }
}