#include "email.h"
#include "emilia/util/codeutil.h"

#include <fstream>
#include <sstream>

namespace emilia{
namespace email{

void MimeEntity::setHeader(const std::string& key, const std::string& value)
{
    m_headers[key] = value;
}

std::string MimeEntity::getHeader(const std::string& key)
{
    auto it = m_headers.find(key);
    return it == m_headers.end() ? "" : it->second;
}

std::string MimeEntity::toString() const
{
    std::stringstream ss;
    for(auto& i : m_headers)
    {
        ss << i.first << ": " << i.second << "\r\n";
    }
    ss << "\r\n";
    ss << m_content << "\r\n";
    return ss.str();
}

MimeEntity::ptr MimeEntity::CreateAttach(const std::string& filename)
{
    std::ifstream ifs(filename, std::ios::binary);
    std::string buf;
    buf.resize(1024);
    MimeEntity::ptr entity(new MimeEntity());
    std::string content;
    while(!ifs.eof())
    {
        ifs.read(&buf[0], buf.size());
        content.append(buf.c_str(), ifs.gcount());
    }

    entity->setHeader("Content-Transfer-Encoding", "base64");
    entity->setHeader("Content-Disposition", "attachment");
    entity->setHeader("Content-Type", "application/octet-stream;name=favicon.ico"); //记得该回去
    entity->setContent(emilia::util::base64encode(content));
    
    return entity;
}

const std::string& Email::getFrom() const
{
    if(!m_from.empty())
        return m_from;
    return m_mailFrom;
}
const std::string Email::getTo() const
{
    //如果m_to人为设置好了，就直接用m_to，不然就使用m_mailTo
    if(!m_to.empty())
        return m_to;

    std::stringstream ss;
    for(size_t i = 0; i < m_mailTo.size(); i++ )
    {
        if(i)
            ss << ",";
        
        ss << m_mailTo[i];
    }
    return ss.str();
}
}
}
