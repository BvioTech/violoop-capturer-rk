#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/videodev2.h>

#include <pthread.h>

#include "rk_comm_video.h"

#include "video.h"
#include "utils.h"

#define VIDEO_WIDTH 1920
#define VIDEO_HEIGHT 1080
#define BIT_RATE 10 * 1024
#define GOP 60

#define VENC_CHANNEL 0
// venc send frame timeout, venc get stream timeout
#define TIMEOUT 1000
// i dont know why stream output buffer count is 8
// in `test_mpi_venc.cpp`, they use 8
#define STREAM_OUTPUT_BUFFER_COUNT 8

// video device path
#define VIDEO_PATH "/dev/video0"
#define BUFFER_COUNT 4

// output path
#define OUTPUT_PATH "data/frame.h264"

// running
static volatile int keep_running = 1;

/**
 * stop running
 */
void stop_running(int)
{
    keep_running = 0;
}

int output_callback(uint32_t frame_id, uint64_t time, void *data, uint32_t size, void *user_data)
{
    FILE *f = fopen(OUTPUT_PATH, "wb");
    if (f == NULL)
    {
        perror("save data file open error");
        return -1;
    }

    fwrite(data, 1, size, f);
    fflush(f);

    fclose(f);

    return 0;
}

typedef struct
{
    int32_t venc_channel_id;
    int video_fd;
    uint32_t width;
    uint32_t height;
    uint32_t vir_width;
    uint32_t vir_height;
} input_args_t;

void *input_loop(void *arg)
{
    input_args_t *args = (input_args_t *)arg;

    while (keep_running)
    {
        int ret = input(args->venc_channel_id,
                        args->video_fd,
                        args->width,
                        args->height,
                        args->vir_width,
                        args->vir_height,
                        !keep_running,
                        TIMEOUT);
        if (ret == -1)
        {
            break;
        }
    }

    stop_running(0);
    return NULL;
}

typedef struct
{
    int32_t venc_channel_id;
    uint32_t width;
    uint32_t height;
} output_args_t;

void *output_loop(void *arg)
{
    output_args_t *args = (output_args_t *)arg;

    VENC_STREAM_S stream;
    stream.pstPack = malloc(sizeof(VENC_PACK_S));

    while (keep_running)
    {
        int ret = output(args->venc_channel_id,
                         &stream,
                         TIMEOUT,
                         output_callback,
                         NULL);
        if (ret == -1)
        {
            // error break
            break;
        }

        if (stream.pstPack->bStreamEnd)
        {
            // end break
            break;
        }
    }

    if (stream.pstPack)
    {
        free(stream.pstPack);
    }

    stop_running(0);
    return NULL;
}

int main()
{
    uint32_t video_width;
    uint32_t video_height;
    char *input_path;
    char *output_path;
    uint32_t bit_rate;
    uint32_t gop;

    // regist signal
    signal(SIGINT, stop_running);
    signal(SIGTERM, stop_running);

    // calculate size
    MB_PIC_CAL_S cal;
    // auto calculate
    // on new hardware, the v4l2 cloud stride correctly. i have no idea why
    // calculate_venc(video_width, video_height, &cal);

    // 1920 1080 will be strided to 1920 1088
    // but v4l2 dma can not fill all byes, then venc think the frame is not end and will not work at all
    // that is why we should calculate manually
    cal.u32VirWidth = video_width;
    cal.u32VirHeight = video_height;
    cal.u32MBSize = calculate_pic_byte_size(video_width, video_height, V4L2_PIX_FMT_NV12);
    printf("manual calculate ok %d %d %d\n", cal.u32VirWidth, cal.u32VirHeight, cal.u32MBSize);

    // init venc
    int ret = init_venc(VENC_CHANNEL, video_width, video_height, bit_rate, gop, STREAM_OUTPUT_BUFFER_COUNT, cal);
    if (ret == -1)
    {
        return -1;
    }

    // init v4l2
    int video_fd = init_v4l2(input_path, video_width, video_height);
    unsigned int buffer_count = init_v4l2_buffers(video_fd, BUFFER_COUNT);
    if (buffer_count == 0)
    {
        goto destroy_venc;
    }

    // init buffers
    unsigned int memory_pool = init_venc_memory(buffer_count, cal);
    if (memory_pool == MB_INVALID_POOLID)
    {
        goto destroy_v4l2;
    }

    void *blocks[BUFFER_COUNT];
    ret = allocate_buffers(memory_pool, video_fd, buffer_count, blocks);
    if (ret == -1)
    {
        goto destroy_v4l2;
    }

    // start venc
    ret = start_venc(VENC_CHANNEL);
    if (ret == -1)
    {
        goto free_buffers;
    }

    // start v4l2
    ret = start_v4l2(video_fd);
    if (ret == -1)
    {
        goto stop_venc;
    }

    // threads
    pthread_t input_thread;
    pthread_t output_thread;

    input_args_t input_args = {
        .venc_channel_id = VENC_CHANNEL,
        .video_fd = video_fd,
        .width = video_width,
        .height = video_height,
        .vir_width = cal.u32VirWidth,
        .vir_height = cal.u32VirHeight,

    };

    pthread_create(&input_thread, NULL, input_loop, &input_args);
    pthread_create(&output_thread, NULL, output_loop, NULL);

    // wait thread end
    pthread_join(input_thread, NULL);
    pthread_join(output_thread, NULL);

    // stop v4l2
    stop_v4l2(video_fd);

stop_venc:
    // stop venc
    ret = stop_venc(VENC_CHANNEL);

free_buffers:
    // free buffer
    ret = free_buffers(BUFFER_COUNT, blocks);

destroy_v4l2:
    // destroy v4l2
    destroy_v4l2(video_fd);

destroy_venc:
    // destroy venc
    destroy_venc(VENC_CHANNEL, memory_pool);

    return 0;
}
