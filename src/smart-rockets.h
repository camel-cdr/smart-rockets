typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define MOVE_COUNT 500
#define MAX_POPULATION_SIZE 500000
#define MAX_ACCELERATION 0.001f

#define ROCKET_SIZE "0.05"
#define OBSTACLE_SIZE 0.05f

#define MAX(a, b) (((a)>(b))?(a):(b))
#define CLAMP(min, v, max) (((v)<(min))?(min):(((v)>(max))?(max):(v)))
#define ARRAY_SIZE(a) (sizeof(a) / sizeof*(a))

/* math */
typedef struct { float x, y; } Vec2;
typedef struct { float x, y, z; } Vec3;
typedef struct { float x, y, z, w; } Vec4;

typedef struct {
	float m00, m01, m02, m03;
	float m10, m11, m12, m13;
	float m20, m21, m22, m23;
	float m30, m31, m32, m33;
} Mat4;

static Mat4 mat4_translation(Vec3 translation);
static Mat4 mat4_orthographic(
		float left, float right,
		float bottom, float top,
		float near, float far);

/* population */
struct Population {
	Vec3 *pos;
	Vec2 *vel;
	float *fit;
	size_t *sorted;
	Vec2 *(acc[MOVE_COUNT]);
	GLuint ivbo, vbo, vao, shader;
	/* adjustable in gui */
	size_t count;
	size_t count_slider;
	float mutationrate;
	struct nk_colorf col;
};
static void pop_init(struct Population *this);
static void pop_new_gen(struct Population *this, Vec2 target);
static void pop_update(struct Population *this, size_t move, size_t num_obs, Vec4 *obs, Vec4 tarobs);
static void pop_render(struct Population *this, Mat4 proj, Mat4 view);

/* symulation */
struct Sym {
	Vec2 target;
	size_t move;
	struct Population pop[2];
	int isrunning;
	size_t skip_slider;
	/* cam */
	Vec3 pos;
	float zoom;

	size_t num_obs;
	Vec4 *obs;
	Vec4 tarobs;
	GLuint ivbo, vbo, vao, shader;
};
static void sym_init(void);
static void sym_update(float dt);
static void sym_render(float width, float height, int isMouseDown);

/* util */
static GLuint compileShaders(char const *source, GLenum type);
static GLuint createShader(char const *vertexSrc, char const *fragSrc);
static u32 rng_next(void);
static void rng_seed(void);
static float fast_atan2(float y, float x);

