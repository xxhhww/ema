#ifndef _EMILIA_E_REDIS_H_
#define _EMILIA_E_REDIS_H_

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <sys/time.h>
#include <hiredis-vip/hiredis.h>
#include <hiredis-vip/hircluster.h>
#include <hiredis-vip/adapters/libevent.h>
#include "loop.h"
#include "e_libevent.h"

namespace emilia{
namespace db{

enum class RdsType{
    REDIS_SYNC_ALONE = 0x01,
    REDIS_SYNC_CLUSTER = 0x02,
    REDIS_ASYNC_ALONE = 0x03,
    REDIS_ASYNC_CLUSTER = 0x04
};

//将typeId转换为对应的string
const char* RdsReplyTypeToString(int typeId);

//重新复制一份redisReply
redisReply* RdsReplyClone(redisReply* source);

/*
SyncRedis:
    普通方式输送命令后必须通过返回值收到回复
    管道输送命令后必须调用getRedisReplay()获得回复

ASyncRedis:
    1.异步输送命令后进入阻塞状态，执行其他任务，异步命令完成后将其唤醒
    2.异步输送后不进入阻塞状态而是直接返回，继续执行当前任务的其他部分，在需要异步命令结果时
        2.1 如果异步命令未执行完毕，进入阻塞状态，执行其他任务，异步命令执行完毕将其唤醒
        2.2 如果异步命令已经执行完毕，则使用异步命令结果，不进入阻塞状态，继续执行当前任务

    Ps当前任务与执行event_base_loop()的线程通过共享内存通信

*/

/*
REDIS_REPLY_STATUS
    表示状态，内容通过str字段查看，字符串长度是len字段
REDIS_REPLY_ERROR
    表示出错，查看出错信息，如上的str,len字段
REDIS_REPLY_INTEGER
    返回整数，从integer字段获取值
REDIS_REPLY_NIL
    没有数据返回
REDIS_REPLY_STRING
    返回字符串，查看str,len字段
REDIS_REPLY_ARRAY
    返回一个数组，查看elements的值（数组个数），通过element[index]的方式访问数组元素，每个数组元素是 一个redisReply对象的指针
*/
using RdsReplyPtr = std::shared_ptr<redisReply>;

class ERedis
{
public:
    using ptr = std::shared_ptr<ERedis>;

    ERedis(const std::string& name,
           const std::string& ip,
           const std::string& port,
           const std::string& password,
           const RdsType& type)
    :m_name(name)
    ,m_ip(ip)
    ,m_port(port)
    ,m_passWord(password)
    ,m_type(type){}

    virtual ~ERedis() {}

    const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    const std::string& getIp() const { return m_ip; }
    void setIp(const std::string& ip) { m_ip = ip; }

    const std::string& getPort() const { return m_port; }
    void setPort(const std::string& port) { m_port = port; }

    const std::string& getPassWord() const { return m_passWord; }
    void setPassWord(const std::string& password) { m_passWord = password; }

    const RdsType getRdsType() const { return m_type; }
    void setRdsType(const RdsType& type) { m_type = type; }

    //获得毫秒级超时
    const uint64_t& getCmdTimeOut() const { 
        return m_cmdTimeOut.tv_sec * 1000 + m_cmdTimeOut.tv_usec / 1000; 
    }
    void setCmdTimeOut(const uint64_t& cmdTimeOut) { 
        struct timeval tempVal;
        tempVal.tv_sec = cmdTimeOut / 1000;
        tempVal.tv_usec = cmdTimeOut % 1000 * 1000;
        m_cmdTimeOut = tempVal;
    }

    //获得毫秒级超时
    const uint64_t& getConnTimeOut() const { 
        return m_connTimeOut.tv_sec * 1000 + m_connTimeOut.tv_usec / 1000; 
    }
    void setConnTimeOut(const uint64_t& connTimeOut){
        struct timeval tempVal;
        tempVal.tv_sec = connTimeOut / 1000;
        tempVal.tv_usec = connTimeOut % 1000 * 1000;
        m_connTimeOut = tempVal;
    }

    //获得分布式锁(lockTime为锁的最长占用时间)
    //lockValue为全局唯一随机值
    virtual bool setLock(const char* lockName,
                         uint64_t lockValue, 
                         uint64_t lockTime) = 0;
    //释放分布式锁
    //如果lockValue与锁的值不相符则释放锁而是直接返回
    //lockValue的值与锁的值不同就意味着
    //此锁占用时间超过了最长占用时间，已经自动释放了，并且被其他任务占用了
    virtual bool freeLock(const char* lockName, uint64_t lockValue) = 0;

protected:
    std::string m_name;
    std::string m_ip;
    std::string m_port;
    std::string m_passWord;
    RdsType m_type;
    struct timeval m_cmdTimeOut;
    struct timeval m_connTimeOut;
};

class ESyncRedis : public ERedis
{
public:
    using ptr = std::shared_ptr<ESyncRedis>;
    using RedisCtx = std::shared_ptr<redisContext>;

