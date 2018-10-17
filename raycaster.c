/*
 * File: raycaster.c
 * ==================
 * Implements a raycasting module.
 */

#include "gl.h"
#include "raycaster.h"
#include "keyboard.h"
#include "ps2.h"

#define _WIDTH 640
#define _HEIGHT 480
#define MAP_WIDTH 24
#define MAP_HEIGHT 24
#define WALL_DESIGN_WIDTH 64
#define WALL_DESIGN_HEIGHT 64

// Modes
#define MAZE 0
#define MINECRAFT 1

// Maximum number of levels in Minecraft mode
#define MAX_LEVELS 10

// Math Util Macros
#define PI 3.14159265358979323846
#define MAX(operand1, operand2) ((operand1) > (operand2) ? (operand1) : (operand2))
#define MIN(operand1, operand2) ((operand1) < (operand2) ? (operand1) : (operand2))
#define ABS(operand1) ((operand1) < 0 ? (-operand1) : (operand1))

// Directions
#define FORWARD 1
#define BACKWARD -1

// Wall directions
#define NORTH_SOUTH 1
#define EAST_WEST 0

// Number of turns for a quarter turn
#define QUARTER_TURN 15

extern int world_map[MAX_LEVELS][MAP_WIDTH][MAP_HEIGHT];
extern unsigned int texture[10][WALL_DESIGN_WIDTH * WALL_DESIGN_HEIGHT];
unsigned int z_indices[_HEIGHT][_WIDTH];

// Levels (relevant for MINECRAFT mode only)
int curr_level = 1;
int max_level = 3;	// Recommended: Set this to 1 for a single-level MINECRAFT
int min_level = 1;

// Movement speeds for the main player
const double translational_speed = 1.0;
const double rotational_speed = 6.0 * PI / 180;

// (The height of the player) / (the height of a wall)
double player_wall_height_ratio = 0.50;

/**
 * Math Util Functions
 * ====================
 * sin(theta) and cos(theta) return same value since we only use theta = 6.0 here.
 * floor(int) returns integer part of double.
 *
 */
double sin(double theta)
{
	/* theta = (6 * PI / 180) */
    return 0.1045;
}

double cos(double theta)
{
	/* theta = (6 * PI / 180) */
    return 0.9945;
}

int floor(double number)
{
    return (int)(number);
}

/**
 * Resets the z-indices of all pixels.
 */
void reset_z_indices(void)
{
	for (int y = 0; y < _HEIGHT; ++y) {
		for (int x = 0; x < _WIDTH; ++x) {
			z_indices[y][x] = 2147483647;
		}
	}
}

/**
 * Helper function to draw an entire stripe of a wall at a given x-coordinate.
 *
 * @param 	x 						the x-coordinate on the screen
 * @param 	wall_lowest_point 		the lowest point of the wall (lowest x-coordinate but highest on screen)
 * @param 	wall_highest_point 		the highest point of the wall (highest x-coordinate but lowest on screen)
 * @param 	wall_height 			the height of the wall
 * @param 	x_coordinate_on_wall 	the x-coordinate of the texture to draw at the given point
 * @param 	wall_design_number 		the design number of the wall to draw
 * @oaram 	level 					the current level
 * @param 	distance_to_wall 		the perpendicular distance to the current wall
 * @param 	mode 					the current mode (MAZE or MINECRAFT)
 */
