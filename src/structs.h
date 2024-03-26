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
  Vector2 position_room;
  int width;
  Color color;
  bool alive;
};

struct Room {
  int id;
  struct Brick *bricks;
  int brick_count;
  int brick_width;
  int brick_height;
  Vector2 position_world;
  Color base_color;
  Rectangle kill_box;
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

enum Direction {
  DIRECTION_NONE = 0,
  LEFT = 1,
  TOP = 2,
  RIGHT = 3,
  BOTTOM = 4,
};

struct StageSettings {
  Vector2 room_size;
  Vector2 kill_box_position;
  int num_rooms;
};

struct Stage {
  int num_rooms;
  struct Room *rooms;
  enum Direction *room_layout;
  Vector2 room_size;
  int room_layout_length;
  struct Room *current_room;
};

struct Game {
  Vector2 window_size;
  Rectangle ui_area;
  Rectangle play_area;
  int target_fps;
  Vector2 camera_target;
  bool move_camera;
  struct Player *player;
  struct Settings *settings;
  Camera2D camera;
  Vector2 camera_velocity;
  struct Stage *current_stage;
};

#endif
