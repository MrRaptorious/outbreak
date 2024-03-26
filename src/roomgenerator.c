#include "roomgenerator.h"
#include "colorConversion.h"
#include "structs.h"
#include <assert.h>
#include <raylib.h>
#include <raymath.h>

void createRoom(struct Room *room, int id, int num_bricks_in_row, int rows,
                Vector2 position_world, struct StageSettings settings) {
  room->id = id;
  room->base_color = hsl2rgb(GetRandomValue(0, 255), 128, 128);
  room->brick_count = num_bricks_in_row * rows;
  room->position_world = position_world;
  room->kill_box = (Rectangle){
      .x = settings.kill_box_position.x,
      .y = settings.kill_box_position.y,
      .width = settings.room_size.x,
      .height = 20,
  };

  int render_width = settings.room_size.x;
  int brick_height = 50;
  float padding = 2;
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

    Vector3 hsl_color =
        rgb2hsl(room->base_color.r, room->base_color.g, room->base_color.b);

    hsl_color.z += 0.2 * (row + 1);

    Color row_color = hsl2rgb(hsl_color.x, hsl_color.y, hsl_color.z);

    for (int column = 0; column < num_bricks_in_row; column++) {
      int position = (row * num_bricks_in_row) + column;
      bricks[position].color = row_color;
      bricks[position].position_room.y = y_pos;
      bricks[position].position_room.x =
          padding + brick_width * column + padding * column;
      bricks[position].alive = true;
    }
  }

  room->bricks = bricks;
  room->brick_width = brick_width;
  room->brick_height = brick_height;
}

enum Direction *initAdjacencyMatrix(int num_rooms) {
  enum Direction *adjacencyMatrix;

  // int total_elements = ((num_rooms * (num_rooms + 1)) / 2) - num_rooms;

  // adjacencyMatrix = MemAlloc(sizeof(enum Direction) * total_elements);

  // if (!adjacencyMatrix) {
  //   // out of memory
  // }

  // for (int source_room = 0; source_room < num_rooms; source_room++) {
  //   for (int target_room = 0; target_room < source_room; target_room++) {
  //     adjacencyMatrix[source_room + target_room] = DIRECTION_NONE;
  //   }
  // }

  int total_elements = num_rooms * num_rooms;
  adjacencyMatrix = MemAlloc(sizeof(enum Direction) * total_elements);

  if (!adjacencyMatrix) {
    // out of memory
  }
  for (int row = 0; row < num_rooms; row++) {
    for (int col = 0; col < num_rooms; col++) {
      adjacencyMatrix[(row * num_rooms) + col] = DIRECTION_NONE;
    }
  }

  return adjacencyMatrix;
}

enum Direction negateDirection(enum Direction direction) {
  enum Direction negated_direction = DIRECTION_NONE;

  switch (direction) {
  case LEFT:
    negated_direction = RIGHT;
    break;
  case TOP:
    negated_direction = BOTTOM;
    break;
  case RIGHT:
    negated_direction = LEFT;
    break;
  case BOTTOM:
    negated_direction = TOP;
    break;
  default:
    negated_direction = DIRECTION_NONE;
  }

  return negated_direction;
}

// ################## interface ####################
int getRoom(struct Stage *stage, int room_num, enum Direction direction) {
  // position of n->m in array:
  // if n < m : (((n * (n + 1)) / 2) - n) + m;
  // if n > m : inverse{(((n * (n + 1)) / 2) - n) + m}

  // int search_offset = ((room_num * (room_num + 1)) / 2) - room_num;
  int search_offset = room_num * stage->num_rooms;
  int target_room_num = -1;

  // search horizontally
  // for (int i = 0; i < room_num; i++) {
  //  int search_pos = search_offset + i;
  //  if (stage->room_layout[search_pos] == direction) {
  //    room_number = i;
  //    break;
  //  }
  //}

  // search vertically
  // (negate direction because we only use half of our adjacencyMatrix because
  // we mirrored along the diagonal axis)
  // enum Direction negated_direction = negateDirection(direction);

  // if (room_number < 0) {
  //   for (int i = room_num; i < stage->num_rooms - 1; i++) {
  //     int search_pos = (((i * (i + 1)) / 2) - i) + room_num;
  //     if (stage->room_layout[search_pos] == negated_direction) {
  //       room_number = i;
  //       break;
  //     }
  //   }
  // }
  //

  for (int i = 0; i < stage->num_rooms; i++) {
    if (stage->room_layout[search_offset + i] == direction) {
      target_room_num = i;
      break;
    }
  }

  return target_room_num;
}