void design_wall(int x, int wall_lowest_point, int wall_highest_point, int wall_height, int x_coordinate_on_wall, int wall_design_number, int wall_facing, int level,
				 int distance_to_wall, int mode)
{
	unsigned int width = gl_get_width();
	unsigned (*current_framebuffer)[width] = (unsigned (*)[width])fb_get_draw_buffer();

	// y-coordinate adjusted accoridng to the level (only important for multi-level minecraft)
	int leveled_y = 0;
	for(int y = wall_lowest_point; y < wall_highest_point; ++y) {
		unsigned int color = 0;
		if (wall_design_number >= 0) {
			// Find the y-coordinate on the wall
			int y_wall_intersection = (y * 256) - (_HEIGHT * 256 * player_wall_height_ratio) + (wall_height * 128);
			int y_coordinate_on_wall = ((y_wall_intersection * WALL_DESIGN_HEIGHT) / wall_height) / 256;

			// Calculate the color at the given pixel from the design
			color = texture[wall_design_number][y_coordinate_on_wall * WALL_DESIGN_WIDTH + x_coordinate_on_wall];

			// For an EAST-WEST facing wall, make the color darker by halving each of the red, green, and blue values
			if (wall_facing == EAST_WEST)
				color = ((color >> 1) /* this is the RGB value from the RGBA */ & 0x7F7F7F /* now we halve each value */);
		}

		// For MINECRAFT mode, the levels may be different than the actual y
		if (level < curr_level) {
			leveled_y = MIN(_HEIGHT - 1, y + (curr_level - level) * wall_height);
		} else if (level == curr_level) {
			leveled_y = y;
		} else if (level > curr_level) {
			leveled_y = MAX(0, y - (level - curr_level) * wall_height);
		}

		// Draw the pixel
		if (mode == MAZE || distance_to_wall <= z_indices[leveled_y][x]) {	// in MINECRAFT mode, check that the current obstacle has a smaller z-index, i.e. it is closer
			current_framebuffer[leveled_y][x] = color;
			if (mode == MINECRAFT)
				z_indices[leveled_y][x] = distance_to_wall;	// update the z-index at the current pixel
		}
	}
}

/**
 * Helper function that extends a ray until a wall is hit. It updates wall_x, wall_y, and wall_facing with the correct values, depending on the wall.
 *
 * @param 	new_ray 					the ray to be extended
 * @param 	wall_x 						a pointer to the x-coordinate of the wall on the grid
 * @param 	wall_y 						a pointer to the y-coordinate of the wall on the grid
 * @param 	overall_ray_direction_x 	the x direction of the ray (not precise, an integer estimate)
 * @param 	overall_ray_direction_y 	the y direction of the ray (not precise, an integer estimate)
 * @param 	wall_facing 				a pointer to the direction that the wall is facing
 * @param 	level 						the level being drawn
 * @return 								1 if a wall was found, 0 otherwise
 */
int find_wall(struct ray *new_ray, int *wall_x, int *wall_y, int overall_ray_direction_x, int overall_ray_direction_y, int *wall_facing, int level)
{
	// As long as the ray is within the bounds of the map but has not hit a wall
	while (*wall_x >= 0 && *wall_x < MAP_WIDTH && *wall_y >= 0 && *wall_y < MAP_HEIGHT) {
		// Continue extending the ray
		if (new_ray->x_distance_to_next_wall < new_ray->y_distance_to_next_wall) {	// we have to find the closest wall, so pick the smaller of the x- and y-distance
			new_ray->x_distance_to_next_wall += new_ray->x_distance_between_two_sides;
			*wall_x += overall_ray_direction_x;
			*wall_facing = NORTH_SOUTH;
		} else {
			new_ray->y_distance_to_next_wall += new_ray->y_distance_between_two_sides;
			*wall_y += overall_ray_direction_y;
			*wall_facing = EAST_WEST;
		}
		
		if (world_map[level][*wall_x][*wall_y] > 0) {	// a wall was hit
			return 1;
		}
	}
	return 0;
}

/**
 * Casts rays all over the screen (for all levels if operated in Minecraft mode). Uses helper functions to find
 * and draw the nearest wall. The algorithm is:
 *			
 *			for each x-coordinate on the screen:
 *				start the ray from the player
 *				extend it to the nearest wall
 *				calculate the height of the wall, and the indices of the edges
 *				find the index of the texture to draw
 *				for each y-coordinate on the wall:
 *					draw the texture pixel
 *
 * @param 	main_player 	the user w.r.t. whom the field of view is set
 * @param 	main_camera 	the camera that tracks the field of view
 * @param 	new_ray 		the ray to be cast
 * @param 	mode 			the mode of operation (MAZE or MINECRAFT)
 */
