#ifndef minimap
#define minimap

#include "structs.h"
#include <raylib.h>

struct Minimap {
  Vector2 position;
  Vector2 size;
  Color background_color;
  Color room_color;
  Vector2 *relative_room_positions;
  int num_rooms;
  Vector2 current_position;
};

struct MinimapSettings {
  Vector2 position;
  Vector2 size;
  Color background_color;
  Color room_color;
};

void initMinimap(struct Minimap *mm, struct MinimapSettings settings,
                 struct Stage *stage);

void drawMinimap(struct Minimap *mm, int current_room_id);

#endif
