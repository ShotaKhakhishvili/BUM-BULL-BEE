#ifndef MYSRC_SHARP_SUGGEST_H
#define MYSRC_SHARP_SUGGEST_H

#include <stdbool.h>

typedef struct
{
    int values[6];
    int count;
    int direction;
    int anomaly_score;
} SharpTrendState;

typedef struct
{
    SharpTrendState long_state;
    SharpTrendState short_state;
} SharpSuggest;

void SharpSuggest_Init(SharpSuggest *self);
void SharpSuggest_Reset(SharpSuggest *self);
void SharpSuggest_Update(SharpSuggest *self, int raw_long_adc, int raw_short_adc);

bool SharpSuggest_ShouldSuggestShortFromLong(const SharpSuggest *self);
bool SharpSuggest_ShouldSuggestLongFromShort(const SharpSuggest *self);

int SharpSuggest_GetLongAnomalyScore(const SharpSuggest *self);
int SharpSuggest_GetShortAnomalyScore(const SharpSuggest *self);

#endif /* MYSRC_SHARP_SUGGEST_H */
