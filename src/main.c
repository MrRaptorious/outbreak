#include "debugUtil.h"
#include "definitions.h"
#include "raylib.h"
#include "raymath.h"
#include "roomgenerator.h"
#include "structs.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// TODO:
// - use delta time!
// - use shaders
// - portals

// TODO: NEXT:
// conditional rendering (source, target)

// ######################### Global vars ############################
struct Game *game;
// ######################### End Global vars ############################

// ############################ Util Functions ############################
void spawnBall(struct Ball *ball) {
  int render_width = game->play_area.width;
  int render_height = game->play_area.height;

  ball->size = 10;
  ball->color = BLACK;
  ball->speed = 5;
  ball->position = (Vector2){.x = render_width / 2.0, .y = render_height / 2.0};
  ball->direction = (Vector2){-.1, -1};
}
// ########################### End Util Functions ###########################

// ############################## Setup Functions ##############################
// ############################ End Setup Functions ############################

// ############################## Tick Functions ##############################
void doCollision(struct Ball *ball, struct Room *room, struct Player *player,
                 struct Settings *settings) {
  // go through each brick and check collision with the ball
  // for now use AABB Collision with Ball
  Rectangle ball_box = {
      .x = ball->position.x,
      .y = ball->position.y,
      .width = ball->size,
      .height = ball->size,
  };

  // death zone
  if (CheckCollisionRecs(room->kill_box, ball_box)) {
    settings->restart = true;
  }

  // bricks
  bool reflected_current_frame = false;
  for (int i = 0; i < room->brick_count; i++) {

    if (room->bricks[i].alive) {

      Rectangle brick_collision = {
          .x = room->bricks[i].position_room.x,
          .y = room->bricks[i].position_room.y,
          .width = room->brick_width,
          .height = room->brick_height,
      };

      // only check if brick is alive
      if (room->bricks[i].alive) {
        if (CheckCollisionRecs(brick_collision, ball_box)) {
          // kill brick
          room->bricks[i].alive = false;

          if (!reflected_current_frame) {
            // TODO: reflect based on hit position!
            Vector2 new_direction =
                Vector2Reflect(ball->direction, WALL_TOP_NORMAL);
            ball->direction.y = new_direction.y;
            ball->direction.x = new_direction.x;
            reflected_current_frame = true;
          }

          player->score += 10;
        }
      }
    }
  }

  // player
  Rectangle player_box = {
      .x = player->position.x,
      .y = player->position.y,
      .width = player->width,
      .height = player->height,
  };

  if (CheckCollisionRecs(player_box, ball_box)) {
    // P.x
    float hit_position_local = -((ball->position.x + (ball->size / 2.0)) -
                                 (player->position.x + (player->width / 2.0)));

    Vector2 normal = PLAYER_NORMAL;

    if (settings->use_function) {
      // P.x in interval -1..1
      float hit_position_normalized =
          hit_position_local / (player->width / 2.0);

      // calculate slope at x
      float hit_scaled = hit_position_normalized * settings->player_slope_scale;
      float x = hit_scaled;

      // get slope with first derivitive of ax^4
      float m = -1 / (4.0 * settings->a * powf(x, 3));

      normal = Vector2Normalize((Vector2){hit_scaled, m});
      normal.x = hit_position_normalized;

    } else {
      Vector2 ball_middle = (Vector2){ball->position.x + ball->size / 2.0,
                                      ball->position.y + ball->size / 2.0};

      float d = settings->radius + player->position.y;
      float theta = hit_position_local / settings->radius;

      Vector2 projected = {hit_position_local + d * sin(theta),
                           player->position.y + cos(theta)};

      normal = Vector2Normalize(projected);
    }

    Vector2 new_direction = Vector2Reflect(ball->direction, normal);
    ball->direction.y = new_direction.y;
    ball->direction.x = new_direction.x;

    // move ball up a little
    // TODO: little hack ?
    ball->position.y -= 2;
  }

  // WALLS
  int render_width = game->play_area.width;
  int render_height = game->play_area.height;

  if (ball->position.x <= 0) {
    ball->position.x = 0;
    Vector2 new_direction = Vector2Reflect(ball->direction, WALL_LEFT_NORMAL);
    ball->direction.y = new_direction.y;
    ball->direction.x = new_direction.x;
  }

  if (ball->position.x >= render_width - ball->size) {
    ball->position.x = render_width - ball->size;
    Vector2 new_direction = Vector2Reflect(ball->direction, WALL_RIGHT_NORMAL);
    ball->direction.y = new_direction.y;
    ball->direction.x = new_direction.x;
  }

  if (ball->position.y <= 0) {
    ball->position.y = 0;
    ball->direction.y = -ball->direction.y;
  }
}