    //redisConf从配置文件中读取
    ESyncRedis(std::map<std::string, std::string>& redisConf);

    //连接超时判定失败
    bool connect();
    bool reConnect();

    //普通方式输送命令
    RdsReplyPtr cmd(const char* fmt, ...);
    RdsReplyPtr cmd(const std::vector<std::string>& argv);

    //使用管道输送命令
    int pipeCmd(const char* fmt, ...);
    int pipeCmd(const std::vector<std::string>& argv);
    RdsReplyPtr getReply();

    bool setLock(const char* lockName,
                 uint64_t lockValue, 
                 uint64_t lockTime) override;

    bool freeLock(const char* lockName, uint64_t lockValue) override;

private:
    RdsReplyPtr cmd(const char* fmt, va_list ap);
    int pipeCmd(const char* fmt, va_list ap);
private:
    ESyncRedis::RedisCtx m_context;
};

class EASyncRedis : public ERedis
                  , public std::enable_shared_from_this<EASyncRedis>
{
public:
    using ptr = std::shared_ptr<EASyncRedis>;
    using RedisCtx = std::shared_ptr<redisAsyncContext>;
    enum Status{
        UNCONNECTED = 0x01,
        CONNECTING = 0x02,
        CONNECTED = 0x03
    };

    //用作(void* privdata)代表命令的出生信息
    struct BornContext{
        using ptr = std::shared_ptr<EASyncRedis::BornContext>;
        
        BornContext()
        :bornClass(nullptr)
        ,bornFiber(nullptr)
        ,bornLoop(nullptr)
        ,isReply(false)
        ,cmdContent("")
        ,cmdReply(nullptr)
        ,isTimeOut(false){}

        //命令产生于那个Redis类
        EASyncRedis::ptr bornClass;
        //命令产生于哪里一个任务(协程)
        Fiber::ptr bornFiber;
        //任务属于哪一个线程(事件循环)
        Loop* bornLoop;
        //命令是否已经处理完毕
        bool isReply;
        //命令的内容
        std::string cmdContent;
        //命令的返回信息(异步回调执行之后，void* Reply会自行释放，所以需要Clone一下)
        RdsReplyPtr cmdReply;
        //命令是否超时
        bool isTimeOut;
    };

    EASyncRedis(std::map<std::string, std::string>& redisConf);
    //将通过redisLibeventAttach将异步连接与libevent事件触发库绑定
    bool init();
    void setEvent2Loop(Event2Loop::ptr thread) { m_thread = thread; }

    //任务发出异步命令之后，会进入阻塞态，线程执行其他任务，当异步命令完成后，阻塞任务被唤醒
    RdsReplyPtr cmdWithHold(const char* fmt, ...);
    RdsReplyPtr cmdWithHold(const std::vector<std::string>& argv);

    //任务发出异步命令之后，不会进入阻塞态，之后需要使用异步命令结果时
    //如果异步命令已经完成，则直接使用其返回结果
    //否则进入阻塞态，线程执行其他任务，当异步命令完成后，阻塞任务被唤醒
    EASyncRedis::BornContext::ptr cmdNoHold(const char* fmt, ...);
    EASyncRedis::BornContext::ptr cmdNoHold(const std::vector<std::string>& argv);

    bool setLock(const char* lockName,
                 uint64_t lockValue, 
                 uint64_t lockTime) override;

    bool freeLock(const char* lockName, uint64_t lockValue) override;
    
public:
    //连接回调
    //c : 代表异步连接上下文
    //status : 当前连接的状态
    static void ConnectCb(const redisAsyncContext* c, int status);
    //身份验证回调(参数同CmbCb)
    static void AuthCb(redisAsyncContext* c, void* reply, void* privdata);
    //断开连接回调(参数同ConnectCb)
    static void DisConnectCb(const redisAsyncContext* c, int status);
    //普通命令回调
    //c : 代表异步连接上下文
    //reply : 与同步中的reply相同，不过异步中的reply会自动释放
    //privdata : 用户传入的参数
    static void CmdCb(redisAsyncContext* c, void* reply, void* privdata);
    //设置锁回调(参数同CmbCb)
    static void SetLockCb(redisAsyncContext* c, void* reply, void* privdata);
private:
    RdsReplyPtr cmdWithHold(const char* fmt, va_list ap);
    EASyncRedis::BornContext::ptr cmdNoHold(const char* fmt, va_list ap);
private:
    EASyncRedis::RedisCtx m_context;
    EASyncRedis::Status m_status;
    Event2Loop::ptr m_thread;
};

}
}

#endif