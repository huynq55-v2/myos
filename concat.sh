#!/bin/bash

# Đặt tên file kết quả
OUTPUT_FILE="all_code_combined.txt"

# Xóa file kết quả nếu đã tồn tại
rm -f $OUTPUT_FILE

# Tìm tất cả các file mã nguồn trong repo (ví dụ với đuôi .c, .h, .cpp, .py, .php, .js, ...)
# và gộp nội dung của chúng vào file kết quả
find . -type f \( -name "*.c" -o -name "*.h" -o -name "*.cpp" -o -name "*.py" -o -name "*.php" -o -name "*.js" \) \
-exec cat {} + >> $OUTPUT_FILE

echo "All source code has been concatenated into $OUTPUT_FILE"
