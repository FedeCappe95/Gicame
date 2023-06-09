#ifndef __GICAME__SERIALIZATION__TREE_H__
#define __GICAME__SERIALIZATION__TREE_H__


#include "../common.h"
#include <unordered_map>
#include <string>
#include <any>
#include <stdexcept>
#include <type_traits>
#include "../reflection/Property.h"


namespace Gicame::Serialization {

	struct ThreeNodeField {
		std::any data;
		int type;  // TODO

		template<typename Type>
		constexpr ThreeNodeField(Type&& o) : data{ o }, type{ 0 } {}
	};

	struct TreeNode {
		std::unordered_map<std::string, ThreeNodeField> fields;
		std::unordered_map<std::string, TreeNode> children;
	};

	struct Tree {
		TreeNode root;
	};

	template<typename OType>
	static inline TreeNode buildTreeNode(const OType& object) {
		// Check for properties
		static_assert(Gicame::Reflection::HasProperies<OType>::has(), "Trying to build a tree from an object without properties");

		// JSON root
		TreeNode node;

		// We first get the number of properties
		constexpr auto propertyCount = std::tuple_size<decltype(OType::properties)>::value;
		constexpr auto propertyIndexes = std::make_index_sequence<propertyCount>{};

		// We iterate on the index sequence of size propertyCount
		Gicame::Reflection::apply_on_index_sequence(propertyIndexes, [&](auto i) {
			constexpr auto prop = std::get<i>(OType::properties);
			using PropType = decltype(prop);
			using PropMemberType = PropType::template MemberType;

			if constexpr (Gicame::Reflection::HasProperies<PropMemberType>::has()) {
				node.children.emplace(prop.name, buildTreeNode(object.*(prop.member)));
			}
			else {
				node.fields.emplace(prop.name, ThreeNodeField(object.*(prop.member)));
			}
		});

		return node;
	}

	template<typename Type>
	static inline Tree buildTree(const Type& o) {
		return Tree{ buildTreeNode(o) };
	}

};


#endif
