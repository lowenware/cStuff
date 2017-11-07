#ifndef _AGENT_VERSION_H_
#define _AGENT_VERSION_H_

#include <stdint.h>

#define VERSION_RELEASE 0
#define VERSION_RC      1
#define VERSION_BETHA   2
#define VERSION_ALPHA   3
#define VERSION_PRE     4

typedef struct {
  uint16_t major;
  uint16_t minor;
  uint16_t tweak;
  uint16_t cycle;
} version_t;


#endif
