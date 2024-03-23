#include "raylib.h"
#include "raymath.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define WALL_LEFT_NORMAL                                                       \
  (Vector2) { 1, 0 }
#define WALL_RIGHT_NORMAL                                                      \
  (Vector2) { -1, 0 }
#define WALL_TOP_NORMAL                                                        \
  (Vector2) { 0, 1 }
#define PLAYER_NORMAL                                                          \
  (Vector2) { 0, -1 }
#define NORMAL_COLOR                                                           \
  (Color) { 255, 140, 0, 150 }

// TODO:
// - use delta time!
// https://learnopengl.com/In-Practice/2D-Game/Collisions/Collision-detection
// - use shaders
// - portals
// - ui with score

// ############################## Structs ##############################
struct Player {
  Vector2 position;
  float speed;
  int width;
  int height;
  float steering_strength;
  int score;
};

struct Ball {
  Vector2 position;
  int size;
  float speed;
  Vector2 direction;
  Color color;
};

struct Brick {
  Vector2 position;
  int width;
  Color color;
  bool alive;
};

struct Stage {
  int id;
  struct Brick *bricks;
  int brick_count;
  int brick_width;
  int brick_height;
};

struct Settings {
  float radius;
  float player_slope_scale;
  float a;
  bool use_function;
  bool debug_mode;
  bool restart;
  Rectangle kill_box;
  bool mouse_mode;
  bool draw_fps;
};

struct Game {
  Vector2 window_size;
  Rectangle ui_area;
  Rectangle play_area;
  int target_fps;
};

// ############################## End Structs ##############################

// ######################### Global vars ############################
struct Game *game;
// ######################### End Global vars ############################

// ############################## Setup Functions ##############################
void createStage(struct Stage *stage) {
  int render_width = game->play_area.width;
  int num_bricks_in_row = 20;
  int rows = 4;
  int brick_height = 20;
  float padding = 1;
  int base_offset = 50;

  float brick_width = (render_width - (padding * (num_bricks_in_row + 1))) /
                      (num_bricks_in_row);

  struct Brick *bricks;
  bricks = MemAlloc(sizeof(struct Brick) * num_bricks_in_row * rows);

  if (!bricks) {
    // handel error... or do we?
  }

  for (int row = 0; row < rows; row++) {
    int y_pos = base_offset + (row * (brick_height + padding));

    for (int column = 0; column < num_bricks_in_row; column++) {
      int position = (row * num_bricks_in_row) + column;
      bricks[position].color =
          (Color){GetRandomValue(0, 255), GetRandomValue(0, 255),
                  GetRandomValue(0, 255), 255};
      bricks[position].color =
          (Color){((row + 1) * 10) % 255, ((row + 1) * 60) % 255,
                  ((row + 1) * 40) % 255, 255};
      bricks[position].position.y = y_pos;
      bricks[position].position.x =
          padding + brick_width * column + padding * column;
      bricks[position].alive = true;
    }
  }

  stage->id = 1;
  stage->bricks = bricks;
  stage->brick_count = num_bricks_in_row * rows;
  stage->brick_width = brick_width;
  stage->brick_height = brick_height;
}
// ############################ End Setup Functions ############################

// ############################ Util Functions ############################

void visualizePlayer(struct Player *player, struct Settings *settings) {
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
      Vector2 end = {origin.x - normal.x * normal_scale,
                     origin.y - normal_scale};

      DrawLine(origin.x + game->play_area.x, origin.y + game->play_area.y,
               end.x + game->play_area.x, end.y + game->play_area.y,
               NORMAL_COLOR);
    }
  }
}

void spawnBall(struct Ball *ball) {
  int render_width = game->play_area.width;
  int render_height = game->play_area.height;

  ball->size = 10;
  ball->color = BLACK;
  ball->speed = 4;
  ball->position = (Vector2){.x = render_width / 2.0, .y = render_height / 2.0};
  ball->direction = (Vector2){-.1, -1};
}

void visualizeBoxes(struct Settings *settings) {

  DrawRectangle(settings->kill_box.x + game->play_area.x,
                settings->kill_box.y + game->play_area.y,
                settings->kill_box.width, settings->kill_box.height,
                (Color){255, 0, 0, 100});
}

void visualizeBricks(struct Settings *settings, struct Stage *stage) {
  for (int i = 0; i < stage->brick_count; i++) {
    Color transparent = stage->bricks[i].color;
    transparent.a = 50;

    DrawRectangle(stage->bricks[i].position.x + game->play_area.x,
                  stage->bricks[i].position.y + game->play_area.y,
                  stage->brick_width, stage->brick_height, transparent);
  }
}

// ########################### End Util Functions ###########################

// ############################## Tick Functions ##############################

