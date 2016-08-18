- [Red Alert Configuration](#red-alert-configuration)
- [Logging Configuration](#logging-configuration)

## Red Alert Configuration ##

Here are the options of Red Alert configuration:

- `"alarmThreadNum"`: number of threads sending alarm messages.
- `"checkerThreadNum"`: number of threads checking if an alarm is trigged based user-defined policy.
- `"fetcherThreadNum"`: number of threads fetching metric data from supported monitoring services.
- `"alarmQueueSize"`, `"checkerQueueSize"` and `"fetcherQueueSize"`: queue size of the corresponding thread pool.
- `"retrieveMetricsPointCount"`: number of data points retrieved for each mointored metric path in a fetch request.
- `"maxRandomLastRetriveTimeValueSec"`: the maximum margin allowed to randomize metric data fetching interval.
- `"reloadTreeIntervalSec"`: time interval of fetching metric trees from supported monitoring services (seconds).
- `"mailUser"`:  username of smtp server.
- `"mailPassword"`: password of smtp server.
- `"mailServerUrl"`: smtp server address, e.g. "smtps://smtp.example.com"
- `"heartbeatHost"`: IP address of Red Alert front-end web server.
- `"heartbeatPort"`: port of Red Alert front-end web server.

The following options are for internal users. External users can safely ignore them.

- `"aliMonitorUrl"`
- `"amonitorAgentPort"`
- `"internalAlarmGroup"`
- `"internalAlarmLevel"`
- `"interalMinAlarmInterval"`
- `"sendOkCodeIntervalSec"` 

A reference Red Alert configuration is given at [`bootstrap.json`](conf/bootstrap.json).

## Logging Configuration ##

Red Alert adopts [`log4cplus`](https://github.com/log4cplus/log4cplus/) as the logging library.
The alarm logs are put in a file called `alarm.log`. All the other logs are put in `ra.log`.
The following is a sample logging configuration:

```ini
log4cplus.rootLogger=INFO, raAppender
log4cplus.logger.ra.common.AlarmLog=INFO, AlarmAppender
log4cplus.additivity.ra.common.AlarmLog=false

log4cplus.appender.raAppender=log4cplus::RollingFileAppender
log4cplus.appender.raAppender.File=ra.log
log4cplus.appender.raAppender.layout=log4cplus::PatternLayout
log4cplus.appender.raAppender.layout.ConversionPattern=[%D{%Y-%m-%d %H:%M:%Q}] [%p] [%T,%F -- %M:%L] %m%n

log4cplus.appender.AlarmAppender=log4cplus::RollingFileAppender
log4cplus.appender.AlarmAppender.File=alarm.log
log4cplus.appender.AlarmAppender.layout=log4cplus::PatternLayout
log4cplus.appender.AlarmAppender.layout.ConversionPattern=[%D{%Y-%m-%d %H:%M:%Q}] [%p] [%T,%F -- %M:%L] %m%n
```

[`ra_log.conf`](/conf/ra_log.conf) is a reference logging configuration for production.

The specification of log format can be found at
[here](http://log4cplus.sourceforge.net/docs/html/classlog4cplus_1_1PatternLayout.html#details).
The options for `RollingFileAppender` can be found at
[here](http://log4cplus.sourceforge.net/docs/html/classlog4cplus_1_1RollingFileAppender.html#details).
