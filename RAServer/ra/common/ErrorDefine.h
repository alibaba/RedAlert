#ifndef RA_ERRORDEFINE_H
#define RA_ERRORDEFINE_H

#include <ra/common/CommonDefine.h>

RA_NS_BEGIN;

enum ErrorCode
{
    RA_SUCC = 0,
    RA_INVALID_APP = 1,/*internal error, ptr null or something else*/
    RA_PARSE_ERROR = 2,/*config parse failed*/
    RA_DOWNLOAD_ERROR = 3,/*download config failed*/
    RA_VERSION_ERROR = 4, /*remote version is not bigger than local config version */
    RA_LOAD_CONFIG_ERROR = 5, /*ra load config failed */
};

RA_NS_END;

#endif
