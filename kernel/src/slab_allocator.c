#include "slab_allocator.h"
#include "limine.h"

// Các yêu cầu từ Limine
extern volatile struct limine_memmap_request memmap_request;
extern volatile struct limine_hhdm_request hhdm_request;

// Con trỏ đến danh sách các khối bộ nhớ tự do
static free_block_t* free_list = NULL;
// Biến toàn cục để lưu trữ HHDM offset
static uint64_t hhdm_offset = 0;

// Hàm khởi tạo bộ phân bổ bộ nhớ
void init_memory_allocator() {
    // Đảm bảo rằng Limine đã cung cấp phản hồi
    if (memmap_request.response == NULL || hhdm_request.response == NULL) {
        // Xử lý lỗi nếu cần
        return;
    }

    hhdm_offset = hhdm_request.response->offset;

    // Duyệt qua các mục trong bản đồ bộ nhớ
    for (size_t i = 0; i < memmap_request.response->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap_request.response->entries[i];

        // Chỉ sử dụng các vùng bộ nhớ khả dụng
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            // Chuyển đổi địa chỉ vật lý sang địa chỉ ảo bằng cách thêm offset HHDM
            uintptr_t virt_addr = entry->base + hhdm_offset;
            size_t region_size = entry->length;

            // Tạo một khối bộ nhớ tự do mới và thêm vào danh sách
            free_block_t* block = (free_block_t*)virt_addr;
            block->size = region_size - sizeof(free_block_t);
            block->next = free_list;
            block->is_free = 1; // Đánh dấu là tự do
            free_list = block;
        }
    }
}

// Hàm tìm khối bộ nhớ tự do phù hợp
static free_block_t** find_free_block(size_t size) {
    free_block_t** curr = &free_list;
    while (*curr != NULL) {
        if ((*curr)->size >= size && (*curr)->is_free) {
            return curr;
        }
        curr = &(*curr)->next;
    }
    return NULL;
}

// Hàm cấp phát bộ nhớ
void* slab_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    // Căn chỉnh kích thước theo kích thước của con trỏ
    size = (size + sizeof(void*) - 1) & ~(sizeof(void*) - 1);

    // Tìm khối bộ nhớ tự do phù hợp
    free_block_t** block_ptr = find_free_block(size);
    if (block_ptr == NULL) {
        // Không tìm thấy khối phù hợp
        return NULL;
    }

    free_block_t* block = *block_ptr;

    // Nếu kích thước khối lớn hơn nhiều so với yêu cầu, chia nhỏ khối
    if (block->size > size + sizeof(free_block_t)) {
        free_block_t* new_block = (free_block_t*)((uintptr_t)block + sizeof(free_block_t) + size);
        new_block->size = block->size - size - sizeof(free_block_t);
        new_block->next = block->next;
        new_block->is_free = 1; // Đánh dấu là tự do
        *block_ptr = new_block;

        // Cập nhật kích thước của khối được cấp phát
        block->size = size;
    } else {
        // Sử dụng toàn bộ khối
        *block_ptr = block->next;
    }

    block->is_free = 0; // Đánh dấu là đã cấp phát

    // Trả về con trỏ đến vùng bộ nhớ có thể sử dụng
    return (void*)((uintptr_t)block + sizeof(free_block_t));
}

// Hàm giải phóng bộ nhớ
void slab_free(void* ptr) {
    if (ptr == NULL) {
        return;
    }

    // Lấy địa chỉ của khối bộ nhớ
    free_block_t* block = (free_block_t*)((uintptr_t)ptr - sizeof(free_block_t));

    // Đánh dấu khối là tự do
    block->is_free = 1;

    // Thêm khối vào danh sách các khối tự do
    block->next = free_list;
    free_list = block;
}

// Hàm tái phân bổ bộ nhớ
void* slab_realloc(void* ptr, size_t size) {
    if (ptr == NULL) {
        return slab_malloc(size);
    }

    if (size == 0) {
        slab_free(ptr);
        return NULL;
    }

    // Lấy khối bộ nhớ hiện tại
    free_block_t* block = (free_block_t*)((uintptr_t)ptr - sizeof(free_block_t));

    if (block->size >= size) {
        // Khối hiện tại đủ lớn
        return ptr;
    } else {
        // Cấp phát khối mới và sao chép dữ liệu
        void* new_ptr = slab_malloc(size);
        if (new_ptr == NULL) {
            return NULL;
        }
        // Sao chép dữ liệu
        for (size_t i = 0; i < block->size; i++) {
            ((char*)new_ptr)[i] = ((char*)ptr)[i];
        }
        slab_free(ptr);
        return new_ptr;
    }
}

// Hàm tính tổng dung lượng bộ nhớ còn lại
size_t slab_total_free_memory() {
    size_t total_free_memory = 0;
    free_block_t* current = free_list;

    // Duyệt qua danh sách các khối bộ nhớ tự do
    while (current != NULL) {
        if (current->is_free) {
            total_free_memory += current->size;
        }
        current = current->next;
    }

    return total_free_memory;
}

// Hàm lấy kích thước bộ nhớ đã cấp phát
size_t slab_allocated_size(void* ptr) {
    if (ptr == NULL) {
        return 0;
    }

    // Lấy địa chỉ của metadata (free_block_t) ngay trước con trỏ được trả về
    free_block_t* block = (free_block_t*)((uintptr_t)ptr - sizeof(free_block_t));

    // Kiểm tra xem khối có đang được cấp phát hay không
    if (block->is_free) {
        return 0; // Đã được giải phóng
    }

    // Trả về kích thước của khối bộ nhớ
    return block->size;
}
