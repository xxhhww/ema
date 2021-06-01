#include "emilia/email/email.h"
#include "emilia/email/smtp_client.h"
#include "emilia/base/scheduler.h"
#include "emilia/util/codeutil.h"
#include "emilia/log/logmarco.h"

using namespace emilia::base;
using namespace emilia::log;

void test(){
    EMILIA_LOG_INFO("test") << "测试开始";
    emilia::email::SmtpClient::ptr smtpClient = emilia::email::SmtpClient::Create("smtp.163.com:465", true);
   
    emilia::email::Email::ptr email_(new emilia::email::Email);
    
    email_->setMailFrom("a15190919945@163.com");
    email_->setPassWord("SOKVHAZWUNJIOLFU");

    std::vector<std::string> MailTo;
    MailTo.push_back("2908903586@qq.com");

    email_->setMailTo(MailTo);
    email_->setSubject("testSmtpClient");


    //emilia::email::MimeEntity::ptr mEntity2(new emilia::email::MimeEntity);
    //mEntity2->setHeader("Content-Type", "text/html;charset=\"utf-8\"");
    //mEntity2->setHeader("Content-Transfer-Encoding", "base64");
    //mEntity2->setContent(emilia::base64encode("<html><body><h1>wuhu起飞</h1><img src=\"cid:0\"></body></html>"));

    emilia::email::MimeEntity::ptr image = emilia::email::MimeEntity::CreateAttach("/home/emilia/workspace/emilia/bin/favicon.ico");
    //image->setHeader("Content-ID", "<0>");
    

    emilia::email::MimeEntity::ptr mEntity(new emilia::email::MimeEntity);
    mEntity->setHeader("Content-Type", "text/plain;charset=\"utf-8\"");
    mEntity->setHeader("Content-Transfer-Encoding", "base64");   
    mEntity->setContent(emilia::util::base64encode("15906276127"));
    
    //email_->addMimeEntity(mEntity2);
    email_->addMimeEntity(mEntity);
    email_->addMimeEntity(image);
    email_->setMultipart("multipart/mixed");
 
    if(smtpClient->send(email_)){
        EMILIA_LOG_INFO("test") << "发送完成";
        return;
    }
    EMILIA_LOG_INFO("test") << "发送失败";
    return;
}

int main(){
    Scheduler* scheduler = new Scheduler;
    scheduler->assign(std::bind(&test));
    scheduler->initMainScheduler();
    scheduler->run();
    delete scheduler;
    return 0;
}