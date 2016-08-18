#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <stdlib.h>
#include <ra/util/Util.h>
#include <ra/common/Common.h>
#include <fnmatch.h>
#include <limits>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace std;

RA_BEGIN_NAMESPACE(util);
RA_LOG_SETUP(util, Util);

Util::Util() { 
}

Util::~Util() { 
}

bool Util::isPatternMatch(const std::string& pattern, const std::string& str)
{
    return isSimplePatternMatch(pattern.c_str(), str.c_str());
}

bool Util::isSimplePatternMatch(const char* pattern, const char* str)
{
    assert(pattern && str);
    char c;

    while (true) {
        c = *pattern++;
        switch (c) {
        case '\0':
            return *str == '\0';
        case '?':
            if (*str == '\0') {
                return false;
            }

            // simply match any character
            ++str;
            break;
        case '*':
            c = *pattern;   // next pattern char
            /* Collapse multiple stars. */
            while (c == '*') {
                c = *++pattern;
            }

            /* Optimize for pattern with * at end. */
            if (c == '\0') {
                return true;
            }

            /* General case, use recursion. */
            while (*str != '\0') {
                if (isSimplePatternMatch(pattern, str)) {
                    return true;
                }
                ++str;
            }

            return false;
        case '\\':
            // escape
            if ((c = *pattern++) == '\0') {
                c = '\\';
                --pattern;
            }
            /*FALL THROUGH*/
        default:
            if (c != *str)
                return false;
            ++str;
            break;
        }
    }
    /* NOT REACHED */
    assert(false);
    return false;
}

std::string Util::joinString(const std::vector<std::string> tokens, std::string sep)
{
    stringstream ss;
    for (size_t i = 0; i < tokens.size(); i++) {
        if (i > 0) ss << sep;
        ss << tokens[i];
    }
    return ss.str();
}

std::vector<std::string> Util::splitString(const std::string& str, const std::string& sep, bool skipEmpty) {
    std::vector<std::string> tokens;
    size_t i, j;
    for (i = 0; i < str.size(); i = j + sep.size()) {
        j = str.find(sep, i);
        if (j == std::string::npos) break;
        string token = str.substr(i, j - i);
        if (!skipEmpty || !token.empty()) tokens.push_back(token);
    }
    if (i < str.size()) tokens.push_back(str.substr(i));
    return tokens;
}

std::string Util::makeShieldKey(const std::string& group, const std::string& metricPath, uint32_t host)
{
    char hostStr[32];
    size_t hostStrLen = snprintf(hostStr, sizeof(hostStr), "%u", host);
    string ret;
    ret.reserve(group.length() + metricPath.length() + hostStrLen + 3);
    ret.append(group).append("#").append(metricPath).append("#").append(hostStr);
    return ret;
}

std::string Util::char2hex( char dec )
{
    char dig1 = (dec&0xF0)>>4;
    char dig2 = (dec&0x0F);
    if ( 0<= dig1 && dig1<= 9) dig1+=48;    //0,48 in ascii
    if (10<= dig1 && dig1<=15) dig1+=65-10; //A,65 in ascii
    if ( 0<= dig2 && dig2<= 9) dig2+=48;
    if (10<= dig2 && dig2<=15) dig2+=65-10;

    std::string r;
    r.append( &dig1, 1);
    r.append( &dig2, 1);
    return r;
}

std::string Util::urlEncode( const std::string &c )
{
    std::string escaped;
    int max = c.length();
    for(int i=0; i<max; i++) {
        if ( (48 <= c[i] && c[i] <= 57) ||//0-9
             (65 <= c[i] && c[i] <= 90) ||//ABC...XYZ
             (97 <= c[i] && c[i] <= 122) || //abc...xyz
             (c[i]=='~' || c[i]=='-' || c[i]=='_' || c[i]=='.')
             ) {
            escaped.append( &c[i], 1);
        }
        else {
            escaped.append("%");
            escaped.append( char2hex(c[i]) );//converts char 255 to string "FF"
        }
    }
    return escaped;
}

