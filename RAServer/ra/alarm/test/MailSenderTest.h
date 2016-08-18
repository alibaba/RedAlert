#ifndef RA_MAIL_SENDER_TEST_H
#define RA_MAIL_SENDER_TEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(alarm);

class MailSenderTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(MailSenderTest);
    CPPUNIT_TEST(testCurl);
    CPPUNIT_TEST(testMailSender);
    CPPUNIT_TEST_SUITE_END();
public:
    MailSenderTest();
    ~MailSenderTest();
public:
    void setUp();
    void tearDown();
    void testCurl();
    void testMailSender();
private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(alarm);

#endif //RA_MAIL_SENDER_TEST_H
