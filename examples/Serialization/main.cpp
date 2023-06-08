#include <iostream>
#include <stdint.h>
#include <vector>
#include <string>
#include <tuple>
#include "../../headers/reflection/Property.h"
#include "../../headers/serialization/Tree.h"


using namespace Gicame::Reflection;
using namespace Gicame::Serialization;


struct Dog {

	std::string barkType;
	std::string color;
	int weight = 0;
	bool awesome = true;  // All dos are awesome
	std::vector<std::string> owners;

	static constexpr auto properties = std::make_tuple(
		makeProperty(&Dog::barkType, "barkType"),
		makeProperty(&Dog::color, "color"),
		makeProperty(&Dog::weight, "weight"),
		makeProperty(&Dog::awesome, "awesome"),
		makeProperty(&Dog::owners, "owners")
	);

};

struct Family {

	std::string name;
	Dog dog;

	static constexpr auto properties = std::make_tuple(
		makeProperty(&Family::name, "name"),
		makeProperty(&Family::dog, "dog")
	);

};


int main() {
	std::cout << "Gicame example: Serialization" << std::endl;

	Dog dog;
	dog.barkType = "loud";
	dog.color = "brown";
	Family family{ "test", dog };
	const auto tree = Gicame::Serialization::buildTree(family);

	return 0;
}