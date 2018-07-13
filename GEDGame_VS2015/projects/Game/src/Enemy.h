#pragma once

#include <string>

// using XMVECTOR causes an error in the doubly linked list, 
// and the solution to it does work in debug modus, that is why we declare our own struct for position and velocity
struct Vector3 
{
	float x, y, z;
	Vector3(float a, float b, float c)
	{
		x = a; y = b; z = c;
	};
	Vector3() {};

	Vector3 operator+ (const Vector3& v) 
	{
		Vector3 out(this->x + v.x, this->y + v.y, this->z + v.z);
		return out;
	};

	Vector3 operator* (const float& w) 
	{
		Vector3 out(this->x * w, this->y * w, this->z * w);
		return out;
	};
};

class Enemy
{
public:
	Enemy(std::string t, float s, Vector3 p, Vector3 v, float sze, float hp);

	std::string getType();
	float getSpeed();
	Vector3 getPosition();
	Vector3 getVelocity();
	float getSize();
	float getHitpoints();

	void setType(std::string s);
	void setSpeed(float s);
	void setPosition(Vector3 p);
	void setVelocity(Vector3 v);
	void setHitpoints(float hp);
	
private:
	std::string type;
	float speed;
	Vector3 position;
	Vector3 velocity;
	float size;
	float hitpoints;
};