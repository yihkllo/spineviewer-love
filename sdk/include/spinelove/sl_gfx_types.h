#ifndef SPINELOVE_SL_GFX_TYPES_H_
#define SPINELOVE_SL_GFX_TYPES_H_

#include <cmath>

struct SlVec2
{
	float x = 0.0f;
	float y = 0.0f;

	constexpr SlVec2() noexcept = default;
	constexpr SlVec2(float inX, float inY) noexcept
		: x(inX), y(inY)
	{
	}
};

struct SlVec3
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	constexpr SlVec3() noexcept = default;
	constexpr SlVec3(float inX, float inY, float inZ) noexcept
		: x(inX), y(inY), z(inZ)
	{
	}
};

struct SlVec4
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 0.0f;

	constexpr SlVec4() noexcept = default;
	constexpr SlVec4(float inX, float inY, float inZ, float inW) noexcept
		: x(inX), y(inY), z(inZ), w(inW)
	{
	}
};

struct SlRect
{
	float x = 0.0f;
	float y = 0.0f;
	float w = 0.0f;
	float h = 0.0f;

	constexpr SlRect() noexcept = default;
	constexpr SlRect(float inX, float inY, float inW, float inH) noexcept
		: x(inX), y(inY), w(inW), h(inH)
	{
	}
};

struct SlColor
{
	float r = 1.0f;
	float g = 1.0f;
	float b = 1.0f;
	float a = 1.0f;

	constexpr SlColor() noexcept = default;
	constexpr SlColor(float inR, float inG, float inB, float inA) noexcept
		: r(inR), g(inG), b(inB), a(inA)
	{
	}
};

struct SlMatrix4
{
	float m[4][4] = {};
};

struct SlVertex2D
{
	SlVec3 pos;
	float rhw = 1.0f;
	SlColor color;
	SlVec2 uv;
};

using SlTextureId = unsigned int;

inline SlMatrix4 SlIdentityMatrix4() noexcept
{
	SlMatrix4 matrix{};
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	return matrix;
}

inline SlMatrix4 SlMatrixTranslate(float x, float y, float z) noexcept
{
	SlMatrix4 matrix = SlIdentityMatrix4();
	matrix.m[3][0] = x;
	matrix.m[3][1] = y;
	matrix.m[3][2] = z;
	return matrix;
}

inline SlMatrix4 SlMatrixMul(const SlMatrix4& a, const SlMatrix4& b) noexcept
{
	SlMatrix4 result{};
	for (int row = 0; row < 4; ++row)
	{
		for (int col = 0; col < 4; ++col)
		{
			for (int k = 0; k < 4; ++k)
				result.m[row][col] += a.m[row][k] * b.m[k][col];
		}
	}
	return result;
}

inline bool SlMatrixInverse(const SlMatrix4& in, SlMatrix4& out) noexcept
{
	float aug[4][8]{};
	for (int row = 0; row < 4; ++row)
	{
		for (int col = 0; col < 4; ++col)
			aug[row][col] = in.m[row][col];
		aug[row][row + 4] = 1.0f;
	}

	for (int col = 0; col < 4; ++col)
	{
		int pivotRow = col;
		float pivotAbs = std::fabs(aug[pivotRow][col]);
		for (int row = col + 1; row < 4; ++row)
		{
			const float valueAbs = std::fabs(aug[row][col]);
			if (valueAbs > pivotAbs)
			{
				pivotAbs = valueAbs;
				pivotRow = row;
			}
		}
		if (pivotAbs < 1.0e-8f)
			return false;

		if (pivotRow != col)
		{
			for (int k = 0; k < 8; ++k)
			{
				const float tmp = aug[col][k];
				aug[col][k] = aug[pivotRow][k];
				aug[pivotRow][k] = tmp;
			}
		}

		const float pivot = aug[col][col];
		for (int k = 0; k < 8; ++k)
			aug[col][k] /= pivot;

		for (int row = 0; row < 4; ++row)
		{
			if (row == col) continue;
			const float factor = aug[row][col];
			if (factor == 0.0f) continue;
			for (int k = 0; k < 8; ++k)
				aug[row][k] -= factor * aug[col][k];
		}
	}

	for (int row = 0; row < 4; ++row)
		for (int col = 0; col < 4; ++col)
			out.m[row][col] = aug[row][col + 4];
	return true;
}

inline SlVec3 SlVec3Transform(const SlVec3& v, const SlMatrix4& m) noexcept
{
	return SlVec3(
		v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + m.m[3][0],
		v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + m.m[3][1],
		v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + m.m[3][2]);
}

#endif
