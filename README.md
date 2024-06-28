# NODEPP-REDIS
Run **Redis** in Nodepp

## Dependencies

- libhiredis-dev
  - 🪟: `pacman -S mingw-w64-x86_64-hiredis`
  - 🐧: `sudo apt install libhiredis-dev`

## Example
```cpp
#include <nodepp/nodepp.h>
#include <http.h>

using namespace nodepp;

void onMain() {

    redis_t db ("db://auth@localhost:8000");

    db.exec(" SET FOO BAT ");

    db.exec("GET FOO",[]( string_t data ){
        console::log( "->", data )
    });

}
```

## Compilation
`g++ -o main main.cpp -I ./include -lhiredis ; ./main`
