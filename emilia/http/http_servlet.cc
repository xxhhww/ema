#include "http_serve.h"

#include <fstream>

namespace emilia{
namespace http{

//================================================================================
DefaultServlet::DefaultServlet(const std::string& name)
:Servlet(name)
{
    m_body = "<html><head><title>404 Not Found</title></head><body>"
             "<center><h1>Hello emilia</h1></center>"
             "<hr><center>emilia/1.0.0</center></body></html>";
}

bool DefaultServlet::handle(HttpRequest::ptr req, HttpSession::ptr session)
{
    HttpResponse::ptr res(new HttpResponse(req->isclose()));
    res->setHeader("Serve", "emilia/1.0");
    res->setHeader("Content-type", "text/html");
    res->setBody(m_body);

    int rt = session->sendResponse(res);
    if( rt == -1 )
        return false;

    return true;
}
//================================================================================

//================================================================================
FileServlet::FileServlet(const std::string& path, const std::string& name)
:Servlet(name)
,m_path(path)
{}

bool FileServlet::handle(HttpRequest::ptr req, HttpSession::ptr session)
{
    auto it = m_path + req->getPath();
    EMILIA_LOG_INFO("test") << "handle path = " << it;

    std::ifstream ifs(it);
    if(!ifs)
    {
        EMILIA_LOG_INFO("test") << "test";
        return false;
    }

    std::stringstream ss;
    std::string line;

    while(std::getline(ifs, line))
    {
        ss << line << std::endl;
    }

    HttpResponse::ptr res(new HttpResponse(req->isclose()));
    res->setBody(ss.str());
    res->setHeader("Serve", "emilia/1.0");
    res->setHeader("Content-type", "text/html");
    EMILIA_LOG_INFO("test") << "test";
    int rt = session->sendResponse(res);
    if( rt == -1 )
    {
        return false;
        EMILIA_LOG_INFO("test") << "test";
    }

    EMILIA_LOG_INFO("test") << "test";
    return true;
}
//================================================================================

//================================================================================
ImageServlet::ImageServlet(const std::string& path, const std::string& name)
:Servlet(name)
,m_path(path)
{}

bool ImageServlet::handle(HttpRequest::ptr req, HttpSession::ptr session)
{
    return true;
}

//================================================================================

//================================================================================
ServletManger::ServletManger(const std::string& name)
:m_name(name)
,m_defaultServlet(new DefaultServlet("DefaultServlet"))
{}

bool ServletManger::handle(HttpRequest::ptr req, HttpSession::ptr session)
{
    auto servlet = getServlet(req->getPath());
    if(servlet)
    {
        return servlet->handle(req, session);
    }

    return false;
}

void ServletManger::addServlet(const std::string& path, Servlet::ptr servlet)
{
    m_servlets.insert(std::make_pair(path, servlet));
}

void ServletManger::delServlet(const std::string& path)
{
    return;
}

Servlet::ptr ServletManger::getServlet(const std::string& path)
{
    auto it = m_servlets.find(path);
    return it == m_servlets.end() ? m_defaultServlet : it->second;
}
//================================================================================

}
}
