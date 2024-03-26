#ifndef entity_management
#define entity_management

#include "../component/component.h"
#include "../queue.h"
#include <raylib.h>

#define ENTITY_SIZE 32
#define MAX_ENTITIES 5000

typedef unsigned int Entity;

struct EntityManager {
  struct Queue *available_entities;
  // Array of signatures where the index corresponds to the entity id
  struct Queue *signatures;
};

Entity createEntity() {}

struct EntityManager createManager() {

  struct EntityManager *manager = MemAlloc(sizeof(struct EntityManager));
  if (!manager) {
    // well well
  }

  manager->available_entities = createQueue(MAX_ENTITIES);
  manager->signatures = createQueue(MAX_ENTITIES);

  for (Entity e = 0; e < MAX_ENTITIES; e++) {
  }
}

void destroyEntity(Entity entity);
void setSignature(Entity entity, Signature signature);
Signature getSignature(Entity entity);

#endif
