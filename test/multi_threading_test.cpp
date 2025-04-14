#include <gtest/gtest.h>
#include "connection_pool.h"
#include <thread>
#include <vector>

using namespace std;

class Conn: public ConnectionPool::Connection {
public:
    void echoAddr() {}
    bool ping() override  {
        return true;
    }
    void close() override {}
};

using Ptr=shared_ptr<Conn>;

class Creator: public ConnectionPool::Creator {
public:
    Creator() {}
    ConnectionPool::Ptr create() override {
        auto ret=make_shared<Conn>();
        cout<<ret<<" created"<<endl;
        return ret;
    }
};

TEST(MultiThreaded,Simple) {
    ConnectionPool pool{{.max_conn_num=1000},make_unique<Creator>()};
    pool.start();
    vector<thread> ths;
    for(int i=0;i<100;++i) {
        ths.push_back(thread([&pool,i]{
            for(int i=0;i<1000;++i) {
                auto conn=pool.getConn();
                dynamic_pointer_cast<Conn>(conn)->echoAddr();
                pool.relConn(conn);
            }
        }));
    }
    for(auto& i:ths)i.join();
}

