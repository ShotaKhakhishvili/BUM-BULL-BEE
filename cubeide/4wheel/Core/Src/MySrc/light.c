#include "MySrc/light.h"

#include "MySrc/defines.h"
#include "MySrc/platform_adapter.h"

void Light_Init(Light *self, uint8_t input_id, bool rev)
{
    if (self == 0)
    {
        return;
    }

    self->input_id = input_id;
    self->last_col = BBB_BLACK;
    self->last_refresh = 0U;
    self->rev = rev;
    self->last_white = 0U;
}

bool Light_GetCol(Light *self)
{
    bool was_white;
    uint32_t now;

    if (self == 0)
    {
        return BBB_BLACK;
    }

    now = Platform_Millis();
    was_white = (self->last_col == BBB_WHITE);

    if ((now - self->last_refresh) > BBB_LIGHT_REFRESH_MS)
    {
        self->last_col = Platform_ReadDigitalInput(self->input_id);

        if (self->rev)
        {
            self->last_col = !self->last_col;
        }

        self->last_refresh = now;
    }

    if ((!was_white) && (self->last_col == BBB_WHITE))
    {
        self->last_white = now;
    }

    return self->last_col;
}
