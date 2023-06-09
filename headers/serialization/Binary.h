#ifndef __GICAME__SERIALIZATION__BINARYSERIALIZER_H__
#define __GICAME__SERIALIZATION__BINARYSERIALIZER_H__


#include "../common.h"
#include <stdexcept>
#include <type_traits>
#include <ostream>
#include "../reflection/Property.h"
#include "../stream/MemoryStream.h"


namespace Gicame::Serialization {

	namespace Internals {

		/*
		 * @brief Serialize an object to a binary form
		 * 
		 * Override for objects without properties
		 */
		template<typename OType>
		static inline void toBinary(const OType& object, std::ostream& out);

		/*
		 * @brief Serialize an object to a binary form
		 * 
		 * It works only for fundamental (primitive) types
		 */
		template<typename OType, class = std::enable_if<std::is_fundamental<OType>::value>>
		static inline void fundamentalToBinary(const OType& object, std::ostream& out) {
			out.write((const char*)(&object), sizeof(object));
		}

		/*
		 * @brief Deserialize an object from binary
		 * 
		 * Override for objects without properties
		 */
		template<typename OType>
		static inline void fromBinary(OType& object, std::istream& in);

		/*
		 * @brief Deserialize an object from binary form
		 *
		 * It works only for fundamental (primitive) types
		 */
		template<typename OType, class = std::enable_if<std::is_fundamental<OType>::value>>
		static inline void binaryToFundamental(OType& object, std::istream& in) {
			in.read((char*)(&object), sizeof(object));
		}

	}

	/*
	 * @brief Recursively serialize an object to a binary form. The object needs properties.
	 */
	template<typename OType>
	static inline void binarySerialize(const OType& object, std::ostream& out) {
		// Check for properties
		static_assert(Gicame::Reflection::HasProperies<OType>::has(), "Trying to serialize an object without properties");

		// We first get the number of properties
		constexpr auto propertyCount = std::tuple_size<decltype(OType::properties)>::value;
		constexpr auto propertyIndexes = std::make_index_sequence<propertyCount>{};

		// We iterate on the index sequence of size propertyCount
		Gicame::Reflection::apply_on_index_sequence(propertyIndexes, [&](auto i) {
			constexpr auto prop = std::get<i>(OType::properties);
			using PropType = decltype(prop);
			using PropMemberType = PropType::template MemberType;

			if constexpr (Gicame::Reflection::HasProperies<PropMemberType>::has()) {
				binarySerialize(object.*(prop.member), out);
			}
			else if constexpr (std::is_fundamental<PropMemberType>::value) {
				Internals::fundamentalToBinary(object.*(prop.member), out);
			}
			else if constexpr (!std::is_fundamental<PropMemberType>::value) {
				Internals::toBinary(object.*(prop.member), out);
			}
		});
	}

	/*
	 * @brief Recursively serialize an object to a binary form. The object needs properties.
	 */
	template<typename OType>
	static inline size_t binarySerialize(const OType& object, void* buffer, const size_t bufferSize) {
		// TODO: check bufferSize
		Gicame::Stream::MemoryOStream memoryStream((uint8_t*)buffer, std::streampos(bufferSize));
		binarySerialize(object, memoryStream);
		return size_t(memoryStream.tellp());
	}

	/*
	 * @brief Recursively deserialize an object from binary form. The object needs properties.
	 */
	template<typename OType>
	static inline void binaryDeserialize(OType& object, std::istream& in) {
		// Check for properties
		static_assert(Gicame::Reflection::HasProperies<OType>::has(), "Trying to deserialize an object without properties");

		// We first get the number of properties
		constexpr auto propertyCount = std::tuple_size<decltype(OType::properties)>::value;
		constexpr auto propertyIndexes = std::make_index_sequence<propertyCount>{};

		// We iterate on the index sequence of size propertyCount
		Gicame::Reflection::apply_on_index_sequence(propertyIndexes, [&](auto i) {
			constexpr auto prop = std::get<i>(OType::properties);
			using PropType = decltype(prop);
			using PropMemberType = PropType::template MemberType;

			if constexpr (Gicame::Reflection::HasProperies<PropMemberType>::has()) {
				binaryDeserialize(object.*(prop.member), in);
			}
			else if constexpr (std::is_fundamental<PropMemberType>::value) {
				Internals::binaryToFundamental(object.*(prop.member), in);
			}
			else if constexpr (!std::is_fundamental<PropMemberType>::value) {
				Internals::fromBinary(object.*(prop.member), in);
			}
		});
	}

	/*
	 * @brief Recursively deserialize an object from binary form. The object needs properties.
	 */
	template<typename OType>
	static inline size_t binaryDeserialize(OType& object, void* buffer, const size_t bufferSize) {
		// TODO: check bufferSize
		Gicame::Stream::MemoryIStream memoryStream((uint8_t*)buffer, std::streampos(bufferSize));
		binaryDeserialize(object, memoryStream);
		return size_t(memoryStream.tellg());
	}

};


#endif
