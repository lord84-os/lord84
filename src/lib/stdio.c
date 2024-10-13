#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include "../flanterm/flanterm.h"
#include "../flanterm/backends/fb.h"
#include "../../limine/limine.h"
#include "../include/stdio.h"
#include "../include/string.h"


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


void klog(int level, const char *func, const char *msg){
    switch (level) {
        case LOG_INFO:
            kprintf("{k}KLOG_INFO{k}: {s}: {sn}", ANSI_COLOR_MAGENTA, ANSI_COLOR_RESET, func, msg);
            return;
        case LOG_WARN:
            kprintf("{k}KLOG_WARN{k}: {s}: {sn}", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET, func, msg);
            return;
        case LOG_ERROR:
            kprintf("{k}KLOG_ERROR{k}: {s}: {sn}", ANSI_COLOR_RED, ANSI_COLOR_RESET, func, msg);
            return;
        case LOG_SUCCESS:
            kprintf("{k}KLOG_SUCCESS{k}: {s}: {sn}", ANSI_COLOR_GREEN, ANSI_COLOR_RESET, func, msg);
            return;
    }

    return;
    
}


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
 */



int kprintf(const char *format_string, ...){

    extern struct flanterm_context *ft_ctx;
    int state = NORMAL;
    va_list a_list;
    va_start(a_list, format_string);
    for(int i = 0; i < strlen(format_string); i++){
        char current = format_string[i]; // current char in string
        switch (state){
            case NORMAL:
                switch (current) {
                    case '{':
                        state = FORMAT_SPECIFIER;
                        break;
                    default:
                        print_char(ft_ctx, current); // FAIL
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


    return 0;
}

#define MAX_INTERGER_SIZE 128

void print_char(struct flanterm_context *ft_ctx, char c){
    flanterm_write(ft_ctx, &c, 1);
}

void print_str(struct flanterm_context *ft_ctx, char *str){
    flanterm_write(ft_ctx, str, strlen(str));
}

void print_int(struct flanterm_context *ft_ctx, uint64_t num){
    int arr[MAX_INTERGER_SIZE];
    int j = 0;
    int digitscount = 0;

    while(num != 0){
        int mod = num % 10;
        arr[j] = mod;
        num /= 10;
        j++;

        if(j == MAX_INTERGER_SIZE){            // ub handling like any good programmer
                return;
        }

        digitscount = j;
        if(num == 0){
                arr[j] = mod;
        }
    }
    for(int i = digitscount-1; i != -1; i--){
        switch(arr[i]){
                case 1:
                        print_char(ft_ctx, '1');
                        break;
                case 2:
                        print_char(ft_ctx, '2');
                        break;
                case 3:
                        print_char(ft_ctx, '3');
                        break;
                case 4:
                        print_char(ft_ctx, '4');
                        break;
                case 5:
                        print_char(ft_ctx, '5');
                        break;
                case 6:
                        print_char(ft_ctx, '6');
                        break;
                case 7:
                        print_char(ft_ctx, '7');
                        break;
                case 8:
                        print_char(ft_ctx, '8');
                        break;
                case 9:
                        print_char(ft_ctx, '9');
                        break;
                case 0:
                        print_char(ft_ctx, '0');
                        break;
        }
                
    }
    if(digitscount == 0){
        print_char(ft_ctx, '0');
    }
}

void print_hex(struct flanterm_context *ft_ctx, uint64_t num){
    int arr[MAX_INTERGER_SIZE];
    int j = 0;
    int digitscount = 0;

    while(num != 0){
        int mod = num % 16;
        arr[j] = mod;
        num /= 16;
        j++;

        if(j == MAX_INTERGER_SIZE){            // ub handling like any good programmer
                return;
        }

        digitscount = j;
        if(num == 0){
                arr[j] = mod;
        }
    }
    for(int i = digitscount-1; i != -1; i--){
        switch(arr[i]){
                case 1:
                        print_char(ft_ctx, '1');
                        break;
                case 2:
                        print_char(ft_ctx, '2');
                        break;
                case 3:
                        print_char(ft_ctx, '3');
                        break;
                case 4:
                        print_char(ft_ctx, '4');
                        break;
                case 5:
                        print_char(ft_ctx, '5');
                        break;
                case 6:
                        print_char(ft_ctx, '6');
                        break;
                case 7:
                        print_char(ft_ctx, '7');
                        break;
                case 8:
                        print_char(ft_ctx, '8');
                        break;
                case 9:
                        print_char(ft_ctx, '9');
                        break;
                case 10:
                        print_char(ft_ctx, 'A');
                        break;
                case 11:
                        print_char(ft_ctx, 'B');
                        break;
                case 12:
                        print_char(ft_ctx, 'C');
                        break;
                case 13:
                        print_char(ft_ctx, 'D');
                        break;
                case 14:
                        print_char(ft_ctx, 'E');
                        break;
                case 15:
                        print_char(ft_ctx, 'F');
                        break;
                case 0:
                        print_char(ft_ctx, '0');
                        break;
        }
                
    }
    if(digitscount == 0){
        print_char(ft_ctx, '0');
    }
}

void print_bin(struct flanterm_context *ft_ctx, uint64_t num){
    int arr[MAX_INTERGER_SIZE];
    int j = 0;
    int digitscount = 0;

    while(num != 0){
        int mod = num % 2;
        arr[j] = mod;
        num /= 2;
        j++;

        if(j == MAX_INTERGER_SIZE){            // ub handling like any good programmer
                return;
        }

        digitscount = j;

        if(num == 0){
                arr[j] = mod;
        }
    }
    for(int i = digitscount-1; i != -1; i--){
        switch(arr[i]){
                case 1:
                        print_char(ft_ctx, '1');
                        break;
                case 0:
                        print_char(ft_ctx, '0');
                        break;               
        }
                
    }
    if(digitscount == 0){
        print_char(ft_ctx, '0');
    }
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