#include <stdint.h>
#include <avr/io.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#include "nvm.h"


uint8_t nvm_read_byte(uint8_t command, uint8_t index)
{
    uint8_t result;

    NVM.CMD = command;
    result = pgm_read_byte(index);
    NVM.CMD = NVM_CMD_NO_OPERATION_gc;

    return result;
}
