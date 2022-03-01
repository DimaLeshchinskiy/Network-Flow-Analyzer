#include "util.h"

namespace util{
    uint32_t now() {
        using namespace std::chrono;
        return static_cast<uint32_t>(duration_cast<milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    }
}