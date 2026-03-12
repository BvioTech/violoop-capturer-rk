#!/bin/bash
build_rv1106(){
    docker run -it --rm -v .:/app builder:luckfox-pico /app/scripts/build_1.sh
}

build_rk3576(){
    docker run -it --rm -v .:/app builder:firefly /app/scripts/build_1.sh
}

if [ $# -eq 0 ]; then
	echo "default build for rv1106"
    build_rv1106
elif [ $# -eq 1 ]; then
    case "$1" in
		"rv1106")
	        echo "build for rv1106"
            build_rv1106
            ;;
		"rk3576")
	        echo "build for rk3576"
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
