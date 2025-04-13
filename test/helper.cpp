#include "helper.h"

using namespace std;

bool Conn::ping() {
    return true;
}
void Conn::close() {
}

ConnectionPool::Ptr Creator::create() {
    return make_shared<Conn>();
}

CreatorWithSpeedControl::CreatorWithSpeedControl(unsigned int retry_count):
    retry_count(retry_count) {}

ConnectionPool::Ptr CreatorWithSpeedControl::create() {
    index+=1;
    if(index%retry_count==0)return make_shared<Conn>();
    return nullptr;
}
