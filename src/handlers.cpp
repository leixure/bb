#include "handlers.h"

#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <httplib/httplib.h>

#include "html.h"
#include "service.h"

using namespace std;

namespace bb {

static constexpr const char * COMMON_HEADER = R"(<html>
<head>
<style>
body {
    margin: 0px;
}

span {
    font-weight: bold;
    border: 2px solid coral;
    background-color: white;
    color: coral;
    border-radius: 10px;
    padding: 5px;
}
.footer span {
    color: royalblue;
    border: royalblue;
    background-color: white;
}

div.container {
    margin: 10px;
}

.header {
    background-color: royalblue;
    color: white;
    list-style-type: none;
    padding: 0;
    margin: 0;
    display: flex;
}
.header li {
    padding: 10px;
}
.header a {
    color: inherit;
    background-color: inherit;
    text-decoration: none;
}
.header a:hover {
    text-decoration: underline;
}

.selected {
    background-color: white;
    color: royalblue;
    font-weight: bold;
}

.footer {
    background-color: royalblue;
    color: white;
    text-align: center;
    position: fixed;
    bottom: 0;
    width: 100%;
    height: 50px;
    line-height: 50px;
}

.seat-map {
    display: grid;
    grid-template-columns: repeat(5, 1fr);
    gap: 10px;
}
.seat {
    display: flex;
    justify-content: center;
    align-items: center;
}
.seat label {
    display: block;
    width: 40px;
    height: 40px;
    background-color: #ffffff;
    border: 1px solid #ccc;
    border-radius: 5px;
    text-align: center;
    line-height: 40px;
    cursor: pointer;
}
.seat input[type="checkbox"] {
    display: none;
}
.seat input[type="checkbox"]:checked + label {
    background-color: royalblue;
    color: white;
    cursor: pointer;
}
.seat input[type="checkbox"]:disabled + label {
    background-color: coral;
    color: white;
    cursor: not-allowed;
}
.form-container {
    display: none;
    text-align: center;
}
.book-button {
    margin-top: 20px;
    padding: 10px 20px;
    background-color: royalblue;
    color: white;
    border: none;
    border-radius: 5px;
    cursor: pointer;
}
.book-button:hover {
    background-color: navy;
}

#status {
    padding: 5px;
}
.success {
    color: darkgreen;
    background-color: #e0ffe0;
}
.fail {
    color: #ff6060;
    background-color: #ffe0e0;
}
</style>
<script>
    function setStatus(ok, message) {
        const status = document.getElementById('status');
        status.className = ok ? 'success' : 'fail';
        status.innerHTML = '<pre>' + message + '</pre>';
    }

    function addSeatMap(movie, theater, availableSeats) {
        const seatMap = document.querySelector('.seat-map');

        for (let i = 1; i <= 20; i++) {
            const seatId = `seat${i}`;
            const seatDiv = document.createElement('div');
            seatDiv.classList.add('seat');
            const seatCheckbox = document.createElement('input');
            seatCheckbox.type = 'checkbox';
            seatCheckbox.id = seatId;
            seatCheckbox.name = 'seats';
            seatCheckbox.value = seatId;

            if (!((1 << (i - 1)) & availableSeats)) {
                seatCheckbox.checked = true;
                seatCheckbox.disabled = true;
            }

            const seatLabel = document.createElement('label');
            seatLabel.htmlFor = seatId;
            seatLabel.textContent = i;

            seatDiv.appendChild(seatCheckbox);
            seatDiv.appendChild(seatLabel);
            seatMap.appendChild(seatDiv);
        }

        const seatForm = document.getElementById('seatForm');
        seatForm.addEventListener('submit', function(event) {
            event.preventDefault();
            const selectedSeats = Array.from(seatForm.querySelectorAll('input[type="checkbox"]:checked:not(:disabled)'))
                                        .map(checkbox => parseInt(checkbox.value.substr(4)));
            let seatMask = selectedSeats.reduce((acc, bitPosition) => acc | (1 << (bitPosition - 1)), 0);
            fetch(`/book?movie=${movie}&theater=${theater}&seatMask=${seatMask}`, {method: 'POST'})
            .then(response => {
                if (response.ok) {
                    setStatus(true, "Seat(s) booked successfully.");
                } else {
                    return response.json().then(errorData => {
                        throw new Error(errorData.error + ': ' + errorData.message);
                    });
                }
            })
            .catch(error => {
                setStatus(false, error.message);
            });
        });
    }
</script>
</head>
<body>
    <ul class="header">
        <li id="movie"><a href="/movie">Movies</a></li>
        <li id="theater"><a href="/theater">Theaters</a></li>
        <li id="doc"><a href="/doc/index.html" target="_blank">Documentation</a></li>
    </ul>
    <div id="status"></div>
<div class="container">
)";

static constexpr const char * COMMON_TAIL = R"(
    <div class="form-container">
        <form id="seatForm" action="/book" method="post">
            <div class="seat-map">
            </div>
            <button type="submit" class="book-button">Book</button>
        </form>
    </div>
