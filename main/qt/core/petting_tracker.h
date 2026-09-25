#pragma once
#include <cmath>
#include <cstdint>

namespace slqt::detail {
class PettingTracker {
public:
    void reset() noexcept { *this = PettingTracker{}; }
    bool update(bool onHead, bool dragging, float screenX, float screenY, std::uint64_t nowMs) noexcept {
        if (!std::isfinite(screenX) || !std::isfinite(screenY)) { tracking = false; distance = 0; return false; }
        if (nowMs < lastTick) reset();
        lastTick = nowMs;
        if (!onHead || dragging) { tracking = false; distance = 0; return false; }
        const float moved = tracking ? std::fabs(screenX - lastX) + std::fabs(screenY - lastY) : 0;
        distance += moved;
        tracking = true; lastX = screenX; lastY = screenY;
        if (moved > 0 && distance >= 500 && nowMs >= cooldownUntil) {
            distance = 0;
            cooldownUntil = nowMs + 4000;
            return true;
        }
        return false;
    }
private:
    bool tracking = false;
    float distance = 0, lastX = 0, lastY = 0;
    std::uint64_t cooldownUntil = 0, lastTick = 0;
};
}
