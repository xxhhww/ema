#include "chatserve.h"
#include "emilia/log/logmarco.h"

using namespace emilia::log;

void chatServe::handleClient(Socket::ptr connSock){
    EMILIA_LOG_DEBUG("chat") << connSock->getReAddr()->toString();
    //创建新的用户对象
    chatSessionPtr parter(new chatSession(connSock, m_room, shared_from_this()));
    //将用户加入房间
    m_room->join(parter);
    parter->readBody();
}

#define SockErrorStr(rt) \
    if(rt == 0) \
        EMILIA_LOG_INFO("chat") << "客户端主动断开"; \
    if(rt == -1) \
        EMILIA_LOG_ERROR("chat") << "服务器系统错误"; \
    if(rt == -2) \
        EMILIA_LOG_INFO("chat") << "客户端响应超时"; \

bool chatSession::readBody(){
    while(true){
        m_readMsg.clear();
        //读包体
        char message[200] = {0};
        int rt = m_connSock->recv(message, 200);
        //如果发生错误
        if(rt <= 0){
            SockErrorStr(rt)
            //将其从room中踢出
            m_room->leave(shared_from_this());
            return false;
        }

        std::string temp = "[" + m_connSock->getReAddr()->toString() + "]: " + message;
        memcpy(m_readMsg.data(), temp.c_str(), temp.size());
        m_readMsg.body_length(temp.size());


        //将包体发送给room，room负责消息的广播
        m_room->deliver(m_readMsg, shared_from_this());
    }
}

//向用户发送信息
void chatSession::deliver(const chatMessage message){
    m_connSock->send(message.data(), message.body_length());
}

void chatSession::getJoinInfo(chatMessage& joinMsg){
    std::string temp = "[" + m_connSock->getReAddr()->toString() + "]" + "加入聊天室\n";
    //拷贝
    memcpy(joinMsg.data(), temp.c_str(), temp.size());
    joinMsg.body_length(temp.size());
}

void chatSession::getLeaveInfo(chatMessage& leaveMsg){
    std::string temp = "[" + m_connSock->getReAddr()->toString() + "]" + "离开聊天室\n";
    //拷贝
    memcpy(leaveMsg.data(), temp.c_str(), temp.size());
    leaveMsg.body_length(temp.size());
}

//有用户加入
void chatRoom::join(chatSessionPtr parter){
    m_participants.insert(parter);
    
    chatMessage temp;
    parter->getJoinInfo(temp);

    Mutex::Lock lock(m_mutex);
    //向其他成员发送XXX加入聊天室的信息
    deliverNoLock(temp, parter);
    //将之前用户所发信息发送给这个新加入的用户
    for(const auto& m : m_recentMsgs)
        parter->deliver(m);
}

//有用户离开
void chatRoom::leave(chatSessionPtr parter){
    chatMessage temp;
    parter->getLeaveInfo(temp);

    Mutex::Lock lock(m_mutex);
    //向其他成员广播
    deliverNoLock(temp, parter);
    //从容器中删除
    m_participants.erase(parter);
}

void chatRoom::deliver(const chatMessage message, chatSessionPtr sender){
    Mutex::Lock lock(m_mutex);
    deliverNoLock(message, sender);
}

//消息广播(不广播给发送者)
void chatRoom::deliverNoLock(const chatMessage message, chatSessionPtr sender){
    //最多保存100条信息
    m_recentMsgs.push_back(message);
    while (m_recentMsgs.size() > max_recent_msgs)
        m_recentMsgs.pop_front();

    for(auto& parter : m_participants){
        if(parter != sender)
            parter->deliver(message);
    }
}