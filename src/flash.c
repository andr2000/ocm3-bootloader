/**
 * @file    flash.c
 * @author  Ferenc Nemeth
 * @date    21 Dec 2018
 * @brief   This module handles the memory related functions.
 *
 *          Copyright (c) 2018 Ferenc Nemeth - https://github.com/ferenc-nemeth
 *          Copyright (c) 2023 Oleksandr Andrushchenko
 */

#include <libopencm3/stm32/desig.h>
#include <libopencm3/stm32/flash.h>

#include "flash.h"
#include "uart.h"

/* Start address of the user application. */
#define FLASH_APP_START_ADDRESS ((uint32_t)0x08001000u)

#define FLASH_PAGE_SHIFT        11
#define FLASH_PAGE_SIZE         (1U << FLASH_PAGE_SHIFT)
#define FLASH_PAGE_MASK	        (~(FLASH_PAGE_SIZE-1))

/* Function pointer for jumping to user application. */
typedef void (*fnc_ptr)(void);

static uint32_t flash_get_size(void)
{
  return desig_get_flash_size() * 1024;
}

static uint32_t flash_get_end(void)
{
  return FLASH_BASE + flash_get_size();
}

uint32_t flash_get_app_start(void)
{
  return FLASH_APP_START_ADDRESS;
}

/**
 * @brief   This function erases the memory.
 * @param   address: First address to be erased (the last is the end of the flash).
 * @return  status: Report about the success of the erasing.
 */
flash_status flash_erase(uint32_t address)
{
  flash_unlock();

  address &= FLASH_PAGE_MASK;
  uint32_t num_pages = (flash_get_end() - address) / FLASH_PAGE_SIZE;
  for (uint32_t i = 0u; i < num_pages; i++)
  {
    flash_erase_page(address);
    address += FLASH_PAGE_SIZE;
  }

  flash_lock();

  return FLASH_OK;
}

/**
 * @brief   This function flashes the memory.
 * @param   address: First address to be written to.
 * @param   *data:   Array of the data that we want to write.
 * @param   *length: Size of the array.
 * @return  status: Report about the success of the writing.
 */
flash_status flash_write(uint32_t address, uint32_t *data, uint32_t length)
{
  flash_status status = FLASH_OK;

  flash_unlock();

  /* Loop through the array. */
  for (uint32_t i = 0u; (i < length) && (FLASH_OK == status); i++)
  {
    /* If we reached the end of the memory, then report an error and don't do anything else.*/
    if (flash_get_end() <= address)
    {
      status |= FLASH_ERROR_SIZE;
    }
    else
    {
      flash_wait_for_last_operation();
      flash_program_word(address, data[i]);
      /* Read back the content of the memory. If it is wrong, then report an error. */
      if (((data[i])) != (*(volatile uint32_t*)address))
      {
        status |= FLASH_ERROR_READBACK;
      }

      /* Shift the address by a word. */
      address += sizeof(uint32_t);
    }
  }

  flash_lock();

  return status;
}

/**
 * @brief   Actually jumps to the user application.
 * @param   void
 * @return  void
 */
void flash_jump_to_app(void)
{
  /* Function pointer to the address of the user application. */
  fnc_ptr jump_to_app;
  jump_to_app = (fnc_ptr)(*(volatile uint32_t*) (FLASH_APP_START_ADDRESS+4u));
  uart_deinit();
  /* Change the main stack pointer. */
  asm volatile("msr msp, %0"::"g"(*(volatile uint32_t *)FLASH_APP_START_ADDRESS));
  jump_to_app();
}

