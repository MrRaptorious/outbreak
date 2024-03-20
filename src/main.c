#include "raylib.h"
#include <stdio.h>

// TODO:
// - use delta time!
// - use:
// https://learnopengl.com/In-Practice/2D-Game/Collisions/Collision-detection

// Begin Definition: Stage

struct Brick {
  Vector2 position;
  int width;
  Color color;
};

struct Stage {
  int id;
  struct Brick *bricks;
  int brick_count;
  int brick_width;
};

void createStage(struct Stage *stage) {
  int render_width = GetRenderWidth();
  int num_bricks_in_row = 10;
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
  }

  stage->id = 1;
  stage->bricks = bricks;
  stage->brick_count = num_bricks_in_row;
  stage->brick_width = brick_width;
}

// End Definition: Stage

// Begin Definition: Player
struct Player {
  Vector2 position;
  float speed;
  int width;
  void (*move_player)(struct Player *);
};

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
// End Definition: Player

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
  int player_width = 50;
  int bottom_offset = 50;

  struct Player player = {
      .speed = 2,
      .width = player_width,
      .position = {(render_width / 2.0) - player_width / 2.0,
                   render_height - bottom_offset},
      .move_player = move_player,
  };

  struct Stage s = {};
  createStage(&s);

  // Gameloop
  while (!WindowShouldClose()) {

    // calculate state
    player.move_player(&player);

    // render
    BeginDrawing();
    // 1. CLEAR
    ClearBackground(WHITE);

    // 2. DRAW PLAYER AND STAGE
    DrawRectangle(player.position.x, player.position.y, player.width, 10,
                  BLACK);

    for (int i = 0; i < s.brick_count; i++) {
      DrawRectangle(s.bricks[i].position.x, s.bricks[i].position.y,
                    s.brick_width, 20, s.bricks[i].color);
    }

    // 3. DRAW UI
    DrawFPS(0, 0);
    EndDrawing();
  }

  // cleanup (here for now)
  MemFree(s.bricks);
}

void cleanup() { CloseWindow(); }

int main(void) {

  init();
  loop();
  cleanup();

  return 0;
}
