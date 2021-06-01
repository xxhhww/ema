#ifndef _EMILIA_CHATSERVER_H_
#define _EMILIA_CHATSERVER_H_

#include "emilia/net/tcpserver.h"
#include "emilia/demo/chatroom/chatmessage.h"
#include <deque>
#include <list>

using namespace emilia::base;
using namespace emilia::net;

using chatMessageQueue = std::deque<chatMessage>;

class chatSession;
using chatSessionPtr = std::shared_ptr<chatSession>;

class chatRoom{
public:
    using ptr = std::shared_ptr<chatRoom>;
    //有用户加入
	void join(chatSessionPtr parter);
    //有用户离开
	void leave(chatSessionPtr parter);
    //消息广播有锁
    void deliver(const chatMessage message, chatSessionPtr sender);
private:
    //消息广播(无锁)
	void deliverNoLock(const chatMessage message, chatSessionPtr sender);
private:
    //所有成员
    std::set<chatSessionPtr> m_participants;
    //最大存储消息
    enum { max_recent_msgs = 100 };
    //用户发送的所有消息，最大100条
    chatMessageQueue m_recentMsgs;
    //
    Mutex m_mutex;
};

class chatServe : public TcpServe
                , public std::enable_shared_from_this<chatServe>
{
public:
    using ptr = std::shared_ptr<chatServe>;
    chatServe(IOManger* ioManger, IPAddress::ptr serveAddr)
    :TcpServe(ioManger, serveAddr)
    ,m_room(new chatRoom){}

    virtual void handleClient(Socket::ptr connSock) override;

private:
    chatRoom::ptr m_room;
};

class chatSession : public std::enable_shared_from_this<chatSession>{
public:
    chatSession(Socket::ptr connSock, chatRoom::ptr room, chatServe::ptr serve)
    :m_connSock(connSock)
    ,m_room(room)
    ,m_serve(serve){}

    bool readBody();

    //向用户发送信息
    void deliver(const chatMessage message);

    //返回对应消息
    void getJoinInfo(chatMessage& joinMsg);
    void getLeaveInfo(chatMessage& leaveMsg);
private:
    Socket::ptr m_connSock;
    //用户所处房间
    chatRoom::ptr m_room;
    //用户发送的消息
    chatMessage m_readMsg;
    //对应的serve
    chatServe::ptr m_serve;
};

#endif