#ifndef RA_UTIL_H
#define RA_UTIL_H

#include <cmath>
#include <sstream>
#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <cmath>

RA_BEGIN_NAMESPACE(util);

class Util
{
public:
    Util();
    ~Util();

private:
    Util(const Util &);
    Util& operator=(const Util &);

public:
    static bool formatTime(const std::string& timeStr, time_t& out);

    // return if l < r
    static bool isPatternMatch(const std::string& pattern, const std::string& str);
    static std::string joinString(const std::vector<std::string> tokens, std::string sep);
    static std::vector<std::string> splitString(const std::string& str, const std::string& sep, bool skipEmpty = true);
    static std::string makeShieldKey(const std::string& group, const std::string& metricPath, uint32_t host);

    static std::string char2hex( char dec );
    static std::string urlEncode( const std::string &c );

    static bool getLocalAddress(const std::string& remoteHost, uint16_t remotePort, std::string& localHost);
    // convert network byte order ipv4 to string
    static bool IPToString(uint32_t ipv4, std::string& str);
    static std::string IPToString(uint32_t ipv4);
    // convert string ipv4 addr to network byte order uint32
    static bool StringToIP(const std::string& ipAddr, uint32_t& ipv4);
    // is valid metric data
    static bool doubleEqual(double l, double r)
    {
        return fabs(l - r) < 0.0001;
    }

    static std::string randomAlphabet(size_t size);
    static void rtrim(std::string &str, char c);
    static void ltrim(std::string &str, char c);
    static void trim(std::string &str, char c);

    static int64_t currentTimeInSeconds();
    static int64_t currentTimeInMilliseconds();
    static int64_t currentTimeInMicroseconds();

    template <typename T>
    static bool fromString(const std::string& str, T& val);
    template <typename T>
    static bool toString(const T& val, std::string& str);
    template <typename T>
    static std::string toString(const T& val);

private:
    static bool isSimplePatternMatch(const char* pattern, const char* str);

private:
    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(Util);

template<typename T>
inline bool Util::fromString(const std::string& str, T& val) {
    std::stringstream ss(str);
    return ss >> val;
}

template<typename T>
inline bool Util::toString(const T& val, std::string& str) {
    std::stringstream ss;
    bool ok = ss << val;
    if (ok) str = ss.str();
    return ok;
}

template <typename T>
inline std::string toString(const T& val) {
    std::stringstream ss;
    ss << val;
    return ss.str();
}

RA_END_NAMESPACE(util);

#endif //RA_UTIL_H
