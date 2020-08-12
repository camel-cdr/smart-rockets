#define STR_BUFFER_LENGTH 1024

static GLuint compileShaders(char const *source, GLenum type)
{
	int success;
	static char buf[STR_BUFFER_LENGTH];
	GLuint shader;

	GL_CALL(shader = glCreateShader(type));
	GL_CALL(glShaderSource(shader, 1, (char const * const*)&source, NULL));
	GL_CALL(glCompileShader(shader));

	GL_CALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
	if (!success) {
		glGetShaderInfoLog(shader, STR_BUFFER_LENGTH, NULL, buf);
		die("Shader compilation failed!\n\n%s\n", buf);
	}
	return shader;
}

static GLuint createShader(char const *vertexSrc, char const *fragSrc)
{
	int success;
	static char buf[STR_BUFFER_LENGTH];
	GLuint program, vs, fs;

	program = glCreateProgram();
	vs = compileShaders(vertexSrc, GL_VERTEX_SHADER);
	fs = compileShaders(fragSrc, GL_FRAGMENT_SHADER);

	GL_CALL(glAttachShader(program, vs));
	GL_CALL(glAttachShader(program, fs));
	GL_CALL(glLinkProgram(program));
	GL_CALL(glValidateProgram(program));

	GL_CALL(glGetProgramiv(program, GL_LINK_STATUS, &success));
	if (!success) {
		glGetProgramInfoLog(program, STR_BUFFER_LENGTH, NULL, buf);
		die("Linking shader failed!\n\n%s\n", buf);
	}
	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

static float fast_atan2(float y, float x)
{
	static float const c1 = PI32 / 4.0;
	static float const c2 = PI32 * 3.0 / 4.0;
	float absy, angle;
	if (y == 0 && x == 0)
		return 0;
	absy = fabs(y);
	if (x >= 0)
		angle = c1 - c1 * ((x - absy) / (x + absy));
	else
		angle = c2 - c1 * ((x + absy) / (absy - x));
	if (y < 0)
		return -angle;
	return angle;
}



static u32 s[4] = { 0x7ba00338, 0x923ee159, 0x92b0d236, 0x5ae756dc };

static u32 rng_next(void)
{
	const u32 result = s[0] + s[3];

	const u32 t = s[1] << 9;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;

	s[3] = (s[3] << 11) | (s[3] >> 31);

	return result;
}


/* This is the jump function for the generator. It is equivalent
   to 2^64 calls to rng_next(); it can be used to generate 2^64
   non-overlapping subsequences for parallel computations. */

static void rng_seed(void)
{
	static const u32 JUMP[] = { 0x8764000b, 0xf542d2d3, 0x6fa035c3, 0x77f2db5b };

	u32 s0 = 0;
	u32 s1 = 0;
	u32 s2 = 0;
	u32 s3 = 0;
	for (size_t i = 0; i < (sizeof JUMP / sizeof *JUMP); i++)
		for (uint32_t b = 0; b < 32; b++) {
			if (JUMP[i] & UINT32_C(1) << b) {
				s0 ^= s[0];
				s1 ^= s[1];
				s2 ^= s[2];
				s3 ^= s[3];
			}
			rng_next();
		}

	s[0] = s0 + time(NULL);
	s[1] = s1;
	s[2] = s2;
	s[3] = s3;
}


