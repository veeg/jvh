#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include "libv4l2.h"
#include "libv4lconvert.h"
#include "errno.h"
#include "string.h"
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>


#define NUM_THREADS 20



typedef struct thread_args thread_args_t;
typedef struct buffer buffer_t;
typedef struct queue_node queue_node_t;

struct thread_args {
    buffer_t *buffer;
    void **queue;
    int count;
    int fd;
};


struct buffer {
    void *start;
    int length;
};

struct queue_node {
    struct v4l2_buffer *buf;
    void *start;
};



buffer_t *init_buffers(int fd, int count)
{
    struct v4l2_buffer *buf;
    buffer_t *buffers;
    int i;
    void *buf_start;

    buffers = calloc(sizeof(buffer_t) * count, 1);
    buf = calloc(sizeof(struct v4l2_buffer) * count, 1);

    for (i = 0; i < count; i++)
    {
        //memset(&buf, 0, sizeof(struct v4l2_buffer));

        buf[i].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf[i].memory = V4L2_MEMORY_MMAP;
        buf[i].index = i;

        if (v4l2_ioctl(fd, VIDIOC_QUERYBUF, &buf[i]) < 0)
        {
            fprintf(stderr, "Couldn't get buffer info\n");
            return NULL;
        }
        buffers[i].length = buf[i].length;
        buf_start = v4l2_mmap(NULL, buf[i].length, PROT_READ | PROT_WRITE,
                              MAP_SHARED, fd, buf[i].m.offset);

        if (buf_start == MAP_FAILED)
        {
            printf("%s\n", strerror(errno));
            fprintf(stderr, "Failed to allocate buffer\n");
            return NULL;
        }

        buffers[i].start = buf_start;
        if (v4l2_ioctl(fd, VIDIOC_QBUF, &buf[i]) < 0)
        {
            printf("%s\n", strerror(errno));
            fprintf(stderr, "Cant queue buffer\n");
            return NULL;
        }

    }
    return buffers;
}


int init_device(int fd)
{
    struct v4l2_capability cap;
    struct v4l2_format format;
    struct v4l2_requestbuffers bufreq;
    struct v4l2_streamparm stream;
    struct v4l2_fract frac;

    if (v4l2_ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0)
    {
        fprintf(stderr, "Couldn't retrieve capabilites\n");
        return -1;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && !(cap.capabilities  & V4L2_CAP_STREAMING))
    {
        fprintf(stderr, "Device not capeable of streaming");
        return -1;
    }

    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    format.fmt.pix.width = 800;
    format.fmt.pix.height = 600;

    if (v4l2_ioctl(fd, VIDIOC_S_FMT, &format) < 0)
    {
        fprintf(stderr, "Couldn't set format\n");
        return -1;
    }
    if (cap.capabilities & V4L2_CAP_TIMEPERFRAME)
    {
        printf("Setting framerate\n");
        if (v4l2_ioctl(fd, VIDIOC_G_PARM, &stream) < 0)
        {
            printf("%s\n", strerror(errno));
            fprintf(stderr, "Couldn't retrieve stream capabilites\n");
            return -1;
        }

        frac.numerator = 120;
        frac.denominator = 2;
        stream.parm.capture.timeperframe = frac;
        if (v4l2_ioctl(fd, VIDIOC_S_PARM, &stream) < 0)
        {
            fprintf(stderr, "Cant set stream params\n");
            return -1;
        }
    }
    bufreq.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufreq.memory = V4L2_MEMORY_MMAP;
    bufreq.count = 20;

    if (v4l2_ioctl(fd, VIDIOC_REQBUFS, &bufreq) < 0)
    {
        fprintf(stderr, "Couldn't set buffer info\n");
        return -1;
    }

    return bufreq.count;
}


void *compute_frame(void *args)
{
    int i = 0;
    queue_node_t node;
    struct v4l2_buffer buf;
    thread_args_t *vals;

    buf.memory = V4L2_MEMORY_MMAP;
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    vals = (thread_args_t*)args;

    printf("THREAD COUNT : %d\n", vals->count);

    while(1)
    {
        if (v4l2_ioctl(vals->fd, VIDIOC_DQBUF, &buf) < 0)
        {
            printf("%s\n", strerror(errno));
            fprintf(stderr, "Cant dequeue buffer\n");
            return NULL;
        }

        node.buf = &buf;
        node.start = vals->buffer[buf.index].start;
        vals->queue[i] = &node;

        printf("DEQUE : %d\n", i);

        i = (i + 1) % vals->count;
    }

    return NULL;
}

void start_stream(int fd, int count)
{
    queue_node_t *current;
    FILE *f;
    int type, ret;
    int i = 0, d;
    buffer_t *buffers;
    void *address;
    void **queue;
    pthread_t thread;
    thread_args_t *args;

    args    = calloc(sizeof(thread_args_t), 1);
    queue   = calloc(sizeof(void*) * count, 1);
    buffers = init_buffers(fd, count);

    type    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (v4l2_ioctl(fd, VIDIOC_STREAMON, &type) < 0)
    {
        fprintf(stderr, "Cant start stream\n");
        return;
    }

    //current.memory = V4L2_MEMORY_MMAP;
    //current.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    printf("COUNT : %d\n", count);

    args->buffer    = buffers;
    args->count     = count;
    args->queue     = queue;
    args->fd        = fd;

    f = fopen("./test2", "w");
    ret = pthread_create(&thread, NULL, compute_frame, (void*) args);

    while(1)
    {
        if (queue[i] != NULL)
        {
            current = queue[i];
            printf("LEN : %d\n", current->buf->length);
            //printf("LEN : %d\n", current->buf->index);
            d = fwrite(current->start, 1, current->buf->length, f);
           // printf("%p\n", current->start);
            printf("Wrote : %d\n", d);
            //printf("%s\n", strerror(errno));
            queue[i] = NULL;
            if (v4l2_ioctl(fd, VIDIOC_QBUF, current->buf) < 0)
            {
                printf("%s\n", strerror(errno));
                fprintf(stderr, "Cant queue buffer\n");
                return;
            }
            printf("QUE : %d\n", i);
            i = (i + 1) % count;

        }
    }

    if (v4l2_ioctl(fd, VIDIOC_STREAMOFF, &type) < 0)
    {
        fprintf(stderr, "Cant shut down stream stream\n");
        return;
    }

}

int main(int argc, char **argv)
{
    int fd, count;

    fd = v4l2_open("/dev/video0", O_RDWR);
    if (fd < 0)
    {
        fprintf(stderr, "Cant find device\n");
        return 0;
    }

    count = init_device(fd);

    start_stream(fd, count);
}
