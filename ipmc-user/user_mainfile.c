/***********************************************************************

Nom ......... : user_mainfile.c
Role ........ : Example of user function definition
Auteur ...... : Julian Mendez <julian.mendez@cern.ch>
Version ..... : V1.0 - 25/10/2017

Compilation :
	- Add the file into the FILES list located in ipmc-user/nr-mkfrag

***********************************************************************/
#include <defs.h>
#include <cfgint.h>
#include <app.h>

/* Required to control I/Os */
#include <app/signal.h>

#include <user_gpio.h>
#include <user_version.h>
#include <user_zynq.h>

// let's lock Zynq I2C poll before initialization
static char init_lock = 1;


/* INIT_CALLBACK(fctname) is called during the IPMC initialisation */
INIT_CALLBACK(usermain_init)
{
  // init gpios
  user_gpio_init();

  // unlock Zynq I2C poll
  init_lock = 0; 
}


// This is a function dumps gpio states for debug purposes
TIMER_CALLBACK(1s, use_debug_timercback)
{
  static int cnt = 0;

  if (++cnt % 10){
    return;
  }

  unsigned char version[70];
  unsigned char v;

  user_get_version(version);
  debug_printf("Version: %s", version);
  user_dump_gpios();

  if (user_zynq_i2c_read(0x60, 0, &v, 1) == 0) {
    debug_printf("Zynq 0x60: 0x%02x\n", v);
  }
  
  return;
}