void doCollision(struct Ball *ball, struct Stage *stage, struct Player *player,
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
  if (CheckCollisionRecs(settings->kill_box, ball_box)) {
    settings->restart = true;
  }

  // bricks
  bool reflected_current_frame = false;
  for (int i = 0; i < stage->brick_count; i++) {

    if (stage->bricks[i].alive) {

      Rectangle brick_collision = {
          .x = stage->bricks[i].position.x,
          .y = stage->bricks[i].position.y,
          .width = stage->brick_width,
          .height = stage->brick_height,
      };

      // only check if brick is alive
      if (stage->bricks[i].alive) {
        if (CheckCollisionRecs(brick_collision, ball_box)) {
          // kill brick
          stage->bricks[i].alive = false;

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

void moveBall(struct Ball *ball) {
  Vector2 new_pos =
      Vector2Add(ball->position, (Vector2){ball->direction.x * ball->speed,
                                           ball->direction.y * ball->speed});

  ball->position.x = new_pos.x;
  ball->position.y = new_pos.y;
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

void handleInputs(struct Settings *settings, struct Ball *ball,
                  struct Stage *stage, struct Player *player) {
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

  if (IsKeyPressed(KEY_D)) {

    settings->debug_mode = !settings->debug_mode;
    if (settings->debug_mode) {
      printf("Debug-mode enabled");
    } else {
      printf("Debug-mode disabled");
    }
  }

  if (IsKeyPressed(KEY_N) || settings->restart) {
    settings->restart = false;
    printf("Restarting Game\n");

    MemFree(stage->bricks);
    createStage(stage);
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

  assert(game != NULL);

  InitWindow(game->window_size.x, game->window_size.y, "Outbreak");
  SetTargetFPS(game->target_fps);
}

void loop() {
  int render_width = game->play_area.width;
  int render_height = game->play_area.height;
  int player_width = 100;
  int player_height = 10;
  int bottom_offset = 50;
  float steering_strength = .5;

  struct Player player = {
      .speed = 5,
      .width = player_width,
      .height = player_height,
      .steering_strength = steering_strength,
      .position = {(render_width / 2.0) - player_width / 2.0,
                   render_height - bottom_offset},
      .score = 0,
  };

  struct Stage stage = {};
  createStage(&stage);

  struct Ball ball;
  spawnBall(&ball);

  struct Settings settings = {
      .radius = 1000,
      .player_slope_scale = -.6,
      .a = .1,
      .use_function = false,
      .debug_mode = false,
      .restart = false,
      .kill_box =
          {
              .x = 0,
              .y = game->play_area.height - 10,
              .width = game->play_area.width,
              .height = 10,
          },
      .mouse_mode = false,
      .draw_fps = true,
  };

  // Gameloop
  while (!WindowShouldClose()) {
    handleInputs(&settings, &ball, &stage, &player);
    doCollision(&ball, &stage, &player, &settings);
    movePlayer(&player);
    if (!settings.mouse_mode) {
      moveBall(&ball);
    } else {
      ball.position.x = GetMouseX() - game->play_area.x;
      ball.position.y = GetMouseY() - game->play_area.y;
    }
    BeginDrawing();
    {
      // ############ CLEAR ############
      ClearBackground(WHITE);

      // ############ PLAYER ############
      DrawRectangle(player.position.x + game->ui_area.x,
                    player.position.y + game->play_area.y, player.width, 10,
                    BLACK);

      // ############ BRICKS ############
      for (int i = 0; i < stage.brick_count; i++) {
        if (stage.bricks[i].alive) {
          DrawRectangle(stage.bricks[i].position.x + game->play_area.x,
                        stage.bricks[i].position.y + game->play_area.y,
                        stage.brick_width, stage.brick_height,
                        stage.bricks[i].color);
        }
      }

      // ############ BALL ############
      DrawRectangle(ball.position.x + game->play_area.x,
                    ball.position.y + game->play_area.y, ball.size, ball.size,
                    ball.color);

      // ############ UI ############
      DrawRectangle(game->ui_area.x, game->ui_area.y, game->ui_area.width,
                    game->ui_area.height, BLACK);

      DrawText(TextFormat("Score: %d", player.score),
               game->ui_area.x + game->ui_area.width - 130,
               game->ui_area.y + game->ui_area.height / 2 - 10, 20, WHITE);

      if (settings.draw_fps) {
        DrawFPS(0, 0);
      }

      // ############ DEBUG ############
      // #ifdef DEBUG
      if (settings.debug_mode) {
        visualizePlayer(&player, &settings);
        visualizeBoxes(&settings);
        visualizeBricks(&settings, &stage);
      }
      // #endif
    }
    EndDrawing();
  }

  // cleanup (here for now)
  MemFree(stage.bricks);
}

void cleanup() { CloseWindow(); }

int main(void) {

  int window_width = 800;
  int window_height = 600;
  int ui_height = 50;

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
              .height = window_height - ui_height,
              .width = window_width,
          },
      .target_fps = 100,
  };

  game = &local_game;

  init();
  loop();
  cleanup();

  return EXIT_SUCCESS;
}
