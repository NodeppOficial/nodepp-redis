#ifndef NODEPP_REDIS
#define NODEPP_REDIS

/*────────────────────────────────────────────────────────────────────────────*/

#include <hiredis/hiredis.h>
#include <nodepp/nodepp.h>
#include <nodepp/url.h>

namespace nodepp { class redis_t {
protected:

    struct NODE {
        redisContext *fd = nullptr;
        int state = 1;
    };  ptr_t<NODE> obj;

public:
    
    virtual ~redis_t() noexcept {
        if( obj.count() > 1 || obj->fd == nullptr ){ return; }
        if( obj->state == 0 ){ return; } free();
    }
    
    /*─······································································─*/

    virtual void free() const noexcept {
        if( obj->fd == nullptr ){ return; }
        if( obj->state == 0 )   { return; }
        redisFree( obj->fd );
        obj->state = 0; 
    }
    
    /*─······································································─*/
    
    redis_t ( string_t uri ) : obj( new NODE ) {

        auto host = url::hostname( uri );
        auto port = url::port( uri );
        auto auth = url::auth( uri );

        obj->fd = redisConnect( host.get(), port.get(), auth.get() );

        if( obj->fd->err ) 
          { process::error( obj->fd->errstr ); }

    }
    
    /*─······································································─*/

    void exec( const string_t& cmd, const function_t<void,string_t>& cb ) const {

        redisReply *reply = redisCommand( obj->fd, cmd );
        if( reply == NULL ) {
            freeReplyObject( reply );
            process::error( obj->fd->errstr );
        }   

        cb( reply->str ); freeReplyObject( reply );

    }

    void exec( const string_t& cmd ) const {
        redisReply *reply = redisCommand( obj->fd, cmd );
        if( reply == NULL ) {
            freeReplyObject( reply );
            process::error( obj->fd->errstr );
        }  
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
