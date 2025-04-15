#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <lock.h>
#include "../flanterm/flanterm.h"
#include "../include/stdio.h"
#include "../drivers/serial.h"
#include "../hal/tsc.h"


#define FORMAT_LENGTH       1
#define NORMAL              0
#define STATE_SHORT         2
#define STATE_LONG          3
#define FORMAT_SPECIFIER    1

#define LENGTH_DEFAULT      0
#define LENGTH_SHORT_SHORT  1
#define LENGTH_SHORT        2
#define LENGTH_LONG         3
#define LENGTH_LONG_LONG    4

extern bool serial_enabled;

void klog(int level, const char *func, const char *msg){
    switch (level) {
        case LOG_INFO:
            kprintf("[{d}] info: {s}: {sn}", tsc_get_timestamp(), func, msg);
            if(serial_enabled){
                serial_kprintf("{k}KLOG_INFO{k}: {s}: {sn}", ANSI_COLOR_MAGENTA, ANSI_COLOR_RESET, func, msg);
            }
            return;
        case LOG_WARN:
            kprintf("[{d}] {k}warning{k}: {s}: {sn}", tsc_get_timestamp(), ANSI_COLOR_YELLOW, ANSI_COLOR_RESET, func, msg);
            if(serial_enabled){
                serial_kprintf("{k}KLOG_WARN{k}: {s}: {sn}", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET, func, msg);
            }
            return;
        case LOG_ERROR:
            kprintf("[{d}] {k}error{k}: {s}: {sn}", tsc_get_timestamp(), ANSI_COLOR_RED, ANSI_COLOR_RESET, func, msg);
            if(serial_enabled){
                serial_kprintf("{k}KLOG_ERROR{k}: {s}: {sn}", ANSI_COLOR_RED, ANSI_COLOR_RESET, func, msg);
            }
            return;
        case LOG_SUCCESS:
            kprintf("[{d}] {k}success{k}: {s}: {sn}", tsc_get_timestamp(), ANSI_COLOR_GREEN, ANSI_COLOR_RESET, func, msg);
            if(serial_enabled){
                serial_kprintf("{k}KLOG_SUCCESS{k}: {s}: {sn}", ANSI_COLOR_GREEN, ANSI_COLOR_RESET, func, msg);
            }
            return;
    }

    return;
    
}

int logprintf(const char *format_string, ...){
    kprintf("[{d}] ", tsc_get_timestamp());
    va_list va;
    va_start(va, format_string);
    vkprintf(format_string, va);
    va_end(va);
    return 0;
}



atomic_flag printf_lock = ATOMIC_FLAG_INIT;

/* 
    printf()
    params:
        string
        arguments
    
    available format specifiers:
        {i}, {d} - integer
        {s}      - string
        {c}      - char
        {k}      - color
        {n}      - newline (doesnt take in a argument)
        {x}      - base16
        {b}      - binary
 */

int kprintf(const char *format_string, ...){
    extern struct flanterm_context *ft_ctx;
    acquire_lock(&printf_lock);
    int state = NORMAL;
    va_list a_list;
    va_start(a_list, format_string);
    for(uint64_t i = 0; i < strlen(format_string); i++){
        char current = format_string[i]; // current char in string
        switch (state){
            case NORMAL:
                switch (current) {
                    case '{':
                        state = FORMAT_SPECIFIER;
                        break;
                    default:
                        print_char(ft_ctx, current);
                        break;                     
                }
                break;
            case FORMAT_SPECIFIER:
                switch (current) {
                    case 'n':
                        print_str(ft_ctx, "\n");
                        break;
                    case 'k':
                        print_str(ft_ctx, va_arg(a_list, char*));
                        break;
                    case 'd':
                    case 'i':
                        print_int(ft_ctx, va_arg(a_list, long long));
                        break;
                    case 's':
                        print_str(ft_ctx, va_arg(a_list, char*));
                        break;
                    case 'c':
                        ;
                        int ch = va_arg(a_list, int);
                        print_char(ft_ctx, ch);
                        break;
                    case 'x':
                        print_hex(ft_ctx, va_arg(a_list, uint64_t));
                        break;
                    case 'b':
                        print_bin(ft_ctx, va_arg(a_list, uint64_t));
                        break;
                    case 'l':
                        current++;
                        switch (current) {
                            case 'd':
                                print_int(ft_ctx, va_arg(a_list, long long int));
                                break;
                        
                        }
                        break;
                    case '}':
                        state = NORMAL;
                        break;

                }
                break;
        }

    }

    va_end(a_list);
    free_lock(&printf_lock);
    return 0;
}