void moveBall(struct Game *game, struct Ball *ball) {
  if (!game->player->is_holding_ball) {
    if (game->settings->mouse_mode) {
      Vector2 mouse_pos =
          GetScreenToWorld2D((Vector2){GetMouseX(), GetMouseY()}, game->camera);
      ball->position.x = mouse_pos.x;
      ball->position.y = mouse_pos.y - game->ui_area.height;
    } else {
      Vector2 new_pos = Vector2Add(ball->position,
                                   (Vector2){ball->direction.x * ball->speed,
                                             ball->direction.y * ball->speed});
      ball->position.x = new_pos.x;
      ball->position.y = new_pos.y;
    }
  } else {

    Vector2 new_pos =
        (Vector2){game->player->position.x + game->player->width / 2.0,
                  game->player->position.y - 50};

    ball->position.x = new_pos.x;
    ball->position.y = new_pos.y;
  }
}

void movePlayer(struct Player *player) {
  int render_width = game->play_area.width;
  // move left
  if (IsKeyDown(KEY_H) && player->position.x > 0) {
    player->position.x -= player->speed;
    // line up with edge
    if (player->position.x < 0) {
      player->position.x = 0;
    }
  }

  // move right
  if (IsKeyDown(KEY_L) && (player->position.x < render_width - player->width)) {
    player->position.x += player->speed;
    // line up with edge
    if (player->position.x > render_width - player->width) {
      player->position.x = render_width - player->width;
    }
  }
}

void handleDebugInputs(struct Settings *settings, struct Ball *ball,
                       struct Room *room, struct Player *player) {
  if (IsKeyDown(KEY_V)) {
    settings->radius += 1;
  }

  if (IsKeyDown(KEY_B)) {
    settings->radius -= 1;
  }

  if (IsKeyDown(KEY_R)) {
    settings->player_slope_scale += .01;
    printf("player_slope_scale:%f\n", settings->player_slope_scale);
  }

  if (IsKeyDown(KEY_T)) {
    settings->player_slope_scale -= .01;
    printf("player_slope_scale:%f\n", settings->player_slope_scale);
  }

  if (IsKeyDown(KEY_F)) {
    settings->a += .001;
    printf("a:%f\n", settings->a);
  }

  if (IsKeyDown(KEY_G)) {
    settings->a -= .001;
    printf("a:%f\n", settings->a);
  }
}

