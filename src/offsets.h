#pragma once
#include <cstdint>

namespace Offsets {
    // Offsets for Hyperion's integrity checks (example values)
    constexpr uint16_t SCF_INSERTED_JMP = 0x04EB;
    constexpr uint32_t SCF_END_MARKER = 0xF4CC02EB;
    constexpr uintptr_t SCF_MARKER_STK = 0xDEADBEEFDEADC0DE;
    constexpr uint64_t kPageMask = 0xfffffffffffff000;
    constexpr uint8_t kPageShift = 0xc;
    constexpr uint64_t kPageHash = 0x84B3A57D90E73527;
    constexpr uint64_t Offset_InsertSet = 0xC43D00;
    constexpr uint64_t Offset_WhitelistedPages = 0x29C758;

    // Core Roblox structure offsets (examples)
    constexpr uintptr_t DataModel = 0x0;
    constexpr uintptr_t ScriptContext = 0x0;
    constexpr uintptr_t lua_State = 0x0;
}
