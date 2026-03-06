#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include "rk_common.h"
#include "rk_mpi_cal.h"
#include "rk_mpi_mb.h"
#include "rk_mpi_mmz.h"
#include "rk_mpi_sys.h"
#include "rk_mpi_venc.h"
#include "rk_type.h"

#define VIDEO_WIDTH 1920
#define VIDEO_HEIGHT 1080
#define BIT_RATE 10 * 1024
#define GOP 60

#define INPUT_PATH = "images/frame.raw"
#define OUTPUT_PATH = "images/frame.h264"

#define VENC_CHANNEL 0
#define TIMEOUT 1000
#define STREAM_OUTPUT_BUFFER_COUNT 8

int init_venc(int32_t channel_id, uint32_t width, uint32_t height, uint32_t bit_rate, uint32_t gop, uint32_t buffer_count, MB_PIC_CAL_S mb_pic_cal)
{
    int32_t ret = RK_MPI_SYS_Init();
    if (ret != RK_SUCCESS)
    {
        errno = ret;
        perror("mpi init error");
        return -1;
    }
    printf("mpi init ok\n");

    VENC_CHN_ATTR_S venc_attr;
    memset(&venc_attr, 0, sizeof(venc_attr));

    // set h264
    venc_attr.stVencAttr.enType = RK_VIDEO_ID_AVC; // 8 h264 9 mjpeg 12 h265 15 jpeg
    venc_attr.stVencAttr.enPixelFormat = RK_FMT_YUV420SP;
    venc_attr.stVencAttr.u32Profile = H264E_PROFILE_MAIN;
    venc_attr.stVencAttr.u32PicWidth = width;
    venc_attr.stVencAttr.u32PicHeight = height;
    venc_attr.stVencAttr.u32VirWidth = mb_pic_cal.u32VirWidth;
    venc_attr.stVencAttr.u32VirHeight = mb_pic_cal.u32VirHeight;
    venc_attr.stVencAttr.u32StreamBufCnt = buffer_count;
    venc_attr.stVencAttr.u32BufSize = mb_pic_cal.u32MBSize;

    // set h264 struct props
    venc_attr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
    venc_attr.stRcAttr.stH264Cbr.u32BitRate = bit_rate;
    venc_attr.stRcAttr.stH264Cbr.u32Gop = gop;

    ret = RK_MPI_VENC_CreateChn(channel_id, &venc_attr);
    if (ret != RK_SUCCESS)
    {
        errno = ret;
        perror("mpi venc create channel error");
        RK_MPI_SYS_Exit();
        return -1;
    }
    printf("mpi venc %d create channel ok\n", channel_id);

    return 0;
}

uint32_t init_venc_memory(uint32_t buffer_count, MB_PIC_CAL_S mb_pic_cal)
{
    // init memory pool
    MB_POOL_CONFIG_S memory_pool_config;
    memset(&memory_pool_config, 0, sizeof(MB_POOL_CONFIG_S));
    memory_pool_config.u64MBSize = mb_pic_cal.u32MBSize;
    memory_pool_config.u32MBCnt = buffer_count;
    memory_pool_config.enAllocType = MB_ALLOC_TYPE_DMA;
    memory_pool_config.bPreAlloc = RK_TRUE;

    uint32_t memory_pool_id = RK_MPI_MB_CreatePool(&memory_pool_config);
    if (memory_pool_id == MB_INVALID_POOLID)
    {
        errno = -1;
        perror("mpi memory pool create error");
        return MB_INVALID_POOLID;
    }
    printf("mpi memory pool %d %d create ok\n", memory_pool_id, buffer_count);

    return memory_pool_id;
}

void destroy_venc(int32_t channel_id, uint32_t memory_pool_id)
{
    int32_t ret = RK_SUCCESS;

    // avoid invalid pool
    if (memory_pool_id != MB_INVALID_POOLID)
    {
        ret = RK_MPI_MB_DestroyPool(memory_pool_id);
        if (ret != RK_SUCCESS)
        {
            errno = ret;
            perror("mpi memory pool destory error");
        }
        printf("mpi memory pool %d destory ok\n", memory_pool_id);
    }

    ret = RK_MPI_VENC_DestroyChn(channel_id);
    if (ret != RK_SUCCESS)
    {
        errno = ret;
        perror("mpi venc destory error");
    }
    printf("mpi venc %d destory ok\n", channel_id);

    ret = RK_MPI_SYS_Exit();
    if (ret != RK_SUCCESS)
    {
        errno = ret;
        perror("mpi exit error");
    }
    printf("mpi exit ok\n");
}

int allocate_buffers(uint32_t memory_pool_id, unsigned int buffer_count, void *blocks[])
{
    for (unsigned int i = 0; i < buffer_count; i += 1)
    {
        // get mpi block
        blocks[i] = RK_MPI_MB_GetMB(memory_pool_id, 1, RK_TRUE);
        if (blocks[i] == RK_NULL)
        {
            errno = -1;
            perror("mpi memory get block error");
            return -1;
        }
        printf("mpi memory get block %d ok\n", i);

        // get block fd
        int32_t block_fd = RK_MPI_MB_Handle2Fd(blocks[i]);
        if (block_fd == -1)
        {
            errno = -1;
            perror("mpi memory get block fd error");
            return -1;
        }
        printf("mpi memory get block %d ok\n", i);
    }

    return 0;
}

int free_buffers(unsigned int buffer_count, void *blocks[])
{
    for (unsigned int i = 0; i < buffer_count; i += 1)
    {
        int32_t ret = RK_MPI_MB_ReleaseMB(blocks[i]);
        if (ret != RK_SUCCESS)
        {
            errno = ret;
            perror("mpi memory release block error");
            continue;
        }
        printf("mpi memory release block ok\n");
    }

    return 0;
}

int main()
{
    // calculate size
    MB_PIC_CAL_S cal;
    // auto calculate
    // on new hardware, the v4l2 cloud stride correctly. i have no idea why
    // calculate_venc(video_width, video_height, &cal);

    // 1920 1080 will be strided to 1920 1088
    // but v4l2 dma can not fill all byes, then venc think the frame is not end and will not work at all
    // that is why we should calculate manually
    cal.u32VirWidth = VIDEO_WIDTH;
    cal.u32VirHeight = VIDEO_HEIGHT;
    // nv12
    cal.u32MBSize = VIDEO_WIDTH * VIDEO_HEIGHT * 3 / 2;
    printf("manual calculate ok %d %d %d\n", cal.u32VirWidth, cal.u32VirHeight, cal.u32MBSize);

    // init venc
    int ret = init_venc(VENC_CHANNEL, VIDEO_WIDTH, VIDEO_HEIGHT, BIT_RATE, GOP, STREAM_OUTPUT_BUFFER_COUNT, cal);
    if (ret == -1)
    {
        return -1;
    }
    unsigned int memory_pool = MB_INVALID_POOLID;

    // init buffers
    memory_pool = init_venc_memory(STREAM_OUTPUT_BUFFER_COUNT, cal);
    if (memory_pool == MB_INVALID_POOLID)
    {
        goto destroy_venc;
    }

    void *blocks[STREAM_OUTPUT_BUFFER_COUNT];
    ret = allocate_buffers(memory_pool, STREAM_OUTPUT_BUFFER_COUNT, blocks);
    if (ret == -1)
    {
        goto destroy_venc;
    }

free_buffers:
    // free buffer
    ret = free_buffers(STREAM_OUTPUT_BUFFER_COUNT, blocks);

destroy_venc:
    // destroy venc
    destroy_venc(VENC_CHANNEL, memory_pool);

    return 0;
}
