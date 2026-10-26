#pragma once
#include <climits>
#include <cstdio>
#include <cstring>
#include <vector>

// Read one complete contiguous preset code block. The callback has the same
// contract as GetFastString: return the sentinel for a missing numbered key.
// Never compile a truncated block, and never advance/write after overflow.
template <size_t Capacity, class GetLine>
bool ReadPresetCode(char (&output)[Capacity], const char* prefix,
                    char lineFeed, GetLine getLine)
{
    static_assert(Capacity > 8 && Capacity <= INT_MAX, "Invalid code buffer capacity");
    output[0] = 0;
    std::vector<char> storage(Capacity, 0);
    size_t used = 0;
    for (unsigned line = 1; ; ++line) {
        char key[64];
        const int keyLength = std::snprintf(key, sizeof(key), "%s%u", prefix, line);
        if (keyLength < 0 || static_cast<size_t>(keyLength) >= sizeof(key)) {
            output[0] = 0;
            return false;
        }
        getLine(key, storage.data(), static_cast<int>(Capacity));
        storage.back() = 0;
        if (std::strcmp(storage.data(), "~!@#$") == 0)
            return true;
        const size_t rawLength = std::strlen(storage.data());
        // GetFastString reserves several bytes in its line reader. A line
        // reaching that boundary may already have been truncated; reject it.
        const char* text = storage.data() + (storage[0] == '`' ? 1 : 0);
        const size_t length = std::strlen(text);
        if (rawLength >= Capacity - 8 || used > Capacity - 2 || length > Capacity - used - 2) {
            output[0] = 0;
            return false;
        }
        std::memcpy(output + used, text, length);
        used += length;
        output[used++] = lineFeed;
        output[used] = 0;
    }
}
