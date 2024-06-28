/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_REDIS_TCP
#define NODEPP_REDIS_TCP

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/nodepp.h>
#include <nodepp/stream.h>
#include <nodepp/tcp.h>
#include <nodepp/url.h>

namespace nodepp { class redis_http_t {
protected:

    struct NODE {
        socket_t fd;
        int state=1;
    };  ptr_t<NODE> obj;

public:
    
    virtual ~redis_http_t() noexcept {
        if( obj.count() > 1 )
          { return; } free();
    }
    
    /*─······································································─*/

    virtual void free() const noexcept {
        if( obj->state == 0 )
          { return; }
            obj->state  = 0; 
    }
    
    /*─······································································─*/
    
    redis_http_t ( string_t uri ) : obj( new NODE ) {

        auto host = url::hostname( uri );
        auto port = url::port( uri );
        auto auth = url::auth( uri );
        auto user = url::user( uri );
        auto pass = url::pass( uri );

        string_t Auth = nullptr;

        if( !user.empty() && !pass.empty() ){
             Auth = string::format("AUTH %s %s \n", user.get(), pass.get() );
        } elif( !auth.empty() ){
             Auth = string::format("AUTH %s \n", auth.get() );
        }

        console::log( Auth, Auth.size() );

        auto agent = agent_t();
        agent.recv_timeout = 0;
        agent.send_timeout = 0;

        obj->fd = socket_t();
        obj->fd.IPPROTO = IPPROTO_TCP;
        obj->fd.socket( dns::lookup(host), port ); 
        obj->fd.set_sockopt( agent ); obj->fd.connect();

        if( !Auth.empty() ){ obj->fd.write( Auth ); }

    }
    
    /*─······································································─*/

    void exec( const string_t& cmd, const function_t<void,string_t>& cb ) const {
        if( obj->state == 0 || obj->fd.is_closed() )
          { return; } obj->fd.write( cmd + "\n" );

        string_t data; do { 
            data += obj->fd.read();
        } while( data.slice(-2) != "\r\n" );

        if( !data.empty() ){ cb( data.slice(0,-2) ); }

    }

    void exec( const string_t& cmd ) const {
        if( obj->state == 0 || obj->fd.is_closed() )
          { return; } obj->fd.write( cmd + "\n" ); 
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace redis { namespace http {

    template<class...T>
    redis_http_t add( const T&... args ) {
        return redis_http_t( args... );
    }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