void cast_rays(struct player *main_player, struct camera *main_camera, struct ray *new_ray, int mode)
{
	if (mode == MINECRAFT)
		reset_z_indices();

	for (int world_number = max_level; world_number >= 0 && world_number >= min_level ; --world_number) {
		if (mode != MINECRAFT && world_number != curr_level)
			continue;

		for(int x = 0; x < _WIDTH; x++) {
			// Scale the field of view into a -1 to 1 field so the direction maps to the cosine function
			main_camera->x = 2 * x / (double)(_WIDTH) - 1;

			// Along the x-axis, we look at the contribution of the player's direction, and the camera's direction
			// (projection of position vector r on x is (r x cos(t)))
			new_ray->x_direction = main_player->x_direction + main_camera->projection_plane_x * main_camera->x;
			// and we do the same on the y-axis
			new_ray->y_direction = main_player->y_direction + main_camera->projection_plane_y * main_camera->x;

			/* Let the direction we just calculated be cos(t), and the distance between two x-facing walls be d, then
			 * 		d cos(t) = 1
			 * since each grid square is 1 unit. So
			 *		d = 1 / cos(t)
			 * gives the distance the ray has to travel to hit another x-facing wall.
			 *
			 * We do the same for y-facing walls.
			 */
			new_ray->x_distance_between_two_sides = ABS(1 / new_ray->x_direction);
			new_ray->y_distance_between_two_sides = ABS(1 / new_ray->y_direction);

			// For precision, the main player's position is more precise than just grid squares,
			// so we also calculate the grid square we are in.
			int wall_x = (int)(main_player->x);
			int wall_y = (int)(main_player->y);

			int overall_ray_direction_x = (new_ray->x_direction < 0) ? BACKWARD : FORWARD;
			int overall_ray_direction_y = (new_ray->y_direction < 0) ? BACKWARD : FORWARD;

			/* Now we find the nearest wall (obstacle) from the given position of the main player, at the current x.
			 * Here, we initialise the distance the ray has to travel to hit the nearest wall along both x- and y-directions.
			 */
			new_ray->x_distance_to_next_wall = new_ray->x_distance_between_two_sides * ((new_ray->x_direction < 0) ? (main_player->x - wall_x) : (wall_x - main_player->x + 1.0));
			new_ray->y_distance_to_next_wall = new_ray->y_distance_between_two_sides * ((new_ray->y_direction < 0) ? (main_player->y - wall_y) : (wall_y - main_player->y + 1.0));
			
			int wall_facing;
			if (find_wall(new_ray, &wall_x, &wall_y, overall_ray_direction_x, overall_ray_direction_y, &wall_facing, world_number) == 0) {
				continue;	// if no wall was found, we are done
			}

			/* We have the x and y distance the ray had to travel to hit a wall. Now we find the overall distance. Euclidean distance,
			 *		d = sqrt(x^2 + y^2)
			 * will result in the fisehye effect -- the center will appear to bulge. So, we find the distance using the direction cos(t).
			 *
			 *		|	 /
			 *		|	/
			 *	  y |  / d
			 *		| /
			 *		|/____
			 *			x
			 *
			 * Again, d cos(t) = x
			 * so 	  d = x / cos(t)
			 *
			 * and if the wall is y-facing, then
			 *		  d = y / sin(t).
			 *
			 * Note that for simplicity of notation, we represent x-facing walls as NORTH-SOUTH facing walls, and y-facing as EAST-WEST facing walls.
			 *
			 */
			double distance_to_wall;
			if (wall_facing == NORTH_SOUTH)
				distance_to_wall = (wall_x - main_player->x + (1 - overall_ray_direction_x) / 2) / new_ray->x_direction;
			else
				distance_to_wall = (wall_y - main_player->y + (1 - overall_ray_direction_y) / 2) / new_ray->y_direction;

			// Scale the wall height according to the distance
			int wall_height = (int)(_HEIGHT / distance_to_wall);

			// The lowest point of the wall (lowest x-coordinate but highest on screen since (0,0) is at the top-left corner)
			int wall_lowest_point =  MAX(0, _HEIGHT * player_wall_height_ratio - wall_height / 2);
			// The highest point of the wall (highest x-coordinate but lowest on screen)
			int wall_highest_point = MIN(_HEIGHT - 1, _HEIGHT * player_wall_height_ratio + wall_height / 2);
			
			// Find the wall design to draw
			int wall_design_number = world_map[world_number][wall_x][wall_y] - 1; //1 subtracted from it so that texture 0 can be used!

			/* In order to draw textured walls, we need to find where in the texture the ray intersected the wall.
			 *
			 * To find the x-coordinate on the wall,
			 *		for a NORTH-SOUTH facing wall,
			 *			+---+---+
			 *			|	| \ |
			 *			+---+--\+
			 *			|	|	\		x-intersection = (original y) + (perpendicular y-distance)
			 *			+---+---+\
			 *					  \
			 *					   x
			 *
			 *		and for an EAST-WEST facing wall,
			 *				  x
			 *				 /|
			 *				/ |
			 *			   /  |
			 *			  /| \|
			 *			 / | /\			x-intersection = (original x) + (perpendicular x-distance)
			 *			/  |/ |\
			 *		   |   |  | \
			 *		   |  /| /   \
			 *		   | / |/	  x
			 *		   |/  /
			 *		   |  /
			 *		   | /
			 *		   |/
			 *
			 */
			double x_wall_intersection;
			if (wall_facing == NORTH_SOUTH)
				x_wall_intersection = main_player->y + distance_to_wall * new_ray->y_direction;
			else
				x_wall_intersection = main_player->x + distance_to_wall * new_ray->x_direction;

			// This gives us the distance within the grid square that the ray had to travel to hit the right spot on the wall
			x_wall_intersection -= floor((x_wall_intersection));

			// Now we find the x-coordinate of the actual wall design we want to use
			int x_coordinate_on_wall = (int)(x_wall_intersection * (double)(WALL_DESIGN_WIDTH));
			// If it is a NORTH or EAST facing wall, flip the coordinate
			if ((wall_facing == NORTH_SOUTH && new_ray->x_direction > 0) || (wall_facing == EAST_WEST && new_ray->y_direction < 0))
				x_coordinate_on_wall = WALL_DESIGN_WIDTH - x_coordinate_on_wall - 1;

			// Draw the current stripe
			design_wall(x, wall_lowest_point, wall_highest_point, wall_height, x_coordinate_on_wall, wall_design_number, wall_facing, world_number, distance_to_wall, mode);
		}

		if (mode == MAZE)
			break;
	}
	// Swap buffers so that we can see the changes
	gl_swap_buffer();
}

