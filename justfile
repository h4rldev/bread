set shell := ["bash", "-c"]
set quiet := true

## Metadata

lib_name := "bread"
test_name := "bread-test"

## Directories

src := 'src'
include := 'include'
out := 'out'
lib := 'lib'
bin := 'bin'

## Type dirs

test_out := out + '/test'
bread_out := out + '/bread'

## General flags

include_flags := '-I' + include
link_flags := '-lhtils'

## Shared flags

shared_flags_debug := '-ggdb -g -Og -fsanitize=address,undefined,leak -fno-sanitize-recover=all -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fno-common -std=gnu11 -DBREAD_DEBUG'
shared_flags_release := '-O2 -std=gnu11'

## Link flags

release_link_flags := shared_flags_release + ' ' + link_flags
debug_link_flags := shared_flags_debug + ' ' + link_flags

## Static link flags

release_static_link_flags := shared_flags_release + ' -static -fPIC'
debug_static_link_flags := shared_flags_debug + ' -static -fPIC'

## Compile flags

release_compile_flags := shared_flags_release
debug_compile_flags := shared_flags_debug + ' -Wall -Wextra -Wpedantic -Wno-unused-parameter'

## Platform specific flags

wayland_compile_flags := '-DBREAD_WAYLAND'
x11_compile_flags := '-DBREAD_X11'
wayland_link_flags := '-lwayland-client'
x11_link_flags := '-lX11'

## Colors

red := "\\x1b[31m"
green := "\\x1b[32m"
yellow := "\\x1b[33m"
reset := "\\x1b[0m"

default:
    just --list

## Utilities

clean:
    ! [[ -d {{ out }} ]] || rm -fr {{ out }}
    ! [[ -d {{ bin }} ]] || rm -fr {{ bin }}

bear platform="wayland":
    bear -- just compile-bread {{ platform }} debug force

## Compile

compile-bread platform="wayland" target="debug" force="dont_force" threads=num_cpus():
    #!/usr/bin/env bash
    shopt -s globstar

    CURRENT_PLATFORM_COMPILE_FLAGS=""
    CURRENT_TARGET_COMPILE_FLAGS=""

    [[ -d {{ src }} ]] || exit 0

    if [[ {{ platform }} == "wayland" ]]; then
      CURRENT_PLATFORM_COMPILE_FLAGS="{{ wayland_compile_flags }}"
    else
      CURRENT_PLATFORM_COMPILE_FLAGS="{{ x11_compile_flags }}"
    fi


    if [[ {{ target }} == "release" ]]; then
      CURRENT_TARGET_COMPILE_FLAGS="{{ release_compile_flags }}"
    else
      CURRENT_TARGET_COMPILE_FLAGS="{{ debug_compile_flags }}"
    fi

    [[ -d {{ bread_out }} ]] || mkdir -p {{ bread_out }}


    WILL_COMPILE=false

    export WILL_COMPILE
    export CURRENT_PLATFORM_COMPILE_FLAGS
    export CURRENT_TARGET_COMPILE_FLAGS

    check_flags() {
      if [[ {{ force }} == "force" || {{ force }} == "true" ]]; then
        WILL_COMPILE=true; echo -e "Compile: Forcing"; return
      fi

      for file in {{ src }}/**/*.c; do
        local current_out_file="$(basename "${file%.c}")-{{ platform }}-{{ target }}.o"

        if ! [[ -f {{ bread_out }}/${current_out_file} ]]; then
          WILL_COMPILE=true; return
        fi
        if [[ $file -nt {{ bread_out }}/${current_out_file} ]]; then
          WILL_COMPILE=true; return
        fi
      done
    }

    compile() {
      local file="$1"
      local current_out_file="$(basename "${file%.c}")-{{ platform }}-{{ target }}.o"

      if [[ "$file" == *"wayland"* && {{ platform }} != "wayland" ]]; then
        return
      fi

      if [[ "$file" == *"x11"* && {{ platform }} != "x11" ]]; then
        return
      fi

      if [[ "$file" -nt "$current_out_file" || {{ force }} == "true" || {{ force }} == "force" ]]; then
        echo -e "Compiling {{ green }}$file{{ reset }}..."
        gcc {{ include_flags }} ${CURRENT_TARGET_COMPILE_FLAGS} ${CURRENT_PLATFORM_COMPILE_FLAGS} -c "$file" -o "{{ bread_out }}/${current_out_file}"
      fi
    }
    export -f compile
    check_flags

    if [[ ${WILL_COMPILE} == false ]]; then
      echo -e "Compile: Nothing to do"
      exit 0
    fi

    echo -e "Using {{ red }}{{ threads }}{{ reset }} threads"
    echo -e "Target: {{ green }}{{ target }}{{ reset }}"
    echo -e "Platform: {{ green }}{{ platform }}{{ reset }}"

    find {{ src }} -name "*.c" -print0 | xargs -0 -P{{ threads }} -n1 bash -c 'compile "$0"'

    echo -e "Compile: Compiling {{ green }}{{ target }}{{ reset }} complete"

