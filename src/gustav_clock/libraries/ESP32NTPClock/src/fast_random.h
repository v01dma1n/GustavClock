
#ifndef FAST_RANDOM_H
#define FAST_RANDOM_H

#include <stdint.h>

// A simple, fast, non-blocking pseudo-random number generator based on the xorshift32 algorithm.
class FastRandom {
public:
    // Constructor: Seeds the generator with a non-zero value.
    // Using millis() or another changing value at startup is a good simple seed.
    FastRandom(uint32_t seed) {
        if (seed == 0) {
            _state = 314159265; // A default non-zero seed
        } else {
            _state = seed;
        }
    }

    // Generates the next 32-bit pseudo-random number.
    uint32_t next() {
        uint32_t x = _state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        _state = x;
        return x;
    }

    // Generates a pseudo-random number within a given range [min, max].
    uint32_t nextRange(uint32_t min, uint32_t max) {
        // Use modulo to map the 32-bit number to the desired range.
        return min + (next() % (max - min + 1));
    }

private:
    uint32_t _state; // The internal state of the generator
};

#endif // FAST_RANDOM_H