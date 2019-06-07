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

/* INIT_CALLBACK(fctname) is called during the IPMC initialisation */
INIT_CALLBACK(usermain_init)
{
  gpio_init();
}
