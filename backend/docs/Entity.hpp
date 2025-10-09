#pragma once



class Entity {
	int id;
public:

	Entity(int _id) { id = _id; }
	Entity() {}

	int getId()		  { return id; }
	int setId(int _id) { id = _id; }
};