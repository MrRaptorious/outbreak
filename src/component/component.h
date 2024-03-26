
#ifndef component_management
#define component_management

#include <raylib.h>

// Bitset
// https://c-faq.com/misc/bitsets.html
#define BIT_SET(bitset, bits) ((bitset) & (bits))
#define BIT_TEST(bitset, bits) ((bitset) & (bits))

#define MAX_COMPONENTS 32

typedef unsigned int ComponentType;
typedef unsigned int Signature;

// ##### components ######
struct TransformComponent {
  Vector2 position;
};

#endif
