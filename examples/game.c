#include "include/c3d.h"
#include <math.h>

int main(int argc, char* argv[]) {
	SDL_Window* window = window_create(WIDTH, HEIGHT, "Testing");
	if (window == NULL) {
		c3d_quit();
		return EXIT_FAILURE;
	}

	if (!initOpenGL()) {
		c3d_quit();
		return EXIT_FAILURE;
	}
	
	/* Attach and compile shaders */
	shader *S = shader_loadFromFile("src/glsl/shader.vert", "src/glsl/shader.frag", PHONG);
	
	if (S == NULL) {
		c3d_quit();
		return 1;
	}

	SDL_Event e;
	bool running = true, mouseGrab = true;
	float verticalAngle = 0, horizontalAngle = C3D_PI, pastTime = 0, currentTime = 0, frameTime = 0;
	int mouse_x, mouse_y;
	
	vec3 init_cam_position = {0, 1.5f, -2.0f};
	camera *C = camera_init(init_cam_position, horizontalAngle, verticalAngle);
	mat4x4 projection;
	mat4x4_perspective(projection, FOV, (float)WIDTH/(float)HEIGHT, 0.001f, 1000.f);

	vec3 lightColor = {1.0f, 1.0f, 1.0f}, lightPosition = {0.0f, 35.0f, 0.0f};

	ambientLight *ambient = initAmbientLight(lightColor, 1.0f);
	setAmbientLight(ambient, S);

	level* mainLevel = level_loadMeshes("res/maps/map.bmp");
	list_insert(mainLevel->meshList, mesh_genFlatFloor(mainLevel->size, "res/textures/grass.jpg")); // CHECK FOR ERRORS

	linkedList* meshList = list_create();
	mesh_loadFromFileToList("res/obj/raptor.obj", "res/textures/raptor.png", meshList);

	Movement* movement = (Movement*)malloc(sizeof(Movement));
	movement->forward = movement->backward = movement->right = movement->left = false;
	
	// SDL_WarpMouseInWindow(window, WIDTH/2, HEIGHT/2);

	while (running) {
		pastTime = currentTime;
		currentTime = SDL_GetTicks();
		frameTime = (currentTime - pastTime);

		unsigned int forward = 0;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE)
				running = false;
			if (e.type == SDL_KEYDOWN)
				switch(e.key.keysym.sym) {
					case SDLK_ESCAPE:
						mouseGrab = !mouseGrab;
						window_grabCursor(window, mouseGrab);
						break;
					case SDLK_q:
						running = false;
						break;
					case SDLK_w:
						movement->forward = true;
						break;
					case SDLK_s:
						movement->backward = true;
						break;
					case SDLK_d:
						movement->right = true;
						break;
					case SDLK_a:
						movement->left = true;
						break;
				}
			else if (e.type == SDL_KEYUP)
				switch(e.key.keysym.sym) {
					case SDLK_w:
						movement->forward = false;
						break;
					case SDLK_s:
						movement->backward = false;
						break;
					case SDLK_d:
						movement->right = false;
						break;
					case SDLK_a:
						movement->left = false;
						break;
				}
		}

		/* FPS camera control */
		SDL_GetMouseState(&mouse_x, &mouse_y);

		float deltax = mouse_x - WIDTH/2;
		float deltay = mouse_y - HEIGHT/2;
		
		if ((deltax != 0 || deltay != 0) && mouseGrab) {
			horizontalAngle += (float)(WIDTH/2 - mouse_x) * SENSITIVITY;
			verticalAngle += (float)(HEIGHT/2 - mouse_y) * SENSITIVITY;
		
			if (verticalAngle > C3D_PI/2.0f)
				verticalAngle = C3D_PI/2.0f;
			else if (verticalAngle < -C3D_PI/2.0f)
				verticalAngle = -C3D_PI/2.0f;
		
			SDL_WarpMouseInWindow(window, WIDTH/2, HEIGHT/2);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		camera_angle(C, horizontalAngle, verticalAngle);
		camera_move(C, movement, 0.1 * frameTime);
		camera_update(C);

		mesh_drawList(mainLevel->meshList, S, C, projection);
	 	mesh_drawList(meshList, S, C, projection);

		SDL_GL_SwapWindow(window);
		SDL_Delay(1);
	}

	// TODO: Cleanup function
	free(movement);
	free(C);
	free(S);
	free(mainLevel);
	free(meshList);
	free(ambient);

	c3d_quit();

	return EXIT_SUCCESS;
}
