#ifndef _EMILIA_HTTP_SERVLET_H_
#define _EMILIA_HTTP_SERVLET_H_

#include "http.h"
#include "http_session.h"
#include "emilia/base/mutex.h"

#include <unordered_map>

using namespace emilia::base;

namespace emilia{
namespace http{

//对于某个客户端请求，用对应的Servlet匹配
class Servlet{
public:
    using ptr = std::shared_ptr<Servlet>;

    Servlet(const std::string& name) { m_name = name; }
    virtual ~Servlet() {}

    virtual bool handle(HttpRequest::ptr req, HttpSession::ptr session) = 0;

    const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

protected:
    std::string m_name;
};

//默认的Servlet，其他情况都不匹配时，返回此Servlet(Not Found 404)
class DefaultServlet : public Servlet{
public:
    using ptr = std::shared_ptr<DefaultServlet>;

    DefaultServlet(const std::string& name = "DefaultServlet");
    ~DefaultServlet(){}

    virtual bool handle(HttpRequest::ptr req, HttpSession::ptr session) override;
private:
    std::string m_body;
};

//返回文件的Servlet
class FileServlet : public Servlet{
public:
    using ptr = std::shared_ptr<FileServlet>;

    FileServlet(const std::string& path, const std::string& name = "FileServlet");
    ~FileServlet(){}

    virtual bool handle(HttpRequest::ptr req, HttpSession::ptr session) override;

private:
    std::string m_path; //文件存储的路径
};

class ImageServlet : public Servlet{
public:
    ImageServlet(const std::string& path, const std::string& name = "ImageServlet");
    ~ImageServlet(){}

    virtual bool handle(HttpRequest::ptr req, HttpSession::ptr session) override;

private:
    std::string m_path; //图片存储的路径
};

//管理Servlet对象，并针对不同请求分发Servlet对象
class ServletManger{
public:
    using ptr = std::shared_ptr<ServletManger>;

    ServletManger(const std::string& name = "Servlet For Get");
    ~ServletManger(){}

    bool handle(HttpRequest::ptr req, HttpSession::ptr session);

    void addServlet(const std::string& path, Servlet::ptr servlet);
    void delServlet(const std::string& path);

    Servlet::ptr getServlet(const std::string& path);

private:
    //读写锁，因为一个http_serve只有一个servletmanger
    RWMutex m_mutex;

    std::string m_name;

    std::unordered_map<std::string, Servlet::ptr> m_servlets;
    //默认的Servlet
    Servlet::ptr m_defaultServlet;

};

}
}

#endif