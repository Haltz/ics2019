#include "fs.h"

typedef size_t (*ReadFn)(void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn)(const void *buf, size_t offset, size_t len);
size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
size_t fbsync_write(const void *buf, size_t offset, size_t len);

typedef struct {
	char *	name;
	size_t	size;
	size_t	disk_offset;
	size_t	open_offset;
	ReadFn	read;
	WriteFn write;
} Finfo;

enum { FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB };

size_t invalid_read(void *buf, size_t offset, size_t len) {
	panic("should not reach here");
	return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
	panic("should not reach here");
	return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
	{ "stdin", 0, 0, 0, invalid_read, invalid_write },
	{ "stdout", 0, 0, 0, invalid_read, serial_write },
	{ "stderr", 0, 0, 0, invalid_read, serial_write },
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
	// initialize the size of /dev/fb
	file_table[FD_FB].size = (screen_width() * screen_height()) << 2;
}
