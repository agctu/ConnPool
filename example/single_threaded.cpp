#include <vector>
#include <iostream>
#include "connection_pool.h"

using namespace std;

class Conn: public ConnectionPool::Connection {
public:
    void echo(const string& msg) {
        cout<<msg<<endl;
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
        return make_shared<Conn>();
    }
private:
    string addr;
};

int main() {
    ConnectionPool pool{{},make_unique<Creator>("localhost")};
    vector<ConnectionPool::Ptr>conns;
    for(int i=0;i<10;++i) conns.push_back(pool.getConn());

    for(auto i:conns) {
        dynamic_pointer_cast<Conn>(i)->echo(to_string((uint64_t)i.get()));
        pool.relConn(i);
    }
    return 0;
}

