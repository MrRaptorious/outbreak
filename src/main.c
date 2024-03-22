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

// TODO:
// - use delta time!
// https://learnopengl.com/In-Practice/2D-Game/Collisions/Collision-detection

// ############################## Structs ##############################
struct Player {
  Vector2 position;
  float speed;
  int width;
  int height;
  float steering_strength;
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
};

// ############################## End Structs ##############################

// ############################## Setup Functions ##############################
void createStage(struct Stage *stage) {
  int render_width = GetRenderWidth();
  int num_bricks_in_row = 20;
  int brick_height = 20;
  float padding = 1;

  float brick_width = (render_width - (padding * (num_bricks_in_row + 1))) /
                      (num_bricks_in_row);

  struct Brick *bricks;
  bricks = MemAlloc(sizeof(struct Brick) * num_bricks_in_row);

  if (!bricks) {
    // handel error... or do we?
  }

  for (int i = 0; i < num_bricks_in_row; i++) {
    bricks[i].color = (Color){GetRandomValue(0, 255), GetRandomValue(0, 255),
                              GetRandomValue(0, 255), 255};
    bricks[i].position.y = 50;
    bricks[i].position.x = padding + brick_width * i + padding * i;
    bricks[i].alive = true;
  }

  stage->id = 1;
  stage->bricks = bricks;
  stage->brick_count = num_bricks_in_row;
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

      DrawLine(origin.x, origin.y, end.x, end.y, ORANGE);
    }

  } else {
    for (int i = 0; i < player->width; i++) {

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

      DrawLine(origin.x, origin.y, end.x, end.y, ORANGE);
    }
  }
}

void spawnBall(struct Ball *ball) {
  int render_width = GetRenderWidth();
  int render_height = GetRenderHeight();

  ball->size = 10;
  ball->color = BLACK;
  ball->speed = 4;
  ball->position = (Vector2){.x = render_width / 2.0, .y = render_height / 2.0};
  ball->direction = (Vector2){-.1, -1};
}

void visualizeBoxes(struct Settings *settings) {

  DrawRectangle(settings->kill_box.x, settings->kill_box.y,
                settings->kill_box.width, settings->kill_box.height,
                (Color){255, 0, 0, 100});
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

      if (CheckCollisionRecs(brick_collision, ball_box)) {
        // kill brick
        stage->bricks[i].alive = false;

        if (!reflected_current_frame) {
          Vector2 new_direction =
              Vector2Reflect(ball->direction, WALL_TOP_NORMAL);
          ball->direction.y = new_direction.y;
          ball->direction.x = new_direction.x;
          reflected_current_frame = true;
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
  int render_width = GetRenderWidth();
  int render_height = GetRenderHeight();

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
  int render_width = GetRenderWidth();
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
                  struct Stage *stage) {
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

    createStage(stage);
    spawnBall(ball);
  }
}
// ############################ End Tick Functions
// ############################

void init() {
  printf("Raylib Version: %s", RAYLIB_VERSION);

  int window_width = 800;
  int window_height = 600;

  InitWindow(window_width, window_height, "Outbreak");
  SetTargetFPS(100);
}

void loop() {
  int render_width = GetRenderWidth();
  int render_height = GetRenderHeight();
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
      .debug_mode = true,
      .restart = false,
      .kill_box =
          {
              .x = 0,
              .y = render_height - 10,
              .width = render_width,
              .height = 10,
          },
  };

  // Gameloop
  while (!WindowShouldClose()) {
    handleInputs(&settings, &ball, &stage);
    doCollision(&ball, &stage, &player, &settings);
    movePlayer(&player);
    moveBall(&ball);

    BeginDrawing();
    {
      // ############ CLEAR ############
      ClearBackground(WHITE);

      // ############ PLAYER ############
      DrawRectangle(player.position.x, player.position.y, player.width, 10,
                    BLACK);

      // ############ BRICKS ############
      for (int i = 0; i < stage.brick_count; i++) {
        if (stage.bricks[i].alive) {
          DrawRectangle(stage.bricks[i].position.x, stage.bricks[i].position.y,
                        stage.brick_width, stage.brick_height,
                        stage.bricks[i].color);
        }
      }

      // ############ BALL ############
      DrawRectangle(ball.position.x, ball.position.y, ball.size, ball.size,
                    ball.color);

      // ############ UI ############
      DrawFPS(0, 0);

      // ############ DEBUG ############
      // #ifdef DEBUG
      if (settings.debug_mode) {
        visualizePlayer(&player, &settings);
        visualizeBoxes(&settings);
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
  init();
  loop();
  cleanup();

  return EXIT_SUCCESS;
}
