#include "camera.h"
#include "engine.h"
#include <math.h>

void camera_fps_mouse_look(camera* C, float horizontalAngle, float verticalAngle) {
	C->direction[0] = cosf(verticalAngle) * sinf(horizontalAngle);
	C->direction[1] = sinf(verticalAngle);
	C->direction[2] = cosf(verticalAngle) * cosf(horizontalAngle);

	C->right[0] = sinf(horizontalAngle - PI/2.0f);
	C->right[1] = 0;
	C->right[2] = cosf(horizontalAngle - PI/2.0f);

	vec3_mul_cross(C->up, C->right, C->direction);
}

void camera_fps_move(camera* C, unsigned short mvup, unsigned short mvdown, unsigned short mvleft, unsigned short mvright) {
	if (mvup)
		vec3_add(C->eye, C->eye, C->direction);
	if (mvdown)
		vec3_sub(C->eye, C->eye, C->direction);
	if (mvright)
		vec3_add(C->eye, C->eye, C->right);
	if (mvleft)
		vec3_sub(C->eye, C->eye, C->right);
}