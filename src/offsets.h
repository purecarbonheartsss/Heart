#pragma once
#include <cstdint>

namespace Offsets {
    // Hyperion / Byfron Bypass
    constexpr uint64_t HyperionBase = 0x0; 
    
    // Luau VM Hooks
    constexpr uint64_t LuaPCallK = 0x00123456; 
    constexpr uint64_t LuaCallK = 0x00789ABC;  

    // Thread Identity (Crucial for bypassing detection)
    constexpr uint64_t ThreadIdentityOffset = 0x1C0; 
    constexpr uint64_t TrustedIdentity = 0x303FFFFFFFFFFFFF; 

    // Script Context
    constexpr uint64_t ScriptContext = 0x3F0;
}
