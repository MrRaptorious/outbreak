#ifndef roomgenerator
#define roomgenerator

#include "structs.h"
#include <raylib.h>

struct Stage *generateStage(struct StageSettings stageSettings);
int getRoom(struct Stage *stage, int room_index, enum Direction direction);
#endif
