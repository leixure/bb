/**
 * @file service.h
 * @brief Header file for the bb::Service interface.
 * 
 * @mainpage Boring Booking
 *
 * @section intro Introduction
 * This is the introduction section of the main page.
 *
 * @section install Installation
 * Instructions for installing the project.
 *
 * @section usage Usage
 * How to use the project.
 *
 * @section license License
 * Information about the project's license.
 */
#pragma once

#include <list>
#include <string>

namespace bb {

/**
 * @brief Each bit represents a seat.
 */
using SeatMask = size_t;
/**
 * @internal
 */
constexpr int BITS_PER_BYTE = 8;
/**
 * @brief The maximum number of seats a theater has.
 */
constexpr int MAX_SEATS = 20;
static_assert(sizeof(SeatMask) * BITS_PER_BYTE > MAX_SEATS, "Insufficient bits for representing the seats");
constexpr SeatMask NO_SEATS = 0;
constexpr SeatMask ALL_SEATS = (1 << MAX_SEATS) - 1;

/**
 * @brief The interface class for finding movies and booking seats in the showing theaters.
 * 
 * There's not much to say about it.
 */
class Service
{
public:
    using NameList = std::list<std::string>;
    /**
     * @brief Get the service instance.
     */
    static Service& instance();

    /**
     * @brief List all movies that are showing.
     * @return a NameList which contains all the movies that are showing in some theaters.
     */
    virtual NameList movies() const = 0;

    /**
     * @brief List all movies that are showing in the specified theater.
     * @param theater the name of the theater.
     * @return a NameList which contains all the movies that are showing in the specified theater.
     */
    virtual NameList movies(const std::string& theater) const = 0;

    /**
     * @brief List all theaters that are known by this service.
     * @return a NameList which contains all theaters that are known by this service.
     */
    virtual NameList theaters() const = 0;

    /**
     * @brief List all theaters that are showing in the specified movie.
     * @param movie the name of the movie.
     * @return a NameList which contains all the theaters that are showing the specified movie.
     */
    virtual NameList theaters(const std::string& movie) const = 0;

    /**
     * @brief Get the available seats of the specified movie that is showing in the specified theater.
     * @param movie the name of the movie.
     * @param theater the name of the theater.
     * @return a SeatMask that represent the available seats in the specified theater, with each
     *         bit represents a seat.
     */
    virtual SeatMask availableSeats(const std::string& movie, const std::string& theater) const = 0;

    /**
     * @brief Book seat(s) in the specified theater for the specified movie.
     * @param movie the name of the movie.
     * @param theater the name of the theater.
     * @param seat_mask the seats to book.
     * @return True if the seats are successfully booked, otherwise False.
     */
    virtual bool book(const std::string& movie, const std::string& theater, SeatMask seat_mask) = 0;
};

}   // namespace bb
