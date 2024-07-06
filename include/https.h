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

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_REDIS_GENERATOR
#define NODEPP_REDIS_GENERATOR

namespace nodepp { namespace _redis_ { GENERATOR( cb ){
protected:
    
    string_t raw, data;
    ptr_t<ulong> pos; 

public:

    template< class T, class V > gnEmit( T& fd, V& cb ){
    gnStart pos = ptr_t<ulong>({ 1, 0 }); coYield(1); raw = fd.read_line();
        
        if( !regex::test( raw, "[$*:]-?\\d+" ) ){ process::error( raw.slice(0,-2) ); }
        if(  regex::test( raw, "[$*]-1",true ) ){ coEnd; }

        if( regex::test( raw, "[*]\\d+" ) ){
            pos[0] = string::to_ulong( regex::match( raw, "\\d+" ) );
            if( pos[0] == 0 ){ coEnd; } coGoto(1);
        } elif( regex::test( raw, "[$]\\d+" ) ) {
            pos[1] = string::to_ulong( regex::match( raw, "\\d+" ) ) + 2;
        } elif( regex::test( raw, "[:]\\d+" ) ) {
            cb( regex::match( raw, "\\d+" ) );
        }

        while( pos[0]-->0 ){ data.clear();
        while( data.size() != pos[1] ){
               data += fd.read( pos[1]-data.size() );
        }      cb( data.slice( 0,-2 ) ); coNext;
        if ( pos[0] != 0 ){ coGoto(1); }
        }

    gnStop
    }

};}}

#endif

/*────────────────────────────────────────────────────────────────────────────*/

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
        _redis_::cb _cb_; process::add( _cb_, obj->fd, cb );
    }

    array_t<string_t> exec( const string_t& cmd ) const {
        if( obj->state == 0 || obj->fd.is_closed() )
          { return nullptr; } obj->fd.write( cmd + "\n" ); 
            array_t<string_t> res;

        function_t<void,string_t> cb([&]( string_t data ){ res.push( data ); });
        _redis_::cb _cb_; process::await( _cb_, obj->fd, cb ); return res;
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