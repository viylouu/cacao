mkdir -p build

COMPILER=("zig" "cc")
CFLAGS="-std=c99 -Wall -Wextra -Icacao -I. -Iinclude -Icacao/include"

BUILD_TEST=false
EXAMPLE=""

[[ "$OSTYPE" == "linux-gnu" ]] && BUILD_WINDOWS=false || BUILD_WINDOWS=true

for arg in "$@"; do
    if [ "$arg" = "-t" ]; then
        BUILD_TEST=true
    elif [ "$arg" = "-w" ]; then
        BUILD_WINDOWS=true
    else
        EXAMPLE="$arg"
    fi
done

if $USE_ZIG_CC && ! zig cc --version &> /dev/null; then
    echo "zig cc not installed / not in path!"
    exit 1
fi

if $BUILD_TEST; then
    CFLAGS+=" -O0 -g"
else
    CFLAGS+=" -O2 -flto"
fi

if $BUILD_WINDOWS; then
    CFLAGS+=" -target x86_64-windows-gnu -lvulkan -lopengl32"
else
    CFLAGS+=" -lvulkan -lwayland-egl -lEGL -lGLESv2 -lwayland-client -lGL -lxcb -lrt -lxkbcommon"
fi

if [ -n "$EXAMPLE" ]; then
    SRC_DIRS=("." "examples/$EXAMPLE")
else
    SRC_DIRS=("cacao" "src")
fi

FILES=()
for dir in "${SRC_DIRS[@]}"; do
    while IFS= read -r file; do
        FILES+=("$file")
    done < <(find "$dir" -name "*.c" ! -path "*/examples/*")
done

echo -e "COMPILING: ${FILES[@]}\n"

if $BUILD_WINDOWS; then
    "${COMPILER[@]}" "${FILES[@]}" $CFLAGS -o build/out.exe
    if [[ "$OSTYPE" == "linux-gnu" ]]; then
        wine ./build/out.exe
    else
        ./build/out.exe
    fi
else
    "${COMPILER[@]}" "${FILES[@]}" $CFLAGS -o build/out.game && ./build/out.game
fi
