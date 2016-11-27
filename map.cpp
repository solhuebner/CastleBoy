#include "map.h"

#include "assets.h"
#include "candle.h"

namespace
{
uint8_t data[MAP_WIDTH_MAX * MAP_HEIGHT_MAX];
int16_t mapY;
int16_t mapWidth;
int16_t mapHeight;
}

void Map::init(const uint8_t* source)
{
  mapWidth = pgm_read_byte(source);
  mapHeight = pgm_read_byte(++source);
  mapY = 64 - mapHeight * TILE_HEIGHT;
  cameraX = 0;

  // FIXME currently map is read in two pass:
  // first pass we read PGM by row
  // second pass we decided tiles by column
  // once maps are generated by a tool we could directly encode them with correct orientation
  for (int16_t iy = 0; iy < mapHeight; iy++)
  {
    for (int16_t ix = 0; ix < mapWidth; ix++)
    {
      data[iy * mapWidth + ix] = pgm_read_byte(++source);
    }
  }

  for (int16_t ix = 0; ix < mapWidth; ix++)
  {
    bool isGround = false;
    bool isBlock = false;
    for (int16_t iy = 0; iy < mapHeight; iy++)
    {
      switch (data[iy * mapWidth + ix])
      {
        case MAP_DATA_CANDLE:
          Candles::add(ix * TILE_WIDTH, mapY + iy * TILE_HEIGHT);
        case MAP_DATA_EMPTY:
          if (isBlock)
          {
            isBlock = false;
            data[iy * mapWidth + ix] = 4;
          }
          else
          {
            data[iy * mapWidth + ix] = 0;
          }

          isGround = false;
          break;
        case MAP_DATA_BLOCK:
          data[iy * mapWidth + ix] = 1;
          isBlock = true;
          isGround = false;
          break;
        case MAP_DATA_GROUND:
          if (isGround)
          {
            // already in ground, use inner ground tile
            data[iy * mapWidth + ix] = 2;
          }
          else
          {
            // first ground tile
            data[iy * mapWidth + ix] = 3;
            isGround = true;
          }
          isBlock = false;
          break;
      }
    }
  }
}

int16_t Map::width()
{
  return mapWidth;
}

bool Map::collide(int16_t x, int16_t y, const Rect& hitbox)
{
  x -= hitbox.x;
  y -= hitbox.y;
  
  if (x < 0 || x + hitbox.width > mapWidth * TILE_WIDTH)
  {
    // cannot get out on the sides, collide
    //LOG_DEBUG("side");
    return true;
  }

  y -= mapY; // compensate map offset

  int16_t tx1 = x / TILE_WIDTH;
  int16_t ty1 = y / TILE_HEIGHT;
  int16_t tx2 = (x + hitbox.width - 1) / TILE_WIDTH;
  int16_t ty2 = (y + hitbox.height - 1) / TILE_HEIGHT;

  if (ty2 < 0 || ty2 >= mapHeight)
  {
    // either higher or lower than map, no collision
    //LOG_DEBUG("higher or lower");
    return false;
  }

  // clamp positions
  if (tx1 < 0) tx1 = 0;
  if (tx2 >= mapWidth) tx2 = mapWidth - 1;
  if (ty1 < 0) ty1 = 0;
  if (ty2 >= mapHeight) ty2 = mapHeight - 1;

  for (int16_t ix = tx1; ix <= tx2; ix++)
  {
    for (int16_t iy = ty1; iy <= ty2; iy++)
    {
      // FIXME
      if (data[iy * mapWidth + ix] > 0 && data[iy * mapWidth + ix] <= SOLID_TILE_COUNT)
      {
        // check for rectangle intersection
        if (ix * TILE_WIDTH + TILE_WIDTH > x && iy * TILE_HEIGHT + TILE_HEIGHT > y && ix * TILE_WIDTH < x + hitbox.width && iy * TILE_HEIGHT < y + hitbox.height)
        {
          //LOG_DEBUG("hit");
          return true;
        }
      }
    }
  }

  return false;
}

void Map::draw()
{
  // TODO dont draw tiles out of screen
  for (int8_t ix = 0; ix < mapWidth; ix++)
  {
    for (int8_t iy = 0; iy < mapHeight; iy++)
    {
      uint8_t tile = data[iy * mapWidth + ix];
      if (tile > 0)
      {
        sprites.drawOverwrite(ix * TILE_WIDTH - cameraX, mapY + iy * TILE_HEIGHT, tileset, tile - 1);
      }
    }
  }

  LOG_DEBUG(mapWidth);
}

