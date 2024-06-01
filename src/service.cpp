#include "service.h"

#include <algorithm>
#include <iterator>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <unordered_set>

using namespace std;
using namespace bb;

struct BookingRecord
{
    string movie_name;
    string theater_name;
    SeatMask booked_mask;
};

static BookingRecord booking_table[] = {
    {"Kingdom of the Planet of the Apes", "Landmark Cinemas", ALL_SEATS},
    {"Kingdom of the Planet of the Apes", "Galaxy Cinemas", 0},
    {"Kingdom of the Planet of the Apes", "Cinema Paradiso", 0},
    {"Furiosa: A Mad Max Saga", "Landmark Cinemas", 0xdead},
    {"Furiosa: A Mad Max Saga", "Galaxy Cinemas", 0xbeef},
    {"Garfield Movie, The", "Landmark Cinemas", 0xc0ca},
    {"Garfield Movie, The", "Cinema Paradiso", 0xc01a},
    {"Back to Black", "Galaxy Cinemas", 0},
    {"Back to Black", "Cinema Paradiso", 0},
};

class ServiceImpl : public Service
{
    class GuardedRecord
    {
        BookingRecord& _record;
        mutex _m;

        mutex& getMutex()
        {
            // NOTE: use a static mutex if per-record mutex is deemed too expensive
            return _m;
        }

    public:
        explicit GuardedRecord(BookingRecord& record) : _record(record) {}

        SeatMask availableSeats() const
        {
            return (~_record.booked_mask) & ALL_SEATS;
        }

        bool book(SeatMask seat_mask)
        {
            if (seat_mask == 0 || (seat_mask & ALL_SEATS) != seat_mask) {
                throw invalid_argument("book: invalid seat_mask");
            }

            const lock_guard<mutex> lock(getMutex());

            if (_record.booked_mask & seat_mask) {
                // not all seats are available
                return false;
            }

            _record.booked_mask |= seat_mask;

            return true;
        }
    };

    using TheaterMap = unordered_map<string, GuardedRecord*>;
    using MovieMap = unordered_map<string, TheaterMap>;
    MovieMap _movie_map;

public:
    template<typename Iter>
    ServiceImpl(Iter first, Iter last)
    {
        // build index from booking records
        for (auto it = first; it != last; ++it) {
            _movie_map[it->movie_name][it->theater_name] = new GuardedRecord(*it);
        }
    }
    ~ServiceImpl()
    {
        for (auto& [movie, theaters] : _movie_map) {
            for (auto& [theater, rec] : theaters) {
                delete rec;
            }
        }
    }

    virtual NameList movies() const
    {
        NameList names;
        transform(_movie_map.begin(), _movie_map.end(), back_inserter(names),
            [](const auto& elem){ return elem.first; });
        return names;
    }

    virtual NameList movies(const std::string& theater) const
    {
        NameList names;
        for (auto& [movie, theaters] : _movie_map) {
            if (theaters.find(theater) != theaters.end()) {
                names.push_back(movie);
            }
        }
        if (names.empty()) {
            throw invalid_argument("movies: theater not found");
        }
        return names;
    }

    virtual NameList theaters() const
    {
        unordered_set<string> names;
        for (auto& [movie, theaters] : _movie_map) {
            transform(theaters.begin(), theaters.end(), inserter(names, names.begin()),
                [](const auto& elem){ return elem.first; });
        }
        return NameList{names.begin(), names.end()};
    }

    virtual NameList theaters(const string& movie) const
    {
        auto it = _movie_map.find(movie);
        if (it == _movie_map.end()) {
            throw invalid_argument("theaters: movie not found");
        }
        const auto& theater_map = it->second;
        NameList names;
        transform(theater_map.begin(), theater_map.end(), back_inserter(names),
            [](const auto& elem){ return elem.first; });
        return names;
    }

    virtual SeatMask availableSeats(const string& movie, const string& theater) const
    {
        return record(movie, theater)->availableSeats();
    }

    virtual bool book(const string& movie, const string& theater, SeatMask seat_mask)
    {
        return record(movie, theater)->book(seat_mask);
    }

private:
    GuardedRecord* record(const string& movie, const string& theater)
    {
        auto it_movie = _movie_map.find(movie);
        if (it_movie == _movie_map.end()) {
            throw invalid_argument("record: movie not found");
        }
        auto& theater_map = it_movie->second;
        auto it_theater = theater_map.find(theater);
        if (it_theater == theater_map.end()) {
            throw invalid_argument("record: theater not found");
        }
        return it_theater->second;
    }

    const GuardedRecord* record(const string& movie, const string& theater) const
    {
        return const_cast<ServiceImpl*>(this)->record(movie, theater);
    }
};

Service& Service::instance()
{
    static ServiceImpl service{begin(booking_table), end(booking_table)};
    return service;
}
