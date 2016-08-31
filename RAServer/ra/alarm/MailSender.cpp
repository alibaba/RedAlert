#include <ra/alarm/MailSender.h>
#include <curl/curl.h>

using namespace std;

RA_BEGIN_NAMESPACE(alarm);

MailSender::MailSender(const string& mailUser, 
                       const string& mailPwd, 
                       const string& mailServerUrl)
{
    _mailUser = mailUser;
    _mailPwd = mailPwd;
    _mailServerUrl = mailServerUrl;
    _stage = PS_DATE;
}

MailSender::~MailSender() { 
}

size_t MailSender::readMailPayload(void *ptr, size_t size, size_t nmemb, void* userp)
{
    MailSender* self = (MailSender*)userp;
    char* buffer = (char*)ptr;
    size_t bufferLen = size * nmemb;
    if (NULL == self || NULL == ptr || bufferLen <= 0) {
        LOG(ERROR) << "invalid payload param, self=" << self << ", ptr= " 
		   << ptr << ", bufsize=" << bufferLen; 
        return 0;
    }

    // for now, we dont take charset/encode .etc into consideration
    size_t ret = 0;
    switch (self->getStage()) {
        case PS_DATE:
            {
                time_t t;
                time(&t);
                ret = strftime(buffer, bufferLen, "Date: %d %b %y %H:%M:%S %Z\r\n", localtime(&t));
            }
            break;
        case PS_TO:
            {
                ret = snprintf(buffer, bufferLen, "To: %s\r\n", self->getRcpt().c_str());
            }
            break;
        case PS_FROM:
            {
                ret = snprintf(buffer, bufferLen, "From: %s\r\n", self->getMailUser().c_str());
            }
            break;
        case PS_SUBJECT:
            {
                ret = snprintf(buffer, bufferLen, "Subject: RA Alarm[%s]\r\n", 
                        self->getTitle().c_str());
            }
            break;
        case PS_CRLF:
            {
                ret = snprintf(buffer, bufferLen, "\r\n");
            }
            break;
        case PS_BODY:
            {
                ret = snprintf(buffer, bufferLen, "%s\r\n", self->getBody().c_str());
            }
            break;
        case PS_END:
            // do nothing
            break;
    }
    self->incrStage();
    return ret;
}

bool MailSender::sendMail(const vector<string>& addressVec,
                          const string& title, const string& body)
{
    CURL *curlHandler = NULL;
    CURLcode res = CURLE_OK;

    curlHandler = curl_easy_init();
    if (NULL == curlHandler) {
        LOG(ERROR) << "curl_easy_init failure";
        return false;
    }

    curl_slist *recipients = NULL;
    for (size_t i = 0; i < addressVec.size(); ++i) {
        recipients = curl_slist_append(recipients, addressVec[i].c_str());
        _rcpt.append(addressVec[i]);
        if (addressVec.size() != i + 1) {
            _rcpt.append(",\r\n ");
        }
    }

    _title = title;
    _body = body;

    curl_easy_setopt(curlHandler, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curlHandler, CURLOPT_USERNAME, _mailUser.c_str());
    curl_easy_setopt(curlHandler, CURLOPT_PASSWORD, _mailPwd.c_str());
    curl_easy_setopt(curlHandler, CURLOPT_URL, _mailServerUrl.c_str());
    curl_easy_setopt(curlHandler, CURLOPT_MAIL_FROM, _mailUser.c_str());
    curl_easy_setopt(curlHandler, CURLOPT_MAIL_RCPT, recipients);
    curl_easy_setopt(curlHandler, CURLOPT_READFUNCTION, MailSender::readMailPayload);
    curl_easy_setopt(curlHandler, CURLOPT_READDATA, this);
    curl_easy_setopt(curlHandler, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curlHandler, CURLOPT_SSL_VERIFYPEER, 0);

    res = curl_easy_perform(curlHandler);
    if (CURLE_OK != res) {
        LOG(WARNING) << "send mail failure, curl_easy_perform error: " << curl_easy_strerror(res);
    }
    else {
        VLOG(1) << "alarm msg (" << _body << ") delivered to " << _rcpt;
    }
    curl_slist_free_all(recipients);
    curl_easy_cleanup(curlHandler);
    return CURLE_OK == res;
}

RA_END_NAMESPACE(alarm);

