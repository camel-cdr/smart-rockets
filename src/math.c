
static Mat4
mat4_translation(Vec3 translation)
{
	Mat4 ret = {0};
	ret.m00 = 1;
	ret.m11 = 1;
	ret.m22 = 1;
	ret.m33 = 1;
	ret.m30 = translation.x;
	ret.m31 = translation.y;
	ret.m32 = translation.z;
	return ret;
}

static Mat4
mat4_orthographic(
		float left, float right,
		float bottom, float top,
		float near, float far)
{
	Mat4 ret = {0};
	ret.m00 = 2 / (right - left);
	ret.m11 = 2 / (top - bottom);
	ret.m22 = 2 / (near - far);
	ret.m33 = 1;

	ret.m30 = (left + right) / (left - right);
	ret.m31 = (bottom + top) / (bottom - top);
	ret.m32 = (far + near) / (near - far);
	return ret;
}
