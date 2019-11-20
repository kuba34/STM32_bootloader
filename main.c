#include "stm32f103xb.h"
#include <stdint.h>

#define _RAM __attribute__((section (".data#"), noinline))
#define SCB_AIRCR   ((uint32_t*)0x0CUL)
#define PAGE_SIZE   1024

static _RAM uint8_t Flash_Page_Erase(uint32_t *address);
static _RAM uint8_t Flash_Page_Write(uint32_t *address, uint32_t *data);

void system_Config(void);
void SPI_Config(void);
void Error_Handler(void);
uint8_t SPI_Transmit(uint8_t data);
uint8_t SPI_Receive(uint8_t *data);
void reset(void);
void (*app_start)(void) = 0x00UL;

SPI_TypeDef *SPI = SPI1;

uint32_t to_flash[PAGE_SIZE];

int main()
{
    const unsigned KEY1 = 0x45670123;
    const unsigned KEY2 = 0xCDEF89AB;

    // Unlock flash
    FLASH->KEYR = (KEY2<<16U|KEY1);
    if(!!(FLASH->CR&FLASH_CR_LOCK)) Error_Handler();
}

void SPI_Config()
{
    // Enable clock
    RCC_APB2ENR |= RCC_APB2ENR_SPI1EN;
    SPI->CR1    |= SPI_CR1_BR|SPI_CR1_LSBFIRST|SPI_CR1_SSM;
    SPI->CR2    |= SPI_CR2_RXNEIE;
}

uint8_t SPI_Transmit(uint8_t data)
{
    // Check if Tx buffer is empty
    if(!(SPI->CR2&SPI_SR_TXE)) return 0;
    SPI->DR = (uint32_t)data;
    return 1;
}

uint8_t SPI_Receive(uint8_t *data)
{
    // Check if Rx buffer is not empty
    if(!!(SPI->CR2&SPI_SR_RXNE)) return 0;
    *data = (uint8_t)SPI->DR;
    return 1;
}

static uint8_t Flash_Page_Erase(uint32_t *address)
{
    while(FLASH->SR & FLASH_SR_BSY);
    FLASH->CR |= FLASH_CR_SER;

    FLASH->AR = address;
    FLASH->CR |= FLASH_CR_STRT;
}

void reset()
{
    *SCB_AIRCR |= (0xFA05<<16);
    *SCB_AIRCR |= (1<<2);
}

void Error_Handler()
{
    while(1);
}
