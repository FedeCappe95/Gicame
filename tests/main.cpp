#define CATCH_CONFIG_MAIN
#include "catch2/catch_amalgamated.hpp"
#include "../headers/utils/Numbers.h"
#include "../headers/utils/Memory.h"
#include "../headers/serialization/Binary.h"
#include "../headers/reflection/Property.h"
#include "../headers/reflection/Comparable.h"
#include "../headers/sm/SharedMemory.h"
#include "../headers/concurrency/Semaphore.h"
#include "../headers/concurrency/MultiLock.h"
#include "../headers/concurrency/Signal.h"
#include <stdint.h>
#include <stdexcept>
#include <string>
#include <tuple>
#include <array>
#include <thread>
#include <new>
#include <atomic>
#include <cstddef>
#include <type_traits>
#include <vector>


using namespace Gicame;


struct Dog : public Reflection::Comparable<Dog> {

    std::string barkType;
    std::string color;
    int weight = 0;
    bool awesome = true;  // All dogs are awesome
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

TEST_CASE("Semaphore 1", "[concurrency]") {
    Semaphore testSem("TestSem", 2u);   // Starts with 2 free resource
    testSem.setUnlinkOnDestruction(true);

    bool success = testSem.acquire();   // 1 free resources over a max of 2
    REQUIRE(success);

    success = testSem.acquire();        // 0 free resources over a max of 2
    REQUIRE(success);

    REQUIRE(!testSem.try_lock());       // No free resources, it should return false

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

TEST_CASE("Semaphore 2", "[concurrency]") {
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

TEST_CASE("MultiLock", "[concurrency]") {
    Semaphore testSem("TestSem", 1u);   // Starts with 2 free resource
    testSem.setUnlinkOnDestruction(true);

    Concurrency::MultiLock ml(testSem);

    REQUIRE_NOTHROW(ml.lock());
    REQUIRE_NOTHROW(ml.unlock());
    REQUIRE(ml.try_lock());
    REQUIRE(ml.try_lock());
    REQUIRE_NOTHROW(ml.unlock());
    REQUIRE_NOTHROW(ml.unlock());
    REQUIRE_THROWS(ml.unlock());
}

TEST_CASE("Signal - initial state is not signaled", "[concurrency]") {
    Concurrency::Signal sig;

    // A timeout of 0 ms acts as a non-blocking tryWait; on a fresh Signal
    // nothing has been signaled yet, so it must return false immediately.
    REQUIRE(!sig.wait(0u));
}

TEST_CASE("Signal - wait(0) is non-blocking and returns fast", "[concurrency]") {
    Concurrency::Signal sig;

    const auto start = std::chrono::steady_clock::now();
    const bool result = sig.wait(0u);
    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();

    REQUIRE(!result);
    REQUIRE(elapsedMs < 50);   // Must not block at all
}

TEST_CASE("Signal - signal() then tryWait consumes it exactly once", "[concurrency]") {
    Concurrency::Signal sig;

    sig.signal();
    REQUIRE(sig.wait(0u));    // Consumes the pending signal
    REQUIRE(!sig.wait(0u));   // Nothing left to consume
}

TEST_CASE("Signal - is sticky and does not accumulate multiple signal() calls", "[concurrency]") {
    Concurrency::Signal sig;

    // Signaling repeatedly while nobody consumes must not create a counter:
    // the class only has two levels, 0 and 1.
    sig.signal();
    sig.signal();
    sig.signal();

    REQUIRE(sig.wait(0u));    // Consumes the single pending level
    REQUIRE(!sig.wait(0u));   // No leftover signal, even though signal() was called 3 times
}

TEST_CASE("Signal - signal() raised before wait() is not lost", "[concurrency]") {
    Concurrency::Signal sig;

    sig.signal();   // Nobody is waiting yet

    std::atomic<bool> waiterDone{false};
    std::thread waiter([&]() {
        sig.wait();
        waiterDone = true;
    });

    waiter.join();
    REQUIRE(waiterDone);
}

TEST_CASE("Signal - blocking wait() stays blocked until signaled by another thread", "[concurrency]") {
    Concurrency::Signal sig;
    std::atomic<bool> waiterDone{false};

    std::thread waiter([&]() {
        sig.wait();   // Indefinite blocking wait
        waiterDone = true;
    });

    // Give the waiter thread time to actually enter the wait
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    REQUIRE(!waiterDone);   // Must still be blocked, since nobody signaled yet

    sig.signal();
    waiter.join();

    REQUIRE(waiterDone);
}

TEST_CASE("Signal - wait(timeoutMs) times out and returns false if never signaled", "[concurrency]") {
    Concurrency::Signal sig;

    const auto start = std::chrono::steady_clock::now();
    const bool result = sig.wait(300u);
    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();

    REQUIRE(!result);
    REQUIRE(elapsedMs >= 290);   // Small slack to absorb scheduling jitter
}

TEST_CASE("Signal - wait(timeoutMs) returns true promptly when signaled before the deadline", "[concurrency]") {
    Concurrency::Signal sig;

    std::thread signaler([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        sig.signal();
    });

    const auto start = std::chrono::steady_clock::now();
    const bool result = sig.wait(5000u);   // Much larger than the 100ms delay above
    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();

    signaler.join();

    REQUIRE(result);
    REQUIRE(elapsedMs < 4000);   // Must return well before the timeout expires
}

TEST_CASE("Signal - wait(timeoutMs) performs a final check right at the deadline", "[concurrency]") {
    Concurrency::Signal sig;

    std::thread signaler([&]() {
        // Signal right around when the timeout is expected to fire
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        sig.signal();
    });

    const bool result = sig.wait(200u);
    signaler.join();

    // Either outcome is technically valid depending on scheduling, but the
    // call must not throw and must complete in bounded time; if it does
    // catch the race it must correctly report success.
    if (result) {
        REQUIRE(result);
    } else {
        // The signal may have arrived just after the deadline: make sure
        // it is still there and consumable afterwards.
        REQUIRE(sig.wait(500u));
    }
}

TEST_CASE("Signal - only one of several waiters is released per signal", "[concurrency]") {
    Concurrency::Signal sig;
    std::atomic<int> consumedCount{0};

    auto waiterBody = [&]() {
        if (sig.wait(3000u))
            ++consumedCount;
    };

    std::thread w1(waiterBody);
    std::thread w2(waiterBody);

    // Let both threads reach the blocking wait
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    sig.signal();   // Only one of the two waiters must wake up and consume it
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    REQUIRE(consumedCount == 1);

    sig.signal();   // Release the second waiter too
    w1.join();
    w2.join();

    REQUIRE(consumedCount == 2);
}

TEST_CASE("Signal - repeated signal/consume cycles behave like an auto-reset event", "[concurrency]") {
    Concurrency::Signal sig;

    for (int i = 0; i < 10; ++i) {
        REQUIRE(!sig.wait(0u));   // Nothing pending at the start of each cycle
        sig.signal();
        REQUIRE(sig.wait(0u));   // Consumes it
        REQUIRE(!sig.wait(0u));  // And it stays consumed
    }
}

TEST_CASE("Signal - producer/consumer handshake across threads", "[concurrency]") {
    Concurrency::Signal ready;
    Concurrency::Signal processed;
    std::atomic<int> sharedValue{0};
    std::atomic<int> mismatches{0};
    constexpr int iterations = 50;

    // REQUIRE is not thread safe, so the consumer only counts and the check is done on join
    std::thread consumer([&]() {
        for (int i = 0; i < iterations; ++i) {
            ready.wait();
            if (sharedValue != i + 1)
                ++mismatches;
            processed.signal();
        }
    });

    for (int i = 0; i < iterations; ++i) {
        sharedValue = i + 1;
        ready.signal();
        processed.wait();
    }

    consumer.join();

    REQUIRE(mismatches == 0);
}


/*
 * ===== SignalGroup and SignalOfAGroup =====
 */

// Generic helper proving that Signal and SignalOfAGroup expose the very same contract: it is
// instantiated on both types and must behave identically on each of them.
template <typename AnySignal>
static void checkSignalContract(AnySignal& sig) {
    // Fresh or fully drained signal: there is nothing to consume
    REQUIRE(!sig.wait(0u));

    // Sticky: signaling twice does not create a counter
    sig.signal();
    sig.signal();
    REQUIRE(sig.wait(0u));
    REQUIRE(!sig.wait(0u));

    // A blocking wait is released by another thread
    std::thread signaler([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        sig.signal();
    });
    sig.wait();
    signaler.join();

    // The timeout really elapses when nobody signals
    REQUIRE(!sig.wait(200u));
}


TEST_CASE("SignalGroup - initial state has no signal pending on any index", "[concurrency]") {
    Concurrency::SignalGroup<4> group;

    for (size_t i = 0; i < 4; ++i)
        REQUIRE(!group.wait(i, 0u));

    REQUIRE(group.waitOne(0u) == Concurrency::SignalGroup<4>::NO_SIGNALS);
}

TEST_CASE("SignalGroup - each signal of the group is sticky and independent", "[concurrency]") {
    Concurrency::SignalGroup<4> group;

    // Signaling index 1 three times must not create a counter, and must not touch the others
    group.signal(1);
    group.signal(1);
    group.signal(1);

    REQUIRE(!group.wait(0u, 0u));
    REQUIRE(!group.wait(2u, 0u));
    REQUIRE(!group.wait(3u, 0u));

    REQUIRE(group.wait(1u, 0u));    // Consumes the single pending level
    REQUIRE(!group.wait(1u, 0u));   // Nothing left, even after three signal() calls
}

TEST_CASE("SignalGroup - out of range indexes throw", "[concurrency]") {
    Concurrency::SignalGroup<4> group;

    SECTION("signal") {
        REQUIRE_THROWS_AS(group.signal(4), std::runtime_error);
        REQUIRE_THROWS_AS(group.signal(~size_t(0)), std::runtime_error);
    }

    SECTION("wait") {
        REQUIRE_THROWS_AS(group.wait(4), std::runtime_error);
        REQUIRE_THROWS_AS(group.wait(100), std::runtime_error);
    }

    SECTION("wait with timeout") {
        REQUIRE_THROWS_AS(group.wait(4, 0u), std::runtime_error);
        REQUIRE_THROWS_AS(group.wait(4, 1000u), std::runtime_error);
    }

    SECTION("getSignal") {
        REQUIRE_THROWS_AS(group.getSignal(4), std::runtime_error);
        REQUIRE_THROWS_AS(group.getSignal(Concurrency::SignalGroup<4>::NO_SIGNALS), std::runtime_error);
    }

    SECTION("the last valid index does not throw") {
        REQUIRE_NOTHROW(group.signal(3));
        REQUIRE_NOTHROW(group.wait(3, 0u));
        REQUIRE_NOTHROW(group.getSignal(3));
    }
}

TEST_CASE("SignalGroup - waitOne returns the index of the signaled one", "[concurrency]") {
    Concurrency::SignalGroup<4> group;

    group.signal(2);
    REQUIRE(group.waitOne(0u) == 2u);
    REQUIRE(group.waitOne(0u) == Concurrency::SignalGroup<4>::NO_SIGNALS);
}

TEST_CASE("SignalGroup - waitOne consumes exactly one signal, lowest index first", "[concurrency]") {
    Concurrency::SignalGroup<4> group;

    group.signal(3);
    group.signal(1);
    group.signal(2);

    // Documented behaviour: when more than one signal is pending the lowest index wins
    REQUIRE(group.waitOne(0u) == 1u);
    REQUIRE(group.waitOne(0u) == 2u);
    REQUIRE(group.waitOne(0u) == 3u);
    REQUIRE(group.waitOne(0u) == Concurrency::SignalGroup<4>::NO_SIGNALS);
}

TEST_CASE("SignalGroup - waitOne(0) is non-blocking and returns fast", "[concurrency]") {
    Concurrency::SignalGroup<4> group;

    const auto start = std::chrono::steady_clock::now();
    const size_t index = group.waitOne(0u);
    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();

    REQUIRE(index == Concurrency::SignalGroup<4>::NO_SIGNALS);
    REQUIRE(elapsedMs < 50);   // Must not block at all
}

TEST_CASE("SignalGroup - waitOne(timeoutMs) times out and returns NO_SIGNALS", "[concurrency]") {
    Concurrency::SignalGroup<4> group;

    const auto start = std::chrono::steady_clock::now();
    const size_t index = group.waitOne(300u);
    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();

    REQUIRE(index == Concurrency::SignalGroup<4>::NO_SIGNALS);
    REQUIRE(elapsedMs >= 290);   // Small slack to absorb scheduling jitter
}

TEST_CASE("SignalGroup - blocking waitOne stays blocked until signaled by another thread", "[concurrency]") {
    Concurrency::SignalGroup<4> group;
    std::atomic<size_t> firedIndex{ 0u };
    std::atomic<bool> waiterDone{ false };

    std::thread waiter([&]() {
        firedIndex = group.waitOne();   // Indefinite blocking wait
        waiterDone = true;
    });

    // Give the waiter thread time to actually enter the wait
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    REQUIRE(!waiterDone);   // Must still be blocked, since nobody signaled yet

    group.signal(3);
    waiter.join();

    REQUIRE(waiterDone);
    REQUIRE(firedIndex == 3u);
}

TEST_CASE("SignalGroup - signal() raised before waitOne() is not lost", "[concurrency]") {
    Concurrency::SignalGroup<4> group;

    group.signal(2);   // Nobody is waiting yet

    std::atomic<size_t> firedIndex{ 0u };
    std::thread waiter([&]() { firedIndex = group.waitOne(); });
    waiter.join();

    REQUIRE(firedIndex == 2u);
}

TEST_CASE("SignalGroup - a signal does not release a waiter waiting on another index", "[concurrency]") {
    Concurrency::SignalGroup<4> group;
    std::atomic<bool> consumed{ false };

    std::thread waiter([&]() { consumed = group.wait(0u, 3000u); });

    // Let the waiter reach the blocking wait, then signal an index it is not waiting for
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    group.signal(2);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    REQUIRE(!consumed);   // Still blocked: index 2 is not the one it is waiting for

    group.signal(0);
    waiter.join();

    REQUIRE(consumed);
    REQUIRE(group.waitOne(0u) == 2u);   // The unrelated signal is still pending, untouched
}

TEST_CASE("SignalGroup - only one waiter is released per signal", "[concurrency]") {
    Concurrency::SignalGroup<4> group;
    std::atomic<int> consumedCount{ 0 };

    auto waiterBody = [&]() {
        if (group.waitOne(3000u) != Concurrency::SignalGroup<4>::NO_SIGNALS)
            ++consumedCount;
    };

    std::thread w1(waiterBody);
    std::thread w2(waiterBody);

    // Let both threads reach the blocking wait
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    group.signal(1);   // Only one of the two waiters must wake up and consume it
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    REQUIRE(consumedCount == 1);

    group.signal(3);   // Release the second waiter too
    w1.join();
    w2.join();

    REQUIRE(consumedCount == 2);
}

TEST_CASE("SignalGroup - repeated signal/consume cycles behave like auto-reset events", "[concurrency]") {
    Concurrency::SignalGroup<3> group;

    for (int i = 0; i < 10; ++i) {
        for (size_t index = 0; index < 3; ++index) {
            REQUIRE(!group.wait(index, 0u));   // Nothing pending at the start of each cycle
            group.signal(index);
            REQUIRE(group.wait(index, 0u));    // Consumes it
            REQUIRE(!group.wait(index, 0u));   // And it stays consumed
        }
    }
}

TEST_CASE("SignalGroup - a group of one behaves as a standalone Signal", "[concurrency]") {
    Concurrency::SignalGroup<1> group;

    REQUIRE(group.waitOne(0u) == Concurrency::SignalGroup<1>::NO_SIGNALS);
    group.signal(0);
    REQUIRE(group.waitOne(0u) == 0u);
    REQUIRE(group.waitOne(0u) == Concurrency::SignalGroup<1>::NO_SIGNALS);
    REQUIRE_THROWS_AS(group.signal(1), std::runtime_error);
}

TEST_CASE("SignalOfAGroup - proxies the group in both directions", "[concurrency]") {
    Concurrency::SignalGroup<4> group;
    auto proxy = group.getSignal(0);

    // Signaled through the proxy, consumed through the group
    proxy.signal();
    REQUIRE(group.waitOne(0u) == 0u);

    // Signaled through the group, consumed through the proxy
    group.signal(0);
    REQUIRE(proxy.wait(0u));
    REQUIRE(group.waitOne(0u) == Concurrency::SignalGroup<4>::NO_SIGNALS);
}

TEST_CASE("SignalOfAGroup - proxies of different indexes stay independent", "[concurrency]") {
    Concurrency::SignalGroup<4> group;
    auto proxy0 = group.getSignal(0);
    auto proxy1 = group.getSignal(1);

    proxy1.signal();

    REQUIRE(!proxy0.wait(0u));
    REQUIRE(proxy1.wait(0u));
    REQUIRE(!proxy1.wait(0u));
}

TEST_CASE("SignalOfAGroup - is a copyable handle onto the same signal", "[concurrency]") {
    Concurrency::SignalGroup<4> group;
    auto proxy = group.getSignal(2);
    auto copy = proxy;   // Lightweight handle: the copy points to the very same signal

    proxy.signal();
    REQUIRE(copy.wait(0u));     // Consumed through the copy
    REQUIRE(!proxy.wait(0u));   // So the original sees nothing left

    copy.signal();
    REQUIRE(group.waitOne(0u) == 2u);
}

TEST_CASE("SignalOfAGroup - both getSignal overloads address the same signal", "[concurrency]") {
    Concurrency::SignalGroup<4> group;
    auto runtimeProxy = group.getSignal(3);         // Bound checked at runtime
    auto compileTimeProxy = group.getSignal<3>();   // Bound checked by static_assert

    runtimeProxy.signal();
    REQUIRE(compileTimeProxy.wait(0u));

    compileTimeProxy.signal();
    REQUIRE(runtimeProxy.wait(0u));
}

TEST_CASE("SignalOfAGroup - exposes the same contract as Signal", "[concurrency]") {
    // The very same generic code must compile and behave identically on both types
    SECTION("on Signal") {
        Concurrency::Signal sig;
        checkSignalContract(sig);
    }

    SECTION("on SignalOfAGroup") {
        Concurrency::SignalGroup<4> group;
        auto proxy = group.getSignal(1);
        checkSignalContract(proxy);
    }
}

TEST_CASE("SignalOfAGroup - waitOne sees the signals raised through a proxy", "[concurrency]") {
    Concurrency::SignalGroup<4> group;
    auto proxy = group.getSignal(2);

    std::thread signaler([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        proxy.signal();
    });

    const size_t index = group.waitOne(5000u);
    signaler.join();

    REQUIRE(index == 2u);
}

// Static storage duration: getSignal<INDEX>() is a constant expression on it, so the proxies
// below are constant initialized, with no dynamic initialization at program startup
static Concurrency::SignalGroup<4> staticGroup;
static Concurrency::SignalOfAGroup<4> constantInitializedProxy = staticGroup.getSignal<2>();
constexpr auto constexprProxy = staticGroup.getSignal<1>();

TEST_CASE("SignalOfAGroup - the compile time getSignal<INDEX>() is constexpr and noexcept", "[concurrency]") {
    // The compile time overload replaces the runtime check with a static_assert, so it cannot throw
    STATIC_REQUIRE(noexcept(staticGroup.getSignal<0>()));
    STATIC_REQUIRE(!noexcept(staticGroup.getSignal(0u)));

    // The proxy is a literal type, which is what makes the constant initialization above possible
    STATIC_REQUIRE(std::is_trivially_copyable<Concurrency::SignalOfAGroup<4>>::value);
    STATIC_REQUIRE(std::is_trivially_destructible<Concurrency::SignalOfAGroup<4>>::value);

    // A constant initialized proxy is a fully working one
    constantInitializedProxy.signal();
    REQUIRE(staticGroup.waitOne(0u) == 2u);

    // A constexpr proxy is const, and wait()/signal() are non const as in Signal: copy it to use it
    auto usable = constexprProxy;
    usable.signal();
    REQUIRE(staticGroup.waitOne(0u) == 1u);
}

TEST_CASE("SignalGroup - producer/consumer handshake through proxies", "[concurrency]") {
    Concurrency::SignalGroup<2> group;
    auto ready = group.getSignal<0>();
    auto processed = group.getSignal<1>();
    std::atomic<int> sharedValue{ 0 };
    std::atomic<int> mismatches{ 0 };
    constexpr int iterations = 50;

    // REQUIRE is not thread safe, so the consumer only counts and the check is done on join
    std::thread consumer([&]() {
        for (int i = 0; i < iterations; ++i) {
            ready.wait();
            if (sharedValue != i + 1)
                ++mismatches;
            processed.signal();
        }
    });

    for (int i = 0; i < iterations; ++i) {
        sharedValue = i + 1;
        ready.signal();
        processed.wait();
    }

    consumer.join();

    REQUIRE(mismatches == 0);
}

TEST_CASE("SignalGroup - concurrent producers on every index are all served", "[concurrency]") {
    constexpr size_t signalCount = 4;
    constexpr int roundsPerProducer = 200;

    Concurrency::SignalGroup<signalCount> group;
    std::vector<std::thread> producers;

    for (size_t i = 0; i < signalCount; ++i) {
        auto proxy = group.getSignal(i);   // Captured by value: a proxy is a copyable handle
        producers.emplace_back([proxy]() mutable {
            for (int round = 0; round < roundsPerProducer; ++round) {
                proxy.signal();
                std::this_thread::yield();
            }
        });
    }

    int consumed[signalCount] = { 0 };
    for (;;) {
        const size_t index = group.waitOne(500u);
        if (index == Concurrency::SignalGroup<signalCount>::NO_SIGNALS)
            break;
        ++consumed[index];
    }

    for (auto& producer : producers)
        producer.join();

    // Drain whatever was signaled while the consumer was giving up
    for (;;) {
        const size_t index = group.waitOne(0u);
        if (index == Concurrency::SignalGroup<signalCount>::NO_SIGNALS)
            break;
        ++consumed[index];
    }

    for (size_t i = 0; i < signalCount; ++i) {
        // Signals are sticky, so a producer signaling faster than the consumer collapses several
        // signal() calls into one: the count is bounded, but no index may be starved completely
        REQUIRE(consumed[i] > 0);
        REQUIRE(consumed[i] <= roundsPerProducer);
    }
}