int vkprintf(const char *format_string, va_list a_list){
    extern struct flanterm_context *ft_ctx;
    acquire_lock(&printf_lock);
    int state = NORMAL;
    for(uint64_t i = 0; i < strlen(format_string); i++){
        char current = format_string[i]; // current char in string
        switch (state){
            case NORMAL:
                switch (current) {
                    case '{':
                        state = FORMAT_SPECIFIER;
                        break;
                    default:
                        print_char(ft_ctx, current);
                        break;                     
                }
                break;
            case FORMAT_SPECIFIER:
                switch (current) {
                    case 'n':
                        print_str(ft_ctx, "\n");
                        break;
                    case 'k':
                        print_str(ft_ctx, va_arg(a_list, char*));
                        break;
                    case 'd':
                    case 'i':
                        print_int(ft_ctx, va_arg(a_list, long long));
                        break;
                    case 's':
                        print_str(ft_ctx, va_arg(a_list, char*));
                        break;
                    case 'c':
                        ;
                        int ch = va_arg(a_list, int);
                        print_char(ft_ctx, ch);
                        break;
                    case 'x':
                        print_hex(ft_ctx, va_arg(a_list, uint64_t));
                        break;
                    case 'b':
                        print_bin(ft_ctx, va_arg(a_list, uint64_t));
                        break;
                    case 'l':
                        current++;
                        switch (current) {
                            case 'd':
                                print_int(ft_ctx, va_arg(a_list, long long int));
                                break;
                        
                        }
                        break;
                    case '}':
                        state = NORMAL;
                        break;

                }
                break;
        }

    }

    free_lock(&printf_lock);
    return 0;
}

int serial_kprintf(const char *format_string, ...){
    int state = NORMAL;
    va_list a_list;
    va_start(a_list, format_string);
    for(uint64_t i = 0; i < strlen(format_string); i++){
        char current = format_string[i]; // current char in string
        switch (state){
            case NORMAL:
                switch (current) {
                    case '{':
                        state = FORMAT_SPECIFIER;
                        break;
                    default:
                        serial_print_char(current);
                        break;                     
                }
                break;
            case FORMAT_SPECIFIER:
                switch (current) {
                    case 'n':
                        serial_print("\n");
                        break;
                    case 'k':
                        serial_print(va_arg(a_list, char*));
                        break;
                    case 'd':
                    case 'i':
                        serial_print_int(va_arg(a_list, long long));
                        break;
                    case 's':
                        serial_print(va_arg(a_list, char*));
                        break;
                    case 'c':
                        ;
                        int ch = va_arg(a_list, int);
                        serial_print_char(ch);
                        
                        break;
                    case 'x':
                        serial_print_hex(va_arg(a_list, uint64_t));
                        break;
                    case 'b':
                        serial_print_bin(va_arg(a_list, uint64_t));
                        break;
                    case 'l':
                        current++;
                        switch (current) {
                            case 'd':
                            case 'i':
                                serial_print_int(va_arg(a_list, long long int));
                                break;
                        
                        }
                        break;
                    case '}':
                        state = NORMAL;
                        break;

                }
                break;
        }

    }

    va_end(a_list);

    return 0;
}

#define MAX_INTERGER_SIZE 128

void print_char(struct flanterm_context *ft_ctx, char c){
    kernel_framebuffer_print(&c, 1);
}

void serial_print_char(char c){
    serial_write(c);
}

void print_str(struct flanterm_context *ft_ctx, char *str){
    kernel_framebuffer_print(str, strlen(str));
}

void print_int(struct flanterm_context *ft_ctx, uint64_t num){
    char buffer[MAX_INTERGER_SIZE] = {0};

    if(num == 0){
        buffer[0] = '0';
    }

    int arr[MAX_INTERGER_SIZE] = {0};
    int j = 0;

    while(num != 0){
        int mod = num % 10;
        arr[j] = dtoc(mod);
        num /= 10;
        j++;

        if(j == MAX_INTERGER_SIZE){
            return;
        }
    }

    /* Reverse buffer */
    for(int i = 0; i < j; i++){
        buffer[i] = arr[j - i - 1];
    }
    
    kernel_framebuffer_print(buffer, strlen(buffer));
}

void print_hex(struct flanterm_context *ft_ctx, uint64_t num){
    char buffer[MAX_INTERGER_SIZE] = {0};

    if(num == 0){
        buffer[0] = '0';
    }

    int arr[MAX_INTERGER_SIZE] = {0};
    int j = 0;

    while(num != 0){
        int mod = num % 16;
        arr[j] = dtoc(mod);
        num /= 16;
        j++;

        if(j == MAX_INTERGER_SIZE){
            return;
        }
    }

    /* Reverse buffer */
    for(int i = 0; i < j; i++){
        buffer[i] = arr[j - i - 1];
    }

    kernel_framebuffer_print(buffer, strlen(buffer));
}

