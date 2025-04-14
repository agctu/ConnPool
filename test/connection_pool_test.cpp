#include <gtest/gtest.h>
#include "connection_pool.h"
#include <vector>

using namespace std;

class Conn : public ConnectionPool::Connection {
public:
    bool ping() override {
        return true;
    }
    void close() override {
    }
};

class Creator : public ConnectionPool::Creator {
public:
    ConnectionPool::Ptr create() override {
        return make_shared<Conn>();
    }
};

class CreatorWithSpeedControl : public ConnectionPool::Creator {
public:
    CreatorWithSpeedControl(unsigned int retry_count):
        retry_count(retry_count) {}
    ConnectionPool::Ptr create() {
        index+=1;
        if(index%retry_count==0)return make_shared<Conn>();
        return nullptr;
    }
private:
    unsigned int retry_count;
    unsigned int index=0;
};

TEST(ConnPoolTest, LifeCycle) {
    ConnectionPool pool({},make_unique<Creator>());
    EXPECT_THROW(pool.stop(),ConnectionPool::wrong_operation);
    EXPECT_THROW(pool.getConn(),ConnectionPool::wrong_operation);
    pool.start();
    ConnectionPool::Ptr ptr;
    EXPECT_THROW(pool.start(),ConnectionPool::wrong_operation);
    EXPECT_NO_THROW(ptr=pool.getConn());
    EXPECT_THROW(pool.stop(),ConnectionPool::wrong_operation);
    pool.relConn(ptr);
    pool.stop();
    EXPECT_THROW(pool.stop(),ConnectionPool::wrong_operation);
}

TEST(ConnPoolTest, Timeout) {
    unsigned int retry_count=3;
    ConnectionPool pool(
        {.max_retry_count=retry_count-1,.retry_interval_milli=10},
        make_unique<CreatorWithSpeedControl>(retry_count)
    );
    EXPECT_THROW(pool.start(),ConnectionPool::wrong_operation);
}

TEST(ConnPoolTest, Timein) {
    unsigned int retry_count=3;
    ConnectionPool pool(
        {.max_retry_count=retry_count,.retry_interval_milli=10},
        make_unique<CreatorWithSpeedControl>(retry_count)
    );
    pool.start();
}

TEST(ConnPoolTest,ExeedingMaxConnNum) {
    ConnectionPool pool{{.init_conn_num=2,.max_conn_num=5},make_unique<Creator>()};
    pool.start();
    vector<ConnectionPool::Ptr>conns;
    for(int i=0;i<5;++i) {
        conns.push_back(dynamic_pointer_cast<Conn>(pool.getConn()));
    }
    ASSERT_THROW(pool.getConn(),ConnectionPool::num_limit_exceeded);
    for(auto i:conns) {
        pool.relConn(i);
    }
}

TEST(ConnPoolTest,Sweeping) {
    ConnectionPool::Config config{
        .init_conn_num=4,
        .max_conn_num=20,
        .max_unused_conn_num=2,
        .used_bound_sec=10,
        .sweep_period_sec=20
    };
    ConnectionPool pool{config, make_unique<Creator>()};
    pool.start();
    vector<ConnectionPool::Ptr>conns;
    for(int i=0;i<config.max_conn_num;++i) {
        conns.push_back(pool.getConn());
    }
    for(auto i:conns) {
        pool.relConn(i);
    }
    this_thread::sleep_for(chrono::seconds(config.sweep_period_sec+config.used_bound_sec+3));
    ASSERT_EQ(pool.getIdleCount(),config.max_unused_conn_num);
}
