#include "vector_3d.h"

Vector3::Vector3() 
{
	x = y = z = 0.0f;
}

Vector3::Vector3(const double fx, const double fy, const double fz)
{
	x = fx;
	y = fy;
	z = fz;
}

Vector3::~Vector3() = default;