#include <ra/util/SqlData.h>
#include <ra/common/Common.h>
#include <fnmatch.h>
#include <limits>
#include <arpa/inet.h>
#include <ra/util/FileUtil.h>
#include <ra/util/Util.h>

using namespace std;

RA_BEGIN_NAMESPACE(util);

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
        LOG(ERROR) << "you have already load data, please close first";
        return false;
    }

    if (!FileUtil::localFileExist(_filePath))
    {
        LOG(ERROR) << "file [" << _filePath << "] does not exist";
        return false;
    }
    int err = sqlite3_open(_filePath.c_str(), &_db);
    if (err != SQLITE_OK)
    {
        LOG(ERROR) << "failed to open sqlite file [" << _filePath << "], err: " << err;
        close();
        return false;
    }
    LOG(INFO) << "open sqlite file [" << _filePath << "] success";

    string columnExpr = "*";
    if (!columns.empty()) columnExpr = Util::joinString(columns, ",");
    string cmd = "SELECT " + columnExpr + " FROM " + tableName;
    err = sqlite3_get_table(_db, cmd.c_str(), &_sqlResult, &_row, &_col, NULL);
    if (err != SQLITE_OK) {
        LOG(INFO) << "Error in query database, err: " << err;
        close();
        return false;
    }
    LOG(INFO) << "read table [" << tableName << "], row [" 
	      << _row << "], col [" << _col << "]: " << columnExpr;
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
        LOG(ERROR) << "sqlite has not loaded";
        return false;
    }
    if (row >= _row)
    {
        LOG(ERROR) << "max row is [" << _row <<"]";
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
        LOG(ERROR) << "sqlite has not loaded";
        return false;
    }
    if (row >= _row || col >= _col)
    {
        LOG(ERROR) << "max row is [" << _row << "], max col is [" << _col << "]";
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
