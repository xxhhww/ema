#ifndef _EMILIA_EMAIL_H_
#define _EMILIA_EMAIL_H_

#include <string>
#include <vector>
#include <map>

#include <memory>

namespace emilia{
namespace email{

class MimeEntity
{
public:
    using ptr = std::shared_ptr<MimeEntity>;
    
    void setHeader(const std::string& key, const std::string& value);
    std::string getHeader(const std::string& key);

    void setContent(const std::string& content) { m_content = content; }
    const std::string& getContent() const { return m_content; }

    //用于smtp发送
    std::string toString() const;

    //创建一个附件
    static MimeEntity::ptr CreateAttach(const std::string& filename);
private:
    //每一个Mime对象都有一个头部字段
    //存储类似 
    //Content-type:text/html
    //Content-Transfer-Encoding:base64
    //用于表示附件的Mime还有其他的头部字段，不再举列
    std::map<std::string, std::string> m_headers;

    //Mime对象的内容
    std::string m_content;
};

class Email
{
public:
    using ptr = std::shared_ptr<Email>;

    void setMailFrom(const std::string& mailFrom) { m_mailFrom = mailFrom; }
    const std::string& getMailFrom() const { return m_mailFrom; }

    void setPassWord(const std::string& passWord) { m_passWord = passWord; }
    const std::string& getPassWord() const { return m_passWord; }

    void setMailTo(const std::vector<std::string>& mailTo) { m_mailTo = mailTo; }
    const std::vector<std::string>& getMailTo() const { return m_mailTo; }

    void setMailCc(const std::vector<std::string>& mailCc) { m_mailCc = mailCc; }
    const std::vector<std::string>& getMailCc() const { return m_mailCc; }

    void setMailBcc(const std::vector<std::string>& mailBcc) { m_mailBcc = mailBcc; }
    const std::vector<std::string>& getMailBcc() const { return m_mailBcc; }

    void setFrom(const std::string& from) { m_from = from; }
    const std::string&  getFrom() const;

    void setTo(const std::string& to) { m_to = to; }
    const std::string getTo() const;

    void setDate(const std::string& date) { m_date = date; }
    const std::string& getDate() const { return m_date; }

    void setSubject(const std::string& subject) { m_subject = subject; }
    const std::string& getSubject() const { return m_subject; }

    void setMimeVer(const std::string& mimeVer) { m_mimeVer = mimeVer; }
    const std::string& getMimeVer() const { return m_mimeVer; }

    void setMultipart(const std::string& multipart) { m_multipart = multipart; }
    const std::string& getMultipart() const { return m_multipart; }

    void addMimeEntity(MimeEntity::ptr mimeEntity) { m_mimeEntitys.push_back(mimeEntity); }
    const std::vector<MimeEntity::ptr> getMimeEntity() const { return m_mimeEntitys; }
private:
    //发件人地址
    std::string m_mailFrom;

    //发件人密码
    std::string m_passWord;

    //rcpt to后面的信息包括（to cc bcc）

    //普通收件地址
    std::vector<std::string> m_mailTo;

    //抄送地址
    std::vector<std::string> m_mailCc;

    //密送地址
    std::vector<std::string> m_mailBcc;

    //如果此变量为空，那么smtp的From:
    //就直接使用m_mailFrom
    std::string m_from;

    //如果此变量为空，那么smtp的To:
    //就直接使用m_mailTo;m_mailCc;m_mailBcc
    std::string m_to;

    //邮件发送的时间
    std::string m_date;

    //邮件的主题（标题）
    std::string m_subject;

    //以下为MIME扩展内容

    //MIME版本
    std::string m_mimeVer;

    //使用的Mime的multipart类型
    std::string m_multipart;

    //存储的Mime对象
    //如果m_multipart为空的话，Mime对象只有一个
    std::vector<MimeEntity::ptr> m_mimeEntitys;
};


}
}

#endif
