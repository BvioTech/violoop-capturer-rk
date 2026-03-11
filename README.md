# Capture RK

This is capture project for rk.

## Doc

https://cdn.static.spotpear.cn/uploads/picture/learn/risc-v/luck-fox-pico/Document/Rockchip_Developer_Guide_MPI_FAQ.pdf

## Toolchain

The toolchain is not included in this project. See `https://github.com/BvioTech/violoop-builder`

## Build

Please build in docker.

```bash
./build.sh [rk1106|rk3576]
```

## Files

Lib files are copied from target system.

## Utils

```bash
# list device
v4l2-ctl --list-device

# list format
v4l2-ctl -d /dev/video0 --list-formats-ext 

# get format
v4l2-ctl -d /dev/video0 --get-fmt-video

# set format
v4l2-ctl -d /dev/v4l-subdev2 --set-dv-bt-timings width=1920,height=1080
v4l2-ctl -d /dev/video0 --set-fmt-video=width=1920,height=1080,pixelformat=NV12

# capture frame
v4l2-ctl -d /dev/video0 --stream-mmap --stream-to=frame.raw --stream-count=1

# convert frame.raw
ffmpeg -f rawvideo -pixel_format nv12 -video_size 1920x1080 -i frame.raw output.jpg

# `-f 0` is nv12
# `-t 7` is h264
mpi_enc_test -w 1920 -h 1080 -t 7 -i frame.raw -f 0 -o frame.h264
# `-t 8` is jpeg
mpi_enc_test -w 1920 -h 1080 -t 8 -i frame.raw -f 0 -o frame.jpeg

# convert frame.h264
ffmpeg -i frame.h264 output.jpg
```
