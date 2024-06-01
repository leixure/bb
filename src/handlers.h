/*
 * HTTP request handlers
 */
#pragma once

namespace httplib {
    class Request;
    class Response;
}

namespace bb {

void getMovie(const httplib::Request &req, httplib::Response &res);
void getTheater(const httplib::Request &req, httplib::Response &res);
void postBook(const httplib::Request &req, httplib::Response &res);

}   // namespace bb
