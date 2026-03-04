#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"

/* 重置 getopt 全局状态，避免测试用例间干扰 */
static void reset_getopt()
{
    optind = 1; // 下一个待处理参数索引
    opterr = 0; // 禁止 getopt 打印错误信息
#ifdef __GLIBC__
    // GNU 扩展，重置整个状态（可选）
    optopt = 0;
#endif
}

/* 测试必须参数齐全的正常情况（短选项） */
void test_required_args_short()
{
    printf("test_required_args_short test start\n");

    reset_getopt();
    args_t args;
    char *argv[] = {
        "program",
        "-w",
        "1920",
        "-h",
        "1080",
        "-i",
        "/dev/video0",
        "-o",
        "/tmp/sock",
        NULL,
    };
    int argc = 9;

    int ret = parse_args(argc, argv, &args);
    assert(ret == 0);
    assert(args.width == 1920);
    assert(args.height == 1080);
    assert(strcmp(args.input_path, "/dev/video0") == 0);
    assert(strcmp(args.output_path, "/tmp/sock") == 0);
    assert(args.bit_rate == 10 * 1024); // 默认值
    assert(args.gop == 60);             // 默认值
    assert(args.help_flag == 0);
    assert(args.version_flag == 0);
    destroy_args(&args);

    printf("test_required_args_short test success\n");
}

/* 测试长选项 */
void test_long_options()
{
    printf("test_long_options test start\n");

    reset_getopt();
    args_t args;
    char *argv[] = {
        "program",
        "--width",
        "1280",
        "--height",
        "720",
        "--input",
        "/dev/video1",
        "--output",
        "/var/run/out.sock",
        "--bit-rate",
        "5000",
        "--gop",
        "30",
        NULL,
    };
    int argc = 13;

    int ret = parse_args(argc, argv, &args);
    assert(ret == 0);
    assert(args.width == 1280);
    assert(args.height == 720);
    assert(strcmp(args.input_path, "/dev/video1") == 0);
    assert(strcmp(args.output_path, "/var/run/out.sock") == 0);
    assert(args.bit_rate == 5000);
    assert(args.gop == 30);
    destroy_args(&args);

    printf("test_long_options test success\n");
}

/* 测试默认值：不提供 -b 和 -g */
void test_default_values()
{
    printf("test_default_values test start\n");

    reset_getopt();
    args_t args;
    char *argv[] = {
        "program",
        "-w",
        "800",
        "-h",
        "600",
        "-i",
        "/dev/video0",
        "-o",
        "/tmp/out",
        NULL,
    };
    int argc = 9;

    int ret = parse_args(argc, argv, &args);
    assert(ret == 0);
    assert(args.bit_rate == 10 * 1024); // DEFAULT_BIT_RATE
    assert(args.gop == 60);             // DEFAULT_GOP
    destroy_args(&args);

    printf("test_default_values test success\n");
}

/* 测试缺少必需参数：缺少 -w */
void test_missing_width()
{
    printf("test_missing_width test start\n");

    reset_getopt();
    args_t args;
    char *argv[] = {
        "program",
        "-h",
        "1080",
        "-i",
        "/dev/video0",
        "-o",
        "/tmp/out",
        NULL,
    };
    int argc = 7;

    int ret = parse_args(argc, argv, &args);
    assert(ret == -1);
    destroy_args(&args); // 安全清理（指针均为 NULL）

    printf("test_missing_width test success\n");
}

/* 缺少 -h */
void test_missing_height()
{
    printf("test_missing_height test start\n");

    reset_getopt();
    args_t args;
    char *argv[] = {
        "program",
        "-w",
        "1920",
        "-i",
        "/dev/video0",
        "-o",
        "/tmp/out",
        NULL,
    };
    int argc = 7;

    int ret = parse_args(argc, argv, &args);
    assert(ret == -1);
    destroy_args(&args);

    printf("test_missing_height test success\n");
}

/* 缺少 -i */
void test_missing_input()
{
    printf("test_missing_input test start\n");

    reset_getopt();
    args_t args;
    char *argv[] = {
        "program",
        "-w",
        "1920",
        "-h",
        "1080",
        "-o",
        "/tmp/out",
        NULL,
    };
    int argc = 7;

    int ret = parse_args(argc, argv, &args);
    assert(ret == -1);
    destroy_args(&args);

    printf("test_missing_input test success\n");
}

/* 缺少 -o */
void test_missing_output()
{
    printf("test_missing_output test start\n");

    reset_getopt();
    args_t args;
    char *argv[] = {
        "program",
        "-w",
        "1920",
        "-h",
        "1080",
        "-i",
        "/dev/video0",
        NULL,
    };
    int argc = 7;

    int ret = parse_args(argc, argv, &args);
    assert(ret == -1);
    destroy_args(&args);

    printf("test_missing_output test success\n");
}

/* 测试宽度超出范围：0 或 >8192 */
void test_width_out_of_range()
{
    printf("test_width_out_of_range test start\n");

    reset_getopt();
    args_t args;
    char *argv[] = {
        "program",
        "-w",
        // width = 0
        "0",
        "-h",
        "1080",
        "-i",
        "/dev/video0",
        "-o",
        "/tmp/out",
        NULL,
    };
    int argc = 9;

    int ret = parse_args(argc, argv, &args);
    assert(ret == -1);
    destroy_args(&args);

    reset_getopt();
    char *argv1[] = {
        "program",
        "-w",
        // width = 8193
        "8193",
        "-h",
        "1080",
        "-i",
        "/dev/video0",
        "-o",
        "/tmp/out",
        NULL,
    };
    int argc1 = 9;

    ret = parse_args(argc1, argv1, &args);
    assert(ret == -1);
    destroy_args(&args);

    printf("test_width_out_of_range test success\n");
}

