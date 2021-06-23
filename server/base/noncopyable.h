#ifndef SERVER_BASE_NONCOPYABLE_H
#define SERVER_BASE_NONCOPYABLE_H

namespace server{

class noncopyable {
protected:
    noncopyable() = default;
    ~noncopyable() = default;
public:
    noncopyable(const noncopyable& ) = delete;
    void operator=(const noncopyable& ) = delete;

    // void test();
};

}
#endif