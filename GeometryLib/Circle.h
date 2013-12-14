#ifndef CIRCLE_H
#define CIRCLE_H

#include "Line.h"

namespace Geom{

#define V2V(v) (Vec3d(v.x,v.y,v.z))
#define Vec3d2V(v) (Vec(v.x(),v.y(),v.z()))

// Rodrigues' rotation
#define ROTATE_VEC(v, theta, axis) (v = v * cos(theta) + cross(axis, v) * sin(theta) + axis * dot(axis, v) * (1 - cos(theta)))
#define ROTATED_VEC(v, theta, axis) (v * cos(theta) + cross(axis, v) * sin(theta) + axis * dot(axis, v) * (1 - cos(theta)))

class Circle
{
private:
	double radius;

	int numSides;

	Vec3d normal;
	Vec3d center;

	QVector<Vec3d> point;

public:
	Circle();

	Circle(const Vec3d& circle_center = Vec3d(0,0,0), const Vec3d& circle_normal = Vec3d(0,0,1), 
		double from_radius = 1.0, int number_of_sides = 40);
	Circle& operator= (const Circle& from);

	Vec3d & getNormal();
	Vec3d & getCenter();

	QVector<Vec3d> getPoints();

	void translate(const Vec3d & to);

	void draw(double lineWidth = 1.0, const Vec4d & color = Vec4d(1,1,1,1));
	void drawFilled(const Vec4d & fillColor = Vec4d(1,0,0,1), double lineWidth = 2, const Vec4d & borderColor = Vec4d(1,1,1,1));
};

};
#endif // CIRCLE_H
