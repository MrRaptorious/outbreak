#include "raylib.h"
#include "raymath.h"
#include <stdbool.h>
#include <stdio.h>

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
// ############################## End Structs ##############################

// ############################## Setup Functions ##############################
void createStage(struct Stage *stage) {
  int render_width = GetRenderWidth();
  int num_bricks_in_row = 10;
  int brick_height = 10;
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

// ############################ Helper Functions ############################
//
//
// ########################### End Helper Functions ###########################

// ############################## Tick Functions ##############################
void doCollision(struct Ball *ball, struct Stage *stage,
                 struct Player *player) {
  // go through each brick and check collision with the ball
  // for now use AABB Collision with Ball

  Rectangle ball_box = {
      .x = ball->position.x,
      .y = ball->position.y,
      .width = ball->size,
      .height = ball->size,
  };

  // death zone

  // bricks
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

        // reflect ball
        Vector2 new_direction =
            Vector2Reflect(ball->direction, WALL_TOP_NORMAL);
        ball->direction.y = new_direction.y;
        ball->direction.x = new_direction.x;
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

  ball->color = BLACK;
  if (CheckCollisionRecs(player_box, ball_box)) {
    ball->color = RED;

    // reflect ball from player
    // TODO: reflect more to the edges

    float hit_offset = ((player->position.x + player->width / 2.0) -
                        (ball->position.x + ball->size / 2.0));

    // percentage where was hit
    float scale = hit_offset / (player->width / 2.0);

    Vector2 player_normal = PLAYER_NORMAL;

    Vector2 new_direction = Vector2Reflect(ball->direction, player_normal);

    ball->direction.y = new_direction.y;
    ball->direction.x = new_direction.x;

    // move ball up a little
    // TODO: little hack ?
    ball->position.y -= 2;
  }
}

void move_Ball(struct Ball *ball) {

  // check collisions

  // update movement direction
  Vector2 new_pos =
      Vector2Add(ball->position, (Vector2){ball->direction.x * ball->speed,
                                           ball->direction.y * ball->speed});

  ball->position.x = new_pos.x;
  ball->position.y = new_pos.y;

  // check walls
  int render_width = GetRenderWidth();
  int render_height = GetRenderHeight();

  // reflect from wall left
  if (ball->position.x <= 0) {
    ball->position.x = 0;
    Vector2 new_direction = Vector2Reflect(ball->direction, WALL_LEFT_NORMAL);
    ball->direction.y = new_direction.y;
    ball->direction.x = new_direction.x;
  }

  if (ball->position.x >= render_width - ball->size) {
    // reflect from wall right
    ball->position.x = render_width - ball->size;
    Vector2 new_direction = Vector2Reflect(ball->direction, WALL_RIGHT_NORMAL);
    ball->direction.y = new_direction.y;
    ball->direction.x = new_direction.x;
  }

  // reflect from wall top
  if (ball->position.y <= 0) {
    ball->position.y = 0;
    ball->direction.y = -ball->direction.y;
  }

  // TODO:
  // dont do this, add death zone
  if (ball->position.y >= render_height - ball->size) {
    // reflect from wall bottom
    ball->position.y = render_height - ball->size;
    ball->direction.y = -ball->direction.y;
  }
}

void move_player(struct Player *player) {
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
  if (IsKeyDown(KEY_L) && player->position.x < render_width - player->width) {
    player->position.x += player->speed;
    // line up with edge
    if (player->position.x > render_width - player->width) {
      player->position.x = render_width - player->width;
    }
  }
}
// ############################ End Tick Functions
// ############################

void init() {
  printf("Raylib Version: %s", RAYLIB_VERSION);

  int window_width = 800;
  int window_height = 600;

  InitWindow(window_width, window_height, "Breakout");
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

  struct Ball ball = {
      .size = 10,
      .color = BLACK,
      .speed = 4,
      .position = {.x = render_width / 2.0, .y = render_height / 2.0},
      .direction = {-.1, -1},
  };

  // Gameloop
  while (!WindowShouldClose()) {

    // calculate state
    doCollision(&ball, &stage, &player);
    move_player(&player);
    move_Ball(&ball);

    BeginDrawing();
    // CLEAR
    ClearBackground(WHITE);

    // PLAYER
    DrawRectangle(player.position.x, player.position.y, player.width, 10,
                  BLACK);

    // BRICKS
    for (int i = 0; i < stage.brick_count; i++) {
      if (stage.bricks[i].alive) {
        DrawRectangle(stage.bricks[i].position.x, stage.bricks[i].position.y,
                      stage.brick_width, stage.brick_height,
                      stage.bricks[i].color);
      }
    }

    // BALL
    DrawRectangle(ball.position.x, ball.position.y, ball.size, ball.size,
                  ball.color);

    // DRAW UI
    DrawFPS(0, 0);
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

  return 0;
}
