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
// #include <user_zynq.h>

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


// This is a function to coordenate the initialization of the Zynq and
// the power negotiation with the shelf manager.
TIMER_CALLBACK(1s, usermain_timercback)
{
  static int cnt = 0;

  unsigned char version[70];

  if (++cnt % 10 == 0){
    user_get_version(version);
    debug_printf("Version: %s\n", version);
    user_dump_gpios();
  }
  
  return;
}
