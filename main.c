#include "gl.h"
#include "keyboard.h"
#include "textures.h"
#include "raycaster.h"
#include "world.h"
#include "interrupts.h"
#include "ps2.h"
#include "system.h"

#define _HEIGHT 480
#define _WIDTH  640

#define MAZE 0
#define MINECRAFT 1

const int WIDTH  = 640;
const int HEIGHT = 480;

int mode = MAZE;

/**
 * Runs a simple menu for the user to select the mode.
 */
void intro_screen(void)
{
	gl_draw_string(300, 200, "MAZE", GL_WHITE);
	gl_draw_string(265, 230, "MINECRAFT", GL_WHITE);

	while (1) {
		// Move cursor
		if (mode == MAZE) {
			gl_draw_string(270, 200, ">", GL_WHITE);
			gl_draw_string(235, 230, ">", GL_BLACK);
		} else {
			gl_draw_string(270, 200, ">", GL_BLACK);
			gl_draw_string(235, 230, ">", GL_WHITE);
		}
		gl_swap_buffer();

		char c = keyboard_read_next();
		if (c == PS2_KEY_ARROW_UP || c == PS2_KEY_NUMPAD_8 || c == PS2_KEY_ARROW_DOWN || c == PS2_KEY_NUMPAD_2) { // change mode
			mode = !mode;
		} else if (c == '\n') {	// launch current mode
			break;
		}
	}
}

/**
 * Main function: initializes all helper modules and launches the raycaster.
 */
int main(void)
{
	struct player main_player;
	main_player.x = 3;
	main_player.y = 6;
	main_player.x_direction = -1;
	main_player.y_direction = 0;

	struct camera main_camera;
	main_camera.projection_plane_x = 0;
	main_camera.projection_plane_y = 0.66;

	// Initialization routine
	gl_init(_WIDTH, _HEIGHT, 1);
	world_init();
	textures_init();
	keyboard_init();
	interrupts_global_enable();
	system_enable_cache();

	// Get choice of mode from user
	intro_screen();
	
	// Launch raycasting module
	struct ray new_ray;
	while (1) {
		cast_rays(&main_player, &main_camera, &new_ray, mode);
		move_player(&main_player, &main_camera, mode);
	}
}
