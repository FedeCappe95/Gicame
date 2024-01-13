#include <iostream>
#include <stdint.h>
#include <vector>
#include <string>
#include <tuple>
#include <sstream>
#include <fstream>
#include "../../headers/reflection/Property.h"
#include "../../headers/serialization/Tree.h"
#include "../../headers/serialization/Binary.h"


using namespace Gicame::Reflection;
using namespace Gicame::Serialization;


struct WitoutProp {
	int a;
};

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


struct Pack1 {

	double b;
	int a;
	unsigned c;

	static constexpr auto properties = std::make_tuple(
		makeProperty(&Pack1::a, "a"),
		makeProperty(&Pack1::b, "b"),
		makeProperty(&Pack1::c, "c")
	);

};

struct Pack2 {

	Pack1 pack1;
	double a;

	static constexpr auto properties = std::make_tuple(
		makeProperty(&Pack2::pack1, "pack1"),
		makeProperty(&Pack2::a, "a")
	);

};


static std::string buildSpacer(const size_t spaceCount) {
	std::stringstream ss;
	for (size_t i = 0; i < spaceCount; ++i)
		ss << " ";
	return ss.str();
}

static void printTreeLeaf(const Gicame::Serialization::Tree::Leaf& leaf, const size_t spaceCount = 0) {
	const std::string spacer = buildSpacer(spaceCount);
	std::cout << spacer << leaf.name << " = " << "??" << "\n";
}

static void printTreeNode(const Gicame::Serialization::Tree::Node& node, const size_t spaceCount = 0) {
	const std::string spacer = buildSpacer(spaceCount);
	std::cout << spacer << "Name: " << node.name << "\n";
	std::cout << spacer << "Children (" << node.children.size() << "):\n";
	for (const auto& child : node.children) {
		if (std::holds_alternative<Gicame::Serialization::Tree::Node>(child)) {
			printTreeNode(std::get<Gicame::Serialization::Tree::Node>(child), spaceCount + 2u);
		}
		else if (std::holds_alternative<Gicame::Serialization::Tree::Leaf>(child)) {
			printTreeLeaf(std::get<Gicame::Serialization::Tree::Leaf>(child), spaceCount + 2u);
		}
	}
}


int main() {
	std::cout << "Gicame example: Serialization" << std::endl;

	// Tree
	Dog dog;
	dog.barkType = "loud";
	dog.color = "brown";
	Family family{ "test", dog };
	const auto tree = Gicame::Serialization::Tree::buildTree("Family", family);
	printTreeNode(tree.root);

	// Some buffer
	uint8_t buff[1024];
	memset(buff, 0xFF, sizeof(buff));

	// Binary serialization (1)
	Pack2 pack2{ Pack1{1.0,0,3}, 4.0 };
	Gicame::Serialization::binarySerialize(pack2, &buff[0], sizeof(buff));
	
	// Binary serialization (2)
	std::ofstream outFile;
	outFile.open("out.bin", std::ios::out | std::ios::binary);
	Gicame::Serialization::binarySerialize(pack2, outFile);
	outFile.close();

	// Binary deserialization (1)
	Pack2 result1;
	Gicame::Serialization::binaryDeserialize(result1, &buff[0], sizeof(buff));

	// Binary deserialization (2)
	std::ifstream inFile;
	inFile.open("out.bin", std::ios::in | std::ios::binary);
	Pack2 result2;
	Gicame::Serialization::binaryDeserialize(result2, inFile);
	inFile.close();

	return 0;
}