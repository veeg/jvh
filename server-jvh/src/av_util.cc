using namespace jvh;

// Taken from ffmpeg 3.1.7
const struct fmt_map ff_fmt_conversion_table[] = {
    //ff_fmt              codec_id              v4l2_fmt
    { AV_PIX_FMT_YUV420P, AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_YUV420  },
    { AV_PIX_FMT_YUV420P, AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_YVU420  },
    { AV_PIX_FMT_YUV422P, AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_YUV422P },
    { AV_PIX_FMT_YUYV422, AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_YUYV    },
    { AV_PIX_FMT_UYVY422, AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_UYVY    },
    { AV_PIX_FMT_YUV411P, AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_YUV411P },
    { AV_PIX_FMT_YUV410P, AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_YUV410  },
    { AV_PIX_FMT_YUV410P, AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_YVU410  },
    { AV_PIX_FMT_RGB555LE,AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_RGB555  },
    { AV_PIX_FMT_RGB555BE,AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_RGB555X },
    { AV_PIX_FMT_RGB565LE,AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_RGB565  },
    { AV_PIX_FMT_RGB565BE,AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_RGB565X },
    { AV_PIX_FMT_BGR24,   AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_BGR24   },
    { AV_PIX_FMT_RGB24,   AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_RGB24   },
    { AV_PIX_FMT_BGR0,    AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_BGR32   },
    { AV_PIX_FMT_0RGB,    AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_RGB32   },
    { AV_PIX_FMT_GRAY8,   AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_GREY    },
    { AV_PIX_FMT_GRAY16LE,AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_Y16     },
    { AV_PIX_FMT_NV12,    AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_NV12    },
    { AV_PIX_FMT_NONE,    AV_CODEC_ID_MJPEG,    V4L2_PIX_FMT_MJPEG   },
    { AV_PIX_FMT_NONE,    AV_CODEC_ID_MJPEG,    V4L2_PIX_FMT_JPEG    },
    { AV_PIX_FMT_NONE,    AV_CODEC_ID_H264,     V4L2_PIX_FMT_H264    },
    { AV_PIX_FMT_NONE,    AV_CODEC_ID_MPEG4,    V4L2_PIX_FMT_MPEG4   },
    { AV_PIX_FMT_NONE,    AV_CODEC_ID_CPIA,     V4L2_PIX_FMT_CPIA1   },
    { AV_PIX_FMT_BAYER_BGGR8, AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_SBGGR8 },
    { AV_PIX_FMT_BAYER_GBRG8, AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_SGBRG8 },
    { AV_PIX_FMT_BAYER_GRBG8, AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_SGRBG8 },
    { AV_PIX_FMT_BAYER_RGGB8, AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_SRGGB8 },
    { AV_PIX_FMT_NONE,    AV_CODEC_ID_NONE,     0                    },
};

// Taken from ffmpeg 3.1.7
enum AVPixelFormat ff_fmt_v4l2ff(uint32_t v4l2_fmt, enum AVCodecID codec_id)
{
    int i;

    for (i = 0; ff_fmt_conversion_table[i].codec_id != AV_CODEC_ID_NONE; i++) {
        if (ff_fmt_conversion_table[i].v4l2_fmt == v4l2_fmt &&
            ff_fmt_conversion_table[i].codec_id == codec_id) {
            return ff_fmt_conversion_table[i].ff_fmt;
        }
    }

    return AV_PIX_FMT_NONE;
}
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
