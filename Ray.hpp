
#pragma once

#include <cmath>
#include <limits>
#include <ostream>

#include <glm/glm.hpp>

// This code is taken from 15-462 Scotty3D code

struct Ray {

	Ray() = default;

	/// Create Ray from point and direction
	explicit Ray(glm::vec3 point, glm::vec3 dir,
	             glm::vec2 dist_bounds = glm::vec2{0.0f, std::numeric_limits<float>::infinity()},
	             uint32_t depth = 0)
		: point(point), dir(glm::normalize(dir)), depth(depth), dist_bounds(dist_bounds) {
	}

	Ray(const Ray&) = default;
	Ray& operator=(const Ray&) = default;
	~Ray() = default;

	/// Get point on Ray at time t
	glm::vec3 at(float t) const {
		return point + t * dir;
	}

	/// Move ray into the space defined by this tranform matrix
	// void transform(const Mat4& trans) {
	// 	point = trans * point;
	// 	dir = trans.rotate(dir);
	// 	float d = dir.norm();
	// 	dist_bounds *= d;
	// 	dir /= d;
	// }

	/// The origin or starting point of this ray
	glm::vec3 point;
	/// The direction the ray travels in
	glm::vec3 dir;
	/// Recursive depth of ray
	uint32_t depth = 0;

	/// The minimum and maximum distance at which this ray can encounter collisions
	glm::vec2 dist_bounds = glm::vec2(0.0f, std::numeric_limits<float>::infinity());
};

// inline std::ostream& operator<<(std::ostream& out, Ray r) {
// 	out << "Ray{" << r.point << "," << r.dir << "}";
// 	return out;
// }
