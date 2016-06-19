#include "../include/engine.h"
#include <math.h>

int main(int argc, char* argv[]) {
	if (!createWindow(WIDTH, HEIGHT, "3D Game Engine")) {
		quit();
		return 1;
	}

	if (!initOpenGL()) {
		quit();
		return 1;
	}

    GLuint program;
	if (!compileAndAttachShaders("src/glsl/shader.vert", "src/glsl/shader.frag", &program)) {
		quit();
		return 1;
	}

	unsigned short running = 1, timePassed = 0, frames = 0, pastTime, currentTime = 0;;
	int x = -1, y = -1, deltax = 0, deltay = 0, i;
	float angle = 0.0f, verticalAngle = 0.0f, horizontalAngle = PI;
	
	GLuint MVP = glGetUniformLocation(program, "MVP");
	GLuint lightDirection = glGetUniformLocation(program, "lightDirection");
	GLuint lightColor = glGetUniformLocation(program, "lightColor");
	mat4x4 scale, translate, rotate, view, projection;

	camera C;
	// TODO: function to initialize camera with default values
	C.eye[0] = 0.0f; C.eye[1] = 2.0f; C.eye[2] = 5.0f;
	C.direction[0] = cosf(verticalAngle) * sinf(horizontalAngle); C.direction[1] = sinf(verticalAngle); C.direction[2] = cosf(horizontalAngle) * cosf(verticalAngle);
	C.right[0] = sinf(horizontalAngle - PI/2.0f); C.right[1] = 0.0f; C.right[2] =  cosf(horizontalAngle - PI/2.0f);
	vec3_mul_cross(C.up, C.right, C.direction);
	
	short meshCount = 2;
	mesh* list[meshCount];
	list[1] = initOBJMesh(argv[1], argv[2]);
	list[0] = initOBJMesh("res/obj/plane.obj", "res/textures/test.png");
	list[2] = initOBJMesh("res/obj/lamp.obj", "res/textures/test.png");

	mat4x4 M;
	for (i = 0; i < meshCount; i++) {
		mat4x4_gen_rotate(list[i]->transform.rotate, 1, 0, 0, 0); // TODO: start using linmath.c rotate function
		mat4x4_identity(list[i]->transform.scale);
		mat4x4_translate(list[i]->transform.translate, 0, 0, 0);
	}

	mat4x4_translate(list[0]->transform.translate, 0, 5, 0);
	mat4x4_gen_rotate(list[2]->transform.rotate, 0, 1, 0, 90);

	player* P = initPlayer(C.eye);
	mat4x4_perspective(projection, FOV, (float)WIDTH/(float)HEIGHT, 0.01f, 100.f);

	vec4 pastDirection;
	vec3 center, nextPosition, pastPosition; 
	vec3_add(center, C.eye, C.direction);
	mat4x4_look_at(view, C.eye, center, C.up);

	SDL_WarpMouseInWindow(window, WIDTH/2, HEIGHT/2);

	SDL_Event e;

	while (running) {
		pastTime = currentTime;
		currentTime = SDL_GetTicks();
		timePassed += (currentTime - pastTime);
		frames++;
		if (timePassed >= 1000) {
			timePassed = 0;
			printf("%d FPS\n", frames);
			frames = 0;
		}	

		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_WINDOWEVENT)
				if (e.window.event == SDL_WINDOWEVENT_CLOSE)
					running = 0;
			if (e.type == SDL_KEYDOWN)
				switch(e.key.keysym.sym) {
					case SDLK_ESCAPE:
						running = 0;
						break;
					case SDLK_q:
						running = 0;
						break;
					case SDLK_a:
						P->movement.left = 1;
						break;
					case SDLK_d:
						P->movement.right = 1;
						break;
					case SDLK_w:
						P->movement.forward = 1;
						break;
					case SDLK_s:
						P->movement.backward = 1;
						break;
					case SDLK_LSHIFT:
						printf("Running\n");
						P->movement.run = 1;
						break;
					case SDLK_j:
						angle-=90;
						mat4x4_gen_rotate(rotate, 1, 0, 0, angle);
						break;
					case SDLK_k:
						angle += 90;
						mat4x4_gen_rotate(rotate, 1, 0, 0, angle);
						break;
				}
			if (e.type == SDL_KEYUP)
				switch(e.key.keysym.sym) {
					case SDLK_a:
						P->movement.left = 0;
						break;
					case SDLK_d:
						P->movement.right = 0;
						break;
					case SDLK_w:
						P->movement.forward = 0;
						break;
					case SDLK_s:
						P->movement.backward = 0;
						break;
					case SDLK_LSHIFT:
						P->movement.run = 0;
						break;
				}
		}

		vec3 lightDir; lightDir[0] = 1.0f; lightDir[1] = 1.0f;  lightDir[2] = 1.0f; 
		vec3 lightCol; lightCol[0] = 0.9f; lightCol[1] = 1.0f;  lightCol[2] = 0.6f;  // create diffuse light .h
		glUniform3fv(lightDirection, 1, (GLfloat*)lightDir);
		glUniform3fv(lightColor, 1, (GLfloat*)lightCol);

	    SDL_GetMouseState(&x, &y);
		
		vec3_copy(pastPosition, C.eye);
		vec3_copy(nextPosition, C.eye);
		camera_fps_move_simulate(nextPosition, &C, P->movement, currentTime - pastTime);
		updateHitbox(P, nextPosition);
		if (aabb_collision(P->hitbox, list[1]->hitbox)) {
			DEBUG_PRINT(("Collision!\n"));
			vec3_copy(C.eye, pastPosition);
			updateHitbox(P, C.eye);
		}
		else {
			camera_fps_move(&C, P->movement, currentTime - pastTime);
			updateHitbox(P, C.eye);
		}

	    deltax = x - WIDTH/2;
	    deltay = y - HEIGHT/2;

	    if (deltax != 0 || deltay != 0) {
			horizontalAngle += (float)(WIDTH/2 - x) * SENSITIVITY;
			verticalAngle += (float)(HEIGHT/2 - y) * SENSITIVITY;

			if (verticalAngle > 1.5f)
				verticalAngle = 1.5f;
			else if (verticalAngle < -1.5f)
				verticalAngle = -1.5f;

			camera_fps_mouse_look(&C, horizontalAngle, verticalAngle);

			SDL_WarpMouseInWindow(window, WIDTH/2, HEIGHT/2);
	    }
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		vec3_add(center, C.eye, C.direction);
		mat4x4_look_at(view, C.eye, center, C.up);

		mat4x4 model_view_projection;
		
		//mat4x4_translate(list[0]->transform.translate, 0, 0, C.eye[2]);

		for (i = 0; i < meshCount; i++) {
			//mat4x4_mul(model_view, list[i]->transform.rotate, list[i]->transform.scale);
			mat4x4_mul (model_view_projection, list[i]->transform.translate, list[i]->transform.rotate);
			mat4x4_mul (model_view_projection, view, model_view_projection);
			mat4x4_mul (model_view_projection, projection, model_view_projection);

			glUniformMatrix4fv(MVP, 1, 0, (GLfloat*)model_view_projection);

			draw(list[i]);
		}

		SDL_GL_SwapWindow(window);
		SDL_Delay(1);
	}

	quit();

	return 0;
}
