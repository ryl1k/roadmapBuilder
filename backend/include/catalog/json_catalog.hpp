#include "icatalog.hpp"

class JsonCatalog : public ICatalog {
	std::string path;
public:
	explicit JsonCatalog(const std::string& path) : path(path) {}
	std::vector<Course> getAll() override;
};