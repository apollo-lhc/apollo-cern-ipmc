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
#include <user_zynq.h>

// let's lock Zynq I2C poll before initialization
static char init_lock = 1;


/* INIT_CALLBACK(fctname) is called during the IPMC initialisation */
INIT_CALLBACK(usermain_init)
{
  // init gpios
  gpio_init();

  // unlock Zynq I2C poll
  init_lock = 0; 
}

// This is a function to coordenate the initialization of the Zynq and
// the power negotiation with the shelf manager.
TIMER_CALLBACK(100ms, usermain_timercback)
{

  // only runs if user init was performed
  if (init_lock == 1) {
    return;
  }

  // this function will execute each 500ms only
  static int cnt = 1;
  if (cnt++ % 5){
    return;
  }

  // get the index of the startup_flag index once for all
  // int startup_flag_idx = get_signal_index("startup_flag");

  unsigned char version;
  if (zynq_read_version(& version)) {
    // reading error
    unprotected_deactivate_gpio(startup_flag);
  } else {
    // reading success
    unprotected_activate_gpio(startup_flag);
  }

  return;
}
