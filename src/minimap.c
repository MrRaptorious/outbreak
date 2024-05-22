#include "minimap.h"
#include <assert.h>
#include <raylib.h>
#include <raymath.h>

void initMinimap(struct Minimap *mm, struct MinimapSettings settings,
                 struct Stage *stage) {
  mm->background_color = settings.background_color;
  mm->room_color = settings.room_color;
  mm->position = settings.position;
  mm->size = settings.size;
  mm->num_rooms = stage->num_rooms;
  mm->texture = LoadRenderTexture(mm->size.x * 4, mm->size.y * 4);
  mm->layout = stage->room_layout;

  for (int i = 0; i < stage->num_rooms; i++) {
    mm->relative_room_positions[stage->rooms[i].id] =
        (Vector2){stage->rooms[i].position_world.x / 1920,
                  stage->rooms[i].position_world.y / 1080};
  }
}

void drawMinimap(struct Minimap *mm, int current_room_id) {
  int mm_room_width = 20;
  int mm_room_width_with_offset = 24;
  Vector2 current_room_offset = (Vector2){0, 0};

  BeginTextureMode(mm->texture);
  ClearBackground(mm->background_color);

  // draw relative to red box
  for (int i = 0; i < mm->num_rooms; i++) {
    // draw rooms
    Vector2 mm_room = mm->relative_room_positions[i];

    Vector2 pos = (Vector2){(mm_room.x) * mm_room_width_with_offset,
                            (mm_room.y) * mm_room_width_with_offset};
    pos.x += mm->size.x / 2.0;
    pos.y += mm->size.y / 2.0;

    // color
    Color c = mm->room_color;

    if (i == current_room_id) {
      c = RED;
      current_room_offset = (Vector2){(mm_room.x) * mm_room_width_with_offset,
                                      (mm_room.y) * -mm_room_width_with_offset};
    }

    DrawRectangle(pos.x, pos.y, mm_room_width, mm_room_width, c);

    // draw transitions
    for (int j = 0; j < mm->num_rooms; j++) {
      enum Direction direction = mm->layout[i * mm->num_rooms + j];

      Vector2 start;
      Vector2 end;

      switch (direction) {
      case LEFT:
      case TOP:
      case RIGHT:
      case BOTTOM:
        start = (Vector2){(mm_room.x) * mm_room_width_with_offset,
                          (mm_room.y) * 23};
        start.x += mm->size.x / 2.0;
        start.y += mm->size.y / 2.0;

        start.x += mm_room_width_with_offset / 2;
        start.y += mm_room_width_with_offset / 2;

        end = (Vector2){
            (mm->relative_room_positions[j].x) * mm_room_width_with_offset,
            (mm->relative_room_positions[j].y) * mm_room_width_with_offset};
        end.x += mm->size.x / 2.0;
        end.y += mm->size.y / 2.0;

        end.x += mm_room_width_with_offset / 2;
        end.y += mm_room_width_with_offset / 2;

        DrawLineEx(start, end, 5, PINK);

      case DIRECTION_NONE:
        break;
      }
    }
  }

  EndTextureMode();

  DrawTextureRec(mm->texture.texture,
                 (Rectangle){current_room_offset.x, current_room_offset.y,
                             (float)mm->texture.texture.width,
                             (float)-mm->texture.texture.height},
                 mm->position, WHITE);

  // create texture before loop !
}
