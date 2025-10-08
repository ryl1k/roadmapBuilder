#include "Entity.hpp"
#include <string>
#include <vector>
#include <optional>

class Course : public Entity {
	std::string				   Name;
	std::vector<std::string>   Keywords;
	std::string				   Description;
	int						   Duration;	// Hours
	std::optional<std::string> Reference;	// URL for online resourse
	

public:

	Course(int _id, std::string _Name, std::vector<std::string> _Keywords,
		   std::string _Description, int _Duration, std::optional<std::string> _Reference) {
		Entity(_id);
		Name = _Name;
		Keywords = _Keywords;
		Description = _Description;
		Duration = _Duration;
		Reference = (_Reference) ? _Reference : std::nullopt;
	}


	
	std::string getName() { return Name; }
	std::string setName(std::string _Name) { Name = _Name; }

	std::vector<std::string> getKeywords() { return Keywords; }
	std::vector<std::string> setKeywords(std::vector<std::string> _Keywords) { Keywords = _Keywords; }
	
	std::string getDescription() { return Description; }
	std::string setDescription(std::string _Description) { Description = _Description; }

	int getDuration() { return Duration; }
	int setDuration(int _Duration) { Duration = _Duration; }

	std::optional<std::string> getReference() { return Reference; }
	std::optional<std::string> setReference(std::optional<std::string> _Reference) { Reference = _Reference; }


};