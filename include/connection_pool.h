#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include <memory>
#include <list>
#include <mutex>
#include <thread>

class ConnectionPool {
public:
    struct Config {

    };

    struct Connection {
        virtual bool connect() = 0;
        virtual bool ping() = 0;
        virtual void close() = 0;
    };

    using Ptr=std::shared_ptr<Connection>;

    class Creator {
        virtual Ptr create() = 0;
    };

    ConnectionPool(const Config& config, std::unique_ptr<Creator> creator);
    ~ConnectionPool();

    Ptr getConn();

    void relConn(Ptr conn);

private:
    std::list<Ptr> idle_conns;
    std::list<Ptr> active_conns;
    std::thread reaper;
    std::mutex conn_mutex;
};

#endif
