#define CATCH_CONFIG_MAIN
#include "catch2/catch_amalgamated.hpp"
#include "../headers/utils/Numbers.h"
#include "../headers/serialization/Binary.h"
#include "../headers/reflection/Property.h"
#include "../headers/reflection/Comparable.h"
#include <stdint.h>
#include <stdexcept>
#include <string>
#include <tuple>
#include <array>


using namespace Gicame;


struct Dog : public Reflection::Comparable<Dog> {

    std::string barkType;
    std::string color;
    int weight = 0;
    bool awesome = true;  // All dos are awesome
    std::vector<std::string> owners;

    static constexpr auto properties = std::make_tuple(
        Reflection::makeProperty(&Dog::barkType, "barkType"),
        Reflection::makeProperty(&Dog::color, "color"),
        Reflection::makeProperty(&Dog::weight, "weight"),
        Reflection::makeProperty(&Dog::awesome, "awesome"),
        Reflection::makeProperty(&Dog::owners, "owners")
    );

};

struct Family : public Reflection::Comparable<Family> {

    std::string name;
    Dog dog;

    static constexpr auto properties = std::make_tuple(
        Reflection::makeProperty(&Family::name, "name"),
        Reflection::makeProperty(&Family::dog, "dog")
    );

};

struct Pack1 : public Reflection::Comparable<Pack1> {

    double b;
    int a;
    unsigned c;

    inline Pack1() {}
    constexpr Pack1(double b_, int a_, unsigned c_) : b(b_), a(a_), c(c_) {}

    static constexpr auto properties = std::make_tuple(
        Reflection::makeProperty(&Pack1::a, "a"),
        Reflection::makeProperty(&Pack1::b, "b"),
        Reflection::makeProperty(&Pack1::c, "c")
    );

};

struct Pack2 : public Reflection::Comparable<Pack2> {

    Pack1 pack1;
    double a;

    inline Pack2() {}
    constexpr Pack2(const Pack1& pack1_, const double a_) : pack1(pack1_), a(a_) {}

    static constexpr auto properties = std::make_tuple(
        Reflection::makeProperty(&Pack2::pack1, "pack1"),
        Reflection::makeProperty(&Pack2::a, "a")
    );

};


TEST_CASE("safeNumericCast overflow", "[Utils]") {
    constexpr uint64_t largeU64 = 0b101010ull << 52;
    constexpr uint32_t largeU32 = 0b101010ull << 18;
    constexpr uint16_t largeU16 = 0b101010ull << 9;
    constexpr uint64_t largeI64 = 0b101110ll << 52;
    constexpr uint32_t largeI32 = 0b101110ll << 18;
    constexpr uint16_t largeI16 = 0b101110ll << 9;
    constexpr uint64_t smallU64 = 0b101010ull << 4;
    constexpr uint32_t smallU32 = 0b101010ull << 2;
    constexpr uint16_t smallU16 = 0b101010ull << 0;
    constexpr uint64_t smallI64 = 0b101010ll << 4;
    constexpr uint32_t smallI32 = 0b101010ll << 2;
    constexpr uint16_t smallI16 = 0b101010ll << 0;

    SECTION("U64 to U32 must fail") {
        REQUIRE_THROWS_AS(
            Utilities::safeNumericCast<uint32_t>(largeU64),
            std::runtime_error
        );
    }

    SECTION("U32 to U16 must fail") {
        REQUIRE_THROWS_AS(
            Utilities::safeNumericCast<uint16_t>(largeU32),
            std::runtime_error
        );
    }

    SECTION("U16 to U8 must fail") {
        REQUIRE_THROWS_AS(
            Utilities::safeNumericCast<uint8_t>(largeU16),
            std::runtime_error
        );
    }

    SECTION("I64 to I32 must fail") {
        REQUIRE_THROWS_AS(
            Utilities::safeNumericCast<int32_t>(largeI64),
            std::runtime_error
        );
    }

    SECTION("I32 to I16 must fail") {
        REQUIRE_THROWS_AS(
            Utilities::safeNumericCast<int16_t>(largeI32),
            std::runtime_error
        );
    }

    SECTION("I16 to I8 must fail") {
        REQUIRE_THROWS_AS(
            Utilities::safeNumericCast<int8_t>(largeI16),
            std::runtime_error
        );
    }

    SECTION("U64 to U32 must pass") {
        REQUIRE(smallU64 == Utilities::safeNumericCast<uint32_t>(smallU64));
    }

    SECTION("U32 to U16 must pass") {
        REQUIRE(smallU32 == Utilities::safeNumericCast<uint16_t>(smallU32));
    }

    SECTION("U16 to U8 must pass") {
        REQUIRE(smallU16 == Utilities::safeNumericCast<uint8_t>(smallU16));
    }

    SECTION("I64 to I32 must pass") {
        REQUIRE(smallI64 == Utilities::safeNumericCast<int32_t>(smallI64));
    }

    SECTION("I32 to I16 must pass") {
        REQUIRE(smallI32 == Utilities::safeNumericCast<int16_t>(smallI32));
    }

    SECTION("I16 to I8 must pass") {
        REQUIRE(smallI16 == Utilities::safeNumericCast<int8_t>(smallI16));
    }

    SECTION("Negative numbers can't be unsigned") {
        REQUIRE_THROWS_AS(
            Utilities::safeNumericCast<uint32_t>((int32_t)-42),
            std::runtime_error
        );
    }
}

TEST_CASE("Complarable", "[reflection]") {
    using namespace Gicame::Reflection;
    using namespace Gicame::Serialization;

    Dog dog1;
    dog1.barkType = "loud";
    dog1.color = "brown";
    Family family1;
    family1.name = "The dog's family";
    family1.dog = dog1;

    Dog dog2;
    dog2.barkType = "loud";
    dog2.color = "brown";
    Family family2;
    family2.name = "The dog's family";
    family2.dog = dog2;

    Dog dog3;
    dog3.barkType = "loud";
    dog3.color = "black";
    Family family3;
    family3.name = "The dog's family";
    family3.dog = dog3;

    REQUIRE(dog1 == dog2);
    REQUIRE(dog1 != dog3);
    REQUIRE(dog2 != dog3);
    REQUIRE(family1 == family2);
    REQUIRE(family1 != family3);
    REQUIRE(family2 != family3);
}

TEST_CASE("Binary serialization", "[serialization]") {
    std::array<uint8_t, 1024> buffer;

    // Serialization
    Pack2 pack2( Pack1(1.0, 0, 3), 4.0 );
    Gicame::Serialization::binarySerialize(pack2, buffer.data(), buffer.size());

    // Deserialization
    Pack2 deserializedPack2;
    Gicame::Serialization::binaryDeserialize(deserializedPack2, buffer.data(), buffer.size());

    // Check
    REQUIRE(pack2 == deserializedPack2);
}