void handleControlls(struct Settings *settings, struct Player *player,
                     struct Ball *ball, struct Stage *stage) {
  int next_room_id = -1;

  if (IsKeyPressed(KEY_A)) {
    next_room_id = getRoom(stage, stage->current_room->id, LEFT);
    printf("L:current_room_id: %d\n", stage->current_room->id);
    printf("L:next_room_id: %d\n", next_room_id);
    printf("L:RoomLocation:x %f,y %f\n",
           stage->rooms[next_room_id].position_world.x,
           stage->rooms[next_room_id].position_world.y);
  } else if (IsKeyPressed(KEY_W)) {
    next_room_id = getRoom(stage, stage->current_room->id, TOP);
    printf("U:current_room_id: %d\n", stage->current_room->id);
    printf("U:next_room_id: %d\n", next_room_id);
    printf("U:RoomLocation:x %f,y %f\n",
           stage->rooms[next_room_id].position_world.x,
           stage->rooms[next_room_id].position_world.y);
  } else if (IsKeyPressed(KEY_D)) {
    next_room_id = getRoom(stage, stage->current_room->id, RIGHT);
    printf("R:current_room_id: %d\n", stage->current_room->id);
    printf("R:next_room_id: %d\n", next_room_id);
    printf("R:RoomLocation:x %f,y %f\n",
           stage->rooms[next_room_id].position_world.x,
           stage->rooms[next_room_id].position_world.y);
  } else if (IsKeyPressed(KEY_S)) {
    next_room_id = getRoom(stage, stage->current_room->id, BOTTOM);
    printf("B:current_room_id: %d\n", stage->current_room->id);
    printf("B:next_room_id: %d\n", next_room_id);
    printf("B:RoomLocation:x %f,y %f\n",
           stage->rooms[next_room_id].position_world.x,
           stage->rooms[next_room_id].position_world.y);
  }

  if (next_room_id >= 0) {
    stage->current_room = &stage->rooms[next_room_id];
    game->move_camera = true;
    game->player->is_holding_ball = true;

    if (ball->direction.y > 0) {
      ball->direction.y = -ball->direction.y;
    }
  }

  if (IsKeyPressed(KEY_SPACE)) {
    game->player->is_holding_ball = false;
  }

  if (IsKeyPressed(KEY_T)) {

    settings->debug_mode = !settings->debug_mode;
    if (settings->debug_mode) {
      printf("Debug-mode enabled\n");
    } else {
      printf("Debug-mode disabled\n");
    }
  }

  if (IsKeyPressed(KEY_P)) {
    printf("###### MATRIX #####!\n");
    int num_rooms = game->current_stage->num_rooms;
    printf("NUM_ROOMS:%d\n", num_rooms);

    // for (int source_room_index = 0; source_room_index < num_rooms;
    //      source_room_index++) {
    //   printf("%d\t", source_room_index);
    //   for (int target_room = 0; target_room < source_room_index;
    //        target_room++) {
    //     int search_offset =
    //         ((source_room_index * (source_room_index + 1)) / 2) -
    //         source_room_index;

    //    enum Direction direction =
    //        game->current_stage->room_layout[search_offset + target_room];

    //    switch (direction) {
    //    case DIRECTION_NONE:
    //      printf("0 ");
    //      break;
    //    case LEFT:
    //      printf("L ");
    //      break;
    //    case TOP:
    //      printf("T ");
    //      break;
    //    case RIGHT:
    //      printf("R ");
    //      break;
    //    case BOTTOM:
    //      printf("B ");
    //      break;
    //    }
    //  }

    //  printf("\n");
    //}

    for (int i = 0; i < num_rooms; i++) {
      for (int j = 0; j < num_rooms; j++) {
        int search_offset = i * num_rooms;

        enum Direction direction =
            game->current_stage->room_layout[search_offset + j];

        switch (direction) {
        case DIRECTION_NONE:
          printf("0 ");
          break;
        case LEFT:
          printf("L ");
          break;
        case TOP:
          printf("T ");
          break;
        case RIGHT:
          printf("R ");
          break;
        case BOTTOM:
          printf("B ");
          break;
        }
      }
      printf("\n");
    }
    printf("###### END #####!\n");
  }

  if (IsKeyPressed(KEY_N) || settings->restart) {
    settings->restart = false;
    printf("Respawn Player\n");

    player->score = fmax(player->score - 100, 0);

    spawnBall(ball);
    player->score = 0;
  }

  if (IsKeyPressed(KEY_M)) {
    settings->mouse_mode = !settings->mouse_mode;
  }

  if (IsKeyPressed(KEY_Q)) {
    settings->draw_fps = !settings->draw_fps;
    if (settings->draw_fps) {
      printf("Debug-mode enabled");
    } else {
      printf("Debug-mode disabled");
    }
  }
}
// ############################ End Tick Functions ############################

void init() {
  printf("Raylib Version: %s", RAYLIB_VERSION);

  InitWindow(game->window_size.x, game->window_size.y, "Outbreak");
  SetTargetFPS(game->target_fps);
}

