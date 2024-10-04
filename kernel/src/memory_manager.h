#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdint.h>

#include "bitmap_allocator.h"

// Hàm khởi tạo Memory Manager
void memory_manager_init();

// Hàm cấp phát một khối bộ nhớ vật lý
// Trả về địa chỉ vật lý của khối đã cấp phát hoặc 0 nếu thất bại
uint64_t allocate_physical_block();

// Hàm giải phóng một khối bộ nhớ vật lý
// Nhận vào địa chỉ vật lý của khối cần giải phóng
void free_physical_block(uint64_t phys_address);

// Hàm kiểm tra trạng thái của một khối
// Nhận vào địa chỉ vật lý của khối và trả về 1 nếu đã cấp phát, 0 nếu chưa
int is_block_allocated(uint64_t phys_address);

// Hàm cấp phát nhiều khối bộ nhớ vật lý liên tiếp
// Trả về địa chỉ vật lý của khối đầu tiên trong dãy đã cấp phát hoặc 0 nếu thất bại
uint64_t allocate_physical_blocks(uint64_t count);

// Hàm giải phóng nhiều khối bộ nhớ vật lý liên tiếp
// Nhận vào địa chỉ vật lý của khối đầu tiên và số lượng khối cần giải phóng
void free_physical_blocks(uint64_t phys_address, uint64_t count);

// Hàm kiểm tra xem một dãy các khối đã được cấp phát hay chưa
// Nhận vào địa chỉ vật lý của khối đầu tiên và số lượng khối
// Trả về 1 nếu tất cả đã được cấp phát, 0 nếu có ít nhất một khối chưa
int are_blocks_allocated(uint64_t phys_address, uint64_t count);

// **Hàm mới: Cấp phát bộ nhớ theo kích thước byte**
// Trả về địa chỉ vật lý của vùng nhớ đã cấp phát hoặc 0 nếu thất bại
uint64_t allocate_memory_bytes(uint64_t size);

// **Hàm mới: Giải phóng bộ nhớ theo địa chỉ và kích thước byte**
// Nhận vào địa chỉ vật lý và kích thước của vùng nhớ cần giải phóng
void free_memory_bytes(uint64_t phys_address, uint64_t size);

#endif // MEMORY_MANAGER_H
