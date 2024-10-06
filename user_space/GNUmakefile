.PHONY: all clean

# Thư mục chứa file .o
OBJ_OUTPUT_DIR ?= build

# Đường dẫn đến file linker script
LINKER_SCRIPT ?= user_linker.ld

# Danh sách tất cả các file .c trong thư mục src
SRC_FILES := $(wildcard src/*.c)

# Danh sách các file .o tương ứng
OBJ_FILES := $(patsubst src/%.c, $(OBJ_OUTPUT_DIR)/%.o, $(SRC_FILES))

# Mục tiêu chính để build hello_user.elf
all: src/hello_user.elf

# Quy tắc để build hello_user.elf từ tất cả các file .o
src/hello_user.elf: $(OBJ_FILES)
	$(KLD) -T $(LINKER_SCRIPT) -o $@ $(OBJ_FILES) -L/usr/local/elf/sysroot/x86_64-elf/lib/ -lc

# Quy tắc để compile từng file .c thành file .o
$(OBJ_OUTPUT_DIR)/%.o: src/%.c
	mkdir -p $(OBJ_OUTPUT_DIR)
	$(KCC) $(KCFLAGS) $(KCPPFLAGS) -ffreestanding -c -g $< -o $@

# Quy tắc clean để xóa các file .o và .elf
clean:
	rm -f $(OBJ_OUTPUT_DIR)/*.o src/hello_user.elf