/**
 * Turns the player and the camera to the right.
 *
 * @param 	main_player 	the user w.r.t. whom the field of view is set
 * @param 	main_camera 	the camera that tracks the field of view
 */
void pan_right(struct player *main_player, struct camera *main_camera)
{
	double current_x_direction = main_player->x_direction;
	main_player->x_direction = main_player->x_direction * cos(rotational_speed) + main_player->y_direction * sin(rotational_speed);
	main_player->y_direction = - current_x_direction * sin(rotational_speed) + main_player->y_direction * cos(rotational_speed);

	double current_projection_plane_x = main_camera->projection_plane_x;
	main_camera->projection_plane_x = main_camera->projection_plane_x * cos(rotational_speed) + main_camera->projection_plane_y * sin(rotational_speed);
	main_camera->projection_plane_y = - current_projection_plane_x * sin(rotational_speed) + main_camera->projection_plane_y * cos(rotational_speed);
}

/**
 * Turns the player and the camera to the left.
 *
 * @param 	main_player 	the user w.r.t. whom the field of view is set
 * @param 	main_camera 	the camera that tracks the field of view
 */
void pan_left(struct player *main_player, struct camera *main_camera)
{
	double current_x_direction = main_player->x_direction;
	main_player->x_direction = main_player->x_direction * cos(rotational_speed) - main_player->y_direction * sin(rotational_speed);
	main_player->y_direction = current_x_direction * sin(rotational_speed) + main_player->y_direction * cos(rotational_speed);

	double current_projection_plane_x = main_camera->projection_plane_x;
	main_camera->projection_plane_x = main_camera->projection_plane_x * cos(rotational_speed) - main_camera->projection_plane_y * sin(rotational_speed);
	main_camera->projection_plane_y = current_projection_plane_x * sin(rotational_speed) + main_camera->projection_plane_y * cos(rotational_speed);
}

/**
 * Moves the player forward.
 *
 * @param 	main_player 	the user w.r.t. whom the field of view is set
 * @param 	main_camera 	the camera that tracks the field of view
 */
void move_forward(struct player *main_player, struct camera *main_camera)
{
	double new_x = main_player->x + main_player->x_direction * translational_speed;
	int current_y = (int)(main_player->y);
	if((int)new_x >= 0 && (int)new_x < MAP_WIDTH && world_map[curr_level][(int)new_x][current_y] == 0)
		main_player->x = new_x;

	int current_x = (int)(main_player->x);
	double new_y = main_player->y + main_player->y_direction * translational_speed;
	if((int)new_y >= 0 && (int)new_y < MAP_HEIGHT && world_map[curr_level][current_x][(int)new_y] == 0)
		main_player->y = new_y;
}

/**
 * Moves the player backward.
 *
 * @param 	main_player 	the user w.r.t. whom the field of view is set
 * @param 	main_camera 	the camera that tracks the field of view
 */
