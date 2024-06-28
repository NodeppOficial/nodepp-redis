/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_REDIS_TLS
#define NODEPP_REDIS_TLS

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/nodepp.h>
#include <nodepp/stream.h>
#include <nodepp/tls.h>
#include <nodepp/url.h>

namespace nodepp { class redis_https_t {
protected:

    struct NODE {
        int state =1;
        ssocket_t fd;
    };  ptr_t<NODE> obj;

public:
    
    virtual ~redis_https_t() noexcept {
        if( obj.count() > 1 )
          { return; } free();
    }
    
    /*─······································································─*/

    virtual void free() const noexcept {
        if( obj->state == 0 ){ return; }
            obj->state  = 0; 
    }
    
    /*─······································································─*/
    
    redis_https_t ( string_t uri, ssl_t* ssl ) : obj( new NODE ) {

        auto host = url::hostname( uri );
        auto port = url::port( uri );
        auto auth = url::auth( uri );
        auto user = url::user( uri );
        auto pass = url::pass( uri );

        string_t Auth = nullptr;

        if( !user.empty() && !pass.empty() ){
             Auth = string::format("AUTH%s %s\n", user.get(), pass.get() );
        } elif( !auth.empty() ){
             Auth = string::format("AUTH %s\n", auth.get() );
        }

        auto agent = agent_t();
        agent.recv_timeout = 0;
        agent.send_timeout = 0;

        obj->fd = ssocket_t( ssl );
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
        
        if( !data.empty() ){ cb( data ); }
    }

    void exec( const string_t& cmd ) const {
        if( obj->state == 0 || obj->fd.is_closed() )
          { return; } obj->fd.write( cmd + "\n" ); 
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace redis { namespace https {

    template<class...T>
    redis_https_t add( const T&... args ) {
        return redis_https_t( args... );
    }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
