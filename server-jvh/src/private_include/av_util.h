#ifndef JVH_AV_UTIL_H
#define JVH_AV_UTIL_H

extern "C" {
    #include <libavutil/avutil.h>
    #include <libavcodec/avcodec.h>
    #include <linux/videodev2.h>
}

namespace jvh
{
        struct fmt_map {
            enum AVPixelFormat ff_fmt;
            enum AVCodecID codec_id;
            uint32_t v4l2_fmt;
        };

        std::string av_error_desc (int errnum);

        enum AVPixelFormat ff_fmt_v4l2ff(uint32_t v4l2_fmt, enum AVCodecID codec_id);
}

#endif
