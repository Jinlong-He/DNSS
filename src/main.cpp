#include "DNSS.hpp"
template<> dnss::Symbol FA<dnss::Symbol>::epsilon = 0;
ID State::counter = 0;
ID Var::counter = 0;
ID Value::counter = 0;
unordered_set<Object*> Manage::buffer;
Manage manage;
using namespace dnss;
int main(int argc, const char * argv[]) {
    Parse parse(argv[1], argv[2]);
    DNSS dnss(parse);
    dnss.test();
    return 0;
}
