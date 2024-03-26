#include "debugUtil.h"
#include "definitions.h"
#include <raylib.h>
#include <raymath.h>

void visualizePlayer(struct Game *game, struct Player *player,
                     struct Settings *settings, struct Room *stage) {
  if (settings->use_function) {
    for (int i = 0; i < player->width; i++) {

      float player_x_i = player->position.x + i;
      float hit_position_local =
          player_x_i - (player->position.x + (player->width / 2.0));

      float hit_position_normalized =
          hit_position_local / (player->width / 2.0);

      float hit_scaled = hit_position_normalized * settings->player_slope_scale;
      float x = hit_position_normalized;

      float m = -1 / (4.0 * settings->a * powf(x, 3));

      Vector2 normal = Vector2Normalize((Vector2){hit_scaled, m});
      normal.x = hit_scaled;

      float normal_scale = 150;
      Vector2 origin = {player_x_i, player->position.y};
      Vector2 end = {origin.x - normal.x * normal_scale,
                     origin.y - normal_scale};

      DrawLine(origin.x + game->play_area.x, origin.y + game->play_area.y,
               end.x + game->play_area.x, end.y + game->play_area.y,
               NORMAL_COLOR);
    }

  } else {
    for (int i = 0; i < player->width; i += 2) {

      float player_x_i = player->position.x + i;
      float hit_offset_x =
          -((player->position.x + (player->width / 2.0)) - player_x_i);

      float d = settings->radius + player->position.y;
      float theta = hit_offset_x / settings->radius;

      Vector2 projected = {hit_offset_x + d * sin(theta),
                           player->position.y + cos(theta)};

      Vector2 normal =
          Vector2Normalize(Vector2Subtract((Vector2){0, 0}, projected));

      float normal_scale = 150;
      Vector2 origin = {player_x_i, player->position.y};

      origin = Vector2Add(origin, stage->position_world);
      origin.x += game->play_area.x;
      origin.y += game->play_area.y;

      Vector2 end = {origin.x - normal.x * normal_scale,
                     origin.y - normal_scale};

      DrawLine(origin.x, origin.y, end.x, end.y, NORMAL_COLOR);
    }
  }
}

void visualizeBoxes(struct Game *game, struct Settings *settings,
                    struct Room *room) {

  DrawRectangle(room->kill_box.x + game->play_area.x + room->position_world.x,
                room->kill_box.y + game->play_area.y + room->position_world.y,
                room->kill_box.width, room->kill_box.height,
                (Color){255, 0, 0, 100});
}

void visualizeBricks(struct Game *game, struct Settings *settings,
                     struct Room *stage) {
  for (int i = 0; i < stage->brick_count; i++) {
    Color transparent = stage->bricks[i].color;
    transparent.a = 50;

    Vector2 pos =
        Vector2Add(stage->bricks[i].position_room, stage->position_world);
    pos.x += game->play_area.x;
    pos.y += game->play_area.y;

    DrawRectangle(pos.x, pos.y, stage->brick_width, stage->brick_height,
                  transparent);
  }
}

void visualizeBall(struct Game *game, struct Ball *ball, struct Room *room) {

  float scale = 150;
  Vector2 origin = (Vector2){ball->position.x + ball->size / 2.0,
                             ball->position.y + ball->size / 2.0};

  origin = Vector2Add(origin, room->position_world);
  origin.x += game->play_area.x;
  origin.y += game->play_area.y;

  Vector2 end = {origin.x + ball->direction.x * scale,
                 origin.y + ball->direction.y * scale};

  DrawLine(origin.x, origin.y, end.x, end.y, NORMAL_COLOR);
}

void visualizeRoom(struct Game *game) {
  DrawText(TextFormat("CurrentRoom: %d", game->current_stage->current_room->id),
           game->ui_area.x + game->ui_area.width - 300,
           game->ui_area.y + game->ui_area.height / 2 - 10, 20, WHITE);
}
