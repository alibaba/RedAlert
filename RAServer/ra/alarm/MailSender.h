#ifndef RA_MAILSENDER_H
#define RA_MAILSENDER_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>

RA_BEGIN_NAMESPACE(alarm);

class MailSender
{
public:
    MailSender(const std::string& mailUser, 
               const std::string& mailPwd, 
               const std::string& mailServerUrl);
    ~MailSender();
public:
    enum PayloadStage
    {
        PS_DATE,
        PS_TO,
        PS_FROM,
        PS_SUBJECT,
        PS_CRLF,
        PS_BODY,
        PS_END,
    };

private:
    MailSender(const MailSender &);
    MailSender& operator=(const MailSender &);

public:
    static size_t readMailPayload(void *ptr, size_t size, size_t nmemb, void* userp);
    PayloadStage getStage() const {
        return (PayloadStage)_stage;
    }
    void incrStage() {
        if (_stage < PS_END) {
            _stage += 1;
        }
    }
    const std::string& getRcpt() const {
        return _rcpt;
    }
    const std::string& getMailUser() const {
        return _mailUser;
    }
    const std::string& getTitle() const {
        return _title;
    }
    const std::string& getBody() const {
        return _body;
    }
    
    bool sendMail(const std::vector<std::string>& addressVec,
                  const std::string& title, const std::string& body);

private:
    std::string _rcpt;
    int32_t _stage;
    std::string _mailUser;
    std::string _mailPwd;
    std::string _mailServerUrl;

    std::string _title;
    std::string _body;
};

RA_TYPEDEF_PTR(MailSender);

RA_END_NAMESPACE(alarm);

#endif //RA_MAILSENDER_H
