#include "minimap.h"
#include <raylib.h>
#include <raymath.h>

void initMinimap(struct Minimap *mm, struct MinimapSettings settings,
                 struct Stage *stage) {
  mm->background_color = settings.background_color;
  mm->room_color = settings.room_color;
  mm->position = settings.position;
  mm->size = settings.size;
  mm->num_rooms = stage->num_rooms;

  for (int i = 0; i < stage->num_rooms; i++) {
    //    mm->relative_room_positions[stage->rooms[i].id] =
    //        (Vector2){stage->rooms[i].position_world.x / stage->room_size.x,
    //                  stage->rooms[i].position_world.y / stage->room_size.y};
    //
    mm->relative_room_positions[stage->rooms[i].id] =
        (Vector2){stage->rooms[i].position_world.x / 1920,
                  stage->rooms[i].position_world.y / 1080};
  }
}

void drawMinimap(struct Minimap *mm, int current_room_id) {
  RenderTexture2D target = LoadRenderTexture(mm->size.x, mm->size.y);
  BeginTextureMode(target);
  ClearBackground(mm->background_color);

  for (int i = 0; i < mm->num_rooms; i++) {
    Vector2 mm_room = mm->relative_room_positions[i];
    Color c = mm->room_color;

    if (i == current_room_id) {
      c = RED;
    }

    Vector2 pos = (Vector2){(mm_room.x) * 20, (mm_room.y) * 20};
    pos.x += mm->size.x / 2.0;
    pos.y += mm->size.y / 2.0;

    DrawRectangle(pos.x, pos.y, 20, 20, c);
  }

  EndTextureMode();

  DrawTextureRec(target.texture,
                 (Rectangle){0, 0, (float)target.texture.width,
                             (float)-target.texture.height},
                 mm->position, WHITE);

  // unload texture!!
  // create texture before loop !
}
