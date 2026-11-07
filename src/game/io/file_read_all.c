// Reading a whole file into a buffer without the CD fallback (Pharaoh.inf into the settings): 1 when read, 0 when it
// does not open.

#include "io/file_read_all.h"

#include "io/file.h"
#include "io/io_data.h"
#include "platform/platform_file.h"

#include <stdint.h>
#include <stdio.h>

int32_t io_read_file_whole(const char *path, void *buffer)
{
    FILE *file = platform_fopen(path, file_mode_read_binary);
    if (!file) {
        return 0;
    }
    int32_t length = platform_stream_length(file);
    fread(buffer, 1, (uint32_t) length, file);
    fclose(file);
    return 1;
}
