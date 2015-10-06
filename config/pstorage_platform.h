/*
 * The MIT License (MIT)
 *
 * Copyright (c) [2015] [Marco Russi]
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/


#ifndef PSTORAGE_PL_H__
#define PSTORAGE_PL_H__

#include <stdint.h>

#define PSTORAGE_FLASH_PAGE_SIZE    ((uint16_t)NRF_FICR->CODEPAGESIZE)   /* Size of one flash page */
#define PSTORAGE_FLASH_EMPTY_MASK    0xFFFFFFFF                          /* Bit mask that defines an empty address in flash */

#define PSTORAGE_FLASH_PAGE_END                                     \
        ((NRF_UICR->BOOTLOADERADDR != PSTORAGE_FLASH_EMPTY_MASK)    \
        ? (NRF_UICR->BOOTLOADERADDR / PSTORAGE_FLASH_PAGE_SIZE)     \
        : NRF_FICR->CODESIZE)
        

#define PSTORAGE_NUM_OF_PAGES       2	/* Number of flash pages allocated for the pstorage module excluding the swap page, configurable based on system requirements */
#define PSTORAGE_MIN_BLOCK_SIZE     0x0010	/* Minimum size of block that can be registered with the module. Should be configured based on system requirements, recommendation is not have this value to be at least size of word */

#define PSTORAGE_DATA_START_ADDR    ((PSTORAGE_FLASH_PAGE_END - PSTORAGE_NUM_OF_PAGES) \
                                    * PSTORAGE_FLASH_PAGE_SIZE)	/* Start address for persistent data, configurable according to system requirements */
#define PSTORAGE_DATA_END_ADDR      (PSTORAGE_FLASH_PAGE_END * PSTORAGE_FLASH_PAGE_SIZE)	/* End address for persistent data, configurable according to system requirements */

#define PSTORAGE_MAX_BLOCK_SIZE     PSTORAGE_FLASH_PAGE_SIZE	/* Maximum size of block that can be registered with the module. Should be configured based on system requirements. And should be greater than or equal to the minimum size */
#define PSTORAGE_CMD_QUEUE_SIZE     30	/* Maximum number of flash access commands that can be maintained by the module for all applications. Configurable */


/* Abstracts persistently memory block identifier */
typedef uint32_t pstorage_block_t;

typedef struct
{
    uint32_t            module_id;      
    pstorage_block_t    block_id;       
} pstorage_handle_t;

typedef uint16_t pstorage_size_t;      

/* Handles Flash Access Result Events. To be called in the system event dispatcher of the application */
void pstorage_sys_event_handler (uint32_t sys_evt);

#endif // PSTORAGE_PL_H__


