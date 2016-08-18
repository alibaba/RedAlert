#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <ra/test/test.h>
#include <ra/common/Log.h>
#include <string>

using namespace std;
using namespace CppUnit;

int main( int argc, char **argv)
{
    RA_LOG_CONFIG(DOTEST_LOGGER_CONF);
    bool ok = false;
    {
        TextUi::TestRunner runner;
        runner.setOutputter(new CompilerOutputter(&runner.result(), std::cerr));
        TestFactoryRegistry &registry = TestFactoryRegistry::getRegistry();
        runner.addTest( registry.makeTest() );
        ok = runner.run("", false);
    }
    return ok ? 0 : 1;
}
