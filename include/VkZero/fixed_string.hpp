#pragma once


#include <algorithm>

namespace VkZero
{
    template<size_t N>
    struct FixedString {
        char value[N] = {};
        
        constexpr FixedString(const char (&str)[N]) {
            std::copy_n(str, N, value);
        }
        
        // Constexpr comparison operator
        template<size_t M>
        constexpr bool operator==(const FixedString<M>& other) const {
            if (N != M) return false;
            for (size_t i = 0; i < N; ++i) {
                if (value[i] != other.value[i]) return false;
            }
            return true;
        }
    };
}