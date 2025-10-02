#include <random>
#include <chrono>

#ifndef RAND_H
#define RAND_H

namespace Random {
    inline std::mt19937::result_type getSeed() {
        return static_cast<std::mt19937::result_type>(
            std::chrono::steady_clock::now().time_since_epoch().count());
    }

    inline std::mt19937 getGenerator(int numSeeds) {
        std::vector<uint> seeds (numSeeds, getSeed());
        std::seed_seq seq (seeds.begin(), seeds.end());
        return std::mt19937 { seq };
    }

    inline std::mt19937 generator { getGenerator(10) };

    inline int generateInt(int lower, int upper) {
        std::uniform_int_distribution<> int_dist { lower, upper };
        return int_dist(generator);
    }

    template <typename T>
    T generate(T lower, T upper) {
        return std::uniform_int_distribution<T>{ lower, upper } (generator);
    }

    template <typename R, typename S, typename T>
    R generateFromMixedTypes(S lower, T upper) {
        return generate<R>(static_cast<R>(lower), static_cast<T>(upper));
    }

}

#endif