// HTTPS requests of the program itself (http.c): the update's downloads and the bug reports. Windows has them through
// WinHTTP, the system's own HTTP client (proxies and certificates as the system has them, redirects followed); the
// other systems have none yet, and each call fails there.

#ifndef PLATFORM_HTTP_H
#define PLATFORM_HTTP_H

#include <stddef.h>
#include <stdint.h>

// 0 stops the download
typedef int32_t (*http_sink)(void *context, const uint8_t *data, uint32_t size);

int32_t http_supported(void);
// 1 when the answer is 200 and the sink took all of it
int32_t http_get(const char *url, http_sink sink, void *context);
// the answer's status in *status (0 when there was none); 1 when it is 2xx
int32_t http_post_json(const char *url, const char *body, size_t size, int32_t *status);

#endif
