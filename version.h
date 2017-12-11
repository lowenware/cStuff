#ifndef _AGENT_VERSION_H_
#define _AGENT_VERSION_H_

#include <stdint.h>

typedef struct 
{

  uint16_t major;
  uint16_t minor;
  uint16_t tweak;
  uint16_t build;

} version_t;

#endif
