#ifndef _EMILIA_ADDRESS_H_
#define _EMILIA_ADDRESS_H_

#include <memory>
#include <vector>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

namespace emilia{
namespace net{
//--------------------------------------------------------------
//IPAddress
class IPAddress
{
public:
    using ptr = std::shared_ptr<IPAddress>;

    virtual uint16_t    getPort()           const = 0;
    virtual void        setPort(uint16_t v)       = 0;

    virtual const sockaddr* getAddr() const = 0;
    virtual sockaddr* getAddrnoconst() = 0;

    virtual socklen_t getAddrLen() const = 0;

    virtual void insert(std::ostream& os) const = 0;
    std::string toString();

    static IPAddress::ptr CreateByString(const char* address, uint16_t port = 0);
    //("127.0.0.1", 80)

    static IPAddress::ptr CreateByHost(const std::string& host);
    //("www.xhwio.top", 9999)

    static IPAddress::ptr CreateBySockaddress(const sockaddr* address, socklen_t len);
    // (sockaddr*) 
    
    /*
    static bool GetInterfaceAddresses(std::multimap<std::string,
                                      std::pair<IPAddress::ptr, uint32_t> >& result,
                                      int family = AF_UNSPEC);
    */

    static bool LookupByHost(std::vector<IPAddress::ptr>& result, 
                             const std::string& host,
                             int family = AF_UNSPEC, 
                             int type = 0, 
                             int protocol = 0);
    //通过域名，查找ip地址并存放在result中

    bool operator<  (const IPAddress& rhs) const;
    bool operator== (const IPAddress& rhs) const;
    bool operator!= (const IPAddress& rhs) const;

};

//--------------------------------------------------------------
//IPv4Address
class IPv4Address : public IPAddress
{
public:
    using ptr = std::shared_ptr<IPv4Address>;

    IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);
    IPv4Address(const char* address, uint16_t port = 0);
    IPv4Address(const sockaddr_in& addr);

    const sockaddr* getAddr()                   const override;
    sockaddr* getAddrnoconst() override;
    socklen_t       getAddrLen()                const override;
    void            insert(std::ostream& os)    const override;
    uint16_t        getPort()                   const override;
    void            setPort(uint16_t v)               override;
private:
    sockaddr_in m_addr;
};

//--------------------------------------------------------------
//IPv6Address
class IPv6Address : public IPAddress
{
public:
    using ptr = std::shared_ptr<IPv6Address>;

    IPv6Address(const uint8_t address[16], uint16_t port = 0);
    IPv6Address(const char* address, uint16_t port = 0);
    IPv6Address(const sockaddr_in6& addr);

    const sockaddr* getAddr()                   const override;
    sockaddr* getAddrnoconst() override;
    socklen_t       getAddrLen()                const override;
    void            insert(std::ostream& os)    const override;
    uint16_t        getPort()                   const override;
    void            setPort(uint16_t v)               override;

private:
    sockaddr_in6 m_addr;
};

//--------------------------------------------------------------
//UnixAddress
class UnixAddress
{
public:
    using ptr = std::shared_ptr<UnixAddress>;

    UnixAddress(const std::string& path);

    const sockaddr* getAddr()                   const;
    std::string     getPath()                   const;
    socklen_t       getAddrLen()                const;
    void            setAddrLen(uint32_t v);
    void            insert(std::ostream& os)    const;
private:
    sockaddr_un m_addr;
    socklen_t m_length;

};

}
}

#endif