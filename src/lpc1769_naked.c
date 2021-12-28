



#include "driver.h"
#include "chip.h"


/* Pin muxing configuration */
static const PINMUX_GRP_T led_pinmux =
  {ONBOARD_LED_PN,  ONBOARD_LED_PIN,  IOCON_MODE_PULLDOWN | IOCON_FUNC1} /* LED */;


static void if_init(uint8_t motors, axes_signals_t enabled)
{

	UNUSED(motors);

}

void driver_preinit (motor_map_t motor, trinamic_driver_config_t *config)
{
    config->address = 0;
}

void board_init (void)
{
	static trinamic_driver_if_t driver_if = {
	    .on_drivers_init = if_init,
        .on_driver_preinit = driver_preinit
	};

	trinamic_if_init(&driver_if);

	Chip_IOCON_SetPinMuxing(LPC_IOCON, &led_pinmux, sizeof(led_pinmux) / sizeof(PINMUX_GRP_T));

	ONBOARD_LED_PORT->DIR |= (1u << ONBOARD_LED_PIN);
	DIGITAL_PIN_OUT(ONBOARD_LED_PORT, ONBOARD_LED_PIN, 1);
}
