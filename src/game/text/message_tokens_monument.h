// The functions of message_tokens_monument.c that other files call.

#ifndef TEXT_MESSAGE_TOKENS_MONUMENT_H
#define TEXT_MESSAGE_TOKENS_MONUMENT_H

#include <stdint.h>

const char *message_token_monument_name(void);
const char *message_token_percent_complete(void);
const char *message_token_courses_complete(void);
const char *message_token_needed_current_main(void);
const char *message_token_needed_current_secondary(void);
const char *message_token_needed_remainder_main(void);
const char *message_token_needed_remainder_secondary(void);
const char *message_token_loads_word(int32_t count);
const char *message_token_loads_current_main(void);
const char *message_token_loads_current_secondary(void);
const char *message_token_loads_remainder_main(void);
const char *message_token_loads_remainder_secondary(void);
const char *message_token_total_main(void);
const char *message_token_total_secondary(void);

#endif
