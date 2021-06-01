#ifndef _EMILIA_MACROUTIL_H_
#define _EMILIA_MACROUTIL_H_

#include "emilia/util/generalutil.h"
#include "emilia/log/logmarco.h"

#include "assert.h"
namespace emilia{
namespace util{

#define EMILIA_ASSERT_LOG(x, w) \
    if(!(x)){ \
        EMILIA_LOG_ERROR("system") << "ASSERTION: " << #x \
        << "\n" << w \
        << "\nbacktrace:\n" \
        << emilia::util::BacktraceToString(100, 2, "   "); \
        assert(x); \
    }


}
}

#endif