</div>
<div style="height:50px"></div>
<div class="footer"><span>Boring Booking</span> presents</div>
</body></html>
)";

static void errorResponse(httplib::Response &res, int code, const string& error, const string& message)
{
    ostringstream out;
    out << "{" << endl
        << "  \"error\": \"" << error << "\"," << endl
        << "  \"message\": \"" << message << "\"" << endl
        << "}";
    res.status = code;
    res.set_content(out.str(), "text/json");
}

static string selectTab(const string& tab_id)
{
    ostringstream out;
    out << html::tag("script", html::echo(
        "let tab = document.getElementById(\"", tab_id, "\");",
        "tab.classList.add('selected');"
    ));
    return out.str();
}

static string showSeatForm(const string& movie, const string& theater, SeatMask available_seats)
{
    ostringstream out;
    out << html::li(false, html::echo(hex, available_seats));
    out << html::tag("script", html::echo(R"(
         document.addEventListener('DOMContentLoaded', function() {
            document.querySelector('.form-container').style.display = 'block';)",
        "   addSeatMap(\"", movie, "\", \"", theater, "\", ", dec, available_seats, ");"
        "});"
        ));
    return out.str();
}

void getMovie(const httplib::Request &req, httplib::Response &res)
{
    ostringstream out;

    auto selected_movie{req.get_param_value("name")};
    auto selected_theater{req.get_param_value("theater")};

    // show all movies that are showing
    out << COMMON_HEADER << selectTab("movie");
    for (auto movie : Service::instance().movies()) {
        out << html::li(movie == selected_movie, html::a(html::echo("/movie?name=", movie), movie));
    }

    if (req.has_param("name")) {
        // show the list of theaters that are showing the movie
        try {
            out << html::tag("h1", html::echo("Theaters that are showing ",
                    html::tag("span", selected_movie)));
            for (auto theater : Service::instance().theaters(selected_movie)) {
                out << html::li(theater == selected_theater,
                        html::a(html::echo("/movie?name=", selected_movie, "&theater=", theater), theater));
            }
        } catch (invalid_argument e) {
            ostringstream message;
            message << "The movie '" << selected_movie << "' is not found: " << e.what();
            errorResponse(res, 404, "PageNotFound", message.str());
            return;
        }

        if (req.has_param("theater")) {
            // show seat map
            try {
                auto available_seats = Service::instance().availableSeats(selected_movie, selected_theater);
                out << html::tag("h1", "Book your seat(s):")
                    << showSeatForm(selected_movie, selected_theater, available_seats);
            } catch (invalid_argument e) {
                ostringstream message;
                message << "The movie '" << selected_movie << "' is not showing in '"
                        << selected_theater << "': " << e.what();
                errorResponse(res, 404, "PageNotFound", message.str());
                return;
            }
        }
    }

    out << COMMON_TAIL;
    res.set_content(out.str(), "text/html");
}

void getTheater(const httplib::Request &req, httplib::Response &res)
{
    ostringstream out;

    auto selected_theater{req.get_param_value("name")};
    auto selected_movie{req.get_param_value("movie")};

    // show all theaters
    out << COMMON_HEADER << selectTab("theater");
    for (auto theater : Service::instance().theaters()) {
        out << html::li(theater == selected_theater,
                html::a(html::echo("/theater?name=", theater), theater));
    }

    if (req.has_param("name")) {
        // show the list of movies that are showing in current theater
        try {
            out << html::tag("h1", html::echo("Movies that are showing in ",
                    html::tag("span", selected_theater), ':'));
            for (auto movie : Service::instance().movies(selected_theater)) {
                out << html::li(movie == selected_movie,
                        html::a(html::echo("/theater?name=", selected_theater, "&movie=", movie), movie));
            }
        } catch (invalid_argument e) {
            ostringstream message;
            message << "The theater '" << selected_theater << "' is not found: " << e.what();
            errorResponse(res, 404, "PageNotFound", message.str());
            return;
        }

        if (req.has_param("movie")) {
            // show seat map
            try {
                auto available_seats = Service::instance().availableSeats(selected_movie, selected_theater);
                out << html::tag("h1", "Book your seat(s):")
                    << showSeatForm(selected_movie, selected_theater, available_seats);
            } catch (invalid_argument e) {
                ostringstream message;
                message << "The theater '" << selected_theater << "' is not showing the movie '"
                        << selected_movie << "': " << e.what();
                errorResponse(res, 404, "PageNotFound", message.str());
                return;
            }
        }
    }

    out << COMMON_TAIL;
    res.set_content(out.str(), "text/html");
}

void postBook(const httplib::Request &req, httplib::Response &res)
{
    auto movie = req.get_param_value("movie");
    auto theater = req.get_param_value("theater");
    auto seatMask = stoul(req.get_param_value("seatMask"));
    try {
        if (!Service::instance().book(movie, theater, seatMask)) {
            errorResponse(res, 409, "SeatAlreadyBooked", "The seat(s) you are booking are not available");
        }
    } catch(invalid_argument e) {
        errorResponse(res, 500, "InternalServerError", e.what());
    }
}

}   // namespace bb
