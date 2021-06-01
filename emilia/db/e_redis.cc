#include "e_redis.h"
#include "emilia/util/stringutil.h"
#include <cstring>
#include "log.h"

namespace emilia{
namespace db{

const char* RdsReplyTypeToString(int typeId){
    char tempVal = (char)(typeId + 48);
    switch (tempVal){
        case '1':
            return "REDIS_REPLY_STRING";
        case '2':
            return "REDIS_REPLY_ARRAY";
        case '3':
            return "REDIS_REPLY_INTEGER";
        case '4':
            return "REDIS_REPLY_NIL";
        case '5':
            return "REDIS_REPLY_STATUS";
        case '6':
            return "REDIS_REPLY_ERROR";  
        default:
            return "REDIS_REPLY_UNKNOW";
    }
}

redisReply* RdsReplyClone(redisReply* source){
    //分配内存空间
    redisReply* r = static_cast<redisReply*>(calloc(1, sizeof(redisReply)));
    r->type = source->type;
    switch(source->type){
        case REDIS_REPLY_INTEGER:
            r->integer = source->integer;
            break;
        case REDIS_REPLY_STRING:
        case REDIS_REPLY_ERROR:
        case REDIS_REPLY_STATUS:
            //为str分配内存空间
            if(source->str != nullptr){
                r->str = static_cast<char*>(malloc(source->len + 1));
                memcpy(r->str, source->str, source->len);
            }
            else
                r->str = nullptr;
            break;
        case REDIS_REPLY_ARRAY:
            if(source->element != nullptr && source->elements > 0) {
                r->element = (redisReply**)calloc(source->elements, sizeof(redisReply));
                r->elements = source->elements;
                for(size_t i = 0; i < source->elements; ++i) {
                    r->element[i] = RdsReplyClone(source->element[i]);
                }
            }
            break;
    }
    return r;
}

ESyncRedis::ESyncRedis(std::map<std::string, std::string>& redisConf)
:ERedis(redisConf["name"]
        ,redisConf["ip"]
        ,redisConf["port"]
        ,redisConf["password"]
        ,RdsType::REDIS_SYNC_ALONE)
{
    setCmdTimeOut(util::StringUtil::ToUint64(
                    redisConf["cmdTimeOut"].c_str()
                ));
    setConnTimeOut(util::StringUtil::ToUint64(
                    redisConf["connTimeOut"].c_str()
                ));
}

bool ESyncRedis::connect(){
    if(m_context != nullptr)
        return true;
    //执行连接操作
    redisContext* c = redisConnectWithTimeout(m_ip.c_str() 
                                              ,util::StringUtil::ToInt32(m_port.c_str())
                                              ,m_connTimeOut);
    if(c == nullptr){
        EMILIA_LOG_ERROR("Redis") << "Connect Error: Return Null" << "\n"
        << "(ip: " << m_ip << ", name: " << m_name << ")";
        return false;
    }
    if(c->err != 0){
        EMILIA_LOG_ERROR("Redis") << "Connect Error: " << c->errstr << "\n"
        << "(ip: " << m_ip << ", name: " << m_name << ")";
        redisFree(c);
        return false;
    }

    //转化为智能指针,redisFree为析构函数
    m_context.reset(c, redisFree);
    //执行身份验证操作
    redisReply* r = (redisReply*)redisCommand(c, "auth %s", m_passWord.c_str());
    
    //返回为空指针
    if(r == nullptr){
        EMILIA_LOG_ERROR("Redis") << "Auth Error: Reply Null" << "\n"
        << "(ip: " << m_ip << ", name: " << m_name << ")";
        return false;
    }
    //返回类型错误
    if(r->type != REDIS_REPLY_STATUS){
        EMILIA_LOG_ERROR("Redis") << "Auth Error: Reply Type Is"
        << RdsReplyTypeToString(r->type) << "\n"
        << "(ip: " << m_ip << ", name: " << m_name << ")";
        freeReplyObject((void*)r);
        return false;
    }
    //返回正常
    //密码正确
    if(strcmp(r->str, "OK") == 0){
        EMILIA_LOG_INFO("Redis") << "Auth Success" << "\n"
        << "(ip: " << m_ip << ", name: " << m_name << ")";
        freeReplyObject((void*)r);
        return true;
    }
    //密码错误
    else{
        EMILIA_LOG_ERROR("Redis") << "Auth Error: " << r->str  << "\n"
        << "(ip: " << m_ip << ", name: " << m_name << ")";
        freeReplyObject((void*)r);
        return false;
    }
}

bool ESyncRedis::reConnect(){
    return (redisReconnect(m_context.get()) == REDIS_OK) ? true : false;
}

RdsReplyPtr ESyncRedis::cmd(const char* fmt, ...){
    //获取可变参数列表
    va_list parameters;
    va_start(parameters, fmt);
    RdsReplyPtr r = cmd(fmt, parameters);
    //释放可变参数列表
    va_end(parameters);
    return r;
}

RdsReplyPtr ESyncRedis::cmd(const char* fmt, va_list ap){
    redisReply* r = (redisReply*)redisvCommand(m_context.get(), fmt, ap);
    if(r == nullptr){
        EMILIA_LOG_ERROR("Redis") << "RedisCmd: " << fmt << ", Error: " << "Reply is Null" << "\n"
        << "(ip: " << m_ip << ", name: " << m_name << ")";
    }
    //正确返回
    if(r->type != REDIS_REPLY_ERROR){
        return std::make_shared<redisReply>(r, freeReplyObject);
    }
    if(r->type == REDIS_REPLY_ERROR){
        EMILIA_LOG_ERROR("Redis") << "RedisCmd: " << fmt << ", Error: " << r->str << "\n"
        << "(ip: " << m_ip << ", name: " << m_name << ")";
        freeReplyObject(r);
        return nullptr;
    }
    freeReplyObject(r);
    return nullptr;
}

RdsReplyPtr ESyncRedis::cmd(const std::vector<std::string>& argv){
    std::vector<const char*> cArgv;
    std::vector<size_t> cArgvLen;
    for(auto& i : argv){
        cArgv.push_back(i.c_str());
        cArgvLen.push_back(i.size());
    }
    redisReply* r = (redisReply*)redisCommandArgv(m_context.get(), argv.size(), &cArgv[0], &cArgvLen[0]);
    if(r == nullptr){
        std::stringstream fmt;
        for(auto& i : argv)
            fmt << i << " ";
        EMILIA_LOG_ERROR("Redis") << "RedisCmd: " << fmt <<", Error: " << "Reply is Null" <<"\n"
        << "(ip: " << m_ip << ", name: " << m_name << ")";
    }
    if(r->type != REDIS_REPLY_ERROR){
        return std::make_shared<redisReply>(r, freeReplyObject);
    }
    if(r->type == REDIS_REPLY_ERROR){
        std::stringstream fmt;
        for(auto& i : argv)
            fmt << i << " ";
        EMILIA_LOG_ERROR("Redis") << "RedisCmd: " << fmt <<", Error: " << r->str <<"\n"
        << "(ip: " << m_ip << ", name: " << m_name << ")";
        freeReplyObject(r);
        return nullptr;
    }
    freeReplyObject(r);
    return nullptr;
}

int ESyncRedis::pipeCmd(const char* fmt, ...){
    va_list parameters;
    va_start(parameters, fmt);
    int r = pipeCmd(fmt, parameters);
    va_end(parameters);
    return r;
}

int ESyncRedis::pipeCmd(const char* fmt, va_list ap){
    return redisvAppendCommand(m_context.get(), fmt, ap);
}

int ESyncRedis::pipeCmd(const std::vector<std::string>& argv){
    std::vector<const char*> cArgv;
    std::vector<size_t> cArgvLen;
    for(auto& i : argv){
        cArgv.push_back(i.c_str());
        cArgvLen.push_back(i.size());
    }
    return redisAppendCommandArgv(m_context.get(), argv.size(), &cArgv[0], &cArgvLen[0]);
}

RdsReplyPtr ESyncRedis::getReply(){
    redisReply* reply = nullptr;
    if(redisGetReply(m_context.get(), (void**)&reply) == REDIS_OK);
        return std::make_shared<redisReply>(reply, freeReplyObject);
    EMILIA_LOG_ERROR("Redis") << "GetRedisReply Error" <<"\n"
    <<"(ip: " << m_ip << ", name: " << m_name << ")";
    if(reply != nullptr)
        freeReplyObject(reply);
    return nullptr;
}

//获得分布式锁(lockTime为锁的最长占用时间)
bool ESyncRedis::setLock(const char* lockName
                         ,uint64_t lockValue
                         ,uint64_t lockTime){
    redisReply* r = (redisReply*)redisCommand(m_context.get()
                                              ,"SET %s %lld NX PX %lld"
                                              ,lockName, lockValue, lockTime);
    if(r == nullptr){
        EMILIA_LOG_ERROR("Redis") << "Set Lock Error: Reply Null" <<"\n"
        <<"(ip: " << m_ip << ", name: " << m_name << ")";
        return false;
    }
    if(r->type == REDIS_REPLY_STATUS && strcmp(r->str, "OK") == 0){
        freeReplyObject(r);
        return true;
    }
    EMILIA_LOG_ERROR("Redis") << "Set Lock Error: Reply Typr Is" 
    << RdsReplyTypeToString(r->type) <<"\n"
    <<"(ip: " << m_ip << ", name: " << m_name << ")";
    freeReplyObject(r);
    return false;
}

//释放分布式锁
bool ESyncRedis::freeLock(const char* lockName, uint64_t lockValue){
    redisReply* rGet = (redisReply*)redisCommand(m_context.get(), "GET %s", lockName);
    if(rGet == nullptr){
        EMILIA_LOG_ERROR("Redis") << "FreeLock GetKey Error"
        <<"\n"
        <<"(ip: " << m_ip << ", name: " << m_name << ")";
        return false;
    }
    RdsReplyPtr _rGet(rGet, freeReplyObject);
    //类型一致
    if(_rGet->type == REDIS_REPLY_INTEGER){
        //锁还没被释放
        if(_rGet->integer == lockValue){
            redisReply* rDel = (redisReply*)redisCommand(m_context.get(), "DEL %s", lockName);
            if(rDel == nullptr){
                EMILIA_LOG_ERROR("Redis") << "FreeLock DelKey Error: Reply Is Null" <<"\n"
                <<"(ip: " << m_ip << ", name: " << m_name << ")";
                return false;
            }
            RdsReplyPtr _rDel(rDel, freeReplyObject);
            if(_rDel->type == REDIS_REPLY_INTEGER && _rDel->integer == 1)
                return true;
            
            EMILIA_LOG_ERROR("Redis") << "FreeLock DelKey Error: Reply Type Is" 
            << RdsReplyTypeToString(_rDel->type) << "\n"
            <<"(ip: " << m_ip << ", name: " << m_name << ")";
            return false;
        }
        //锁已经自己释放，并且被其他任务重新设置
        return true;
    }
    //已经超时自动释放
    if(_rGet->type == REDIS_REPLY_NIL)
        return true;
    
    EMILIA_LOG_ERROR("Redis") << "FreeLock GetKey Error: Reply Type Is" 
    << RdsReplyTypeToString(_rGet->type) << "\n"
    <<"(ip: " << m_ip << ", name: " << m_name << ")";
    return false;
}

EASyncRedis::EASyncRedis(std::map<std::string, std::string>& redisConf)
:ERedis(redisConf["name"]
        ,redisConf["ip"]
        ,redisConf["port"]
        ,redisConf["password"]
        ,RdsType::REDIS_ASYNC_ALONE)
{
    setCmdTimeOut(util::StringUtil::ToUint64(
                    redisConf["cmdTimeOut"].c_str()
                ));
    setConnTimeOut(util::StringUtil::ToUint64(
                    redisConf["connTimeOut"].c_str()
                ));
}

//任务发出异步命令之后，会进入阻塞态，线程执行其他任务，当异步命令完成后，阻塞任务被唤醒
RdsReplyPtr EASyncRedis::cmdWithHold(const char* fmt, ...)
{
    va_list parameters;
    va_start(parameters, fmt);
    RdsReplyPtr r = cmdWithHold(fmt, parameters);
    va_end(parameters);
    return r;
}

RdsReplyPtr EASyncRedis::cmdWithHold(const std::vector<std::string>& argv)
{
    std::vector<const char*> cArgv;
    std::vector<size_t> cArgvLen;
    for(auto& i : argv){
        cArgv.push_back(i.c_str());
        cArgvLen.push_back(i.size());
    }
    EASyncRedis::BornContext bornCtx;
    bornCtx.bornClass = shared_from_this();
    bornCtx.bornFiber = Fiber::GetThis();
    bornCtx.bornLoop = Loop::GetThis();
    char* buf = nullptr;
    int len = redisFormatCommandArgv(&buf, argv.size(), &cArgv[0], &cArgvLen[0]);
    if(len == -1){
        EMILIA_LOG_ERROR("Redis") << "redisFormatCommandArgv Error";
        return;
    }
    bornCtx.cmdContent = std::string(buf, len);
    ::free((void*)buf);

    redisAsyncFormattedCommand(m_context.get()
                               ,EASyncRedis::CmdCb
                               ,(void*)&bornCtx
                               ,bornCtx.cmdContent.c_str()
                               ,bornCtx.cmdContent.size());
    Loop::GetThis()->addAsyncTask(Fiber::GetThis());
    Fiber::YieldToHold();
    return bornCtx.cmdReply;
}

RdsReplyPtr EASyncRedis::cmdWithHold(const char* fmt, va_list ap)
{
    EASyncRedis::BornContext bornCtx;
    bornCtx.bornClass = shared_from_this();
    bornCtx.bornFiber = Fiber::GetThis();
    bornCtx.bornLoop = Loop::GetThis();
    char* buf = nullptr;
    int len = redisvFormatCommand(&buf, fmt, ap);
    if(len == -1){
        EMILIA_LOG_ERROR("Redis") << "redisvFormatCommand Error";
        return;
    }
    bornCtx.cmdContent = std::string(buf, len);
    ::free((void*)buf);
    //发出异步命令，然后进入阻塞态，异步命令完成后会调用CmdCb函数，CmbCb函数负责唤醒此任务
    redisAsyncFormattedCommand(m_context.get()
                               ,EASyncRedis::CmdCb
                               ,(void*)&bornCtx
                               ,bornCtx.cmdContent.c_str()
                               ,bornCtx.cmdContent.size());
    Loop::GetThis()->addAsyncTask(Fiber::GetThis());
    Fiber::YieldToHold();
    //返回命令回复
    return bornCtx.cmdReply;
}

//任务发出异步命令之后，不会进入阻塞态，之后需要使用异步命令结果时
//如果异步命令已经完成，则直接使用其返回结果
//否则进入阻塞态，线程执行其他任务，当异步命令完成后，阻塞任务被唤醒
EASyncRedis::BornContext::ptr EASyncRedis::cmdNoHold(const char* fmt, ...)
{
    va_list parameters;
    va_start(parameters, fmt);
    EASyncRedis::BornContext::ptr r = cmdNoHold(fmt, parameters);
    va_end(parameters);
    return r;
}

EASyncRedis::BornContext::ptr EASyncRedis::cmdNoHold(const std::vector<std::string>& argv)
{
    std::vector<const char*> cArgv;
    std::vector<size_t> cArgvLen;
    for(auto& i : argv){
        cArgv.push_back(i.c_str());
        cArgvLen.push_back(i.size());
    }
    char* buf = nullptr;
    int len = redisFormatCommandArgv(&buf, argv.size(), &cArgv[0], &cArgvLen[0]);
    if(len == -1){
        EMILIA_LOG_ERROR("Redis") << "redisFormatCommandArgv Error";
        return nullptr;
    }
    EASyncRedis::BornContext bornCtx;
    bornCtx.bornClass = shared_from_this();
    bornCtx.bornFiber = Fiber::GetThis();
    bornCtx.bornLoop = Loop::GetThis();
    bornCtx.cmdContent = std::string(buf, len);
    ::free((void*)buf);
    //发出异步命令
    redisAsyncFormattedCommand(m_context.get()
                               ,EASyncRedis::CmdCb
                               ,&bornCtx
                               ,bornCtx.cmdContent.c_str()
                               ,bornCtx.cmdContent.size());
    return std::make_shared<EASyncRedis::BornContext>(bornCtx);
}

EASyncRedis::BornContext::ptr EASyncRedis::cmdNoHold(const char* fmt, va_list ap)
{
    char* buf = nullptr;
    int len = redisvFormatCommand(&buf, fmt, ap);
    if(len == -1){
        EMILIA_LOG_ERROR("Redis") << "redisvFormatCommand Error";
        return nullptr;
    }
    EASyncRedis::BornContext bornCtx;
    bornCtx.bornClass = shared_from_this();
    bornCtx.bornFiber = Fiber::GetThis();
    bornCtx.bornLoop = Loop::GetThis();
    bornCtx.cmdContent = std::string(buf, len);
    ::free((void*)buf);
    //发出异步命令
    redisAsyncFormattedCommand(m_context.get()
                               ,EASyncRedis::CmdCb
                               ,&bornCtx
                               ,bornCtx.cmdContent.c_str()
                               ,bornCtx.cmdContent.size());
    //直接返回命令出生信息
    //业务需要使用返回信息时可以通过isReply判定异步命令是否已经执行完毕
    //如果isReply返回为false,可以调用Fiber::YieldToHold()
    return std::make_shared<EASyncRedis::BornContext>(bornCtx);
}

bool EASyncRedis::setLock(const char* lockName,
                          uint64_t lockValue, 
                          uint64_t lockTime)
{
    char* buf = nullptr;
    int len = redisFormatCommand(&buf
                                 ,"SET %s %lld NX PX %lld"
                                 ,lockName, lockValue, lockTime);
    if(len == -1){
        EMILIA_LOG_ERROR("Redis") << "redisvFormatCommand Error";
        return false;
    }

    BornContext bornCtx;
    bornCtx.bornClass = shared_from_this();
    bornCtx.bornFiber = Fiber::GetThis();
    bornCtx.bornLoop = Loop::GetThis();
    bornCtx.cmdContent = std::string(buf, len);
    ::free(buf);
    redisAsyncFormattedCommand(m_context.get()
                               ,EASyncRedis::CmdCb
                               ,&bornCtx
                               ,bornCtx.cmdContent.c_str()
                               ,bornCtx.cmdContent.size());
    Loop::GetThis()->addAsyncTask(Fiber::GetThis());
    Fiber::YieldToHold();
    //被唤醒
    if(bornCtx.cmdReply == nullptr)
        return false;
    if(bornCtx.cmdReply->type == REDIS_REPLY_STATUS){
        if(strcmp(bornCtx.cmdReply->str, "OK") == 0)
            return true;
        //EMILIA_LOG_ERROR("Redis") << "SetLock SetKey Error: Reply Status Is" 
        //<< bornCtx.cmdReply->str << "\n"
        //<<"(ip: " << m_ip << ", name: " << m_name << ")";
        return false;
    }
    EMILIA_LOG_ERROR("Redis") << "SetLock SetKey Error: Reply Type Is" 
    << RdsReplyTypeToString(bornCtx.cmdReply->type) << "\n"
    <<"(ip: " << m_ip << ", name: " << m_name << ")";
    return false;
}

bool EASyncRedis::freeLock(const char* lockName, uint64_t lockValue)
{
    char* buf = nullptr;
    int len = redisFormatCommand(&buf, "GET %s", lockName);
    if(len == -1){
        EMILIA_LOG_ERROR("Redis") << "redisvFormatCommand Error";
        return false;
    }
    BornContext bornCtx;
    bornCtx.bornClass = shared_from_this();
    bornCtx.bornFiber = Fiber::GetThis();
    bornCtx.bornLoop = Loop::GetThis();
    bornCtx.cmdContent = std::string(buf, len);
    ::free(buf);
    redisAsyncFormattedCommand(m_context.get()
                              ,EASyncRedis::CmdCb
                              ,&bornCtx
                              ,bornCtx.cmdContent.c_str()
                              ,bornCtx.cmdContent.size());
    Loop::GetThis()->addAsyncTask(Fiber::GetThis());
    Fiber::YieldToHold();
    //被唤醒
    if(bornCtx.cmdReply == nullptr)
        return false;
    if(bornCtx.cmdReply->type == REDIS_REPLY_INTEGER){
        //锁已经自动释放
        if(bornCtx.cmdReply->integer != lockValue)
            return true;
        //锁仍未释放需要手动释放
        len = redisFormatCommand(&buf, "DEL %s", lockName);
        if(len == -1){
            EMILIA_LOG_ERROR("Redis") << "redisvFormatCommand Error";
            return false;
        }
        bornCtx.cmdContent = std::string(buf, len);
        bornCtx.cmdReply = nullptr;
        bornCtx.isReply = false;
        bornCtx.isTimeOut = false;
        ::free(buf);
        redisAsyncCommand(m_context.get()
                          ,EASyncRedis::CmdCb
                          ,&bornCtx
                          ,bornCtx.cmdContent.c_str()
                          ,bornCtx.cmdContent.size());
        Loop::GetThis()->addAsyncTask(Fiber::GetThis());
        Fiber::YieldToHold();
        //被唤醒
        if(bornCtx.cmdReply == nullptr)
            return false;
        if(bornCtx.cmdReply->type == REDIS_REPLY_STATUS){
            if(strcmp(bornCtx.cmdReply->str, "OK") == 0)
                return true;
            EMILIA_LOG_ERROR("Redis") << "FreeLock DelKey Error: Reply Status Is" 
            << bornCtx.cmdReply->str << "\n"
            <<"(ip: " << m_ip << ", name: " << m_name << ")";
            return false;
        }
        EMILIA_LOG_ERROR("Redis") << "FreeLock DelKey Error: Reply Type Is" 
        << RdsReplyTypeToString(bornCtx.cmdReply->type) << "\n"
        <<"(ip: " << m_ip << ", name: " << m_name << ")";
        return false;
    }
    else if(bornCtx.cmdReply->type == REDIS_REPLY_NIL)
        return true;

    EMILIA_LOG_ERROR("Redis") << "FreeLock GetKey Error: Reply Type Is" 
    << RdsReplyTypeToString(bornCtx.cmdReply->type) << "\n"
    <<"(ip: " << m_ip << ", name: " << m_name << ")";
    return false;
}

bool EASyncRedis::init()
{
    if(m_status != EASyncRedis::Status::UNCONNECTED)
        return false;
    
    redisAsyncContext* ctx = redisAsyncConnect(m_ip.c_str(), util::StringUtil::ToUint32(m_port.c_str()));
    if(ctx == nullptr){
        EMILIA_LOG_ERROR("Redis") << "RedisAsyncConnect Error: Context Is Null"
                                  << "(ip: " << m_ip << ", name: " << m_name << ")";
        return false;
    }
    if(ctx->err){
        EMILIA_LOG_ERROR("Redis") << "RedisAsyncConnect Error: " << ctx->errstr
                                  << "(ip: " << m_ip << ", name: " << m_name << ")";
        return false;
    }
    m_context.reset(ctx, redisAsyncFree);
    m_status = EASyncRedis::Status::CONNECTING;
    ctx->data = this;
    //将句柄与事件循环绑定
    redisLibeventAttach(ctx, m_thread->getEventBase());
    //设置回调
    redisAsyncSetConnectCallback(ctx, EASyncRedis::ConnectCb);
    redisAsyncSetDisconnectCallback(ctx, EASyncRedis::DisConnectCb);

    //心跳检测
    
}

//连接回调
//c : 代表异步连接上下文
//status : 当前连接是否成功
void EASyncRedis::ConnectCb(const redisAsyncContext* c, int status)
{
    EASyncRedis*aRedis = static_cast<EASyncRedis*>(c->data);
    //连接成功
    if(status != 0){
        aRedis->m_status = EASyncRedis::Status::CONNECTED;
        EMILIA_LOG_INFO("Redis") << "Redis Connect Success"
                                 << "(ip: " << aRedis->m_ip << ", name: " << aRedis->m_name << ")";
        //如果有密码就验证身份
        if(!aRedis->m_passWord.empty()){
            redisAsyncCommand(aRedis->m_context.get()
                             ,EASyncRedis::AuthCb
                             ,aRedis
                             ,"auth %s" ,aRedis->m_passWord.c_str());
        }
    }
    else{
        EMILIA_LOG_ERROR("Redis") << "Redis Connect Error"
                                 << "(ip: " << aRedis->m_ip << ", name: " << aRedis->m_name << ")";
    }
}

//身份验证回调(参数同CmbCb)
void EASyncRedis::AuthCb(redisAsyncContext* c, void* reply, void* privdata)
{
    EASyncRedis* aRedis = static_cast<EASyncRedis*>(privdata);
    redisReply* r = static_cast<redisReply*>(reply);
    if(r == nullptr){
        EMILIA_LOG_ERROR("Redis") << "Redis Auth Error"
                                  << "(ip: " << aRedis->m_ip << ", name: " << aRedis->m_name << ")";
    }
    else if(r->type == REDIS_REPLY_STATUS){
        if(strcmp(r->str, "OK") == 0){
            EMILIA_LOG_INFO("Redis") << "Redis Auth Sucess"
                                     << "(ip: " << aRedis->m_ip << ", name: " << aRedis->m_name << ")";
        }
        else{
            EMILIA_LOG_ERROR("Redis") << "Redis Auth Error, Reply Status Is" << r->str
                                      << "(ip: " << aRedis->m_ip << ", name: " << aRedis->m_name << ")";

        }
    }
    else{
        EMILIA_LOG_ERROR("Redis") << "Redis Auth Error, Reply Type Is"
                                 << RdsReplyTypeToString(r->type)
                                 << "(ip: " << aRedis->m_ip << ", name: " << aRedis->m_name << ")";
    }
}

//断开连接回调(参数同ConnectCb)
void EASyncRedis::DisConnectCb(const redisAsyncContext* c, int status)
{
    EASyncRedis* aRedis = static_cast<EASyncRedis*>(c->data);
    EMILIA_LOG_INFO("Redis") << "Redis DisConnect"
                             << "(ip: " << aRedis->m_ip << ", name: " << aRedis->m_name << ")";
    aRedis->m_status = EASyncRedis::Status::UNCONNECTED;
}

//普通命令回调
//c : 代表异步连接上下文
//reply : 与同步中的reply相同，不过异步中的reply会自动释放
//privdata : 用户传入的参数
void EASyncRedis::CmdCb(redisAsyncContext* c, void* reply, void* privdata)
{
    BornContext* bornCtx = static_cast<BornContext*>(privdata);
    //判断是否超时,超时直接返回
    redisReply* r = static_cast<redisReply*>(reply);
    //如果连接出错
    if(c->err){
        EMILIA_LOG_ERROR("Redis") << "redis cmd: " << bornCtx->cmdContent << "Fail"
                                  << ", Error: " << c->errstr;
        //有错就返回nullptr
        bornCtx->cmdReply = nullptr;
        //调用对应Loop 唤醒Fiber
        bornCtx->bornLoop->wakeUpAsyncTask(bornCtx->bornFiber);
    }
    if(r == nullptr){
        EMILIA_LOG_ERROR("Redis") << "redis cmd: " << bornCtx->cmdContent << "Fail"
                                  << ", Error: Reply Null";
        //有错就返回nullptr
        bornCtx->cmdReply = nullptr;
        //调用对应Loop 唤醒Fiber
        bornCtx->bornLoop->wakeUpAsyncTask(bornCtx->bornFiber);
    }
    else if(r->type == REDIS_REPLY_ERROR){
        EMILIA_LOG_ERROR("Redis") << "redis cmd: " << bornCtx->cmdContent << "Fail"
                                  << ", Error: " << r->str;
        //有错就返回nullptr
        bornCtx->cmdReply = nullptr;
        //调用对应Loop 唤醒Fiber
        bornCtx->bornLoop->wakeUpAsyncTask(bornCtx->bornFiber);
    }
    else{
        RdsReplyPtr result(RdsReplyClone(r), freeReplyObject);
        bornCtx->isReply = true;
        bornCtx->cmdReply = result;
        //调用对应Loop 唤醒Fiber
        bornCtx->bornLoop->wakeUpAsyncTask(bornCtx->bornFiber);
    }
}

}
}