// Test ServiceImpl
#include <algorithm>
#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/service.cpp"

using namespace ::testing;

namespace {

class ServiceTest : public Test
{
protected:
    vector<BookingRecord> br = {
        { "MA", "TA", ALL_SEATS },
        { "MA", "TC", 0 },
        { "MB", "TA", 0 },
        { "MB", "TB", ALL_SEATS },
        { "MC", "TB", 0 },
        { "MC", "TC", ALL_SEATS },
    };
    ServiceImpl service{br.begin(), br.end()};
};

TEST_F(ServiceTest, moviesAll) {
    EXPECT_THAT(service.movies(), UnorderedElementsAre("MA", "MB", "MC"));
}

TEST_F(ServiceTest, moviesInTheater) {
    EXPECT_THAT(service.movies("TA"), UnorderedElementsAre("MA", "MB"));
    EXPECT_THAT(service.movies("TB"), UnorderedElementsAre("MB", "MC"));
    EXPECT_THAT(service.movies("TC"), UnorderedElementsAre("MA", "MC"));
    // nonexistent theater
    EXPECT_THROW(service.movies("TD"), invalid_argument);
}

TEST_F(ServiceTest, theatersAll) {
    EXPECT_THAT(service.theaters(), UnorderedElementsAre("TA", "TB", "TC"));
}

TEST_F(ServiceTest, theatersOfMovie) {
    EXPECT_THAT(service.theaters("MA"), UnorderedElementsAre("TA", "TC"));
    EXPECT_THAT(service.theaters("MB"), UnorderedElementsAre("TA", "TB"));
    EXPECT_THAT(service.theaters("MC"), UnorderedElementsAre("TB", "TC"));
    // nonexistent movie
    EXPECT_THROW(service.theaters("MD"), invalid_argument);
}

TEST_F(ServiceTest, availableSeats) {
    EXPECT_EQ(service.availableSeats("MA", "TA"), 0);
    EXPECT_EQ(service.availableSeats("MA", "TC"), ALL_SEATS);
    EXPECT_EQ(service.availableSeats("MB", "TA"), ALL_SEATS);
    EXPECT_EQ(service.availableSeats("MB", "TB"), 0);
    EXPECT_EQ(service.availableSeats("MC", "TB"), ALL_SEATS);
    EXPECT_EQ(service.availableSeats("MC", "TC"), 0);
    // nonexistent combinations
    EXPECT_THROW(service.availableSeats("MA", "TB"), invalid_argument);
    EXPECT_THROW(service.availableSeats("MB", "TC"), invalid_argument);
    EXPECT_THROW(service.availableSeats("MC", "TA"), invalid_argument);
    EXPECT_THROW(service.availableSeats("MD", "TD"), invalid_argument);
}

TEST_F(ServiceTest, bookAvailable) {
    for (SeatMask m = 1; m & ALL_SEATS; m <<= 1) {
        EXPECT_TRUE(service.book("MA", "TC", m));
        EXPECT_TRUE(service.book("MB", "TA", m));
        EXPECT_TRUE(service.book("MC", "TB", m));
    }
    EXPECT_EQ(service.availableSeats("MA", "TC"), NO_SEATS);
    EXPECT_EQ(service.availableSeats("MB", "TA"), NO_SEATS);
    EXPECT_EQ(service.availableSeats("MC", "TB"), NO_SEATS);
}

TEST_F(ServiceTest, bookMultiple) {
    EXPECT_TRUE(service.book("MA", "TC", 0x0a));
    EXPECT_TRUE(service.book("MA", "TC", 0xa0));
    EXPECT_TRUE(service.book("MA", "TC", 0x05));
    EXPECT_TRUE(service.book("MA", "TC", 0x50));
    for (SeatMask i = 1; i < 0x100; i <<= 1) {
        EXPECT_FALSE(service.book("MA", "TC", i));
    }
    EXPECT_FALSE(service.book("MA", "TC", ALL_SEATS));

    EXPECT_TRUE(service.book("MB", "TA", 0x0a));
    // partially overlapped seat masks
    EXPECT_FALSE(service.book("MB", "TA", 0x08));

    EXPECT_TRUE(service.book("MC", "TB", 0xa0));
    // completely overlapped seat masks
    EXPECT_FALSE(service.book("MC", "TB", 0xa0));
}

    vector<BookingRecord> br = {
        { "MA", "TA", ALL_SEATS },
        { "MA", "TC", 0 },
        { "MB", "TA", 0 },
        { "MB", "TB", ALL_SEATS },
        { "MC", "TB", 0 },
        { "MC", "TC", ALL_SEATS },
    };

TEST_F(ServiceTest, bookOccupied) {
    for (SeatMask m = 1; m & ALL_SEATS; m <<= 1) {
        EXPECT_FALSE(service.book("MA", "TA", m));
        EXPECT_FALSE(service.book("MB", "TB", m));
        EXPECT_FALSE(service.book("MC", "TC", m));
    }
}

TEST_F(ServiceTest, bookConcurrent) {
    // XXX use fewer threads if this test is aborted due to system limit
    constexpr size_t TOTAL_THREADS = 1000;

    vector<promise<SeatMask>> promises(TOTAL_THREADS);
    vector<future<SeatMask>> futures;
    transform(promises.begin(), promises.end(), back_inserter(futures), [](auto& p){ return p.get_future(); });

    mutex m;
    condition_variable cv;
    bool ready = false;

    auto booking = [&](SeatMask seat_mask, promise<SeatMask> booking_promise) {
        unique_lock<mutex> lk(m);
        // wait for signal from the test before booking seats
        cv.wait(lk, [&]{ return ready; });
        // MA/TC has no seats booked initially
        booking_promise.set_value(service.book("MA", "TC", seat_mask) ? seat_mask : NO_SEATS);
    };

    vector<thread> threads;
    threads.reserve(TOTAL_THREADS);

    // create the booking threads
    for (SeatMask i = 1; i <= TOTAL_THREADS; ++i) {
        threads.emplace_back(booking, i, move(promises[i - 1]));
    }

    // let the booking threads all start booking
    {
        lock_guard<mutex> lk(m);
        ready = true;
    }
    cv.notify_all();

    for (int i = 0; i < TOTAL_THREADS; ++i) {
        if (threads[i].joinable()) {
            threads[i].join();
        }
    }

    SeatMask booked_mask = NO_SEATS;
    SeatMask cur_mask;
    // verify there's no over bookings
    for (auto& f : futures) {
        cur_mask = f.get();
        EXPECT_EQ(booked_mask & cur_mask, 0);
        booked_mask |= cur_mask;
    }
    EXPECT_EQ(booked_mask & service.availableSeats("MA", "TC"), 0);
    EXPECT_EQ(booked_mask | service.availableSeats("MA", "TC"), ALL_SEATS);
}

TEST_F(ServiceTest, bookInvalidMask) {
    EXPECT_THROW(service.book("MA", "TA", NO_SEATS), invalid_argument);
    EXPECT_THROW(service.book("MA", "TC", ALL_SEATS << 1), invalid_argument);
}

}

