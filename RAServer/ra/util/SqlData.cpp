#include <ra/util/SqlData.h>
#include <ra/common/Common.h>
#include <fnmatch.h>
#include <limits>
#include <arpa/inet.h>
#include <ra/util/FileUtil.h>
#include <ra/util/Util.h>

using namespace std;

RA_BEGIN_NAMESPACE(util);
RA_LOG_SETUP(util, SqlData);

SqlData::SqlData(const string& filePath)
    : _filePath(filePath)
{
    _db = NULL;
    _row = 0;
    _col = 0;
    _sqlResult = NULL;
    _isLoad = false;
}

SqlData::~SqlData() {
    close();
}

bool SqlData::load(const std::string& tableName, const vector<string>& columns)
{
    if (_isLoad)
    {
        RA_LOG(ERROR, "you have already load data, please close first");
        return false;
    }

    if (!FileUtil::localFileExist(_filePath))
    {
        RA_LOG(ERROR, "file [%s] does not exist", _filePath.c_str());
        return false;
    }
    int err = sqlite3_open(_filePath.c_str(), &_db);
    if (err != SQLITE_OK)
    {
        RA_LOG(ERROR, "failed to open sqlite file [%s], err: %d", _filePath.c_str(), err);
        close();
        return false;
    }
    RA_LOG(INFO, "open sqlite file [%s] success", _filePath.c_str());

    string columnExpr = "*";
    if (!columns.empty()) columnExpr = Util::joinString(columns, ",");
    string cmd = "SELECT " + columnExpr + " FROM " + tableName;
    err = sqlite3_get_table(_db, cmd.c_str(), &_sqlResult, &_row, &_col, NULL);
    if (err != SQLITE_OK) {
        RA_LOG(INFO, "Error in query database, err: %d", err);
        close();
        return false;
    }
    RA_LOG(INFO, "read table [%s], row [%d], col [%d]: '%s'",
           tableName.c_str(), _row, _col, columnExpr.c_str());
    _isLoad = true;
    return true;
}

void SqlData::close()
{
    if (_sqlResult) {
        sqlite3_free_table(_sqlResult);
    }
    if (_db) {
        sqlite3_close(_db);
    }
    _row = 0;
    _col = 0;
    _db = NULL;
    _sqlResult = NULL;
    _isLoad = false;
}

bool SqlData::getRow(int32_t row, vector<string>& out) const
{
    out.clear();
    if (!_isLoad)
    {
        RA_LOG(ERROR, "sqlite has not loaded");
        return false;
    }
    if (row >= _row)
    {
        RA_LOG(ERROR, "max row is [%d]", _row);
        return false;
    }
    for (int32_t col = 0; col < _col; ++col)
    {
        out.push_back(string(_sqlResult[(row + 1) * _col + col]));
    }
    return true;
}

bool SqlData::getValue(int32_t row, int32_t col, string& out) const
{
    out = string("");
    if (!_isLoad)
    {
        RA_LOG(ERROR, "sqlite has not loaded");
        return false;
    }
    if (row >= _row || col >= _col)
    {
        RA_LOG(ERROR, "max row is [%d], max col is [%d]", _row, _col);
        return false;
    }
    out = string(_sqlResult[(row + 1) * _col + col]);
    return true;
}

void SqlData::printAll() const
{
    for (int32_t i = 0; i < (_row + 1) * _col; ++i)
    {
        cout << string(_sqlResult[i]) << endl;
    }
}

RA_END_NAMESPACE(util);
