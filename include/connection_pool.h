#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include <memory>
#include <list>
#include <mutex>
#include <thread>
#include <exception>
#include <set>

class ConnectionPool {
public:

    struct wrong_operation : public std::runtime_error {
        wrong_operation(const std::string& msg);
    };
    struct wrong_connection : public std::runtime_error {
        wrong_connection();
    };
    struct connection_timeout : public std::runtime_error {
        connection_timeout(unsigned int retry_count);
    };
    struct fatal_error : public std::runtime_error {
        fatal_error(const std::string& msg);
    };

    struct Config {
        size_t init_conn_num=10;
        unsigned int max_retry_count=5;
        time_t retry_interval_milli=1000;
    };

    struct Connection {
        // ping shouldn't be time consuming
        virtual bool ping() = 0;
        // close shouldn't be time consuming
        virtual void close() = 0;
    };

    using Ptr=std::shared_ptr<Connection>;

    struct Creator {

        /* returned value expected to be available to use if not nullptr
         * no throw
         * return nullptr if failed to create
         */
        virtual Ptr create() = 0;
    };

    ConnectionPool(const Config& config, std::unique_ptr<Creator>&& creator);
    ~ConnectionPool();

    /*
     * throw ConnectionPool::wrong_operation
     */
    void start();

    /*
     * throw ConnectionPool::wrong_operation
     */
    void stop();

    /*
     * throw ConnectionPool::wrong_connection
     */
    Ptr getConn();

    void relConn(Ptr conn);

private:
    enum class Stage {
        IDLE,
        BUSY,
    };

    Stage stage;
    Config config;
    std::list<Ptr> idle_conns;
    std::set<Ptr> active_conns;
    std::thread reaper;
    std::mutex conn_mutex;
    std::unique_ptr<Creator> creator;

    bool populate();
    bool clear();
    Ptr pickOneFromIdleList();
    void appendToActiveList(Ptr conn);
    // create new connection with retry
    Ptr createNewConn();
    bool removeFromActiveList(Ptr conn);
    void appendToIdleList(Ptr conn);
};

#endif
