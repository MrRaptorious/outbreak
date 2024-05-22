#include "debugUtil.h"
#include "definitions.h"
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

void visualizePlayer(struct Game *game, struct Player *player,
                     struct Room *stage) {

  struct Settings *settings = game->settings;
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

void visualizeBoxes(struct Game *game, struct Room *room) {

  struct Settings *settings = game->settings;
  Vector2 kill_box_pos = (Vector2){room->kill_box.x, room->kill_box.y};

  Vector2 pos = Vector2Add(kill_box_pos, room->position_world);
  pos.x += game->play_area.x;
  pos.y += game->play_area.y;

  DrawRectangle(pos.x, pos.y, room->kill_box.width, room->kill_box.height,
                (Color){255, 0, 0, 100});
}

void visualizeBricks(struct Game *game, struct Room *room) {
  for (int i = 0; i < room->brick_count; i++) {
    Color transparent = room->bricks[i].color;
    transparent.a = 50;

    Vector2 pos = Vector2Add(room->bricks[i].position_room,
                             game->current_stage->current_room->position_world);
    pos.x += game->play_area.x;
    pos.y += game->play_area.y;

    DrawRectangle(pos.x, pos.y, room->brick_width, room->brick_height,
                  transparent);
  }
}

void visualizeBall(struct Game *game, struct Ball *ball, struct Room *room) {

  float scale = 150;
  Vector2 origin = (Vector2){ball->position.x + ball->size / 2.0,
                             ball->position.y + ball->size / 2.0};

  origin =
      Vector2Add(origin, game->current_stage->current_room->position_world);
  origin.x += game->play_area.x;
  origin.y += game->play_area.y;

  Vector2 end = {origin.x + ball->direction.x * scale,
                 origin.y + ball->direction.y * scale};

  DrawLine(origin.x, origin.y, end.x, end.y, NORMAL_COLOR);
}

void visualizeRoom(struct Game *game) {
  DrawText(TextFormat("CurrentRoom: %d (x:%.2f,y:%.2f)",
                      game->current_stage->current_room->id,
                      game->current_stage->current_room->position_world.x,
                      game->current_stage->current_room->position_world.y),
           game->ui_area.x + game->ui_area.width - 600,
           game->ui_area.y + game->ui_area.height / 2 - 10, 20, WHITE);
}
