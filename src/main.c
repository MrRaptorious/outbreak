#include "raylib.h"
#include <stdio.h>

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

  // Gameloop
  while (!WindowShouldClose()) {

    // calculate state
    player.move_player(&player);

    // render
    BeginDrawing();
    // 1. CLEAR
    ClearBackground(WHITE);

    // 2. DRAW PLAYER
    DrawRectangle(player.position.x, player.position.y, player.width, 10,
                  BLACK);

    // 3. DRAW UI
    DrawFPS(0, 0);
    EndDrawing();
  }
}

void cleanup() { CloseWindow(); }

int main(void) {

  init();
  loop();
  cleanup();

  return 0;
}
