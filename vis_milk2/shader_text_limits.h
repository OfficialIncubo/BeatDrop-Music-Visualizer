#pragma once

// One setting controls user shader storage and all dependent scratch buffers.
// EEL/custom-shape text continues to use MAX_BIGSTRING_LEN (32 KiB).
enum {
    MAX_SHADER_TEXT_LEN = 128 * 1024,
    MAX_SHADER_INCLUDE_LEN = 32 * 1024,
    MAX_SHADER_COMPILER_TEXT_LEN = 2 * MAX_SHADER_TEXT_LEN + MAX_SHADER_INCLUDE_LEN + 4096,
    MAX_SHADER_EDIT_SCRATCH_LEN = 2 * MAX_SHADER_TEXT_LEN + 32
};
