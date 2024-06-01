#include <csignal>
#include <iostream>
#include <string>
#include <vector>

#include <httplib/httplib.h>

#include "handlers.h"

using namespace std;
using namespace bb;

int main(int argc, char** argv) {
    // ignore Ctrl-C
    signal(SIGINT, [](int signum) {
        cout << "Goodbye!" << endl;
        exit(0);
    });

    httplib::Server svr;

    if (argc > 1) {
        // mount the project doc to /doc 
        svr.set_mount_point("/doc", argv[1]);
    }

    svr.Get("/", getMovie);
    svr.Get("/movie", getMovie);
    svr.Get("/theater", getTheater);
    svr.Post("/book", postBook);

    cout << "Navigate to http://localhost:8080" << endl;
    svr.listen("0.0.0.0", 8080);

    return 0;
}
