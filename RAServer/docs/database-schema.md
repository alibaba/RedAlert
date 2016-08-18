- [Configuration Database Schema](#configuration-database-schema)
    - [Schema Updates](#schema-updates)

## Configuration Database Schema ##

```sql
CREATE TABLE policy (
    a. id integer primary key autoincrement,
    b. groupName text not null,
    c. metric text not null,
    d. type text not null,
    e. validTime text not null,
    f. content text  not null,
    g. author text not null,
    h. modifiedTime text not null);

CREATE TABLE shield (
    a. id integer primary key autoincrement,
    b. groupName text not null,
    c. metric text not null,
    d. host text not null,
    e. endTime text not null,
    f. author text not null,
    g. modifiedTime text not null);

CREATE TABLE dataSource (
    a. id integer primary key autoincrement,
    b. address text not null,
    c. service text not null,
    d. fetcherType text default "DummyMetricFetcher",
    e. optionString text default "",
    f. author text not null,
    g. modifiedTime text not null);

CREATE TABLE redAlert (
    a. id integer primary key autoincrement,
    b. address text not null,
    c. service text not null,
    d. weight integer not null,
    e. author text not null,
    f. modifiedTime text not null);

CREATE TABLE pair (
    a. id integer primary key autoincrement,
    b. key text not null,
    c. value text not null,
    d. author text not null,
    e. modifiedTime text not null);
```

### Schema Updates ###

Add columns `fetcherType` and `optionString` in table `amonitor` and rename table `amonitor` to `dataSource`:

```sql
create table dataSource (
    id integer primary key autoincrement,
    address text not null,
    service text not null,
    fetcherType text default "DummyMetricFetcher",
    optionString text default "",
    author text not null,
    modifiedTime text not null);

insert into dataSource (id, address, service, "AmonMetricFetcher", "", author, modifiedTime)
    select id, address, service, author, modifiedTime from amonitor;

drop table amonitor;
```
