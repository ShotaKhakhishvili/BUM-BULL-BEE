#include "SharpSuggest.hpp"

#include <Arduino.h>
#include <stdlib.h>

namespace SharpSuggest
{
    struct TrendState
    {
        static constexpr int kMaxSamples = 6;

        int values[kMaxSamples] = {0};
        int count = 0;

        // 0 = unknown, +1 = increasing, -1 = decreasing
        int direction = 0;

        int anomalyScore = 0;
    };

    static TrendState longState;
    static TrendState shortState;

    // Tune on hardware
    static constexpr int kNoiseThreshold = 2;

    static constexpr int kMinRunLengthLong  = 4;
    static constexpr int kMinRunLengthShort = 4;

    static constexpr int kAvgDeltaThresholdLong  = 45;
    static constexpr int kAvgDeltaThresholdShort = 60;

    static constexpr int kAnomalyTriggerLong  = 3;
    static constexpr int kAnomalyTriggerShort = 3;

    static void ClearTrend(TrendState& state)
    {
        state.count = 0;
        state.direction = 0;
    }

    static void ResetState(TrendState& state)
    {
        ClearTrend(state);
        state.anomalyScore = 0;

        for (int i = 0; i < TrendState::kMaxSamples; ++i)
            state.values[i] = 0;
    }

    static int Sign(int x)
    {
        if (x > 0) return 1;
        if (x < 0) return -1;
        return 0;
    }

    static void PushValue(TrendState& state, int value)
    {
        if (state.count < TrendState::kMaxSamples)
        {
            state.values[state.count] = value;
            ++state.count;
        }
        else
        {
            for (int i = 1; i < TrendState::kMaxSamples; ++i)
                state.values[i - 1] = state.values[i];

            state.values[TrendState::kMaxSamples - 1] = value;
        }
    }

    static void RestartWithLastTwo(TrendState& state, int previousValue, int newValue, int newDirection)
    {
        state.values[0] = previousValue;
        state.values[1] = newValue;
        state.count = 2;
        state.direction = newDirection;
    }

    static void UpdateTrend(TrendState& state, int newValue)
    {
        if (state.count == 0)
        {
            PushValue(state, newValue);
            return;
        }

        const int lastValue = state.values[state.count - 1];
        const int diff = newValue - lastValue;

        if (abs(diff) < kNoiseThreshold)
            return;

        const int newDirection = Sign(diff);

        if (state.count == 1)
        {
            PushValue(state, newValue);
            state.direction = newDirection;
            return;
        }

        if (state.direction == 0)
        {
            state.direction = newDirection;
            PushValue(state, newValue);
            return;
        }

        if (newDirection == state.direction)
        {
            PushValue(state, newValue);
            return;
        }

        RestartWithLastTwo(state, lastValue, newValue, newDirection);
    }

    static int ComputeAverageAbsDelta(const TrendState& state)
    {
        if (state.count < 2)
            return 0;

        int sum = 0;

        for (int i = 1; i < state.count; ++i)
            sum += abs(state.values[i] - state.values[i - 1]);

        return sum / (state.count - 1);
    }

    static bool IsAnomalousRun(const TrendState& state, int minRunLength, int avgDeltaThreshold)
    {
        if (state.count < minRunLength)
            return false;

        const int avgAbsDelta = ComputeAverageAbsDelta(state);
        return avgAbsDelta >= avgDeltaThreshold;
    }

    static void UpdateScore(int& score, bool anomalous)
    {
        if (anomalous)
        {
            if (score < 255)
                ++score;
        }
        else
        {
            if (score > 0)
                --score;
        }
    }

    void Init()
    {
        Reset();
    }

    void Reset()
    {
        ResetState(longState);
        ResetState(shortState);
    }

    void Update(SharpMode currentMode, int rawLongAdc, int rawShortAdc)
    {
        UpdateTrend(longState, rawLongAdc);
        UpdateTrend(shortState, rawShortAdc);

        const bool longAnomalous =
            IsAnomalousRun(longState, kMinRunLengthLong, kAvgDeltaThresholdLong);

        const bool shortAnomalous =
            IsAnomalousRun(shortState, kMinRunLengthShort, kAvgDeltaThresholdShort);

        UpdateScore(longState.anomalyScore, longAnomalous);
        UpdateScore(shortState.anomalyScore, shortAnomalous);
    }

    bool ShouldSuggestShortFromLong()
    {
        return longState.anomalyScore >= kAnomalyTriggerLong;
    }

    bool ShouldSuggestLongFromShort()
    {
        return shortState.anomalyScore >= kAnomalyTriggerShort;
    }

    int GetLongAnomalyScore()
    {
        return longState.anomalyScore;
    }

    int GetShortAnomalyScore()
    {
        return shortState.anomalyScore;
    }
}