void print_bin(struct flanterm_context *ft_ctx, uint64_t num){
    char buffer[MAX_INTERGER_SIZE] = {0};

    int arr[MAX_INTERGER_SIZE] = {0};
    int j = 0;

    while(num != 0){
        int mod = num % 2;
        arr[j] = dtoc(mod);
        num /= 2;
        j++;

        if(j == MAX_INTERGER_SIZE){
            return;
        }
    }

    /* Reverse buffer */
    for(int i = 0; i < j; i++){
        buffer[i] = arr[j - i - 1];
    }

    kernel_framebuffer_print(buffer, strlen(buffer));
}

void serial_print_int(uint64_t num){
    char buffer[MAX_INTERGER_SIZE] = {0};

    int arr[MAX_INTERGER_SIZE] = {0};
    int j = 0;

    while(num != 0){
        int mod = num % 10;
        arr[j] = dtoc(mod);
        num /= 10;
        j++;

        if(j == MAX_INTERGER_SIZE){
            return;
        }
    }

    /* Reverse buffer */
    for(int i = 0; i < j; i++){
        buffer[i] = arr[j - i - 1];
    }

    kernel_serial_print(buffer, strlen(buffer));
}

void serial_print_hex(uint64_t num){
    char buffer[MAX_INTERGER_SIZE] = {0};

    int arr[MAX_INTERGER_SIZE] = {0};
    int j = 0;

    while(num != 0){
        int mod = num % 16;
        arr[j] = dtoc(mod);
        num /= 16;
        j++;

        if(j == MAX_INTERGER_SIZE){
            return;
        }
    }

    /* Reverse buffer */
    for(int i = 0; i < j; i++){
        buffer[i] = arr[j - i - 1];
    }

    kernel_serial_print(buffer, strlen(buffer));
}

void serial_print_bin(uint64_t num){
    char buffer[MAX_INTERGER_SIZE] = {0};

    int arr[MAX_INTERGER_SIZE] = {0};
    int j = 0;

    while(num != 0){
        int mod = num % 2;
        arr[j] = dtoc(mod);
        num /= 2;
        j++;

        if(j == MAX_INTERGER_SIZE){
            return;
        }
    }

    /* Reverse buffer */
    for(int i = 0; i < j; i++){
        buffer[i] = arr[j - i - 1];
    }

    kernel_serial_print(buffer, strlen(buffer));
}


char toupper(char c){
        switch(c){
                case 'a':
                        return 'A';
                case 'b':
                        return 'B';
                case 'c':
                        return 'C';
                case 'd':
                        return 'D';
                case 'e':
                        return 'E';
                case 'f':
                        return 'F';
                case 'g':
                        return 'G';
                case 'h':
                        return 'H';
                case 'i':
                        return 'I';
                case 'j':
                        return 'J';
                case 'k':
                        return 'K';
                case 'l':
                        return 'L';
                case 'm':
                        return 'M';
                case 'n':
                        return 'N';
                case 'o':
                        return 'O';
                case 'p':
                        return 'P';
                case 't':
                        return 'T';
                case 'r':
                        return 'R';
                case 's':
                        return 'S';
                case 'u':
                        return 'U';
                case 'v':
                        return 'V';
                case 'w':
                        return 'W';
                case 'x':
                        return 'X';
                case 'y':
                        return 'Y';
                case 'z':
                        return 'Z';
                default:
                        return c;

        }   
}


atomic_flag fb_spinlock = ATOMIC_FLAG_INIT;

/* Eventually fix printf so that these print_* functions dont
   write to the framebuffer but instead return to printf */

/* Prints a char array to the framebuffer, thread safe*/
void kernel_framebuffer_print(char *buffer, size_t n){
    extern struct flanterm_context *ft_ctx;
    //acquire_lock(&fb_spinlock);
    flanterm_write(ft_ctx, buffer, n);
    //free_lock(&fb_spinlock);
}

atomic_flag serial_spinlock = ATOMIC_FLAG_INIT;

/* Prints a char array to serial, thread safe*/
void kernel_serial_print(char *buffer, size_t n){
    //acquire_lock(&serial_spinlock);
    for(size_t i = 0; i < n; i++){
        serial_print_char(buffer[i]);
    }
    //free_lock(&serial_spinlock);
}