#ifndef RA_COMMONDEFINE_H
#define RA_COMMONDEFINE_H
#include <cmath>
#include <string>
#include <map>
#include <set>   
#include <vector>
#include <limits>
#include <ra/common/Common.h>

RA_NS_BEGIN;

typedef std::set<std::string> StringSet;

static const std::string HTTP_API_CONTENT_TYPE_KEY = "Content-Type";
static const std::string HTTP_API_JSON_CONTENT_TYPE = "application/json";
static const std::string HTTP_API_FORM_CONTENT_TYPE = "application/x-www-form-urlencoded";

static const std::string HTTP_API_PATH_LIST_METRIC = "/api/list";
static const std::string HTTP_API_PATH_LOAD_CONFIG = "/api/load";
static const std::string HTTP_API_LOAD_CONFIG_PATH_KEY = "configPath";
static const std::string HTTP_API_LIST_METRIC_PATH_KEY = "path";
static const std::string HTTP_API_LIST_METRIC_PATH_SEP = ":";
static const std::string HTTP_API_LIST_START_TIME_KEY = "start";
static const std::string HTTP_API_LIST_END_TIME_KEY = "end";
static const std::string HTTP_API_LIST_LABEL_KEY = "label";
static const std::string HTTP_API_LIST_IS_LEAF_KEY = "is_leaf";

static const uint32_t HEARTBEAT_DEFAULT_INTERVAL = 15;    // seconds
static const uint32_t HEARTBEAT_DEFAULT_PORT = 18080;
static const std::string HEARTBEAT_API_PATH = "/admin/heartbeats";
static const std::string HEARTBEAT_REQ_ADDRESS_KEY = "address";
static const std::string HEARTBEAT_REQ_INTERVAL_KEY = "interval";
static const std::string HEARTBEAT_REQ_VERSION_KEY = "version";
static const std::string HEARTBEAT_RESP_ID_KEY = "id";
static const std::string HEARTBEAT_RESP_ADDRESS_KEY = "address";

static const std::string METRIC_ROOT_NODE_LABEL = "Root";
static const std::string METRIC_DATA_NODE_LABEL = "DataNode";
static const std::string METRIC_NODE_PATH_SEP = ".";

static const std::string POLICY_TYPE_TREND = "trend";
static const std::string POLICY_TYPE_THRESHOLD = "threshold";
static const std::string POLICY_TYPE_AVAILABILITY = "availability";
static const std::string POLICY_TYPE_SINGULARITY = "singularity";
static const std::string POLICY_TYPE_CYCLE = "cycle";

static const std::string CHECK_TYPE_SINGLE = "single";
static const std::string CHECK_TYPE_METRIC_TOTAL = "total";
static const std::string CHECK_TYPE_HOST_TOTAL = "host_total";

static const int64_t MICROSECONDS = 1000000LL;

static const double DEF_SMOOTHING_FACTOR = 0.45;
static const double DEF_TREND_FACTOR = 0.2;

static const int32_t DEF_CHECK_SOURCE_DATA_DURATION = 60000; // in ms
static const int32_t DEF_MIN_RETRIEVE_AMONITOR_FAIL = 10;

static const std::string DEFAULT_FILE_SYSTEM = "LocalFileSystem";
static const std::string LOCAL_CONF_ROOT = "conf";
static const std::string CONF_FILE_NAME = "sqlite";
static const int32_t INVALID_CONFIG_VERSION = -1;

static const std::string TABLE_NAME_PAIR = "pair";
static const std::string TABLE_NAME_RA = "redAlert";
static const std::string TABLE_NAME_DATA_SOURCE = "dataSource";
static const std::string TABLE_NAME_SHIELD = "shield";
static const std::string TABLE_NAME_POLICY = "policy";

static const time_t INVALID_TIME = std::numeric_limits<time_t>::max();
static const uint64_t INVALID_HASH_VALUE = 0;

static const int64_t RETRIEVE_TREE_TIME_START = 600; //600s, 10 minutes, fetch last 10 min metric tree

static const int64_t TREE_MANAGER_LOOP_INTERVAL_US = 500LL * 1000LL; //us = 0.5s
static const int64_t TREE_MANAGER_RELOAD_TREE_INTERVAL_S = 10 * 60; // 10 mins
static const int64_t APP_LOOP_INTERVAL_US = 10LL * 1000LL; //us = 10ms

static const int32_t DEFAULT_LAST_RETRIEVE_TIME_RANDOM_VALUE = 60; //60s = 1 minute

static const double INVALID_METRIC_DATA = std::numeric_limits<double>::max();

static const uint64_t MAX_TREND_HISTORY_MS = 86400 * MICROSECONDS; //keep one day trend history
static const uint64_t TREND_AUTO_EXPIRE_TIMES = 10; // use TREND_AUTO_EXPIRE_TIMES * fetchInterval

//alarm
static const std::string ALARM_LEVEL_UCMT = "ucmt";
static const std::string ALARM_LEVEL_ALIMONITOR = "alimonitor";
static const std::string ALARM_LEVEL_CURLMAIL = "curlmail";
static const int64_t ALARM_RECORD_EXPIRE_TIME_SEC = 86400; // 86400s = 1day
static const int64_t ALIMONITOR_HTTP_TIMEOUT_MS = 1000; // 1000ms = 1s
static const int64_t UCMT_HTTP_TIMEOUT_MS = 1000; // 1000ms = 1s
static const uint32_t INTERNAL_ALARM_POLICY_ID = 0;
static const std::string INTERNAL_ALARM_METRIC = "internal.amonitor";
static const int64_t INTERNAL_ALARM_MIN_ALARM_INTERVAL = 60; //60s
static const uint32_t MAX_ALARM_MESSEG_LENGTH = 500U;
static const std::string MULTI_ALARM_GROUP_SEP = ",";

//monitor
static const std::string RA_MONITOR_SERVICE_NAME = "RedAlert";
static const std::string RA_MONITOR_NODEPATH_PREFIXE = "RA-";
static const int32_t SAMPLE_SEC_INTERVAL = 1;
static const std::string TREE_MANAGER_NODE_NAME = "TreeManager";
static const std::string FETCHER_MANAGER_NODE_NAME = "FetcherManager";
static const std::string CHECKER_MANAGER_NODE_NAME = "CheckerManager";
static const std::string ALARM_MANAGER_NODE_NAME = "AlarmManager";
static const std::string SYSTEM_NODE_NAME = "System";
static const std::string RRD_FILE_SUFFIX = ".rrd";
static const std::string SUMMARY_INFO = "__SummaryInfo__";
static const std::string ACCESS_HOST = "access_host";
static const std::string BATCH_ALLOC_TREE = "batch_alloc_tree";
static const std::string RRD_ROOT = "rrd_root";

RA_NS_END;

#endif //RA_COMMONDEFINE_H
