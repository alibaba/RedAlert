#include <sstream>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <cppunit/TestAssert.h>
#include <ra/test/test.h>
#include <ra/alarm/MailSender.h>
#include <ra/alarm/test/MailSenderTest.h>

RA_BEGIN_NAMESPACE(alarm);
RA_LOG_SETUP(alarm, MailSenderTest);
using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(MailSenderTest);

#define SMTP_URL "smtps://smtp.alibaba-inc.com"
#define USERNAME "shenma-alarm@alibaba-inc.com"
#define PASSWORD "AlibabaV587"

#define FROM     "<shenma-alarm@alibaba-inc.com>"
#define TO       "<shenma-alarm@alibaba-inc.com>"
#define CC       "<shenma-alarm@alibaba-inc.com>"

MailSenderTest::MailSenderTest() {
}

MailSenderTest::~MailSenderTest() {
}

void MailSenderTest::setUp() {
    RA_LOG(INFO, "setUp!");
}

void MailSenderTest::tearDown() {
    RA_LOG(INFO, "tearDown!");
}

struct upload_status {
  const char **payload_text;
};

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp) {
    struct upload_status *upload_ctx = (struct upload_status *) userp;
    const char *data;
    if ((size == 0) || (nmemb == 0) || ((size * nmemb) < 1)) {
        return 0;
    }
    data = *upload_ctx->payload_text;
    if (data) {
        size_t len = strlen(data);
        memcpy(ptr, data, len);
        upload_ctx->payload_text++;

        return len;
    }
    return 0;
}

void MailSenderTest::testCurl() {
    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;
    struct upload_status upload_ctx;

    static const char *payload_text[] = {
        "Date: Mon, 29 Nov 2010 21:54:29 +1100\r\n",
        "To: " TO "\r\n",
        "From: " FROM "(Example User)\r\n",
        "Cc: " CC "(Another example User)\r\n",
        "Message-ID: <dcd7cb36-11db-487a-9f3a-e652a9458efd@"
        "rfcpedant.example.org>\r\n",
        "Subject: SMTP example message\r\n",
        "\r\n", /* empty line to divide headers from body, see RFC5322 */
        "The body of the message starts here.\r\n",
        "\r\n",
        "It could be a lot of lines, could be MIME encoded, whatever.\r\n",
        "Check RFC5322.\r\n",
        NULL
    };

    upload_ctx.payload_text = &payload_text[0];

    curl = curl_easy_init();
    CPPUNIT_ASSERT(curl);
    curl_easy_setopt(curl, CURLOPT_URL, SMTP_URL);
    curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWORD);
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM);
    recipients = curl_slist_append(recipients, TO);
    recipients = curl_slist_append(recipients, CC);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        RA_LOG(ERROR, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
    CPPUNIT_ASSERT_EQUAL(CURLE_OK, res);
}

void MailSenderTest::testMailSender() {
    MailSender sender(USERNAME, PASSWORD, SMTP_URL);
    vector<string> recipients;
    recipients.push_back(TO);
    CPPUNIT_ASSERT(sender.sendMail(recipients, "MailSenderTest::testMailSender()", ""));
}

RA_END_NAMESPACE(alarm);
