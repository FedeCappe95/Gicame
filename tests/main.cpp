#define CATCH_CONFIG_MAIN
#include "catch2/catch_amalgamated.hpp"
#include "../headers/utils/Numbers.h"
#include "../headers/utils/Memory.h"
#include "../headers/serialization/Binary.h"
#include "../headers/reflection/Property.h"
#include "../headers/reflection/Comparable.h"
#include "../headers/sm/SharedMemory.h"
#include "../headers/concurrency/Semaphore.h"
#include <stdint.h>
#include <stdexcept>
#include <string>
#include <tuple>
#include <array>
#include <thread>
#include <new>
#include <atomic>
#include <cstddef>


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

TEST_CASE("Shared memory", "[shared memory]") {
    static size_t constructorCalled;
    constructorCalled = 0;

    struct MyData {
        uint32_t a;
        float b;
        std::atomic<bool> readDone;
        std::atomic<bool> writeDone;

        inline MyData() { ++constructorCalled; }
    };

    constexpr size_t smSize = sizeof(MyData) + alignof(std::max_align_t);  // To make alignment possible

    SharedMemory sm1("MyMemory", smSize);
    sm1.open(true);
    MyData* myData1 = sm1.getAs<MyData>(true);
    REQUIRE(myData1 != NULL);  // It may fails if sm1 has a capacity exactly of sizeof(MyData)
    myData1->a = 5;
    myData1->b = 12.0;
    myData1->readDone = false;
    myData1->writeDone = false;

    bool peer0success = false;
    bool peer1success = false;

    auto peer0 = [&]() {
        SharedMemory sm("MyMemory", smSize);
        sm.open(false);
        MyData* myData = sm.getAs<MyData>(false);
        if (!myData)
            return;
        myData->a = 45;
        myData->b = 96.5f;
        myData->writeDone = true;
        while (!myData->readDone) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        sm.close();
        peer0success = true;
    };

    auto peer1 = [&]() {
        SharedMemory sm("MyMemory", smSize);
        sm.open(false);
        MyData* myData = std::launder(reinterpret_cast<MyData*>(sm.get()));
        if (!myData)
            return;
        while (!myData->writeDone) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        const bool success = myData->a == 45 && myData->b == 96.5f;
        myData->readDone = true;
        sm.close();
        peer1success = success;
    };

    std::thread t0(peer0);
    std::thread t1(peer1);

    t0.join();
    t1.join();

    REQUIRE(constructorCalled == 1u);
    REQUIRE(peer0success);
    REQUIRE(peer1success);
}

TEST_CASE("Semaphore 1", "[semaphore]") {
    Semaphore testSem("TestSem", 2u);   // Starts with 2 free resource
    testSem.setUnlinkOnDestruction(true);

    bool success = testSem.acquire();   // 1 free resources over a max of 2
    REQUIRE(success);

    success = testSem.acquire();        // 0 free resources over a max of 2
    REQUIRE(success);

    success = testSem.release();        // 1 free resource over a max of 2
    REQUIRE(success);

    success = testSem.release();        // 2 free resource over a max of 2
    REQUIRE(success);


    success = testSem.acquire();        // 1 free resources over a max of 2
    REQUIRE(success);

    success = testSem.release();        // 2 free resource over a max of 2
    REQUIRE(success);


#ifdef WINDOWS
    success = testSem.release();        // 3 free resources over a max of 2? (Note: 1)
    REQUIRE(!success);
#else
    success = testSem.release();        // 3 free resources over a max of 2? (Note: 1)
    REQUIRE(success);
#endif

    // Note 1: This is undefined behaviour because it fails on Windows, but
    // it does not fails on Linux, where the maximum value for the sem_t is
    // SEM_VALUE_MAX and it cannot be changed.
}

TEST_CASE("Semaphore 2", "[semaphore]") {
    using namespace std::chrono_literals;

    Semaphore testSem("TestSem", 1u);
    testSem.setUnlinkOnDestruction(true);

    bool success = testSem.acquire();
    REQUIRE(success);

    bool workerAcquiredSem = false;
    auto workerBody = [&]() {
        if (testSem.acquire())
            workerAcquiredSem = true;
    };

    std::thread worker(workerBody);

    // Wait the worker
    std::this_thread::sleep_for(1000ms);

    // Check the worker never succedded to acquire the sem
    REQUIRE(!workerAcquiredSem);

    success = testSem.release();
    REQUIRE(success);

    worker.join();
}