void move_backward(struct player *main_player, struct camera *main_camera)
{
	double new_x = main_player->x - main_player->x_direction * translational_speed;
	int current_y = (int)(main_player->y);
	if((int)new_x >= 0 && (int)new_x < MAP_WIDTH && world_map[curr_level][(int)new_x][current_y] == 0)
		main_player->x = new_x;

	int current_x = (int)(main_player->x);
	double new_y = main_player->y - main_player->y_direction * translational_speed;
	if((int)new_y >= 0 && (int)new_y < MAP_HEIGHT && world_map[curr_level][current_x][(int)new_y] == 0)
		main_player->y = new_y;
}

/**
 * Moves the player to the right.
 *
 * @param 	main_player 	the user w.r.t. whom the field of view is set
 * @param 	main_camera 	the camera that tracks the field of view
 */
void move_right(struct player *main_player, struct camera *main_camera)
{
	for (int i = 0; i < QUARTER_TURN; ++i)
		pan_right(main_player, main_camera);

	move_forward(main_player, main_camera);

	for (int i = 0; i < QUARTER_TURN; ++i)
		pan_left(main_player, main_camera);
}

/**
 * Moves the player to the left.
 *
 * @param 	main_player 	the user w.r.t. whom the field of view is set
 * @param 	main_camera 	the camera that tracks the field of view
 */
void move_left(struct player *main_player, struct camera *main_camera)
{
	for (int i = 0; i < QUARTER_TURN; ++i)
		pan_left(main_player, main_camera);

	move_forward(main_player, main_camera);

	for (int i = 0; i < QUARTER_TURN; ++i)
		pan_right(main_player, main_camera);
}

/**
 * Moves the player and the camera according to user input. it waits for the user's input, and does nothing during the wait.
 *
 * @param 	main_player 	the user w.r.t. whom the field of view is set
 * @param 	main_camera 	the camera that tracks the field of view
 * @param 	mode 			the mode of operation (MAZE or MINECRAFT)
 */
void move_player(struct player *main_player, struct camera *main_camera, int mode)
{	
	while (1) {
		char c = keyboard_read_next();
		if (c == PS2_KEY_ARROW_LEFT || c == PS2_KEY_NUMPAD_4) {
	        pan_left(main_player, main_camera);
	        break;
	    } else if (c == PS2_KEY_ARROW_RIGHT || c == PS2_KEY_NUMPAD_6) {
	        pan_right(main_player, main_camera);
	        break;
	    } else if (c == 'w' || c == PS2_KEY_ARROW_UP || c == PS2_KEY_NUMPAD_8) {
	        move_forward(main_player, main_camera);
	        break;
	    } else if (c == 's' || c == PS2_KEY_ARROW_DOWN || c == PS2_KEY_NUMPAD_2) {
	        move_backward(main_player, main_camera);
	        break;
	    } else if (c == 'a') {
	        move_left(main_player, main_camera);
	        break;
	    } else if (c == 'd') {
	        move_right(main_player, main_camera);
	        break;
	    } else if (c == ' ') {
	    	if (mode == MAZE) {
	    		player_wall_height_ratio += 0.05; // move upwards
	    	} else {
		    	if (curr_level == max_level)
		    		continue;
		    	++curr_level;
	    	}
	    	break;
	    } else if (c == '\n') {
	    	if (mode == MAZE) {
	    		player_wall_height_ratio -= 0.05; // duck
	    	} else {
		    	if (curr_level == min_level)
		    		continue;
		    	--curr_level;
		    }
	    	break;
	    } else if (c >= '1' && c <= '8' && mode == MINECRAFT) {		// add new wall
	    	int new_wall_x = (int)(main_player->x + main_player->x_direction * translational_speed);
	    	int new_wall_y = (int)(main_player->y + main_player->y_direction * translational_speed);
	    	world_map[curr_level][new_wall_x][new_wall_y] = c - '0';
	    	break;
	    } else if (c == '\b' && mode == MINECRAFT) {	// delete the wall in front (if any)
	    	int new_wall_x = (int)(main_player->x + main_player->x_direction * translational_speed);
	    	int new_wall_y = (int)(main_player->y + main_player->y_direction * translational_speed);
	    	world_map[curr_level][new_wall_x][new_wall_y] = 0;
	    	break;
	    }
	}
	gl_clear(GL_BLACK);
}