// TODO: ensure termination!
struct Stage *generateStage(struct StageSettings stage_settings) {
  struct Stage *stage = MemAlloc(sizeof(struct Stage) * 1);
  if (!stage) {
    // out of memory
  }
  enum Direction *adjacencyMatrix;
  adjacencyMatrix = initAdjacencyMatrix(stage_settings.num_rooms);

  struct Room *rooms = MemAlloc(sizeof(struct Room) * stage_settings.num_rooms);

  if (!rooms) {
    // out of memory
  }

  stage->rooms = rooms;
  stage->num_rooms = stage_settings.num_rooms;
  stage->room_size = stage_settings.room_size;
  stage->room_layout = adjacencyMatrix;
  // stage->room_layout_length =
  // j   ((stage_settings.num_rooms * (stage_settings.num_rooms + 1)) / 2) -
  //  stage_settings.num_rooms;

  stage->room_layout_length =
      stage_settings.num_rooms * stage_settings.num_rooms;

  int num_rooms_left = stage_settings.num_rooms;
  int current_room_index = 0;

  // seed generatio with initial room #0
  // this ensures that the picked room in the loop always has a position
  createRoom(&rooms[current_room_index], current_room_index, 30, 2,
             (Vector2){0, 0}, stage_settings);

  num_rooms_left--;
  current_room_index++;

  int loop_limit = 0;

  while (num_rooms_left > 0) {
    assert(num_rooms_left < stage_settings.num_rooms);

    // pick random room of already generated rooms (1 based)
    int random_room_index =
        GetRandomValue(0, stage_settings.num_rooms - num_rooms_left - 1);

    // assert(random_room_index > 0);

    // pick random direction to that room
    enum Direction random_direction = (enum Direction)GetRandomValue(1, 4);

    assert(random_direction == LEFT || random_direction == TOP ||
           random_direction == RIGHT || random_direction == BOTTOM);

    // calculate room position (0 based index in array)
    //  int search_offset =
    //     (((random_room_num * (random_room_num + 1)) / 2) - random_room_num);

    int search_offset = (random_room_index)*stage_settings.num_rooms;

    // check if there isn't a room in the way
    int target_room_index = getRoom(stage, random_room_index, random_direction);

    if (target_room_index < 0) {

      // update matrix
      adjacencyMatrix[((random_room_index)*stage_settings.num_rooms) +
                      current_room_index] = random_direction;

      adjacencyMatrix[((current_room_index)*stage_settings.num_rooms) +
                      random_room_index] = negateDirection(random_direction);

      Vector2 world_position = rooms[random_room_index].position_world;

      switch (random_direction) {
      case LEFT:
        world_position.x -= stage_settings.room_size.x;
        break;
      case RIGHT:
        world_position.x += stage_settings.room_size.x;
        break;
      case BOTTOM:
        world_position.y += stage_settings.room_size.y;
        break;
      case TOP:
        world_position.y -= stage_settings.room_size.y;
        break;
      default:
        break;
      }

      // generate room
      createRoom(&(rooms[current_room_index]), current_room_index, 30, 3,
                 world_position, stage_settings);

      current_room_index++;
      num_rooms_left--;

    } else {
      // perhaps connect rooms
    }
  }

  return stage;
}