bool Util::getLocalAddress(const std::string& remoteHost, uint16_t remotePort, std::string& localHost) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0) {
        RA_LOG(WARN, "Cannot create socket, errno: %d", errno);
        close(sock);
        return false;
    }
    struct sockaddr_in remoteAddr;
    memset(&remoteAddr, 0, sizeof(remoteAddr));
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = htons(remotePort);
    int err = inet_pton(AF_INET, remoteHost.c_str(), &remoteAddr.sin_addr);
    if (err <= 0) {
        RA_LOG(WARN, "Cannot convert '%s' to network address, errno: %d", remoteHost.c_str(), errno);
        close(sock);
        return false;
    }
    err = connect(sock, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr));
    if (err < 0) {
        RA_LOG(WARN, "Cannot connect to '%s:%d', errno: %d", remoteHost.c_str(), remotePort, errno);
        close(sock);
        return false;
    }
    struct sockaddr_in localAddr;
    memset(&localAddr, 0, sizeof(localAddr));
    socklen_t addrLen = sizeof(localAddr);
    err = getsockname(sock, (struct sockaddr *)&localAddr, &addrLen);
    close(sock);
    if (err < 0) {
        RA_LOG(WARN, "Error in calling getsockname, errno: %d", errno);
        return false;
    }
    if (((struct sockaddr *)&localAddr)->sa_family != AF_INET) {
        RA_LOG(WARN, "Error: address type is not AF_INET, type: %d, len: %u", ((struct sockaddr *)&localAddr)->sa_family, addrLen);
    }
    char addrBuffer[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &localAddr.sin_addr, addrBuffer, sizeof(addrBuffer)) == NULL) {
        RA_LOG(WARN, "Cannot convert network address to dotted-decimal format, errno: %d", errno);
        return false;
    }
    localHost.assign(addrBuffer);
    return true;
}

bool Util::IPToString(uint32_t ipv4, std::string& str)
{
    char addrBuf[32] = {0};
    in_addr addr;
    addr.s_addr = ipv4;
    if (NULL == inet_ntop(AF_INET, &addr, addrBuf, sizeof(addrBuf))) {
        snprintf(addrBuf, sizeof(addrBuf), "%u", ipv4);
        str.assign(addrBuf);
        return false;
    }
    str.assign(addrBuf);
    return true;
}

std::string Util::IPToString(uint32_t ipv4)
{
    char addrBuf[32] = {0};
    in_addr addr;
    addr.s_addr = ipv4;
    if (NULL == inet_ntop(AF_INET, &addr, addrBuf, sizeof(addrBuf))) {
        snprintf(addrBuf, sizeof(addrBuf), "%u", ipv4);
    }

    return string(addrBuf);
}

bool Util::StringToIP(const std::string& ipAddr, uint32_t& ipv4)
{
    in_addr addr;
    int ret = inet_pton(AF_INET, ipAddr.c_str(), &addr);
    if (ret <= 0) {
        return false;
    }

    ipv4 = addr.s_addr;
    return true;
}

bool Util::formatTime(const string& timeStr, time_t& out)
{
    tm t;
    memset(&t, 0, sizeof(t));
    char* res = strptime(timeStr.c_str(), "%Y-%m-%d %H:%M:%S", &t);
    if (NULL == res) {
        return false;
    }
    if (*res != '\0') {
        // some string remain unprocessed
        return false;
    }
    out = mktime(&t);
    return true;
}

string Util::randomAlphabet(size_t size) {
    string str(size, '\0');
    unsigned int seed = (unsigned int)time(NULL);
    for (size_t i = 0; i < size; i++) {
        int r = rand_r(&seed);
        char ch = (r % 26) + 'a';
        str[i] = ch;
    }
    return str;
}

void Util::rtrim(string &str, char c) {
    string::size_type pos = str.find_last_not_of(c);
    if(pos != string::npos) {
        str.erase(pos + 1);
    }
}

void Util::ltrim(std::string& str, char c) {
    string::size_type pos = str.find_first_not_of(c);
    if(pos != string::npos) {
        str.erase(0, pos);
    }
}

void Util::trim(std::string& str, char c) {
    ltrim(str, c);
    rtrim(str, c);
}

int64_t Util::currentTimeInMicroseconds() {
    struct timeval tval;
    gettimeofday(&tval, NULL);
    return (tval.tv_sec * 1000000LL + tval.tv_usec);
}

int64_t Util::currentTimeInMilliseconds() {
    return currentTimeInMicroseconds() / 1000;
}

int64_t Util::currentTimeInSeconds() {
    return currentTimeInMicroseconds() / 1000000;
}

RA_END_NAMESPACE(util);

