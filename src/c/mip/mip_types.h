#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <microstrain/embedded_time.h>

typedef microstrain_embedded_timestamp mip_timestamp;
typedef microstrain_embedded_timestamp mip_timeout;


#ifdef MIP_ENABLE_DIAGNOSTICS

// Saturating addition
#define MIP_DIAG_INC(counter, amount) do { if (counter + amount < counter) counter = -1; else counter += amount; } while(false)

#define MIP_DIAG_ZERO(counter) counter = 0

#else // MIP_ENABLE_DIAGNOSTICS

// Do nothing if diagnostic counters diabled. Cast amount to void to avoid "unused local variable" warnings.
#define MIP_DIAG_INC(counter, amount) (void)amount

#define MIP_DIAG_ZERO(counter) (void)0

#endif // MIP_ENABLE_DIAGNOSTICS
