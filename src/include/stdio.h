#include <stdint.h>
#include "../flanterm/flanterm.h"

enum {
    LOG_INFO = 0,
    LOG_WARN,
    LOG_ERROR,
    LOG_SUCCESS,
};

void klog(int level, const char *func, const char *msg);

int kprintf(const char *format_string, ...);

void print_char(struct flanterm_context *ft_ctx, char c);
void print_str(struct flanterm_context *ft_ctx, char *str);
void print_int(struct flanterm_context *ft_ctx, uint64_t i);
void print_hex(struct flanterm_context *ft_ctx, uint64_t num);
void print_bin(struct flanterm_context *ft_ctx, uint64_t num);

char toupper(char c);

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"