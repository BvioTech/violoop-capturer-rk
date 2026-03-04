#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include <sys/time.h>
#include <linux/videodev2.h>

#include "utils.h"

void test_time_to_us()
{
    printf("time_to_us test start\n");

    struct timeval tv;

    tv.tv_sec = 1;
    tv.tv_usec = 500000;
    assert(time_to_us(tv) == 1500000);

    tv.tv_sec = 0;
    tv.tv_usec = 0;
    assert(time_to_us(tv) == 0);

    tv.tv_sec = 10;
    tv.tv_usec = 0;
    assert(time_to_us(tv) == 10000000);

    tv.tv_sec = 0;
    tv.tv_usec = 999999;
    assert(time_to_us(tv) == 999999);

    printf("time_to_us test success\n");
}

void test_get_time_us()
{
    printf("test_get_time_us test start\n");

    uint64_t t1 = get_time_us();
    sleep(1); // 等待约1秒
    uint64_t t2 = get_time_us();
    // 允许 ±10ms 的误差（调度等原因）
    assert(t2 - t1 >= 1000000 - 10000);
    assert(t2 - t1 <= 1000000 + 10000);

    printf("test_get_time_us test success\n");
}

void test_calculate_pic_byte_size()
{
    printf("test_calculate_pic_byte_size test start\n");

    uint32_t width = 1920;
    uint32_t height = 1080;

    // 已知格式：NV12
    uint32_t size = calculate_pic_byte_size(width, height, V4L2_PIX_FMT_NV12);
    assert(size == width * height * 3 / 2);

    // 未知格式：应返回 (uint32_t)-1
    uint32_t unknown = 0x12345678;
    size = calculate_pic_byte_size(width, height, unknown);
    assert(size == (uint32_t)-1);

    // 边界：宽度或高度为零（NV12 仍按公式计算，结果为0）
    size = calculate_pic_byte_size(0, 1080, V4L2_PIX_FMT_NV12);
    assert(size == 0);
    size = calculate_pic_byte_size(1920, 0, V4L2_PIX_FMT_NV12);
    assert(size == 0);

    printf("test_calculate_pic_byte_size test success\n");
}

int main()
{
    printf("unit utils test start\n");

    test_time_to_us();
    test_get_time_us();
    test_calculate_pic_byte_size();

    printf("unit utils test success\n");

    return 0;
}
