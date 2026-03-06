#!/bin/bash
build_rv1106(){
    docker run -it --rm -v .:/app builder:luckfox-pico /app/build_rv1106.sh
}

build_rk3576(){
    docker run -it --rm -v .:/app builder:firefly /app/build_rk3576.sh
}

if [ $# -eq 0 ]; then
	echo "default build for rv1106"
    build_rv1106
elif [ $# -eq 1 ]; then
    case "$1" in
		"rv1106")
            build_rv1106
            ;;
		"rk3576")
            build_rk3576
            ;;
        *)
            echo "Unknown args"
            exit 1
    esac
else
	echo "Invalid args length"
	exit 1
fi
