#include "address.h"
#include "emilia/log/logmarco.h"
#include "emilia/endian.h"

#include <netdb.h>
#include <sstream>
#include <ifaddrs.h>
#include <stddef.h>
#include <errno.h>
#include <error.h>

namespace emilia{
namespace net{

//--------------------------------------------------------------
//IPAddress
bool IPAddress::operator<  (const IPAddress& rhs) const
{
    socklen_t minlen = std::min(getAddrLen(), rhs.getAddrLen());
    //获得最小长度，按最小长度进行比较
    int rt = memcmp(getAddr(), rhs.getAddr(), minlen);
    if( rt < 0 )
    {
        return true;
    }
    else if ( rt > 0 )
    {
        return false;
    }
    else if( getAddrLen() < rhs.getAddrLen() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IPAddress::operator== (const IPAddress& rhs) const
{
    return getAddrLen() == rhs.getAddrLen()
        && memcmp(getAddr(), rhs.getAddr(), getAddrLen());
}

bool IPAddress::operator!= (const IPAddress& rhs) const
{
    return !((*this) == rhs);
}


std::string IPAddress::toString()
{
    std::stringstream os;
    insert(os);
    return os.str();
}


IPAddress::ptr IPAddress::CreateByString(const char* address, uint16_t port)
{
    addrinfo hints, *results;
    bzero(&hints, sizeof(hints));

    hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = AF_UNSPEC;

    int rt = getaddrinfo(address, NULL, &hints, &results);
    if(rt)
    {
        EMILIA_LOG_ERROR("system") << "getaddrinfo for ip:" << address << " error, error = "
        << errno;
        return nullptr;
    }

    IPAddress::ptr result;

    result = CreateBySockaddress(results->ai_addr, sizeof(results->ai_addr));
    if(result)
    {

        result->setPort(port);
        freeaddrinfo(results);
        return result;
    }

    freeaddrinfo(results);
    EMILIA_LOG_ERROR("system") << "IPAddress::Create error for ip:" << address;
    return nullptr;

}

IPAddress::ptr IPAddress::CreateBySockaddress(const sockaddr* address, socklen_t len)
{
    if( address == nullptr )
        return nullptr;

    IPAddress::ptr result;
    switch (address->sa_family)
    {
    case AF_INET:
        result.reset(new IPv4Address( * (const sockaddr_in*)address));
        break;
    case AF_INET6:
        result.reset(new IPv6Address( * (const sockaddr_in6*)address));
        break;
    default:
        EMILIA_LOG_ERROR("system") << "CreateBySockaddress error for" << address;
        EMILIA_LOG_ERROR("system") << address->sa_family;
        EMILIA_LOG_ERROR("system") << address->sa_data;
        return nullptr;
    }

    return result;
}

IPAddress::ptr IPAddress::CreateByHost(const std::string& host)
{
    std::vector<IPAddress::ptr> result;

    LookupByHost(result, host);

    for(auto& i : result)
    {
        if( i != nullptr )
        {
            return i;
        }
    }
    return nullptr;
}

bool IPAddress::LookupByHost(std::vector<IPAddress::ptr>& result, 
                             const std::string& host,
                             int family, 
                             int type, 
                             int protocol)
{
    addrinfo hints, *results, *next;
    hints.ai_flags = 0;
    hints.ai_family = family;
    hints.ai_socktype = type;
    hints.ai_protocol = protocol;
    hints.ai_addrlen = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;


    std::string node;
    const char* service = NULL;

    //检查 ipv6address serivce
    if(!host.empty() && host[0] == '[') {
        const char* endipv6 = (const char*)memchr(host.c_str() + 1, ']', host.size() - 1);
        if(endipv6) {
            //TODO check out of range
            if(*(endipv6 + 1) == ':') {
                service = endipv6 + 2;
            }
            node = host.substr(1, endipv6 - host.c_str() - 1);
        }
    }

    //检查 node serivce
    if(node.empty()) {
        service = (const char*)memchr(host.c_str(), ':', host.size());
        if(service) {
            if(!memchr(service + 1, ':', host.c_str() + host.size() - service - 1)) {
                node = host.substr(0, service - host.c_str());
                ++service;
            }
        }
    }

    if(node.empty()) {
        node = host;
    }


    int error = getaddrinfo(node.c_str(), service, &hints, &results);
    if(error) {
        EMILIA_LOG_ERROR("system") << "Address::Lookup getaddress(" << host << ", "
            << family << ", " << type << ") err=" << error << " errstr="
            << gai_strerror(error);
        return false;
    }
    next = results;
    
    while(next) 
    {
        result.push_back(CreateBySockaddress(next->ai_addr, sizeof(next->ai_addr)));
        next = next->ai_next;
    }

    freeaddrinfo(results);
    return !result.empty();
}

/*
bool IPAddress::GetInterfaceAddresses(std::multimap<std::string,
                                      std::pair<IPAddress::ptr, uint32_t> >& result,
                                      int family)
{
    struct ifaddrs *next, *results;
    if(getifaddrs(&results) != 0) {
        EMILIA_LOG_ERROR("system") << "Address::GetInterfaceAddresses getifaddrs "
            " err=" << errno << " errstr=" << strerror(errno);
        return false;
    }

    try {
        for(next = results; next; next = next->ifa_next) {
            Address::ptr addr;
            uint32_t prefix_len = ~0u;
            if(family != AF_UNSPEC && family != next->ifa_addr->sa_family) {
                continue;
            }
            switch(next->ifa_addr->sa_family) {
                case AF_INET:
                    {
                        addr = CreateBySockaddress(next->ifa_addr, sizeof(sockaddr_in));
                        //uint32_t netmask = ((sockaddr_in*)next->ifa_netmask)->sin_addr.s_addr;
                        //prefix_len = CountBytes(netmask);
                    }
                    break;
                case AF_INET6:
                    {
                        addr = CreateBySockaddress(next->ifa_addr, sizeof(sockaddr_in6));
                        
                        in6_addr& netmask = ((sockaddr_in6*)next->ifa_netmask)->sin6_addr;
                        prefix_len = 0;
                        for(int i = 0; i < 16; ++i) {
                            prefix_len += CountBytes(netmask.s6_addr[i]);
                        }
                        
                    }
                    break;
                default:
                    break;
            }

            if(addr) {
                result.insert(std::make_pair(next->ifa_name,
                            std::make_pair(addr, prefix_len)));
            }
        }
    } catch (...) {
        EMILIA_LOG_ERROR("system") << "Address::GetInterfaceAddresses exception";
        freeifaddrs(results);
        return false;
    }
    freeifaddrs(results);
    return !result.empty();

}
*/

//--------------------------------------------------------------
//IPv4Address
IPv4Address::IPv4Address(uint32_t address, uint16_t port)
{
    bzero(&address, sizeof(address));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = byteswapOnLittleEndian(port);
    m_addr.sin_addr.s_addr = byteswapOnLittleEndian(address);
}

IPv4Address::IPv4Address(const char* address, uint16_t port)
{
    bzero(&m_addr, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = byteswapOnLittleEndian(port);
    int rt = inet_pton(AF_INET, address, &m_addr.sin_addr);
    if( rt <= 0 )
    {
        EMILIA_LOG_ERROR("system") << "inet_pton error for ip: "
                                   << address;
    }
}

IPv4Address::IPv4Address(const sockaddr_in& addr)
{
    m_addr = addr;
}

const sockaddr* IPv4Address::getAddr() const
{
    return (const sockaddr*)&m_addr;
}

sockaddr* IPv4Address::getAddrnoconst()
{
    return (sockaddr*)&m_addr;
}

socklen_t IPv4Address::getAddrLen() const
{
    return sizeof(m_addr);
}

void IPv4Address::insert(std::ostream& os) const
{
    int address = byteswapOnLittleEndian((int)m_addr.sin_addr.s_addr);
    os  << ( (address >> 24) & 0x000000ff ) << "."
        << ( (address >> 16) & 0x000000ff ) << "."
        << ( (address >> 8) & 0x000000ff ) << "."
        << ( (address) & 0x000000ff );

    os  << ":"
        << byteswapOnLittleEndian(m_addr.sin_port);
}

uint16_t IPv4Address::getPort() const
{
    return byteswapOnLittleEndian(m_addr.sin_port);
}

void IPv4Address::setPort(uint16_t v)
{
    m_addr.sin_port = byteswapOnLittleEndian(v);
}

//--------------------------------------------------------------
//IPv6Address
IPv6Address::IPv6Address(const uint8_t address[16], uint16_t port)
{
    bzero(&m_addr, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
    m_addr.sin6_port = byteswapOnLittleEndian(port);
    memcpy(&m_addr.sin6_addr.s6_addr, address, 16);
}

IPv6Address::IPv6Address(const char* address, uint16_t port)
{
    bzero(&m_addr, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
    m_addr.sin6_port = byteswapOnLittleEndian(port);
    int rt = inet_pton(AF_INET6, address, &m_addr.sin6_addr);
    if( rt <= 0 )
    {
        EMILIA_LOG_ERROR("system") << "inet_pton error for ip: "
                                   << address;
    }
}

IPv6Address::IPv6Address(const sockaddr_in6& addr)
{
    m_addr = addr;
}

const sockaddr* IPv6Address::getAddr() const
{
    return (sockaddr*)&m_addr;
}

sockaddr* IPv6Address::getAddrnoconst()
{
    return (sockaddr*)&m_addr;
}

socklen_t IPv6Address::getAddrLen() const
{
    return sizeof(m_addr);
}

void IPv6Address::insert(std::ostream& os) const
{
    os << "[";
    uint16_t* addr = (uint16_t*)m_addr.sin6_addr.s6_addr;
    bool used_zeros = false;
    for(size_t i = 0; i < 8; ++i) {
        if(addr[i] == 0 && !used_zeros) {
            continue;
        }
        if(i && addr[i - 1] == 0 && !used_zeros) {
            os << ":";
            used_zeros = true;
        }
        if(i) {
            os << ":";
        }
        os << std::hex << (int)byteswapOnLittleEndian(addr[i]) << std::dec;
    }

    if(!used_zeros && addr[7] == 0) {
        os << "::";
    }

    os << "]:" << byteswapOnLittleEndian(m_addr.sin6_port);
}

uint16_t IPv6Address::getPort() const
{
    return byteswapOnLittleEndian(m_addr.sin6_port);
}

void IPv6Address::setPort(uint16_t v)
{
    m_addr.sin6_port = byteswapOnLittleEndian(v);
}

//--------------------------------------------------------------
//UnixAddress
UnixAddress::UnixAddress(const std::string& path) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;
    m_length = path.size() + 1;

    if(!path.empty() && path[0] == '\0') {
        --m_length;
    }

    if(m_length > sizeof(m_addr.sun_path)) {
        throw std::logic_error("path too long");
    }
    memcpy(m_addr.sun_path, path.c_str(), m_length);
    m_length += offsetof(sockaddr_un, sun_path);
}

void UnixAddress::setAddrLen(uint32_t v) 
{
    m_length = v;
}

const sockaddr* UnixAddress::getAddr() const 
{
    return (const sockaddr*)&m_addr;
}

socklen_t UnixAddress::getAddrLen() const 
{
    return m_length;
}

std::string UnixAddress::getPath() const 
{
    std::stringstream ss;
    if(m_length > offsetof(sockaddr_un, sun_path)
            && m_addr.sun_path[0] == '\0') {
        ss << "\\0" << std::string(m_addr.sun_path + 1,
                m_length - offsetof(sockaddr_un, sun_path) - 1);
    } else {
        ss << m_addr.sun_path;
    }
    return ss.str();
}

void UnixAddress::insert(std::ostream& os) const {
    if(m_length > offsetof(sockaddr_un, sun_path)
            && m_addr.sun_path[0] == '\0') {
        os << "\\0" << std::string(m_addr.sun_path + 1,
                m_length - offsetof(sockaddr_un, sun_path) - 1);
    }
    os << m_addr.sun_path;
}

}
}
