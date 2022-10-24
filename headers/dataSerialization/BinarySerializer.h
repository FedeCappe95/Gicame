#ifndef __BINARYSERIALIZER_H__
#define __BINARYSERIALIZER_H__


#include "../common.h"
#include "../interfaces/ISerializable.h"
#include <vector>


// Macros
#define STD_SERILIZES_FOR_PRIMITIVE_TYPE(Type) \
template <> \
inline std::vector<byte_t> BinarySerializer::serialize(const Type& data) { \
	std::vector<byte_t> result(sizeof(Type)); \
	*((Type*)result.data()) = data; \
	return result; \
} \
template <> \
inline void BinarySerializer::serialize(const Type& data, void* outBuffer) { \
	*((Type*)outBuffer) = data; \
}

#define BLIND_SERIALIZER(Type) STD_SERILIZES_FOR_PRIMITIVE_TYPE(Type)


namespace Gicame {

	/**
	 * Generic binary serializer
	 */
	class BinarySerializer {

	public:
		constexpr BinarySerializer() {};
		template <class Type>
		std::vector<byte_t> serialize(const Type& data);    // Method A
		template <class Type>
		void serialize(const Type& data, void* outBuffer);  // Method B

	};


	/*
	 * Inline implementation
	 */

	/**
	 * Serialization for ISerializable interface (Method A)
	 */
	template <>
	inline std::vector<byte_t> BinarySerializer::serialize(const ISerializable& data) {
		return data.serialize();
	}

	/**
	 * Serialization for ISerializable interface (Method B)
	 */
	template <>
	inline void BinarySerializer::serialize(const ISerializable& data, void* outBuffer) {
		const std::vector<byte_t> dataBytes = data.serialize();
		const size_t dataSize = dataBytes.size();
		for (size_t i = 0; i < dataSize; ++i)
			((byte_t*)outBuffer)[i] = dataBytes[i];
	}

	/*
	 * Serialization for primitive data types
	 */
	STD_SERILIZES_FOR_PRIMITIVE_TYPE(int8_t)
	STD_SERILIZES_FOR_PRIMITIVE_TYPE(int16_t)
	STD_SERILIZES_FOR_PRIMITIVE_TYPE(int32_t)
	STD_SERILIZES_FOR_PRIMITIVE_TYPE(int64_t)
	STD_SERILIZES_FOR_PRIMITIVE_TYPE(uint8_t)
	STD_SERILIZES_FOR_PRIMITIVE_TYPE(uint16_t)
	STD_SERILIZES_FOR_PRIMITIVE_TYPE(uint32_t)
	STD_SERILIZES_FOR_PRIMITIVE_TYPE(uint64_t)
	STD_SERILIZES_FOR_PRIMITIVE_TYPE(float)
	STD_SERILIZES_FOR_PRIMITIVE_TYPE(double)

};


#endif
