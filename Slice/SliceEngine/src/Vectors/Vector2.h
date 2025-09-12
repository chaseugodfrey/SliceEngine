#ifndef SLICE_VECTOR2_H
#define SLICE_VECTOR2_H

#include <pch.h>

const float PI = 3.14159265358979323846f;

template<typename T> 
struct Vector2
{
	union
	{
		struct
		{
			T x, y;
		};
		T _mAccessor[2];
	};

	//Constructors
	Vector2() : x(0), y(0) {}
	Vector2(T _x, T _y) : x(_x), y(_y) {}
	template<typename O> Vector2(Vector2<O> _other)
	{
		x = static_cast<T>(_other.x);
		y = static_cast<T>(_other.y);
	}

	//Assignment Operators
	Vector2& operator=(const Vector2& rhs) = default;
	Vector2(const Vector2& rhs) = default;
	Vector2(Vector2&&) = default;
	Vector2& operator=(Vector2&&) = default;

	//Compound Assignment Operators
	Vector2& operator += (const Vector2& rhs)
	{
		x += rhs.x;
		y += rhs.y;

		return *this;
	}

	Vector2& operator -= (const Vector2& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}

	Vector2& operator *= (T rhs)
	{
		x *= rhs;
		y *= rhs;
		return *this;
	}

	Vector2& operator /= (T rhs)
	{
		x /= rhs;
		y /= rhs;
		return *this;
	}

	//Unary Operators (flip the +/- sign)
	Vector2 operator -() const
	{
		return Vector2(-x, -y);
	}

	bool operator==(Vector2 other)
	{
		return (std::fabs(x - other.x) < DBL_EPSILON && std::fabs(y - other.y) < DBL_EPSILON);
	}

	bool operator!=(Vector2 other)
	{
		return !(*this == other);
	}

	//Other Member Functions:
	//Set
	void Set(T x, T y)
	{
		this->x = x;
		this->y = y;
	}

	// Self-referenced Dot Product
	T dot(const Vector2& other) const
	{
		return(x * other.x + y * other.y);
	}

	// Self-referenced Cross Product
	Vector2 cross(const Vector2& other) const
	{
		return(x * other.y - y * other.x);
	}

	//Length
	T length() const
	{
		return std::sqrt(x * x + y * y);
	}

	//Square Length
	T squareLength() const
	{
		return (x * x + y * y);
	}

	Vector2 normal() const
	{
		return { -y, -x };
	}

	Vector2 normalUnit() const
	{
		return normal().normalize();
	}

	//Normalize
	Vector2& normalize()
	{
		T len = length();
		if (len == 0)
		{
			len = 1;
			SLICE_LOG_ERROR("Division by zero!");
		}
		*this /= len;
		return *this;
	}

	// Angle Calculation
	T angleRad() const
	{
		return atan2(y, x);
	}

	std::string toString() const
	{
		return '(' + std::to_string(x) + ',' + std::to_string(y) + ')';
	}

	// STATIC MEMBER FUNCTIONS
	// This is used instead of external functions

	//Default Vectors
	static Vector2 Zero() { return Vector2(0, 0); }
	static Vector2 One() { return Vector2(1, 1); }
	static Vector2 Up() { return Vector2(0, 1); }
	static Vector2 Down() { return Vector2(0, -1); }
	static Vector2 Left() { return Vector2(-1, 0); }
	static Vector2 Right() { return Vector2(1, 0); }

	T Length() = delete;
	T SquareLength() = delete;
	Vector2 Normalize() = delete;
	T Dot() = delete;
	Vector2 Cross() = delete;
	T Distance() = delete;
	T SquareDistance() = delete;
	T FromAngle() = delete;

	//Length
	static T Length(const Vector2& rhs)
	{
		T length = sqrt(rhs.x * rhs.x + rhs.y * rhs.y);

		return length;
	}

	//Square Length
	static T SquareLength(const Vector2& rhs)
	{
		T length = rhs.x * rhs.x + rhs.y * rhs.y;

		return length;
	}

