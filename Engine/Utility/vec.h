#ifndef VEC_H
#define VEC_H

#include <cmath>
#include <cassert>
#include <iostream>
#include <deque>

#define assertm(exp, msg) assert(void(msg), exp))

typedef vec2<float> vec2f;
typedef vec2<int> vec2i;
typedef vec3<float> vec3f;
typedef vec3<int> vec3i;

template<typename T> class vec2 {
public:
	T x, y;

public:
	vec2() { x = y = 0; }
	vec2(T x, T y) : x(x), y(y) {
		assert(!isNan());
	}

	T operator[](int i) const {
		assert(i >= 0 && i <= 1);
		if (i == 0) return x;
		return y;
	}

	T &operator[](int i) const {
		assert(i >= 0 && i <= 1);
		if (i == 0) return x;
		return y;
	}

	bool isNan() const {
		return std::isnan(x) || std::isnan(y);
	}

	bool operator==(const vec2<T>& v) const {
		return x == v.x && y == v.y;
	}

	bool operator!=(const vec2<T>& v) const {
		return x != v.x && y != v.y;
	}

	vec2<T> operator+(const vec2<T>& v) const {
		return vec2(x + v.x, y + v.y);
	}

	vec2<T>& operator+=(const vec2<T>& v) {
		x += v.x; y += v.y;
		return *this;
	}

	vec2<T> operator-(const vec2<T>& v) const {
		return vec2(x - v.x, y - v.y);
	}

	vec2<T>& operator-=(const vec2<T>& v) {
		x -= v.x; y -= v.y;
		return *this;
	}

	vec2<T> operator*(T s) const {
		return vec2<T>(s * x, s * y);
	}

	vec2<T>& operator*=(T s) {
		x *= s; y *= s;
		return *this;
	}

	vec2<T> operator/(T s) const {
		assert(s != 0);
		float inv = (float)1 / s;
		return vec2<T>(x * inv, y * inv);
	}

	vec2<T>& operator/=(T s) {
		assert(s != 0);
		float inv = (float)1 / s;
		x *= inv; y *= inv
		return *this;
	}

	vec2<T> operator-() const {
		return vec2<T>(-x, -y);
	}

	float lengthSquared() const { return x * x + y * y; }
	float length() const { return std::sqrt(lengthSquared()); }

	bool near_zero() const {
		const auto& s = 1e-8;
		return(fabs(x) < s) && (fabs(y) < s);
		return(fabs(x) < s) && (fabs(y) < s);
	}
};

template<typename T> class vec3 {
public:
	T x, y, z;

public:
	vec3() { x = y = z = 0; }
	vec3(T x, T y, T z) : x(x), y(y), z(z) {
		assert(!isNan());
	}

	T operator[](int i) const {
		assert(i >= 0 && i <= 2);
		if (i == 0) return x;
		if (i == 1) return y;
		return z;
	}

	T &operator[](int i) const {
		assert(i >= 0 && i <= 2);
		if (i == 0) return x;
		if (i == 1) return y;
		return z;
	}

	bool isNan() const {
		return std::isnan(x) || std::isnan(y) || std::isnan(z);
	}	

	bool operator==(const vec3<T>& v) const {
		return x == v.x && y == v.y && z == v.z;
	}

	bool operator!=(const vec3<T>& v) const {
		return x != v.x && y != v.y && z != v.z;
	}

	vec3<T> operator+(const vec3<T>& v) const {
		return vec3(x + v.x, y + v.y, z + v.z);
	}

	vec3<T>& operator+=(const vec3<T>& v) {
		x += v.x; y += v.y; z += v.z;
		return *this;
	}

	vec3<T> operator-(const vec3<T>& v) const {
		return vec3(x - v.x, y - v.y, z - v.z);
	}

	vec3<T>& operator-=(const vec3<T>& v) {
		x -= v.x; y -= v.y; z -= v.z;
		return *this;
	}

	vec3<T> operator*(T s) const {
		return vec3<T>(s * x, s * y, s * z);
	}

	vec3<T>& operator*=(T s) {
		x *= s; y *= s; z *= s;
		return *this;
	}

	vec3<T> operator/(T s) const {
		assert(s != 0);
		float inv = (float)1 / s;
		return vec3<T>(x * inv, y * inv, z * inv);
	}

	vec3<T>& operator/=(T s) {
		assert(s != 0);
		float inv = (float)1 / s;
		x *= inv; y *= inv; z *= inv;
		return *this;
	}

	vec3<T> operator-() const {
		return vec3<T>(-x, -y, -z);
	}

	float lengthSquared() const { return x * x + y * y + z * z; }
	float length() const { return std::sqrt(lengthSquared()); }

	bool near_zero() const {
		const auto& s = 1e-8;
		return(fabs(x) < s) && (fabs(y) < s) && (fabs(z) < s);
	}
};

template<typename T> inline std::ostream* operator<<(std::ostream& out, const vec2<T>& v) {
	return out << v.x << " " << v.y;
}

template<typename T> inline std::ostream* operator<<(std::ostream& out, const vec3<T>& v) {
	return out << v.x << " " << v.y << " " << v.z;
}

template<typename T> inline vec2<T> operator*(T s, const vec2<T>& v) { return v * s; }

