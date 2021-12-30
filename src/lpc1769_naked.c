
#include "driver.h"
#include "chip.h"


/* Pin muxing configuration */
static const PINMUX_GRP_T led_pinmux =
  {ONBOARD_LED_PN,  ONBOARD_LED_PIN,  IOCON_MODE_PULLDOWN | IOCON_FUNC1} /* LED */;


#if defined(TRINAMIC_ENABLE) && (TRINAMIC_ENABLE == 2130 || TRINAMIC_ENABLE == 5160)

#define ASSERT_CS_PIN(driver_id) \
		cs[driver_id].port->CLR = cs[driver_id].bit; \
		__NOP(); __NOP();

#define DEASSERT_CS_PIN(driver_id) \
		__NOP(); __NOP(); \
		cs[driver_id].port->SET = cs[driver_id].bit;

#include <string.h>
#include "trinamic/common.h"
#include "motors/trinamic.h"
#include "ssp_17xx_40xx.h"
#include "gpio_17xx_40xx.h"

static struct {
    LPC_GPIO_T *port;
    uint32_t bit;
} cs[TMC_N_MOTORS_MAX];


static uint8_t buffer[sizeof(TMC_spi_datagram_t)];
static const uint8_t BUFFER_LEN = sizeof(buffer);
static const uint8_t TMC_SPI_WRITE_FLAG = 0x80u;

/* Pin muxing configuration */
static const PINMUX_GRP_T sspi_pinmux[] = {
    {TMC_SPI_SCK_PORT_PN,  TMC_SPI_SCK_PIN,  IOCON_MODE_PULLDOWN | IOCON_FUNC3}, /* SCK  */
    //{TMC_SPI_CS_X_AXIS_PORT_PN, TMC_SPI_CS_X_AXIS_PIN, IOCON_MODE_INACT | IOCON_FUNC0}, /* CS_X */
	//{TMC_SPI_CS_Y_AXIS_PORT_PN, TMC_SPI_CS_Y_AXIS_PIN, IOCON_MODE_INACT | IOCON_FUNC0}, /* CS_Y */
	{TMC_SPI_MISO_PORT_PN, TMC_SPI_MISO_PIN, IOCON_MODE_PULLUP | IOCON_FUNC3}, /* MISO */
	{TMC_SPI_MOSI_PORT_PN, TMC_SPI_MOSI_PIN, IOCON_MODE_INACT | IOCON_FUNC3}  /* MOSI */
};

inline static void delay (void)
{
    volatile uint32_t dly = 10;

    while(--dly)
        __NOP();
}

TMC_spi_status_t tmc_spi_write (trinamic_motor_t driver, TMC_spi_datagram_t *datagram)
{
	TMC_spi_status_t status;
	buffer[0] = datagram->addr.value | TMC_SPI_WRITE_FLAG;
	memcpy(buffer+1, &datagram->payload.data[0], sizeof(TMC_payload_t));
	buffer[4] = datagram->payload.data[0];
	buffer[3] = datagram->payload.data[1];
	buffer[2] = datagram->payload.data[2];
	buffer[1] = datagram->payload.data[3];


	ASSERT_CS_PIN(driver.id);
	Chip_SSP_WriteFrames_Blocking(TMC_SPI, buffer, BUFFER_LEN);
	DEASSERT_CS_PIN(driver.id);

	status = buffer[0];
	return status;
}

TMC_spi_status_t tmc_spi_read (trinamic_motor_t driver, TMC_spi_datagram_t *datagram)
{
	TMC_spi_status_t status;

	// first step: send address to read from
	buffer[0] = datagram->addr.idx;

	ASSERT_CS_PIN(driver.id);
	Chip_SSP_WriteFrames_Blocking(TMC_SPI, buffer, BUFFER_LEN);
	DEASSERT_CS_PIN(driver.id);

	delay();

	// second step: get response
	ASSERT_CS_PIN(driver.id);
	Chip_SSP_ReadFrames_Blocking(TMC_SPI, buffer, BUFFER_LEN);
	DEASSERT_CS_PIN(driver.id);

	status = buffer[0];
	datagram->payload.data[0] = buffer[4];
	datagram->payload.data[1] = buffer[3];
	datagram->payload.data[2] = buffer[2];
	datagram->payload.data[3] = buffer[1];

	return status;
}

static void add_spi_cs_pin (xbar_t *gpio)
{
    if(gpio->group == PinGroup_MotorChipSelect) {
        switch(gpio->function) {

            case Output_MotorChipSelectX:
                cs[X_AXIS].bit = gpio->bit;
                cs[X_AXIS].port = (LPC_GPIO_T *)gpio->port;
                break;
            case Output_MotorChipSelectY:
                cs[Y_AXIS].bit = gpio->bit;
                cs[Y_AXIS].port = (LPC_GPIO_T *)gpio->port;
                break;
            case Output_MotorChipSelectZ:
                cs[Z_AXIS].bit = gpio->bit;
                cs[Z_AXIS].port = (LPC_GPIO_T *)gpio->port;
                break;
#if N_AXIS > 3
            case Output_MotorChipSelectM3:
                cs[A_AXIS].bit = gpio->bit;
                cs[A_AXIS].port = (LPC_GPIO_T *)gpio->port;
                break;
#endif
#if N_AXIS > 4
            case Output_MotorChipSelectM4:
                cs[B_AXIS].bit = gpio->bit;
                cs[B_AXIS].port = (LPC_GPIO_T *)gpio->port;
                break;
#endif
#if N_AXIS > 5
            case Output_MotorChipSelectM5:
                cs[C_AXIS].bit = gpio->bit;
                cs[C_AXIS].port = (LPC_GPIO_T *)gpio->port;
                break;
#endif
            default:
                break;
        }
    }
}

void sspiInit(uint32_t bitRate)
{
	Chip_IOCON_SetPinMuxing(LPC_IOCON, sspi_pinmux, sizeof(sspi_pinmux) / sizeof(PINMUX_GRP_T));

	Chip_SSP_Init(TMC_SPI);
	Chip_SSP_SetFormat(TMC_SPI, SSP_BITS_8, SSP_FRAMEFORMAT_SPI, SSP_CLOCK_MODE3);
	Chip_SSP_SetBitRate(TMC_SPI, bitRate);
	Chip_SSP_Enable(TMC_SPI);
}
#endif

static void if_init(uint8_t motors, axes_signals_t enabled)
{
    static bool init_ok = false;
	UNUSED(motors);

    if(!init_ok)
    {
        init_ok = true;
    	sspiInit(1000000);
        hal.enumerate_pins(true, add_spi_cs_pin);
    }
}


void board_init (void)
{
	static trinamic_driver_if_t driver_if = {
	    .on_drivers_init = if_init
	};

	trinamic_if_init(&driver_if);

	Chip_IOCON_SetPinMuxing(LPC_IOCON, &led_pinmux, sizeof(led_pinmux) / sizeof(PINMUX_GRP_T));

	ONBOARD_LED_PORT->DIR |= (1u << ONBOARD_LED_PIN);
	DIGITAL_PIN_OUT(ONBOARD_LED_PORT, ONBOARD_LED_PIN, 1);
}
