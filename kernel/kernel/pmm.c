#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <kernel/pmm.h>

#define PMM_MAX_PHYSICAL_ADDRESS 0x100000000ULL
#define PMM_FRAME_COUNT (PMM_MAX_PHYSICAL_ADDRESS / PMM_PAGE_SIZE)
#define PMM_BITMAP_SIZE (PMM_FRAME_COUNT / 8)

static uint8_t frame_bitmap[PMM_BITMAP_SIZE];
static uint32_t free_frame_count;

extern char __kernel_start[];
extern char __kernel_end[];

static void mark_frame_used(uint32_t frame) {
    uint8_t mask = (uint8_t)(1u << (frame % 8));
    uint8_t *byte = &frame_bitmap[frame / 8];

    if (!(*byte & mask)) {
        *byte |= mask;
        free_frame_count--;
    }
}

static void mark_frame_free(uint32_t frame) {
    uint8_t mask = (uint8_t)(1u << (frame % 8));
    uint8_t* byte = &frame_bitmap[frame / 8];

    if (*byte & mask) {
        *byte &= (uint8_t)~mask;
        free_frame_count++;
    }
}

static void mark_range_free(uint64_t address, uint64_t length) {
    uint64_t start;
    uint64_t end;

    if (address >= PMM_MAX_PHYSICAL_ADDRESS) {
        return;
    }

    if (length > PMM_MAX_PHYSICAL_ADDRESS - address) {
        end = PMM_MAX_PHYSICAL_ADDRESS;
    } else {
        end = address + length;
    }

    start = (address + PMM_PAGE_SIZE - 1) & ~(uint64_t)(PMM_PAGE_SIZE - 1);
    end &= ~(uint64_t)(PMM_PAGE_SIZE - 1);

    for (uint64_t physical = start; physical < end; physical += PMM_PAGE_SIZE) {
        mark_frame_free((uint32_t)(physical / PMM_PAGE_SIZE));
    }
}

static void mark_range_used(uint64_t address, uint64_t length) {
      uint64_t start;
      uint64_t end;

      if (address >= PMM_MAX_PHYSICAL_ADDRESS) {
          return;
      }

      if (length > PMM_MAX_PHYSICAL_ADDRESS - address) {
          end = PMM_MAX_PHYSICAL_ADDRESS;
      } else {
          end = address + length;
      }

      start = address & ~(uint64_t)(PMM_PAGE_SIZE - 1);
      end = (end + PMM_PAGE_SIZE - 1) & ~(uint64_t)(PMM_PAGE_SIZE - 1);

      for (uint64_t physical = start; physical < end; physical += PMM_PAGE_SIZE) {
          mark_frame_used((uint32_t)(physical / PMM_PAGE_SIZE));
      }
  }

  bool pmm_init(const multiboot_info_t* mbi) {
      uint32_t current;
      uint32_t end;

      if (!(mbi->flags & MULTIBOOT_INFO_MEM_MAP)) {
          return false;
      }

      memset(frame_bitmap, 0xFF, sizeof(frame_bitmap));
      free_frame_count = 0;

      if (mbi->mmap_length > UINT32_MAX - mbi->mmap_addr) {
          return false;
      }

      current = mbi->mmap_addr;
      end = mbi->mmap_addr + mbi->mmap_length;

      while (current < end) {
          const struct multiboot_mmap_entry* entry;
          uint32_t remaining = end - current;
          uint32_t entry_total_size;

          if (remaining < sizeof(uint32_t)) {
              return false;
          }

          entry = (const struct multiboot_mmap_entry*)current;

          if (entry->size < 20 || entry->size > remaining - sizeof(uint32_t)) {
              return false;
          }

          entry_total_size = entry->size + sizeof(uint32_t);

          if (entry->type == 1) {
              mark_range_free(entry->addr, entry->len);
          }

          current += entry_total_size;
      }

      /* Keep early boot / firmware memory out of the first allocator. */
      mark_range_used(0, 0x100000);

      /* The kernel range includes code, globals, .bss, stack, and bitmap. */
      mark_range_used(
          (uint32_t)__kernel_start,
          (uint32_t)__kernel_end - (uint32_t)__kernel_start
      );

      /* GRUB-owned structures are still in active use by the kernel. */
      mark_range_used((uint32_t)mbi, PMM_PAGE_SIZE);
      mark_range_used(mbi->mmap_addr, mbi->mmap_length);

      return true;
  }

  uint32_t pmm_total_frames(void) {
      return (uint32_t)PMM_FRAME_COUNT;
  }

  uint32_t pmm_free_frames(void) {
      return free_frame_count;
  }

  static bool frame_is_used(uint32_t frame) {
    uint8_t mask = (uint8_t)(1u << (frame % 8));
    return (frame_bitmap[frame / 8] & mask) != 0;
  }

  uint32_t pmm_allocate_frame(void) {
    for (uint32_t frame = 0; frame < PMM_FRAME_COUNT; frame++) {
        if (!frame_is_used(frame)) {
            mark_frame_used(frame);
            return frame * PMM_PAGE_SIZE;
        }
    }

    return PMM_INVALID_FRAME;
  }

  bool pmm_free_frame(uint32_t physical_address) {
    uint32_t frame;

    if (physical_address == PMM_INVALID_FRAME ||
        (physical_address & (PMM_PAGE_SIZE - 1)) != 0) {
            return false;
    }

    frame = physical_address / PMM_PAGE_SIZE;

    if (frame >= PMM_FRAME_COUNT || !frame_is_used(frame)) {
        return false;
    }

    mark_frame_free(frame);
    return true;
  }
