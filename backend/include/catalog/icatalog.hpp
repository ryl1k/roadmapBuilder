#pragma once

#include <vector>
#include "../models/course.hpp"

class ICatalog {
public:
	virtual std::vector<Course> getAll() = 0;
	virtual ~ICatalog() = default;
};