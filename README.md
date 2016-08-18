
Red alert(RA) is a service monitor.  It grabs metric data from dataSource such as graphite or ganglia, to check whether current metric values are normal by some configurable rules. Alarm messages can be reported by smtp or curl.

# Introduction
RA is composed of two parts, RA web and server. RA web is used to CRUD the alarms rules, for example, to adjust the alarm threshold. RA server is used to connect to datasource(ganglia or graphite) and send alarm messages if neccessary.
Heartbeats from RA server to RA web are used to tell the current server state
RA web and server must be disposed at the same machine/filesystem(In fact, currently RA only supports local filesystem, but you can implement your filesystem interface. See the "contrib" directory).
RA configuration files are at the format of sqlite files, which to prevent concurrent modifition problems. 
If some one changes or adds alarm rules, RA web will tell RA server to reload the full copy of the configuration from a specified directory.
The communication protocol between RA web and RA server is HTTP.

## Red Alert Documentation ##

- [RAServer](RAServer/README.md)
- [RAWeb](RAWeb/README.md)
