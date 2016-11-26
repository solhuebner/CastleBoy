#ifndef __MAP_H__
#define __MAP_H__

#include "global.h"

namespace Map
{
void init();
bool collide(int16_t x, int16_t y, int8_t w, int8_t h);
void draw();
int16_t width();
}

#endif


