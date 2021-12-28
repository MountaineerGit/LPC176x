/*
 * spi.c
 *
 *  Created on: 28 Dec 2021
 *      Author: MountaineerGit
 */

#include "driver.h"
#include "ssp_17xx_40xx.h"

/* Pin muxing configuration */
static const PINMUX_GRP_T sspi_pinmux[] = {
    {TMC_SPI_SCK_PORT_PN,  TMC_SPI_SCK_PORT_PIN,  IOCON_MODE_INACT | IOCON_FUNC3}, /* SCK  */
    {TMC_SPI_SSEL_PORT_PN, TMC_SPI_SSEL_PORT_PIN, IOCON_MODE_INACT | IOCON_FUNC3}, /* SSEL */
	{TMC_SPI_MISO_PORT_PN, TMC_SPI_MISO_PORT_PIN, IOCON_MODE_INACT | IOCON_FUNC3}, /* MISO */
	{TMC_SPI_MOSI_PORT_PN, TMC_SPI_MOSI_PORT_PIN, IOCON_MODE_INACT | IOCON_FUNC3}  /* MOSI */
};

void sspiInit(uint32_t bitRate)
{

	Chip_IOCON_SetPinMuxing(LPC_IOCON, sspi_pinmux, sizeof(sspi_pinmux) / sizeof(PINMUX_GRP_T));

	Chip_SSP_Init(TMC_SPI);
	Chip_SSP_SetFormat(TMC_SPI, SSP_BITS_8, SSP_FRAMEFORMAT_SPI, SPI_CLOCK_MODE3);
	Chip_SSP_SetBitRate(TMC_SPI, bitRate);
	Chip_SSP_Enable(TMC_SPI);
}
