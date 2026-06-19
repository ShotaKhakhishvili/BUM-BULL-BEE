#ifndef MYSRC_DEFINES_H
#define MYSRC_DEFINES_H

#include <stdbool.h>
#include <stdint.h>

#define BBB_PRINT_DIGITS_AFTER_DECIMAL 3U

#define BBB_MODEL_SHORT 20150
#define BBB_MODEL_LONG 1080

#define BBB_MAX_DISTANCE_CM 65.0

/* Digital line-sensor levels as wired on this bot: black reads LOW (0), white
 * reads HIGH (1). */
#define BBB_WHITE true
#define BBB_BLACK false

#define BBB_LIGHT_REFRESH_MS 15U

#endif /* MYSRC_DEFINES_H */
