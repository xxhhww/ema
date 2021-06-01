#include "codeutil.h"
#include <openssl/sha.h>

namespace emilia{
namespace util{

std::string SHA1(const std::string& data)
{
    SHA_CTX shactx;
    SHA1_Init(&shactx);
    SHA1_Update(&shactx, data.c_str(), sizeof(data));

    char result[SHA_DIGEST_LENGTH];

    SHA1_Final((unsigned char*)result, &shactx);

    return result;
}

std::string base64encode(const std::string& data)
{
    return base64encode(data.c_str(), data.length());
}

std::string base64encode(const void* data, size_t len)
{
    const char* base64 =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string ret;
    ret.reserve(len * 4 / 3 + 2);

    const unsigned char* ptr = (const unsigned char*)data;
    const unsigned char* end = ptr + len;

    while(ptr < end) {
        unsigned int packed = 0;
        int i = 0;
        int padding = 0;
        for(; i < 3 && ptr < end; ++i, ++ptr) {
            packed = (packed << 8) | *ptr;
        }
        if(i == 2) {
            padding = 1;
        } else if (i == 1) {
            padding = 2;
        }
        for(; i < 3; ++i) {
            packed <<= 8;
        }

        ret.append(1, base64[packed >> 18]);
        ret.append(1, base64[(packed >> 12) & 0x3f]);
        if(padding != 2) {
            ret.append(1, base64[(packed >> 6) & 0x3f]);
        }
        if(padding == 0) {
            ret.append(1, base64[packed & 0x3f]);
        }
        ret.append(padding, '=');
    }

    return ret;

}

}
}