/* 高度超出范围 */
void test_height_out_of_range()
{
    printf("test_height_out_of_range test start\n");

    reset_getopt();
    args_t args;
    char *argv[] = {
        "program",
        "-w",
        "1920",
        "-h",
        // height = 0
        "0",
        "-i",
        "/dev/video0",
        "-o",
        "/tmp/out",
        NULL,
    };
    int argc = 9;

    int ret = parse_args(argc, argv, &args);
    assert(ret == -1);
    destroy_args(&args);

    reset_getopt();
    char *argv1[] = {
        "program",
        "-w",
        "1920",
        "-h",
        // height = 8193
        "8193",
        "-i",
        "/dev/video0",
        "-o",
        "/tmp/out",
        NULL,
    };
    int argc1 = 9;

    ret = parse_args(argc1, argv1, &args);
    assert(ret == -1);
    destroy_args(&args);

    printf("test_height_out_of_range test success\n");
}

/* 测试比特率为 0 */
void test_bit_rate_zero()
{
    printf("test_bit_rate_zero test start\n");

    reset_getopt();
    args_t args;
    char *argv[] = {
        "program",
        "-w",
        "1920",
        "-h",
        "1080",
        "-i",
        "/dev/video0",
        "-o",
        "/tmp/out",
        "-b",
        // bit_rate = 0
        "0",
        NULL,
    };
    int argc = 11;

    int ret = parse_args(argc, argv, &args);
    assert(ret == -1);
    destroy_args(&args);

    printf("test_bit_rate_zero test success\n");
}

/* 测试 gop 为 0 */
void test_gop_zero()
{
    printf("test_gop_zero test start\n");

    reset_getopt();
    args_t args;
    char *argv[] = {
        "program",
        "-w",
        "1920",
        "-h",
        "1080",
        "-i",
        "/dev/video0",
        "-o",
        "/tmp/out",
        "-g",
        // gop = 0
        "0",
        NULL,
    };
    int argc = 11;

    int ret = parse_args(argc, argv, &args);
    assert(ret == -1);
    destroy_args(&args);

    printf("test_gop_zero test success\n");
}

/* 测试 --help 选项 */
void test_help_option()
{
    printf("test_help_option test start\n");

    reset_getopt();
    args_t args;
    char *argv[] = {
        "program",
        "--help",
        NULL,
    };
    int argc = 2;

    int ret = parse_args(argc, argv, &args);
    assert(ret == 0);
    assert(args.help_flag == 1);
    assert(args.version_flag == 0);
    // 其他字段应为默认值（0 或默认参数），但 help 下不验证
    destroy_args(&args);

    printf("test_help_option test success\n");
}

/* 测试 -v / --version 选项 */
void test_version_option()
{
    printf("test_version_option test start\n");

    reset_getopt();
    args_t args;
    char *argv[] = {
        "program",
        "--version",
        NULL,
    };
    int argc = 2;

    int ret = parse_args(argc, argv, &args);
    assert(ret == 0);
    assert(args.version_flag == 1);
    assert(args.help_flag == 0);
    destroy_args(&args);

    printf("test_version_option test success\n");
}

/* 测试无效选项（未知短选项） */
void test_invalid_option()
{
    printf("test_invalid_option test start\n");

    reset_getopt();
    args_t args;
    char *argv[] = {
        "program",
        "-x",
        "-w",
        "1920",
        "-h",
        "1080",
        "-i",
        "/dev/video0",
        "-o",
        "/tmp/out",
        NULL,
    };
    int argc = 10;

    int ret = parse_args(argc, argv, &args);
    assert(ret == -1);
    destroy_args(&args);

    printf("test_invalid_option test success\n");
}

/* 测试额外位置参数（非选项参数）——应被忽略，不影响解析 */
void test_extra_arguments()
{
    printf("test_extra_arguments test start\n");

    reset_getopt();
    args_t args;
    char *argv[] = {
        "program",
        "-w",
        "1920",
        "-h",
        "1080",
        "-i",
        "/dev/video0",
        "-o",
        "/tmp/out",
        "extra1",
        "extra2",
        NULL,
    };
    int argc = 11;

    int ret = parse_args(argc, argv, &args);
    assert(ret == 0);
    assert(args.width == 1920);
    assert(args.height == 1080);
    destroy_args(&args);

    printf("test_extra_arguments test success\n");
}

/* 主函数：运行所有测试 */
int main()
{
    printf("unit utils args start\n");

    test_required_args_short();
    test_long_options();
    test_default_values();
    test_missing_width();
    test_missing_height();
    test_missing_input();
    test_missing_output();
    test_width_out_of_range();
    test_height_out_of_range();
    test_bit_rate_zero();
    test_gop_zero();
    test_help_option();
    test_version_option();
    test_invalid_option();
    test_extra_arguments();

    printf("unit utils args success\n");

    return 0;
}
