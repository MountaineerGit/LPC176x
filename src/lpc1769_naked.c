



#include "driver.h"
#include "chip.h"


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
}
