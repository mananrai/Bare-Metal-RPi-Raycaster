// The main player
struct player {
	double x;
	double y;
	double x_direction;
	double y_direction;
};

// The camera (represents the Field of View of the main player)
struct camera {
	double x;
	double y;
	double projection_plane_x;
	double projection_plane_y;
};

// A single ray
struct ray {
	double x_direction;
	double y_direction;
	double x_distance_between_two_sides;
	double y_distance_between_two_sides;
	double x_distance_to_next_wall;
	double y_distance_to_next_wall;
};

// Draw walls on the screen using raycasting
void cast_rays(struct player *main_player, struct camera *main_camera, struct ray *new_ray, int mode);

// Move the player around
void move_player(struct player *main_player, struct camera *main_camera, int mode);