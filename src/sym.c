
static struct Sym sym = {
	.target = {1.0, 1.0},
	.pos = {0, 0, 0.0},
	.zoom = 1.0,
	.move = MOVE_COUNT
};

static void sym_init(void)
{
	static float const obsVertecies[] = {
		 1.0f,  1.0f,
		 1.0f, -1.0f,
		-1.0f, -1.0f,
		-1.0f, -1.0f,
		-1.0f,  1.0f,
		 1.0f,  1.0f,
	};

	size_t i;
	rng_seed();
	for (i = 0; i < ARRAY_SIZE(sym.pop); ++i) {
		pop_init(&sym.pop[i]);
	}

	sym.skip_slider = 1;
	sym.num_obs = 150;
	sym.obs = malloc(sym.num_obs * sizeof *sym.obs);
	/* The first obstical is the target */
	sym.tarobs.x = sym.target.x - OBSTACLE_SIZE;
	sym.tarobs.y = sym.target.y - OBSTACLE_SIZE;
	sym.tarobs.z = OBSTACLE_SIZE;
	sym.tarobs.w = OBSTACLE_SIZE;
	for (i = 0; i < sym.num_obs; ++i) {
		sym.obs[i].x = (float)rng_next() / UINT32_MAX * 3.0f;
		sym.obs[i].y = (float)rng_next() / UINT32_MAX * 3.0f;
		if (rng_next() % 2)
			sym.obs[i].x += 0.2;
		else
			sym.obs[i].y += 0.2;
		sym.obs[i].x *= (rng_next() % 2) ? 1 : -1;
		sym.obs[i].y *= (rng_next() % 2) ? 1 : -1;

		sym.obs[i].z = (float)rng_next() / UINT32_MAX * 0.1 + 0.05;
		sym.obs[i].w = (float)rng_next() / UINT32_MAX * 0.1 + 0.05;
	}

	sym.shader = createShader(
	"#version 330 \n"
	"layout (location = 0) in vec2 pos; \n"
	"layout (location = 1) in vec4 rect; \n"
	"out vec4 vertColor; \n"
	"uniform mat4 proj; \n"
	"uniform vec4 color; \n"
	"uniform mat4 view; \n"
	"\n"
	"void main() \n"
	"{ \n"
	"	gl_Position = proj * view * vec4(pos * rect.zw + rect.xy, 0.0, 1.0); \n"
	"	vertColor = color; \n"
	"} \n"
	,
	"#version 330 \n"
	"out vec4 fragColor; \n"
	"in vec4 vertColor; \n"
	"\n"
	"void main() \n"
	"{ \n"
	"	fragColor = vertColor; \n"
	"} \n"
	);
	GL_CALL(glUseProgram(sym.shader));

	/* Buffers */
	GL_CALL(glGenVertexArrays(1, &sym.vao));
	GL_CALL(glGenBuffers(1, &sym.ivbo));
	GL_CALL(glGenBuffers(1, &sym.vbo));

	GL_CALL(glBindVertexArray(sym.vao));

	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, sym.vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER,
	                     sizeof(obsVertecies),
	                     obsVertecies,
	                     GL_STATIC_DRAW));
	GL_CALL(glEnableVertexAttribArray(0));
	GL_CALL(glVertexAttribPointer(
				0, 2, GL_FLOAT, GL_FALSE,
				2 * sizeof(float), (void*)0));

	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, sym.ivbo));
	GL_CALL(glEnableVertexAttribArray(1));
	GL_CALL(glVertexAttribPointer(
				1, 4, GL_FLOAT, GL_FALSE,
				4 * sizeof(float), (void*)0));
	GL_CALL(glVertexAttribDivisor(1, 1));

	GL_CALL(glBufferData(GL_ARRAY_BUFFER,
	                     sizeof *sym.obs * sym.num_obs,
	                     sym.obs,
	                     GL_DYNAMIC_DRAW));
}

static void sym_update_ui(float dt)
{
	size_t i;
	size_t newcount;

	if (!nk_begin(nkctx, "Controll", nk_rect(10, 10, 230, 250),
	              NK_WINDOW_MOVABLE |
	              NK_WINDOW_BORDER | NK_WINDOW_SCALABLE |
	              NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE)) {
		nk_end(nkctx);
		return;
	}

	nk_layout_row_dynamic(nkctx, 20, 2);
	nk_value_float(nkctx, "dt = ", dt);

	nk_layout_row_dynamic(nkctx, 20, 2);
	if (nk_button_label(nkctx, "Start"))
		sym.isrunning = 1;
	if (nk_button_label(nkctx, "Pause"))
		sym.isrunning = 0;

	nk_layout_row_dynamic(nkctx, 20, 2);
	if (nk_button_label(nkctx, "Reset")) {
		for (i = 0; i < ARRAY_SIZE(sym.pop); ++i) {
			sym.pop[i].count = 0;
		}
	}

	if (nk_button_label(nkctx, "Skip")) {
		size_t i, j;
		for (i = 0; i < sym.skip_slider; ++i) {
			for (; sym.move < MOVE_COUNT; ++sym.move) {
				for (j = 0; j < ARRAY_SIZE(sym.pop); ++j) {
					pop_update(&sym.pop[j], sym.move, sym.num_obs, sym.obs, sym.tarobs);
				}
			}
			for (j = 0; j < ARRAY_SIZE(sym.pop); ++j) {
				pop_new_gen(&sym.pop[j], sym.target);
			}
			sym.move = 0;

		}
	}

	nk_layout_row_dynamic(nkctx, 20, 1);
	sym.skip_slider = nk_propertyi(nkctx, "#skip amount:", 1, sym.skip_slider, 1000, 1, 5.0);

	for (i = 0; i < ARRAY_SIZE(sym.pop); ++i) {
		nk_layout_row_dynamic(nkctx, 20, 1);
		if (nk_combo_begin_color(nkctx, nk_rgb_cf(sym.pop[i].col), nk_vec2(nk_widget_width(nkctx), 400))) {
			nk_layout_row_dynamic(nkctx, 120, 1);
			sym.pop[i].col = nk_color_picker(nkctx, sym.pop[i].col, NK_RGBA);
			nk_combo_end(nkctx);
		}

		nk_layout_row_dynamic(nkctx, 20, 1);
		newcount = nk_propertyi(nkctx, "#Population size:", 0, sym.pop[i].count_slider, MAX_POPULATION_SIZE, 100, 5.0);
		if (newcount != sym.pop[i].count_slider) {
			sym.pop[i].count_slider = newcount;
		}

		nk_layout_row_dynamic(nkctx, 20, 1);
		sym.pop[i].mutationrate = nk_propertyf(nkctx, "#Mutation rate:", 0, sym.pop[i].mutationrate, 100.0, 0.1, 0.025);
	}
	nk_end(nkctx);
}

