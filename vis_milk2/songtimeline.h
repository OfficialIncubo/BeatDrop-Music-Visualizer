#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

// SMTC positions describe a timestamped snapshot, not the poll instant.
// Keep this clock independent of WinRT so stale/seek sequences can be tested.
class SongTimelineClock {
public:
    void Reset() { *this = SongTimelineClock{}; }

    void Update(int64_t position, int64_t duration, int64_t updatedUtc,
        int64_t nowUtc, int64_t nowSteady, bool playing, double rate)
    {
        if (position < 0 || duration <= 0) {
            Reset();
            return;
        }
        if (!std::isfinite(rate) || rate < 0.0 || rate > 16.0)
            rate = 1.0;

        const bool initialized = duration_ > 0;
        // An out-of-order snapshot cannot undo a newer seek.
        if (initialized && updatedUtc > 0 && updatedUtc < updatedUtc_)
            return;

        const bool newSnapshot = !initialized || updatedUtc != updatedUtc_ ||
            position != reportedPosition_ || duration != duration_;
        const bool stateChanged = playing != playing_ || rate != rate_;
        const double predicted = PositionUnclamped(nowSteady);
        double corrected = static_cast<double>(position);
        if (playing && updatedUtc > 0 && updatedUtc <= nowUtc)
            corrected += static_cast<double>(nowUtc - updatedUtc) * rate;
        corrected = (std::min)(corrected, static_cast<double>(duration));

        if (newSnapshot) {
            // Ignore tiny timestamp/rounding noise during ordinary progress,
            // but accept a changed backward position even for a small seek.
            const bool ordinaryProgress = initialized && !stateChanged &&
                position >= reportedPosition_ && duration == duration_;
            anchorPosition_ = ordinaryProgress && std::abs(corrected - predicted) < 100.0
                ? predicted : corrected;
            anchorSteady_ = nowSteady;
        }
        else if (stateChanged) {
            // Playback status can arrive before the next timeline snapshot.
            // Freeze/resume the extrapolated clock without replaying old PCM time.
            anchorPosition_ = predicted;
            anchorSteady_ = nowSteady;
        }
        reportedPosition_ = position;
        duration_ = duration;
        updatedUtc_ = updatedUtc;
        playing_ = playing;
        rate_ = rate;
    }

    int64_t Position(int64_t nowSteady) const
    {
        if (duration_ <= 0) return -1;
        return static_cast<int64_t>((std::max)(0.0,
            (std::min)(PositionUnclamped(nowSteady), static_cast<double>(duration_))));
    }

    int64_t Duration() const { return duration_; }

private:
    double PositionUnclamped(int64_t nowSteady) const
    {
        return anchorPosition_ + (playing_
            ? static_cast<double>((std::max)(int64_t{0}, nowSteady - anchorSteady_)) * rate_
            : 0.0);
    }

    int64_t reportedPosition_ = -1;
    int64_t duration_ = -1;
    int64_t updatedUtc_ = 0;
    int64_t anchorSteady_ = 0;
    double anchorPosition_ = 0.0;
    double rate_ = 1.0;
    bool playing_ = false;
};
