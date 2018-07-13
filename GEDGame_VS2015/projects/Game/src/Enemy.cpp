#include "Enemy.h"

Enemy::Enemy(std::string t, float s, Vector3 p, Vector3 v, float sze, float hp)
{
	type = t;
	speed = s;
	position = p;
	velocity = v;
	size = sze;
	hitpoints = hp;
}

std::string Enemy::getType() { return type; }
float Enemy::getSpeed() { return speed; }
Vector3 Enemy::getPosition() { return position; }
Vector3 Enemy::getVelocity() { return velocity; }
float Enemy::getSize() { return size; }
float Enemy::getHitpoints() { return hitpoints; };

void Enemy::setType(std::string s) { type = s; }
void Enemy::setSpeed(float s) { speed = s; }
void Enemy::setPosition(Vector3 p) { position = p; }
void Enemy::setVelocity(Vector3 v) { velocity = v; }
void Enemy::setHitpoints(float hp) { hitpoints = hp; }