void loop() {
  int render_width = game->play_area.width;
  int render_height = game->play_area.height;
  int num_rooms_to_render = 5;
  struct Room *rooms_to_render[5];

  struct Ball ball;
  spawnBall(&ball);

  struct StageSettings stage_settings = {
      .num_rooms = 5,
      .room_size = (Vector2){1920, 1080},
      .kill_box_position = (Vector2){0, 1080 - 20},
  };

  game->current_stage = generateStage(stage_settings);
  game->current_stage->current_room = &game->current_stage->rooms[0];

  // set startup room
  //  rooms_to_render[0] = &game->current_stage->rooms[0];

  rooms_to_render[0] = &game->current_stage->rooms[0];
  rooms_to_render[1] = &game->current_stage->rooms[1];
  rooms_to_render[2] = &game->current_stage->rooms[2];
  rooms_to_render[3] = &game->current_stage->rooms[3];
  rooms_to_render[4] = &game->current_stage->rooms[4];
  // shortcuts for structs
  struct Player *player = game->player;
  Camera2D *camera = &(game->camera);
  struct Stage *stage = game->current_stage;
  camera->target = game->current_stage->current_room->position_world;

  // camera targets
  struct Room *start = &game->current_stage->rooms[0];
  struct Room *end = &game->current_stage->rooms[0];

  // Gameloop
  while (!WindowShouldClose()) {
    struct Room *old_room = stage->current_room;

    // ##### Update #####
    // Update stuff like movement, collision etc.
    handleControlls(game->settings, player, &ball, stage);
    doCollision(&ball, stage->current_room, player, game->settings);
    movePlayer(player);
    moveBall(game, &ball);

    struct Room *current_room = stage->current_room;

    // ##### Late-Update #####
    // Update stuff which depends on the updated values of before, like camera
    // movement etc.
    if (game->move_camera) {
      start = old_room;
      end = current_room;
      game->move_camera = false;
    }

    if (Vector2Distance(end->position_world, camera->target) > 5) {
      // move camera
      Vector2 smoothed =
          Vector2Lerp(game->camera.target, end->position_world, 0.04);
      camera->target = smoothed;

      // calculate next move to snap to target
      if (Vector2Distance(end->position_world, camera->target) < 5) {
        camera->target = end->position_world;
      }

      // check what should be rendered
      //  rooms_to_render[0] = start;
      // rooms_to_render[1] = end;
      // num_rooms_to_render = 2;
    } else {
      // check what should be rendered
      // num_rooms_to_render = 1;
      //  rooms_to_render[0] = current_room;
      start = current_room;
      end = current_room;
    }

    // ##### Render #####
    BeginDrawing();
    {
      // ############ CLEAR ############
      ClearBackground(WHITE);
      BeginMode2D(*camera);

      // ############ PLAYER ############
      DrawRectangle(player->position.x + game->ui_area.x +
                        current_room->position_world.x,
                    player->position.y + game->play_area.y +
                        current_room->position_world.y,
                    player->width, 10, BLACK);

      // ############ BRICKS ############

      for (int i = 0; i < num_rooms_to_render; i++) {

        current_room = rooms_to_render[i];

        for (int i = 0; i < current_room->brick_count; i++) {
          if (current_room->bricks[i].alive) {
            DrawRectangle(
                current_room->bricks[i].position_room.x + game->play_area.x +
                    current_room->position_world.x,
                current_room->bricks[i].position_room.y + game->play_area.y +
                    current_room->position_world.y,
                current_room->brick_width, current_room->brick_height,
                current_room->bricks[i].color);
          }
        }
      }

      // ############ BALL ############
      DrawRectangle(
          ball.position.x + game->play_area.x + current_room->position_world.x,
          ball.position.y + game->play_area.y + current_room->position_world.y,
          ball.size, ball.size, ball.color);

      // ############ DEBUG ############
      // #ifdef DEBUG
      if (game->settings->debug_mode) {
        visualizePlayer(game, player, game->settings, current_room);
        visualizeBoxes(game, game->settings, current_room);
        visualizeBricks(game, game->settings, current_room);
        visualizeBall(game, &ball, current_room);
      }
      // #endif
      EndMode2D();

      // ############ UI ############
      DrawRectangle(game->ui_area.x, game->ui_area.y, game->ui_area.width,
                    game->ui_area.height, BLACK);

      DrawText(TextFormat("Score: %d", player->score),
               game->ui_area.x + game->ui_area.width - 130,
               game->ui_area.y + game->ui_area.height / 2 - 10, 20, WHITE);

      if (game->settings->draw_fps) {
        DrawFPS(0, 0);
      }

      if (game->settings->debug_mode) {
        visualizeRoom(game);
      }
    }
    EndDrawing();
  }

  // TODO:cleanup
  // cleanup (here for now)
}

void cleanup() { CloseWindow(); }

int main(void) {
  int window_width = 960;
  int window_height = 540;
  int ui_height = 30;
  Vector2 room_size = (Vector2){1920, 1080};
  int player_width = 100;
  int player_height = 10;
  int bottom_offset = 50;
  float steering_strength = .5;
  Rectangle play_area = {
      .x = 0,
      .y = ui_height,
      .height = room_size.y - ui_height,
      .width = room_size.x,
  };

  struct Player player = {
      .speed = 5,
      .width = player_width,
      .height = player_height,
      .steering_strength = steering_strength,
      .position = {(play_area.width / 2.0) - player_width / 2.0,
                   play_area.height - bottom_offset},
      .score = 0,
      .is_holding_ball = true,
  };

  struct Settings settings = {
      .radius = 1000,
      .player_slope_scale = -.6,
      .a = .1,
      .use_function = false,
      .debug_mode = true,
      .restart = false,
      .mouse_mode = false,
      .draw_fps = true,
  };

  // Camera
  Camera2D camera = {0};
  camera.target = (Vector2){0, 0};
  camera.offset = (Vector2){0, 0};
  camera.rotation = 0.0f;
  camera.zoom = .50f;
  Vector2 camera_velocity = (Vector2){20, 0};

  struct Game local_game = {
      .window_size = {window_width, window_height},
      .ui_area =
          {
              .x = 0,
              .y = 0,
              .width = window_width,
              .height = ui_height,
          },
      .play_area =
          {
              .x = 0,
              .y = ui_height,
              .height = room_size.y - ui_height,
              .width = room_size.x,
          },
      .target_fps = 100,
      //      .room_size = {1920, 1080},
      .camera_target = (Vector2){0.0, 0.0},
      .move_camera = false,
      .player = &player,
      .settings = &settings,
      .camera = camera,
      .camera_velocity = camera_velocity,
  };

  game = &local_game;

  init();
  loop();
  cleanup();

  return EXIT_SUCCESS;
}
