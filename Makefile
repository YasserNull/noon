CC      = gcc
SRC     = $(shell find src/ -type f -name "*.c")
OBJ     = $(patsubst ./%.c,build/obj/%.o,$(SRC))
TARGET  = build/noon
FILES = $(shell find . -type f -name "*.c" -o -name "*.h")
# إعدادات البناء
CFLAGS_DEBUG   = -Wall -Wextra -Wshadow -Wpedantic -g -O0 -Iinclude
CFLAGS_RELEASE = -O2 -flto -ffunction-sections -fdata-sections -Iinclude
LDFLAGS        = -lm
LDFLAGS_RELEASE = -Wl,--gc-sections -s

# الوضع الافتراضي -> Release
all: release

# Debug build
debug: CFLAGS = $(CFLAGS_DEBUG)
debug: LDFLAGS +=
debug: $(TARGET)

# Release build
release: CFLAGS = $(CFLAGS_RELEASE)
release: LDFLAGS += $(LDFLAGS_RELEASE)
release: $(TARGET)

# كيف نبني الهدف النهائي
$(TARGET): $(OBJ)
	@mkdir -p $(@D)
	$(CC) $(OBJ) -o $@ $(CFLAGS) $(LDFLAGS)

# كيف نبني ملفات .o داخل build/obj/
build/obj/%.o: ./%.c
	@mkdir -p $(@D)
	$(CC) -c $< -o $@ $(CFLAGS)

format:
	clang-format -i $(FILES) -style="{BasedOnStyle: LLVM, BinPackArguments: false, AllowShortCaseLabelsOnASingleLine: true, ColumnLimit: 200}"
   
# تنظيف
clean:
	rm -rf build

.PHONY: all debug release format clean
