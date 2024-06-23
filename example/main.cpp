#include <nodepp/nodepp.h>
#include <redis.h>

using namespace nodepp;

void onMain() {

    mariadb_t db ("db://auth@localhost:8000");

    db.exec(R"( SET FOO BAT )");

    db.exec("GET FOO",[]( string_t data ){
        console::log( "->", data )
    });

}
