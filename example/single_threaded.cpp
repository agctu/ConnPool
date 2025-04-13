#include <vector>
#include <iostream>
#include "connection_pool.h"

using namespace std;

class Conn: public ConnectionPool::Connection {
public:
    void echoAddr() {
        cout<<this<<endl;
    }
    bool ping() override  {
        return true;
    }
    void close() override {
    }
};

using Ptr=shared_ptr<Conn>;

class Creator: public ConnectionPool::Creator {
public:
    Creator(const string& addr) : addr(addr) {}
    ConnectionPool::Ptr create() override {
        auto ret=make_shared<Conn>();
        cout<<ret<<" created"<<endl;
        return ret;
    }
private:
    string addr;
};

int main() {
    try{
        ConnectionPool pool{{},make_unique<Creator>("localhost")};

        pool.start();
        vector<ConnectionPool::Ptr>conns;
        for(int i=0;i<10;++i) conns.push_back(pool.getConn());

        for(auto i:conns) {
            dynamic_pointer_cast<Conn>(i)->echoAddr();
            pool.relConn(i);
        }
    } catch (exception& e) {
        cerr<<e.what()<<endl;
    }
    return 0;
}

