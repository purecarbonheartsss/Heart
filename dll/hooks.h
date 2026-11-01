#pragma once
#include <string>
#include <vector>

namespace Hooks {
    bool Initialize();
    void QueueScript(const std::string& script);
}
