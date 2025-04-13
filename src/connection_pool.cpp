#include "connection_pool.h"

using namespace std;

ConnectionPool::wrong_operation::wrong_operation(const string& msg)
    : runtime_error(msg) {}

ConnectionPool::wrong_connection::wrong_connection()
    : runtime_error("you are release an unknown connection") {}

ConnectionPool::connection_timeout::connection_timeout(unsigned int retry_count)
    : runtime_error("failed to get connection after "+to_string(retry_count)+" retries") {}

ConnectionPool::num_limit_exceeded::num_limit_exceeded(const string& msg)
    : runtime_error(msg) {}

ConnectionPool::fatal_error::fatal_error(const string& msg)
    : runtime_error(msg) {}

ConnectionPool::ConnectionPool(
    const Config& config, std::unique_ptr<Creator>&& creator
) : config(config),creator(move(creator)) {
    stage=Stage::IDLE;
}

ConnectionPool::~ConnectionPool() {
    if(stage==Stage::BUSY) {
        stop();
    }
}

void ConnectionPool::start() {
    lock_guard<mutex> lg(pool_mutex);
    if(stage==Stage::BUSY) {
        throw wrong_operation("pool is already BUSY");
    }
    try{
        populate();
        stage=Stage::BUSY;
    } catch (connection_timeout& e) {
        if(!clear()) {
            throw fatal_error("can't clear pool after failing to poplulate pool");
        }
        throw wrong_operation(string("starting pool failed caused by\n")+e.what());
    }
}

void ConnectionPool::populate() {
    for(size_t i=0;i<config.init_conn_num;++i) {
        idle_conns.push_back(createNewConn());
    }
}

void ConnectionPool::stop() {
    lock_guard<mutex> lg(pool_mutex);
    if(stage==Stage::IDLE) {
        throw wrong_operation("pool is already IDLE");
    }
    if(clear()) {
        stage=Stage::IDLE;
    } else {
        throw wrong_operation("failed to stop pool unreleased connections detected");
    }
}

bool ConnectionPool::clear() {
    if(active_conns.size()!=0) {
        return false;
    }
    for(auto i:idle_conns) {
        i->close();
    }
    idle_conns.clear();
    return true;
}

ConnectionPool::Ptr ConnectionPool::getConn() {
    lock_guard<mutex> lg(pool_mutex);
    if(stage!=Stage::BUSY) {
        throw wrong_operation("connection available only in BUSY stage");
    }
    auto conn=pickOneFromIdleList();
    if(!conn) {
        conn=createNewConn();
    }
    appendToActiveList(conn);
    return conn;
}

ConnectionPool::Ptr ConnectionPool::pickOneFromIdleList() {
    for(auto iter=idle_conns.begin();iter!=idle_conns.end();++iter) {
        auto conn=*iter;
        if(conn->ping()) {
            idle_conns.erase(iter);
            return conn;
        }
    }
    return nullptr;
}

void ConnectionPool::appendToActiveList(Ptr conn) {
    active_conns.insert(conn);
}

ConnectionPool::Ptr ConnectionPool::createNewConn() {
    for(unsigned int i=0;i<config.max_retry_count;++i) {
        auto ret=creator->create();
        if(ret) {
            return ret;
        }
        this_thread::sleep_for(chrono::milliseconds(config.retry_interval_milli));
    }
    throw connection_timeout(config.max_retry_count);
}

void ConnectionPool::relConn(Ptr conn) {
    lock_guard<mutex> lg(pool_mutex);
    if(stage!=Stage::BUSY) {
        throw wrong_operation("connection available only in BUSY stage");
    }
    auto ok=removeFromActiveList(conn);
    if(!ok) {
        throw wrong_connection();
    }
    appendToIdleList(conn);
}

bool ConnectionPool::removeFromActiveList(Ptr conn) {
    auto iter=active_conns.find(conn);
    if(iter!=active_conns.end()){
        active_conns.erase(iter);
        return true;
    }
    return false;
}

void ConnectionPool::appendToIdleList(Ptr conn) {
    idle_conns.push_back(conn);
}

size_t getCount() {
    return 0;
}

size_t getActiveCount() {
    return 0;
}

size_t getIdleCount() {
    return 0;
}
