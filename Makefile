# =============================================================================
# Makefile: build device_sim và controller   (Phụ trách: Long)
#
#   make              build cả 2 chương trình vào bin/
#   make test         build + chạy các chương trình thử trong tests/
#   make run-device   chạy device_sim với config/coop_A.conf
#   make run-ctrl     chạy controller
#   make clean        xoá bin/ build/
#   make reset-data   xoá dữ liệu sinh ra (data/, logs/) để thử "lần chạy đầu"
#   make sanitize     build lại với AddressSanitizer (bắt lỗi bộ nhớ)
#
# TẠI SAO VIẾT NHƯ VẬY
#   - Mỗi .c dịch thành một .o riêng trong build/, nên sửa 1 file chỉ dịch lại file đó.
#   - -MMD -MP sinh file .d ghi .c nào include .h nào, nên sửa .h thì make tự dịch lại đúng file.
#   - device/ và controller/ dùng include path riêng để header 2 bên không lẫn nhau.
# =============================================================================

CC       ?= gcc
CFLAGS   ?= -std=gnu11 -Wall -Wextra -g
DEPFLAGS := -MMD -MP
LDFLAGS  ?=

BUILD := build
BIN   := bin

COMMON_SRC := $(wildcard common/*.c)
DEVICE_SRC := $(wildcard device/*.c)
CTRL_SRC   := $(wildcard controller/*.c)
TEST_SRC   := $(wildcard tests/test_*.c)

COMMON_OBJ := $(COMMON_SRC:%.c=$(BUILD)/%.o)
DEVICE_OBJ := $(DEVICE_SRC:%.c=$(BUILD)/%.o)
CTRL_OBJ   := $(CTRL_SRC:%.c=$(BUILD)/%.o)
TEST_OBJ   := $(TEST_SRC:%.c=$(BUILD)/%.o)
TEST_BIN   := $(TEST_SRC:tests/%.c=$(BIN)/%)

.PHONY: all device controller test run-device run-device2 run-ctrl clean reset-data sanitize

# Giữ lại các .o trung gian của test (mặc định make tự xoá, lần sau phải dịch lại)
.SECONDARY:

all: $(BIN)/device_sim $(BIN)/controller

device: $(BIN)/device_sim
controller: $(BIN)/controller

$(BIN)/device_sim: $(DEVICE_OBJ) $(COMMON_OBJ)
	@mkdir -p $(BIN)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BIN)/controller: $(CTRL_OBJ) $(COMMON_OBJ)
	@mkdir -p $(BIN)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Mỗi chương trình thử = tests/test_xxx.c + toàn bộ common/
$(BIN)/test_%: $(BUILD)/tests/test_%.o $(COMMON_OBJ)
	@mkdir -p $(BIN)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD)/common/%.o: common/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -Icommon -c $< -o $@

$(BUILD)/device/%.o: device/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -Icommon -Idevice -c $< -o $@

$(BUILD)/controller/%.o: controller/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -Icommon -Icontroller -c $< -o $@

$(BUILD)/tests/%.o: tests/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -Icommon -c $< -o $@

test: $(TEST_BIN)
	@for t in $(TEST_BIN); do echo "== $$t"; ./$$t || exit 1; done

run-device: $(BIN)/device_sim
	./$(BIN)/device_sim -c config/coop_A.conf

run-device2: $(BIN)/device_sim
	./$(BIN)/device_sim -c config/coop_B.conf

run-ctrl: $(BIN)/controller
	./$(BIN)/controller

clean:
	rm -rf $(BUILD) $(BIN)

reset-data:
	find data logs -type f ! -name README.md -delete

sanitize: clean
	$(MAKE) CFLAGS="-std=gnu11 -Wall -Wextra -g -fsanitize=address,undefined" \
	        LDFLAGS="-fsanitize=address,undefined"

-include $(COMMON_OBJ:.o=.d) $(DEVICE_OBJ:.o=.d) $(CTRL_OBJ:.o=.d) $(TEST_OBJ:.o=.d)
