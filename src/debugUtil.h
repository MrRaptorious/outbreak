#ifndef DEBUG_MODE
#define DEBUG_MODE

#include "structs.h"
#include <raylib.h>
#include <raymath.h>

void visualizePlayer(struct Game *game, struct Player *player,
                     struct Settings *settings, struct Room *room);

void visualizeBoxes(struct Game *game, struct Settings *settings,
                    struct Room *room);

void visualizeBricks(struct Game *game, struct Settings *settings,
                     struct Room *room);

void visualizeBall(struct Game *game, struct Ball *ball, struct Room *room);

void visualizeRoom(struct Game *game);

#endif
