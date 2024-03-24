#ifndef GAME_STRUCTS
#define GAME_STRUCTS

#include <raylib.h>

struct Player {
  Vector2 position;
  float speed;
  int width;
  int height;
  float steering_strength;
  int score;
  bool is_holding_ball;
};

struct Ball {
  Vector2 position;
  int size;
  float speed;
  Vector2 direction;
  Color color;
};

struct Brick {
  Vector2 position_stage;
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
  Vector2 position_world;
  Color base_color;
  Rectangle kill_box;
  struct Stage *right;
  struct Stage *left;
};

struct Settings {
  float radius;
  float player_slope_scale;
  float a;
  bool use_function;
  bool debug_mode;
  bool restart;
  bool mouse_mode;
  bool draw_fps;
};

struct Game {
  Vector2 window_size;
  Rectangle ui_area;
  Rectangle play_area;
  int target_fps;
  Vector2 room_size;
  Vector2 camera_target;
  struct Stage *stages[2];
  int current_stage;
  int num_stages;
  bool move_camera;
  struct Player *player;
  struct Settings *settings;
  Camera2D camera;
  Vector2 camera_velocity;
};

#endif