template<typename T> inline vec2<T> operator/(T s, const vec2<T>& v) { return v / s; }


template<typename T> inline vec3<T> operator*(T s, const vec3<T>& v) { return v * s; }

template<typename T> inline vec3<T> operator/(T s, const vec3<T>& v) { return v / s; }

template <typename T> vec2<T> abs(const vec2<T>& v) {
	return vec2<T>(std::abs(v.x), std::abs(v.y));
}

template<typename T> vec2<T> fabs(const vec2<T>& v) {
	return vec2<T>(std::fabs(v.x), std::fabs(v.y));
}

template <typename T> inline T dot(const vec2<T>& v1, const vec2<T>& v2) {
	return v1.x * v2.x + v1.y * v2.y;
}

template<typename T> inline T absdot(const vec2<T>& v1, const vec2<T>& v2) {
	return std::abs(dot(v1, v2));
}

template<typename T> inline T fabsdot(const vec2<T>& v1, const vec2<T>& v2) {
	return std::fabs(dot(v1, v2));
}

template<typename T> inline vec3<T> unit_vector(const vec2<T>& v) {
	return v / v.length();
}

template<typename T> T minComponent(const vec2<T>& v) {
	return std::min(v.x, v.y);
}

template<typename T> T maxComponent(const vec2<T>& v) {
	return std::max(v.x, v.y);
}

template<typename T> vec2<T> min(const vec2<T>& p1, const vec2<T>& p2) {
	return vec2<T>(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
}

template<typename T> vec3<T> max(const vec2<T>& p1, const vec2<T>& p2) {
	return vec2<T>(std::max(p1.x, p2.x), std::max(p1.y, p2.y));
}

template <typename T> vec3<T> abs(const vec3<T>& v) {
	return vec3<T>(std::abs(v.x), std::abs(v.y), std::abs(v.z));
}

template<typename T> vec3<T> fabs(const vec3<T>& v) {
	return vec3<T>(std::fabs(v.x), std::fabs(v.y), std::fabs(v.z));
}

template <typename T> inline T dot(const vec3<T>& v1, const vec3<T>& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

template<typename T> inline T absdot(const vec3<T>& v1, const vec3<T>& v2) {
	return std::abs(dot(v1, v2));
}

template<typename T> inline T fabsdot(const vec3<T>& v1, const vec3<T>& v2) {
	return std::fabs(dot(v1, v2));
}

template<typename T> inline vec3<T> cross(const vec3<T>& v1, const vec3<T>& v2) {
	double v1x = v1.x, v1y = v1.y, v1z = v1.z;
	double v2x = v2.x, v2y = v2.y, v2z = v2.z;
	return vec3<T>(
		(v1y * v2z) - (v1z * v2y),
		(v1z * v2x) - (v1x * v2z),
		(v1x * v2y) - (v1y * v2x)
	);
}

template<typename T> inline vec3<T> unit_vector(const vec3<T>& v) {
	return v / v.length();
}

template<typename T> T minComponent(const vec3<T>& v) {
	return std::min(v.x, std::min(v.y, v.z));
}

template<typename T> T maxComponent(const vec3<T>& v) {
	return std::max(v.x, std::max(v.y, v.z));
}

template<typename T> vec3<T> min(const vec3<T>& p1, const vec3<T>& p2) {
	return vec3<T>(std::min(p1.x, p2.x), std::min(p1.y, p2.y), std::min(p1.z, p2.z));
}

template<typename T> vec3<T> max(const vec3<T>& p1, const vec3<T>& p2) {
	return vec3<T>(std::max(p1.x, p2.x), std::max(p1.y, p2.y), std::max(p1.z, p2.z));
}
template<typename T> int maxDimension(const vec3<T>& v) {
	return (v.x > v.y) ? ((v.x > v.z) ? 0 : 2) : ((v.y > v.z) ? 1 : 2);
}

template<typename T> vec3<T> permute(const vec3<T>& v, int x, int y, int z) {
	return vec3<T>(v[x], v[y], v[z]);
}

template<typename T> inline void coordinateSystem(const vec3<T>& v1, vec3<T>* v2, vec3<T>* v3) {
	if (std::abs(v1.x) > std::abs(v1.y)) {
		*v2 = vec3<T>(-v1.z, 0, v1.x) / std::sqrt(v1.x * v1.x + v1.z * v1.z);
	}
	else {
		*v2 = vec3<T>(0, v1.z, -v1.y) / std::sqrt(v1.y * v1.y + v1.z * v1.z);
	}
	*v3 = cross(v1, *v2);
}

template<typename T> vec3<T> reflect(const vec3<T>& v, const vec3<T>& n) {
	return v - 2 * dot(v, n) * n;
}

template<typename T> vec3<T> refract(const vec3<T>& uv, const vec3<T>& n, double etai_over_etat) {
	auto cos_theta = std::fmin(dot(-uv, n), 1.0);
	vec3<T> r_out_perp = etai_over_etat * (uv + cos_theta * n);
	vec3<T> r_out_parallel = -sqrt(fabs(1.0 - r_out_perp.lengthSquared())) * n;
	return r_out_perp + r_out_parallel;
}
#endif