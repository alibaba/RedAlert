#ifndef RA_COMMON_H_
#define RA_COMMON_H_

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <assert.h>
#include <inttypes.h>
#include <ra/common/Config.h>

#define RA_NS_BEGIN namespace ra {
#define RA_NS_END }

#define RA_BEGIN_NAMESPACE(x) namespace ra { namespace x{ 

#define RA_END_NAMESPACE(x) } }

#define RA_USE_ROOT_NAMESPACE using namespace ra;

#define RA_USE_NAMESPACE(x) using namespace ra::x

#define RA_NS(x) ra::x

#define RA_ALIAS_NAMESAPCE(x, y) namespace ra { namespace x = y; }

#include <tr1/memory>
#define RA_TYPEDEF_PTR(x) typedef std::tr1::shared_ptr<x> x##Ptr

#endif /*RA_COMMON_H_*/


