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
#include <string.h>
#include "chip.h"
#include "motors/trinamic.h"

static uint8_t buffer[sizeof(TMC_spi_datagram_t)];
static const uint8_t BUFFER_LEN = sizeof(buffer);
static const uint8_t TMC_SPI_WRITE_FLAG = 0x80u;

TMC_spi_status_t tmc_spi_write (trinamic_motor_t driver, TMC_spi_datagram_t *datagram)
{
	TMC_spi_status_t status;
	buffer[0] = datagram->addr.value | TMC_SPI_WRITE_FLAG;
	memcpy(buffer+1, &datagram->payload.data[0], sizeof(TMC_payload_t));

	DIGITAL_PIN_OUT(TMC_SPI_SSEL_PORT, TMC_SPI_SSEL_PIN, 0);
	__NOP();
	__NOP();
	Chip_SSP_WriteFrames_Blocking(TMC_SPI, buffer, BUFFER_LEN);
	__NOP();
	__NOP();
	DIGITAL_PIN_OUT(TMC_SPI_SSEL_PORT, TMC_SPI_SSEL_PIN, 1);

	status = buffer[0];
	return status;
}

TMC_spi_status_t tmc_spi_read (trinamic_motor_t driver, TMC_spi_datagram_t *datagram)
{
	TMC_spi_status_t status;

	// first step: send address to read from
	buffer[0] = datagram->addr.idx;

	DIGITAL_PIN_OUT(TMC_SPI_SSEL_PORT, TMC_SPI_SSEL_PIN, 0);
	__NOP(); __NOP();
	Chip_SSP_WriteFrames_Blocking(TMC_SPI, buffer, BUFFER_LEN);
	__NOP(); __NOP();
	DIGITAL_PIN_OUT(TMC_SPI_SSEL_PORT, TMC_SPI_SSEL_PIN, 1);

	__NOP(); __NOP(); __NOP(); __NOP();	__NOP(); __NOP(); __NOP();

	// second step: get response
	DIGITAL_PIN_OUT(TMC_SPI_SSEL_PORT, TMC_SPI_SSEL_PIN, 0);
	__NOP(); __NOP();
	Chip_SSP_ReadFrames_Blocking(TMC_SPI, buffer, BUFFER_LEN);
	__NOP(); __NOP();
	DIGITAL_PIN_OUT(TMC_SPI_SSEL_PORT, TMC_SPI_SSEL_PIN, 1);

	status = buffer[0];
	memcpy(&datagram->payload.data[0], buffer+1, sizeof(TMC_payload_t));

	return status;
}
