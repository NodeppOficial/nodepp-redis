
#include <nodepp/nodepp.h>
#include <http.h>

using namespace nodepp;

void onMain() {

    auto db = redis::http::add("db://localhost:6379");

    db.exec("LRANGE FOO 0 -1",[]( string_t data ){
        console::log( "->", data );
    });

}