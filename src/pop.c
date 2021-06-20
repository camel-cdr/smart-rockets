static Vec2 *(buf_acc[MOVE_COUNT]) = {NULL};
static size_t buf_count = 0;

static void HSVtoRGB(int H, float S, float V, float *r, float *g, float *b)
{
	const float C = S * V;
	const float X = C * (1 - fabs(fmod(H / 60.0, 2) - 1));
	const float m = V - C;

	if (H >= 0 && H < 60) {
		*r = C + m;
		*g = X + m;
		*b = 0 + m;
	} else if (H >= 60 && H < 120) {
		*r = X + m;
		*g = C + m;
		*b = 0 + m;
	} else if (H >= 120 && H < 180) {
		*r = 0 + m;
		*g = C + m;
		*b = X + m;
	} else if (H >= 180 && H < 240) {
		*r = 0 + m;
		*g = X + m;
		*b = C + m;
	} else if (H >= 240 && H < 300) {
		*r = X + m;
		*g = 0 + m;
		*b = C + m;
	} else {
		*r = C + m;
		*g = 0 + m;
		*b = X + m;
	}
}
static void pop_init(struct Population *this)
{
	static float const rocketVertecies[] = {
		-0.5, -1.0,
		0.5, -1.0,
		0.0,  1.0
	};
	memset(this, 0, sizeof *this);
	HSVtoRGB(rng_next() % 360,
	         (float)rng_next() / UINT32_MAX * 0.5f + 0.5f,
	         (float)rng_next() / UINT32_MAX * 0.5f + 0.5f,
	         &this->col.r, &this->col.g, &this->col.b);
	this->col.a = 0.5;
	this->count_slider = 0;
	this->mutationrate = 0;

	/* shader */
	this->shader = createShader(
	"#version 330 \n"
	"layout (location = 0) in vec2 pos; \n"
	"layout (location = 1) in vec3 offset; \n"
	"uniform mat4 proj; \n"
	"uniform mat4 view; \n"
	"uniform vec4 color; \n"
	"out vec4 vertColor; \n"
	"\n"
	"void main() \n"
	"{ \n"
	"	const vec3 scale = vec3("ROCKET_SIZE"); \n"
	"	float Sin = sin(offset.z), Cos = cos(offset.z);\n"
	"	mat4 model = mat4( \n"
	"		vec4(scale.x * Cos, scale.x * -Sin, 0.0,     0.0), \n"
	"		vec4(scale.y * Sin, scale.y *  Cos, 0.0,     0.0), \n"
	"		vec4(0.0,           0.0,            scale.z, 0.0), \n"
	"		vec4(offset.x,      offset.y,       1.0,     1.0) \n"
	"	); \n"
	"	gl_Position = proj * view * model * vec4(pos, 0.0, 1.0); \n"
	"	vertColor = (offset.z < -6.0 ? vec4(0, 1, 0, color.a) : \n"
	"	             (offset.z > 6.0 ? vec4(1, 0, 0, color.a) : color)); \n"
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
	GL_CALL(glUseProgram(this->shader));

	/* Buffers */
	GL_CALL(glGenVertexArrays(1, &this->vao));
	GL_CALL(glGenBuffers(1, &this->ivbo));
	GL_CALL(glGenBuffers(1, &this->vbo));

	GL_CALL(glBindVertexArray(this->vao));

	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER,
	                     sizeof(rocketVertecies),
	                     rocketVertecies,
	                     GL_STATIC_DRAW));
	GL_CALL(glEnableVertexAttribArray(0));
	GL_CALL(glVertexAttribPointer(
				0, 2, GL_FLOAT, GL_FALSE,
				2 * sizeof(float), (void*)0));

	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->ivbo));
	GL_CALL(glEnableVertexAttribArray(1));
	GL_CALL(glVertexAttribPointer(
				1, 3, GL_FLOAT, GL_FALSE,
				3 * sizeof(float), (void*)0));
	GL_CALL(glVertexAttribDivisor(1, 1));

	{
		Vec2 tmp = {0};
		pop_new_gen(this, tmp);
	}

}
static size_t turnament_selection(struct Population *pop)
{
	size_t i, best = 0;
	float max = 0;
	for (i = 0; i < pop->count_slider / 10; ++i) {
		size_t idx = rng_next() % pop->count;
		if (pop->fit[idx] > max) {
			max = pop->fit[idx];
			best = idx;
		}
	}

	return best;
}
static void pop_new_gen(struct Population *this, Vec2 target)
{
	size_t m, r;


	/* Copy this-> acc to buf_acc */
	if (this->count > buf_count) {
		buf_count = this->count;
		for (m = 0; m < MOVE_COUNT; ++m) {
			buf_acc[m] = realloc(
					buf_acc[m],
					buf_count * sizeof *buf_acc[m]);
		}
	}
	for (m = 0; m < MOVE_COUNT; ++m) {
		for (r = 0; r < this->count; ++r) {
			buf_acc[m][r] = this->acc[m][r];
		}
	}

	if (this->count > 2) {
		/* assign fitness scores */
		for (r = 0; r < this->count; ++r) {
			if (this->fit[r] < 1.0f) {
				float dist = powf(this->pos[r].x - target.x, 2.0) + powf(this->pos[r].y - target.y, 2.0);
				this->fit[r] = 1.0f/dist;
			} else {
				this->fit[r] = powf(this->fit[r], this->fit[r]);
			}
		}

		/* accept reject */
		for (r = 0; r < this->count; ++r) {
			size_t parentA = turnament_selection(this);
			size_t parentB = turnament_selection(this);
			size_t split = rng_next() % MOVE_COUNT;
			for (m = 0; m < split; ++m) {
				this->acc[m][r] = buf_acc[m][parentA];
			}
			for (m = split; m < MOVE_COUNT; ++m) {
				this->acc[m][r] = buf_acc[m][parentB];
			}
		}

		/* mutate */
		for (r = 0; r < this->count; ++r) {
			for (m = 0; m < MOVE_COUNT; ++m) {
				if ((float)rng_next() / UINT32_MAX * 100.0 < this->mutationrate) {
					this->acc[m][r].x = ((float)rng_next() / UINT32_MAX * 2 - 1) * MAX_ACCELERATION;
					this->acc[m][r].y = ((float)rng_next() / UINT32_MAX * 2 - 1) * MAX_ACCELERATION;
				}
			}
		}
	}

	/* resize and randomize dna(acc) between count and count_slider */

	for (m = 0; m < MOVE_COUNT; ++m) {
		this->acc[m] = realloc(
				this->acc[m], this->count_slider * sizeof *this->acc[m]);
		for (r = this->count; r < this->count_slider; ++r) {
			this->acc[m][r].x = ((float)rng_next() / UINT32_MAX * 2 - 1) * MAX_ACCELERATION;
			this->acc[m][r].y = ((float)rng_next() / UINT32_MAX * 2 - 1) * MAX_ACCELERATION;
		}
	}

	this->pos = realloc(this->pos, sizeof *this->pos * this->count_slider);
	this->vel = realloc(this->vel, sizeof *this->vel * this->count_slider);
	this->fit = realloc(this->fit, sizeof *this->fit * this->count_slider);
	for (r = 0; r < this->count_slider; ++r) {
		this->pos[r].x = this->pos[r].y = this->pos[r].z = 0;
		this->vel[r].x = this->vel[r].y = this->fit[r] = 0;
	}


	this->count = this->count_slider;

	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->ivbo));

	GL_CALL(glBufferData(GL_ARRAY_BUFFER,
	                     sizeof *this->pos * this->count,
	                     this->pos,
	                     GL_DYNAMIC_DRAW));
}
static void pop_update(struct Population *this, size_t move, size_t num_obs, Vec4 *obs, Vec4 tarobs)
{
	size_t i, j;
	for (i = 0; i < this->count; ++i) {
		Vec3 *pos = &this->pos[i];
		Vec2 *vel = &this->vel[i];
		float *fit = &this->fit[i];
		Vec2 *acc = &this->acc[move][i];

		if (*fit != 0)
			continue;

		if (fabs(pos->x - tarobs.x) < tarobs.z && fabs(pos->y - tarobs.y) < tarobs.w) {
			*fit = (float)MOVE_COUNT - move;
			pos->z -= 4 * PI32;
			continue;
		}

		/* The first obstical is the target */
		for (j = 1; j < num_obs; ++j) {
			if (fabs(pos->x - obs[j].x) < obs[j].z && fabs(pos->y - obs[j].y) < obs[j].w) {
				*fit = -1.0;
				pos->z += 4 * PI32;
				break;
			}
		}
		if (*fit != 0)
			continue;

		vel->x += acc->x;
		vel->y += acc->y;
		pos->z = fast_atan2(vel->y, vel->x);
		pos->y += vel->x;
		pos->x += vel->y;
	}
}
static void pop_render(struct Population *this, Mat4 proj, Mat4 view)
{
	GL_CALL(glUseProgram(this->shader));
	GL_CALL(glBindVertexArray(this->vao));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->ivbo));

	GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0,
	                        sizeof *this->pos * this->count,
	                        this->pos));

	glUniformMatrix4fv(glGetUniformLocation(this->shader, "proj"),
	                                        1, GL_FALSE, &proj.m00);
	glUniformMatrix4fv(glGetUniformLocation(this->shader, "view"),
	                                        1, GL_FALSE, &view.m00);
	glUniform4f(glGetUniformLocation(this->shader, "color"),
	            this->col.r, this->col.g, this->col.b, this->col.a);
	GL_CALL(glDrawArraysInstanced(GL_TRIANGLES, 0, 3, this->count));
}
