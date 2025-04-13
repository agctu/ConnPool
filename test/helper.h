#pragma once
#include "connection_pool.h"

class Conn : public ConnectionPool::Connection {
public:
    bool ping() override;
    void close() override;
};

class Creator : public ConnectionPool::Creator {
public:
    ConnectionPool::Ptr create() override;
private:
};

class CreatorWithSpeedControl : public ConnectionPool::Creator {
public:
    CreatorWithSpeedControl(unsigned int retry_count);
    ConnectionPool::Ptr create() override;
private:
    unsigned int retry_count;
    unsigned int index=0;
};
