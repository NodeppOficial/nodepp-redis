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

        if( !url::is_valid( uri ) ){ 
            process::error("Invalid Redis Url");
        }

        auto host = url::hostname( uri );
        auto port = url::port( uri );
        auto auth = url::auth( uri );
        auto user = url::user( uri );
        auto pass = url::pass( uri );
        auto Auth = string_t();

        if( !user.empty() && !pass.empty() ){
             Auth = string::format("AUTH %s %s\n", user.get(), pass.get() );
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

        if( !Auth.empty() ){ exec( Auth ); }

    }
    
    /*─······································································─*/

    void exec( const string_t& cmd, const function_t<void,string_t>& cb ) const {
        if( obj->state == 0 || obj->fd.is_closed() )
          { return; }  obj->fd.write( cmd + "\n" ); 

        ptr_t<ulong> pos ({ 1, 0 }); string_t raw;

        START:; raw = obj->fd.read_line();
        
        if( !regex::test( raw, "[$*:]-?\\d+" ) ){ process::error( raw.slice(0,-2) ); }
        if(  regex::test( raw, "[$*]-1",true ) ){ return; }

        if( regex::test( raw, "[*]\\d+" ) ){
            pos[0] = string::to_ulong( regex::match( raw, "\\d+" ) );
            if( pos[0] == 0 ){ return; } goto START;
        } elif( regex::test( raw, "[$]\\d+" ) ) {
            pos[1] = string::to_ulong( regex::match( raw, "\\d+" ) ) + 2;
        } elif( regex::test( raw, "[:]\\d+" ) ) {
            cb( regex::match( raw, "\\d+" ) );
        }

        while( pos[0]-->0 ){ string_t data;
        while( data.size() != pos[1] ){
               data += obj->fd.read( pos[1]-data.size() );
        }      cb( data.slice( 0,-2 ) );
        if ( pos[0] != 0 ){ goto START; }
        }

    }

    array_t<string_t> exec( const string_t& cmd ) const {
        if( obj->state == 0 || obj->fd.is_closed() )
          { return nullptr; }  obj->fd.write( cmd + "\n" ); 

        ptr_t<ulong> pos ({ 1, 0 }); 
        array_t<string_t> res; 
        string_t raw;

        START:; raw = obj->fd.read_line();
        
        if( !regex::test( raw, "[$*:]-?\\d+" ) ){ process::error( raw.slice(0,-2) ); }
        if(  regex::test( raw, "[$*]-1",true ) ){ goto END; }

        if( regex::test( raw, "[*]\\d+" ) ){
            pos[0] = string::to_ulong( regex::match( raw, "\\d+" ) );
            if( pos[0] == 0 ){ goto END; } goto START;
        } elif( regex::test( raw, "[$]\\d+" ) ) {
            pos[1] = string::to_ulong( regex::match( raw, "\\d+" ) ) + 2;
        } elif( regex::test( raw, "[:]\\d+" ) ) {
            res.push( regex::match( raw, "\\d+" ) );
        }

        while( pos[0]-->0 ){ string_t data;
        while( data.size() != pos[1] ){
               data += obj->fd.read( pos[1]-data.size() );
        }      res.push( data.slice( 0,-2 ) );
        if ( pos[0] != 0 ){ goto START; }
        }

        END:; return res;
    }
    
    /*─······································································─*/

    string_t raw( const string_t& cmd ) const noexcept {
        if( obj->state == 0 || obj->fd.is_closed() )
          { return nullptr; }  obj->fd.write( cmd + "\n" );
        return obj->fd.read();
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