static void sym_update(float dt)
{
	size_t i;
	const Uint8* kbd = SDL_GetKeyboardState(NULL);

	dt *= 35.0;

	if (kbd[SDL_SCANCODE_J])
		sym.zoom -= dt * sym.zoom;
	if (kbd[SDL_SCANCODE_K])
		sym.zoom += dt * sym.zoom;

	if (kbd[SDL_SCANCODE_W])
		sym.pos.y -= dt * sym.zoom;
	if (kbd[SDL_SCANCODE_S])
		sym.pos.y += dt * sym.zoom;

	if (kbd[SDL_SCANCODE_A])
		sym.pos.x += dt * sym.zoom;
	if (kbd[SDL_SCANCODE_D])
		sym.pos.x -= dt * sym.zoom;

	if (kbd[SDL_SCANCODE_T])
		sym.isrunning = !sym.isrunning;

	sym_update_ui(dt);
	if (!sym.isrunning)
		return;

	if (++sym.move >= MOVE_COUNT - 1) {
		sym.move = 0;
		float beg = (float)clock() / CLOCKS_PER_SEC;
		for (i = 0; i < ARRAY_SIZE(sym.pop); ++i) {
			pop_new_gen(&sym.pop[i], sym.target);
		}
		printf("newgen took %f\n", ((float)clock() / CLOCKS_PER_SEC)- beg);
	}

	for (i = 0; i < ARRAY_SIZE(sym.pop); ++i) {
		pop_update(&sym.pop[i], sym.move, sym.num_obs, sym.obs, sym.tarobs);
	}
}

static void sym_render(float width, float height, int isMouseDown)
{
	size_t i;
	Mat4 proj;
	int x, y;
	float aspect;

	SDL_GetMouseState(&x, &y);

	if (width > height) {
		aspect = width / height;
		if (isMouseDown) {
			sym.target.x = ((float)x / width * 2.0 - 1.0) * aspect * sym.zoom - sym.pos.x;
			sym.target.y =  -((float)y / height * 2.0 - 1.0) * sym.zoom - sym.pos.y;
		}
		proj = mat4_orthographic(
				-aspect * sym.zoom, aspect * sym.zoom,
				-1 * sym.zoom, 1 * sym.zoom,
				-10.0, 10.0);
	} else {
		aspect = height / width;
		if (isMouseDown) {
			sym.target.x = ((float)x / width * 2.0 - 1.0) * sym.zoom - sym.pos.x;
			sym.target.y =  -((float)y / height * 2.0 - 1.0) * aspect * sym.zoom - sym.pos.y;
		}
		proj = mat4_orthographic(
				-1 * sym.zoom, 1 * sym.zoom,
				-aspect * sym.zoom, aspect * sym.zoom,
				-10.0, 10.0);
	}
	sym.tarobs.x = sym.target.x - OBSTACLE_SIZE;
	sym.tarobs.y = sym.target.y - OBSTACLE_SIZE;
	Mat4 view = mat4_translation(sym.pos);

	GL_CALL(glUseProgram(sym.shader));
	GL_CALL(glBindVertexArray(sym.vao));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, sym.ivbo));

	glUniformMatrix4fv(glGetUniformLocation(sym.shader, "proj"),
	                                        1, GL_FALSE, &proj.m00);
	glUniformMatrix4fv(glGetUniformLocation(sym.shader, "view"),
	                                        1, GL_FALSE, &view.m00);

	GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof sym.tarobs, &sym.tarobs));
	glUniform4f(glGetUniformLocation(sym.shader, "color"), 0, 1, 0, 1);
	GL_CALL(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1));


	GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof *sym.obs * sym.num_obs, sym.obs));
	glUniform4f(glGetUniformLocation(sym.shader, "color"), 1, 1, 1, 1);
	GL_CALL(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, sym.num_obs));


	for (i = 0; i < ARRAY_SIZE(sym.pop); ++i) {
		pop_render(&sym.pop[i], proj, view);
	}
	nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERT_BUF, MAX_ELEM_BUF);
}

