/*
 * trinamic_spi.c
 * overwrites for weak SPI interface for Trinamic stepper drivers
 * init for SPI interface
 *
 * v0.0.1 / 2021-12-28 / MountaineerGit
 */

/*

Copyright (c) 2021, MountaineerGit
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its contributors may
be used to endorse or promote products derived from this software without
specific prior written permission..

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "chip.h"
#include "motors/trinamic.h"


TMC_spi_status_t tmc_spi_write (trinamic_motor_t driver, TMC_spi_datagram_t *datagram)
{
    return 0;
}

TMC_spi_status_t tmc_spi_read (trinamic_motor_t driver, TMC_spi_datagram_t *datagram)
{
	DIGITAL_PIN_OUT(TMC_SPI_SSEL_PORT, TMC_SPI_SSEL_PIN, 0);

	Chip_SSP_Int_FlushData(TMC_SPI);
	uint32_t rx_cnt = 0, tx_cnt = 0;

	static uint8_t buffer[sizeof(TMC_spi_datagram_t)] = {0};
	const uint8_t buffer_len = sizeof(buffer);

	buffer[0] = datagram->addr.idx;

	while (tx_cnt < buffer_len || rx_cnt < buffer_len)
	{
		/* write data to buffer */
		if ((Chip_SSP_GetStatus(TMC_SPI, SSP_STAT_TNF) == SET) && (tx_cnt < buffer_len)) {
			Chip_SSP_SendFrame(TMC_SPI, buffer[tx_cnt]);	/* just send dummy data		 */
			tx_cnt++;
		}

		/* Check overrun error */
		if (Chip_SSP_GetRawIntStatus(TMC_SPI, SSP_RORRIS) == SET) {
			return ERROR;
		}

		/* Check for any data available in RX FIFO */
		while (Chip_SSP_GetStatus(TMC_SPI, SSP_STAT_RNE) == SET && rx_cnt < buffer_len)
		{
			if(rx_cnt == 0) {
				datagram->status = Chip_SSP_ReceiveFrame(TMC_SPI);
			}
			else {
				datagram->payload.data[rx_cnt] = Chip_SSP_ReceiveFrame(TMC_SPI);
			}
			rx_cnt++;
		}
	}

	DIGITAL_PIN_OUT(TMC_SPI_SSEL_PORT, TMC_SPI_SSEL_PIN, 1);

	return datagram->status;
}
