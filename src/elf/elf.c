#include <error.h>
#include <stdint.h>
#include "elf.h"

kstatus check_elf(elf64_ehdr *ehdr){
    if(!ehdr){
        return KERNEL_STATUS_ERROR;
    }

    if( ehdr->e_ident[0] == ELFMAG0 && ehdr->e_ident[1] == ELFMAG1 &&
        ehdr->e_ident[2] == ELFMAG2 && ehdr->e_ident[3] == ELFMAG3){
            return KERNEL_STATUS_SUCCESS;
        }
    
    return KERNEL_STATUS_ERROR;
}

kstatus kernel_load_elf64(elf64_ehdr *ehdr){
    if(!check_elf(ehdr)){
        return KERNEL_STATUS_ERROR;
    }

    
}