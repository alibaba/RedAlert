#ifndef RA_SQLDATA_H
#define RA_SQLDATA_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <sqlite3.h>

RA_BEGIN_NAMESPACE(util);

class SqlData
{
public:
    SqlData(const std::string& filePath);
    ~SqlData();
private:
    SqlData(const SqlData &);
    SqlData& operator=(const SqlData &);
public:
    bool load(const std::string& tableName, const std::vector<std::string>& columns = std::vector<std::string>());
    void close();

    int32_t getRow() const { return _row; }
    int32_t getCol() const { return _col; }
    bool getRow(int32_t row, std::vector<std::string>& out) const;
    bool getValue(int32_t row, int32_t col, std::string& out) const;

    void setFilePath(const std::string& filePath) { _filePath = filePath; }
    const std::string& getFilePath() const { return _filePath; }
private:
    void printAll() const;
private:
    std::string _filePath;
    sqlite3 *_db;
    int32_t _row;
    int32_t _col;
    char **_sqlResult;
    bool _isLoad;
private:
    friend class SqlDataTest;
};

RA_TYPEDEF_PTR(SqlData);

RA_END_NAMESPACE(util);

#endif //RA_SQLDATA_H