	static Vector2 Normalize(const Vector2& rhs)
	{
		T normalisation = sqrt(rhs.x * rhs.x + rhs.y * rhs.y);
		Vector2 result{};

		if (normalisation == 0)
		{
			result.x = 0;
			result.y = 0;
		}

		else
		{
			result.x = rhs.x / normalisation;
			result.y = rhs.y / normalisation;
		}

		return result;
	}

	static T Dot(const Vector2& lhs, const Vector2& rhs)
	{
		T dot_product = lhs.x * rhs.x + lhs.y * rhs.y;

		return dot_product;
	}

	static T Cross(const Vector2& lhs, const Vector2& rhs)
	{
		return lhs.x * rhs.y - lhs.y * rhs.x;
	}

	static T Distance(const Vector2& lhs, const Vector2& rhs)
	{
		Vector2 DistanceVector = lhs - rhs;

		T distance = Vector2Length(DistanceVector);

		return distance;
	}

	static T SquareDistance(const Vector2& lhs, const Vector2& rhs)
	{
		return (rhs - lhs).squareLength();
	}

	static Vector2<T> FromAngle(T angle)
	{
		return Vector2<T>{cos(angle), sin(angle)};
	}

	static float AngleRad(const Vector2& lhs, const Vector2& rhs)
	{
		float dp = Dot(lhs, rhs);
		float magp = lhs.length() * rhs.length();
		if (std::fabs(magp) < FLT_EPSILON)
			return 0;

		float cosTheta = dp / magp;
		cosTheta = std::fmax(-1.0f, std::fmin(1.0f, cosTheta));

		return std::acos(cosTheta);
	}

	static float AngleDeg(const Vector2& lhs, const Vector2& rhs)
	{
		return AngleRad(lhs, rhs) * 180 / PI;
	}
};

// Partial Specialization of division of Vector2<float>
template <> Vector2<float>& Vector2<float>::operator/=(float rhs)
{
	if (std::fabs(rhs) < FLT_EPSILON)
	{
		rhs = 1.0f;
		SLICE_LOG_ERROR("Division by zero!");
	}

	x /= rhs;
	y /= rhs;

	return *this;
}

template<typename T> Vector2<T> operator / (const Vector2<T>& lhs, T rhs)
{
	if (rhs == 0)
	{
		rhs = 1;
		SLICE_LOG_ERROR("Division by 0.");
	}

	return Vector2(lhs.x / rhs, lhs.y / rhs);
}

//Normalize
template <> Vector2<float>& Vector2<float>::normalize()
{
	float len = length();
	if (std::fabs(len) < FLT_EPSILON)
	{
		len = 1;
		SLICE_LOG_ERROR("Cannot normalize zero vector!");
	}
	*this /= len;
	return *this;
}

// Binary operators
template<typename T> Vector2<T> operator + (const Vector2<T>& lhs, const Vector2<T>& rhs)
{
	return Vector2(lhs.x + rhs.x, lhs.y + rhs.y);
}

template<typename T> Vector2<T> operator - (const Vector2<T>& lhs, const Vector2<T>& rhs)
{
	return Vector2(lhs.x - rhs.x, lhs.y - rhs.y);
}

template<typename T> Vector2<T> operator * (const Vector2<T>& lhs, T rhs)
{
	return Vector2(lhs.x * rhs, lhs.y * rhs);
}

template<typename T> Vector2<T> operator * (T lhs, const Vector2<T>& rhs)
{
	return Vector2(rhs.x * lhs, rhs.y * lhs);
}

template<typename T> Vector2<T> operator * (const Vector2<T>& lhs, const Vector2<T>& rhs)
{
	return Vector2(lhs.x * rhs.x, lhs.y * rhs.y);
}

//<< Operator Overload
template <typename T> std::ostream& operator<<(std::ostream& os, const Vector2<T>& rhs)
{
	os << "(" << rhs.x << "," << rhs.y << ")";
	return os;
}

using Vec2f = Vector2<float>;
using Vec2d = Vector2<double>;
using Vec2i = Vector2<int>;

#endif 