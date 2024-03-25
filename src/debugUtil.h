#ifndef DEBUG_MODE
#define DEBUG_MODE

#include "structs.h"
#include <raylib.h>
#include <raymath.h>

void visualizePlayer(struct Game *game, struct Player *player,
                     struct Settings *settings, struct Stage *stage);

void visualizeBoxes(struct Game *game, struct Settings *settings,
                    struct Stage *stage);

void visualizeBricks(struct Game *game, struct Settings *settings,
                     struct Stage *stage);

void visualizeBall(struct Game *game, struct Ball *ball, struct Stage *stage);

#endif
