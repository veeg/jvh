#ifndef JVH_AV_UTIL_H
#define JVH_AV_UTIL_H

extern "C" {
    #include <libavutil/avutil.h>
}

namespace jvh
{
    std::string av_error_desc (int errnum)
    {
        char buf[500];

        auto ret = av_strerror (errnum, buf, 500);

        if (ret < 0)
        {
            return std::string ("Unknown error");
        }

        return std::string (buf);
    }
}

#endif