## Link

link-test platform="wayland" target="debug" force="dont_force" static="dynamic":
    #!/usr/bin/env bash
    shopt -s globstar

    [[ -d {{ bread_out }} ]] || exit 1
    [[ -d {{ bin }} ]] || mkdir -p {{ bin }}

    # Variable for checking if we should link.
    WILL_LINK=false

    check_flags() {
      if [[ {{ target }} == "release" ]] && \
      [[ {{ static }} == "dynamic" || {{ static }} == "false" ]] && \
      ! [[ -f {{ bin }}/{{ test_name }}-{{ platform }} ]]; then
            WILL_LINK=true; return
      fi

      if [[ {{ target }} == "debug" ]] && \
      [[ {{ static }} == "dynamic" || {{ static }} == "false" ]] && \
      ! [[ -f {{ bin }}/{{ test_name }}-{{ platform }}-debug ]]; then
          WILL_LINK=true; return
      fi

      if [[ {{ static }} == "static" || {{ static }} == "true" ]] && \
      ! [[ -f {{ bin }}/{{ test_name }}-{{ platform }}-static ]]; then
          WILL_LINK=true; return
      fi

      for file in {{ out }}/*-{{ target }}.o; do
        if [[ {{ target }} == "debug" ]] && [[ $file -nt {{ bin }}/{{ test_name }}-{{ platform }}-debug ]]; then
          WILL_LINK=true; return
        fi

        if [[ {{ target }} == "release" ]] && [[ $file -nt {{ bin }}/{{ test_name }}-{{ platform }} ]]; then
          WILL_LINK=true; return
        fi

        if [[ {{ static }} == "static" || {{ static }} == "true" ]] && [[ $file -nt {{ bin }}/{{ test_name }}-{{ platform }}-static ]]; then
          WILL_LINK=true; return
        fi
      done
    }

    link() {
      if [[ {{ target }} == "debug" ]]; then
        gcc {{ out }}/*-debug.o {{ debug_link_flags }} -o {{ bin }}/{{ test_name }}-{{ platform }}-debug -fuse-ld=mold
        echo -e "Link: Linking {{ green }}{{ target }}{{ reset }} dynamically complete"
      fi

      if [[ {{ target }} == "release" ]]; then \
        gcc {{ out }}/*-release.o {{ release_link_flags }} -o {{ bin }}/{{ test_name }}-{{ platform }} -fuse-ld=mold
        strip --strip-all {{ bin }}/{{ test_name }}-{{ platform }}
        echo -e "Link: Linking {{ green }}{{ target }}{{ reset }} dynamically complete"
      fi

      if [[ {{ static }} == "static" || {{ static }} == "true" ]]; then
        gcc {{ out }}/*-release.o {{ release_static_link_flags }} -o {{ bin }}/{{ test_name }}-{ platform }-static -fuse-ld=mold
        strip --strip-all {{ bin }}/{{ test_name }}-{{ platform }}-static
        echo -e "Link: Linking {{ green }}{{ target }}{{ reset }} statically complete"
      fi
    }

    check_flags

    if [[ ${WILL_LINK} == false ]]; then
      echo -e "Link: Nothing to do"
      exit 0
    fi

    link

## Aliases

build-bread platform="wayland" target="debug" force="false" static="dynamic" threads=num_cpus():
    just compile-bread {{ platform }} {{ target }} {{ force }} {{ threads }}
    just assemble-bread {{ platform }} {{ target }} {{ force }} {{ threads }}

build-test platform="wayland" target="debug" force="false" static="dynamic" threads=num_cpus():
    just build-bread {{ platform }} {{ target }} {{ force }} {{ static }}  {{ threads }}

    just compile {{ platform }} {{ target }} {{ force }} {{ threads }}
    just link-test {{ platform }} {{ target }} {{ static }}

build platform="wayland" target="debug" force="false" static="dynamic" threads=num_cpus():
    just compile {{ platform }} {{ target }} {{ force }} {{ threads }}
    just link {{ platform }} {{ target }} {{ static }}

release platform="wayland" force="false" static="dynamic" threads=num_cpus():
    just build {{ platform }} release {{ force }} {{ static }}  {{ threads }}

debug platform="wayland" force="false" threads=num_cpus():
    just build {{ platform }} debug {{ force }} dynamic {{ threads }}
