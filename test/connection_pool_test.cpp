#include <gtest/gtest.h>
#include "connection_pool.h"

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

class ConnPoolTest: public testing::Test {
protected:
    ConnPoolTest(): pool({},make_unique<Creator>()) {}

    ConnectionPool pool;
};

TEST_F(ConnPoolTest, LifeTime) {
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

TEST(ConnPoolTestTimeout, Timeout) {
    unsigned int retry_count=3;
    ConnectionPool pool(
        {.max_retry_count=retry_count-1,.retry_interval_milli=10},
        make_unique<CreatorWithSpeedControl>(retry_count)
    );
    EXPECT_THROW(pool.start(),ConnectionPool::wrong_operation);
}

TEST(ConnPoolTestTimeout, Timein) {
    unsigned int retry_count=3;
    ConnectionPool pool(
        {.max_retry_count=retry_count,.retry_interval_milli=10},
        make_unique<CreatorWithSpeedControl>(retry_count)
    );
    pool.start();
}
