/*
 * File: world.c
 * ==================
 * Implements a sample world.
 */

#include "world.h"

#define MAP_WIDTH 24
#define MAP_HEIGHT 24

// Maximum number of levels
#define MAX_LAYERS 10

// Sample map
int world_map_level[MAP_WIDTH][MAP_HEIGHT] =
{
  {4,4,4,4,4,4,4,9,4,4,4,4,4,4,4,7,7,7,7,7,7,7,7,7},
  {4,0,0,0,0,0,0,0,0,0,0,0,0,0,7,7,0,0,0,0,0,0,0,7},
  {4,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {4,4,4,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {4,0,0,0,0,0,1,0,0,2,0,0,7,7,7,7,7,0,0,0,0,0,0,7},
  {4,0,0,0,0,0,1,0,0,2,0,0,0,0,0,0,7,7,0,7,7,7,7,7},
  {4,0,0,0,0,0,1,2,2,2,2,2,2,2,2,2,7,0,0,0,7,7,7,5},
  {4,0,3,1,1,1,1,2,0,0,0,2,0,0,0,2,7,0,0,0,0,0,0,5},
  {4,0,0,0,0,0,1,0,0,0,0,2,0,0,0,0,0,0,0,5,5,5,5,5},
  {4,0,0,0,0,0,1,0,0,2,2,2,0,0,0,5,7,0,0,0,0,0,0,5},
  {4,0,0,0,0,0,1,0,0,0,0,0,0,0,0,5,7,0,0,0,7,7,7,5},
  {4,0,0,0,0,0,1,5,5,0,5,5,5,5,5,5,7,7,7,7,7,7,7,5},
  {6,6,6,6,6,0,6,6,6,0,6,6,6,6,6,6,6,6,6,6,6,6,6,5},
  {8,0,0,0,0,0,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5},
  {8,0,0,0,0,0,6,6,6,6,6,0,6,0,6,0,6,0,6,0,6,0,6,5},
  {6,6,6,6,6,0,4,4,4,4,6,0,6,0,2,2,2,2,2,2,1,3,1,5},
  {1,1,8,8,8,0,0,0,0,4,0,0,6,2,0,0,0,0,7,0,0,0,0,4},
  {1,0,8,0,0,0,0,0,4,0,0,0,6,2,0,5,0,0,7,0,0,0,0,4},
  {1,0,8,0,0,0,0,0,4,0,0,0,6,2,0,0,0,0,0,5,0,1,0,4},
  {1,0,8,0,8,0,0,0,0,4,0,0,0,0,0,0,0,5,0,0,0,3,0,9},
  {1,0,8,0,8,0,0,0,0,4,6,0,0,2,0,0,0,0,0,5,0,1,0,4},
  {1,0,8,0,8,0,0,0,0,4,0,0,6,2,0,0,5,0,7,0,0,0,0,4},
  {3,0,0,0,8,0,0,0,0,4,6,0,0,2,0,0,0,0,7,0,0,0,0,4},
  {1,1,1,1,1,1,1,1,1,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4}
};

// Empty map
int empty_world[MAP_WIDTH][MAP_HEIGHT] =
{
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

// Sample map: may be used for. a game
int game_map[MAP_WIDTH][MAP_HEIGHT] =
{
  {1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,0,0,0,0,0,0,0,0,1},
  {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {4,4,4,4,4,4,4,4,0,0,0,3,1,1,1,1,1,1,1,1,1,1,1,1},
  {0,0,0,0,0,0,0,4,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,4,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,4,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,4,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,4,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,4,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,7,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,7,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,7,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,7,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {5,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {5,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {5,0,0,0,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

/**
 * Initialize the world.
 */
void world_init(void) {
  // Initialize empty world
  for (int i = 0; i < MAX_LAYERS; ++i) {
    for (int j = 0; j < MAP_WIDTH; ++j) {
      for (int k = 0; k < MAP_HEIGHT; ++k) {
        world_map[i][j][k] = 0;
      }
    }
  }
  
  // Sample world sequence: Map 1 for levels 1 through 6
  for (int j = 0; j < MAP_WIDTH; ++j) {
    for (int k = 0; k < MAP_HEIGHT; ++k) {
      world_map[1][j][k] = world_map_level[j][k];
      world_map[2][j][k] = world_map_level[j][k];
      world_map[3][j][k] = world_map_level[j][k];
      world_map[4][j][k] = world_map_level[j][k];
      world_map[5][j][k] = world_map_level[j][k];
      world_map[6][j][k] = world_map_level[j][k];
    }
  }
}