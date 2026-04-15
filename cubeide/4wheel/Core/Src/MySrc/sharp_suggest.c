#include "MySrc/sharp_suggest.h"

#include <stdlib.h>

#define SHARP_SUGGEST_MAX_SAMPLES 6
#define SHARP_SUGGEST_NOISE_THRESHOLD 2
#define SHARP_SUGGEST_MIN_RUN_LENGTH_LONG 4
#define SHARP_SUGGEST_MIN_RUN_LENGTH_SHORT 4
#define SHARP_SUGGEST_AVG_DELTA_THRESHOLD_LONG 45
#define SHARP_SUGGEST_AVG_DELTA_THRESHOLD_SHORT 60
#define SHARP_SUGGEST_ANOMALY_TRIGGER_LONG 3
#define SHARP_SUGGEST_ANOMALY_TRIGGER_SHORT 3

static int SharpSuggest_Sign(int x)
{
    if (x > 0)
    {
        return 1;
    }

    if (x < 0)
    {
        return -1;
    }

    return 0;
}

static void SharpSuggest_ClearTrend(SharpTrendState *state)
{
    if (state == 0)
    {
        return;
    }

    state->count = 0;
    state->direction = 0;
}

static void SharpSuggest_ResetState(SharpTrendState *state)
{
    int i;

    if (state == 0)
    {
        return;
    }

    SharpSuggest_ClearTrend(state);
    state->anomaly_score = 0;

    for (i = 0; i < SHARP_SUGGEST_MAX_SAMPLES; ++i)
    {
        state->values[i] = 0;
    }
}

static void SharpSuggest_PushValue(SharpTrendState *state, int value)
{
    int i;

    if (state == 0)
    {
        return;
    }

    if (state->count < SHARP_SUGGEST_MAX_SAMPLES)
    {
        state->values[state->count] = value;
        ++state->count;
        return;
    }

    for (i = 1; i < SHARP_SUGGEST_MAX_SAMPLES; ++i)
    {
        state->values[i - 1] = state->values[i];
    }

    state->values[SHARP_SUGGEST_MAX_SAMPLES - 1] = value;
}

static void SharpSuggest_RestartWithLastTwo(SharpTrendState *state, int previous_value, int new_value, int new_direction)
{
    if (state == 0)
    {
        return;
    }

    state->values[0] = previous_value;
    state->values[1] = new_value;
    state->count = 2;
    state->direction = new_direction;
}

static void SharpSuggest_UpdateTrend(SharpTrendState *state, int new_value)
{
    int last_value;
    int diff;
    int new_direction;

    if (state == 0)
    {
        return;
    }

    if (state->count == 0)
    {
        SharpSuggest_PushValue(state, new_value);
        return;
    }

    last_value = state->values[state->count - 1];
    diff = new_value - last_value;

    if (abs(diff) < SHARP_SUGGEST_NOISE_THRESHOLD)
    {
        return;
    }

    new_direction = SharpSuggest_Sign(diff);

    if (state->count == 1)
    {
        SharpSuggest_PushValue(state, new_value);
        state->direction = new_direction;
        return;
    }

    if (state->direction == 0)
    {
        state->direction = new_direction;
        SharpSuggest_PushValue(state, new_value);
        return;
    }

    if (new_direction == state->direction)
    {
        SharpSuggest_PushValue(state, new_value);
        return;
    }

    SharpSuggest_RestartWithLastTwo(state, last_value, new_value, new_direction);
}

static int SharpSuggest_ComputeAverageAbsDelta(const SharpTrendState *state)
{
    int i;
    int sum;

    if ((state == 0) || (state->count < 2))
    {
        return 0;
    }

    sum = 0;

    for (i = 1; i < state->count; ++i)
    {
        sum += abs(state->values[i] - state->values[i - 1]);
    }

    return sum / (state->count - 1);
}

static bool SharpSuggest_IsAnomalousRun(const SharpTrendState *state, int min_run_length, int avg_delta_threshold)
{
    int avg_abs_delta;

    if ((state == 0) || (state->count < min_run_length))
    {
        return false;
    }

    avg_abs_delta = SharpSuggest_ComputeAverageAbsDelta(state);
    return (avg_abs_delta >= avg_delta_threshold);
}

static void SharpSuggest_UpdateScore(int *score, bool anomalous)
{
    if (score == 0)
    {
        return;
    }

    if (anomalous)
    {
        if (*score < 255)
        {
            ++(*score);
        }
    }
    else if (*score > 0)
    {
        --(*score);
    }
}

void SharpSuggest_Init(SharpSuggest *self)
{
    SharpSuggest_Reset(self);
}

void SharpSuggest_Reset(SharpSuggest *self)
{
    if (self == 0)
    {
        return;
    }

    SharpSuggest_ResetState(&self->long_state);
    SharpSuggest_ResetState(&self->short_state);
}

void SharpSuggest_Update(SharpSuggest *self, int raw_long_adc, int raw_short_adc)
{
    bool long_anomalous;
    bool short_anomalous;

    if (self == 0)
    {
        return;
    }

    SharpSuggest_UpdateTrend(&self->long_state, raw_long_adc);
    SharpSuggest_UpdateTrend(&self->short_state, raw_short_adc);

    long_anomalous = SharpSuggest_IsAnomalousRun(
        &self->long_state,
        SHARP_SUGGEST_MIN_RUN_LENGTH_LONG,
        SHARP_SUGGEST_AVG_DELTA_THRESHOLD_LONG);

    short_anomalous = SharpSuggest_IsAnomalousRun(
        &self->short_state,
        SHARP_SUGGEST_MIN_RUN_LENGTH_SHORT,
        SHARP_SUGGEST_AVG_DELTA_THRESHOLD_SHORT);

    SharpSuggest_UpdateScore(&self->long_state.anomaly_score, long_anomalous);
    SharpSuggest_UpdateScore(&self->short_state.anomaly_score, short_anomalous);
}

bool SharpSuggest_ShouldSuggestShortFromLong(const SharpSuggest *self)
{
    if (self == 0)
    {
        return false;
    }

    return (self->long_state.anomaly_score >= SHARP_SUGGEST_ANOMALY_TRIGGER_LONG);
}

bool SharpSuggest_ShouldSuggestLongFromShort(const SharpSuggest *self)
{
    if (self == 0)
    {
        return false;
    }

    return (self->short_state.anomaly_score >= SHARP_SUGGEST_ANOMALY_TRIGGER_SHORT);
}

int SharpSuggest_GetLongAnomalyScore(const SharpSuggest *self)
{
    if (self == 0)
    {
        return 0;
    }

    return self->long_state.anomaly_score;
}

int SharpSuggest_GetShortAnomalyScore(const SharpSuggest *self)
{
    if (self == 0)
    {
        return 0;
    }

    return self->short_state.anomaly_score;
}
