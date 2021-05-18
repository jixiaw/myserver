#ifndef SERVER_NET_CHANNEL_H
#define SERVER_NET_CHANNEL_H

#include "base/noncopyable.h"
#include <functional>
namespace server {
namespace net {

class EventLoop;


// 每个Channel管理一个fd，但是不拥有
// 每个Channel属于一个EventLoop，只在一个线程里，不需要加锁
class Channel : public noncopyable 
{
public:
    typedef std::function<void()> EventCallBack;

    Channel(EventLoop* loop, int fdarg);
    ~Channel();

    void handleEvent();     // 处理事件
    void setReadCallBack(const EventCallBack& cb) {readCallBack_ = cb;}
    void setWriteCallBack(const EventCallBack& cb) {writeCallBack_ = cb;}
    void setErrorCallBack(const EventCallBack& cb) {errorCallBack_ = cb;}
    void setCloseCallBack(const EventCallBack& cb) {closeCallBack_ = cb;}

    int fd() const {return fd_;}
    int events() const {return events_;}
    void setRevents(int revt) {revents_ = revt;}
    bool isNoneEvent() const {return events_ == kNoneEvent;}

    void enableReading() {events_ |= kReadEvent; update();}
    void disableAll() {events_ = kNoneEvent; update(); }

    int index() {return index_;};
    void setIndex(int idx) { index_ = idx; };

    void remove();

    EventLoop* ownerLoop() {return loop_;}; 

private:
    void update();

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;
    const int fd_;
    int events_;    // 关心的IO事件，用户设置. bit pattern 来自poll中的 struct pollfd
    int revents_;   // 目前活动的事件，由eventloop/poller设置. bit pattern
    int index_;     // 在poller中pollfds_的index.

    bool eventHandling;

    EventCallBack readCallBack_;
    EventCallBack writeCallBack_;
    EventCallBack errorCallBack_;
    EventCallBack closeCallBack_;
};


}
}

#endif