TLDR
----

To generate a new firmware:

- Copy `config.yml` and `network.yml` files from `templates` folder
  (removing `.tpl` extension) to the root of the repository
  (`ipmc/trunk` for example or in some branch).

- Adjust above files as needed.

- Execute `make` on the root of the repository.


The CERN IPMC
-------------

The CERN IPMC solution is based on the Pigeon Point design. As Pigeon
Point limits the access to their resources by a Non-Disclosure
Agreement (NDA), the CERN IPMC project provides callback hooks, which
with designers are able to customize their IPMC firmware as needed.

At this moment, this project is making use of:
- TCP/IP communication for a command line interface implementation
- Read and write of GPIO signals
- I2C transactions (I2C Mux, Zynq, MCU, EEPROM and temperature sensors)
    - I2C Mux
    - Zynq
    - MCU
    - EEPROM
    - temperature sensors
- I2C watchdog (reset I2C Mux if bus is stuck)

Future plans will include:
- JTAG

A reference implementation is provided by the CERN IPMC project and it
can be accessed by the following link:

    https://gitlab.cern.ch/ep-ese-be-xtca/ipmc-project


Building CERN IPMC firmware
---------------------------

All instructions below assume the working directory is

    common-atca-blade.firmware/ipmc/trunk

Moving files to different places or executing commands from different
paths might produce unexpected results (meaning errors).

- Must be member of CERN e-group epesebe-ipmc-forum

    - Subscription can be done in https://e-groups.cern.ch

- Access to the compilation script provided by the CERN IPMC project.

    - This is a compiler script provided by the CERN IPMC project,
      once we don't have access to Pigeon Point code, as explained
      above. It will upload the custom files, run the compilation in a
      server prepared with all the Pigeon Point files, and download
      the necessary files used to program the IPMC.

    - A version of the compile script is stored in this
      repository. Please do not commit any changes to this file. As
      instructed below, you should work with a copy of this file.

    - Original version can be found here:
    https://gitlab.cern.ch/ep-ese-be-xtca/ipmc-project/raw/master/compile.py

- Prepare configuration file

    - The CERN IPMC compile script uses a XML configuration file to
      set commonly used resources, it is named as config.xml. For this
      project, and at this moment, it means mainly LAN and sensors
      configuration.

    - The above metioned XML file will be generated automatically
      during our compilation process, mainly because each compilation
      might be related to a different blade. Therefore, two YAML files
      are needed.

      One of them shall be named as `config.yml` and should be placed
      on the trunk folder. It covers basic compilation configurations:

          ipmc/trunk/config.yml

      The path to the other YAML file is defined through a variable in
      the first one and it should cover the network information. An
      example template can be found in templates folder:

        ipmc/trunk/templates/config.yml.tpl

        ipmc/trunk/templates/network.yml.tpl

      Case of keys on YAML configuration files are ignored.

      Please, do not commit configuration files.

- Compile the code

    - To compile the code, our project makes use of the remote
    framework provided by the CERN IPMC project.

    - Since version is generated during compilation, a Make recipe. It
      will generate a C header file to be included in the code during
      compilation. It also provides extra rules to ease upgrade,
      activation of the new firmware, as well as to SOL/SDI use. Since
      there are targets to help dealing with the IPMC/Shelf Manager,
      an environment file "env" is needed by the make recipe. There is
      also an example of it in the templates folder. Adjust it as
      required.

    - Under the hood, the Make recipe uses the CERN IPMC compilation
      script provided by the CERN team.

    - The CERN IPMC compilation script is based on Python 3 and
      requires the following libraries (there are below suggestions
      about how to install these libraries, but you should use what
      makes more sense for your system):

        - colorama (pip install colorama)

        - lxml (pip install lxml)

        - requests (pip install requests)

      Since we also use, YAML:
      
        - yaml (pip install PyYAML)

    - There is a read only version of this file in our repository
      named as "compile_ref.py". If you want to make changes, you need
      to make a copy of that. We suggest the name of the changed file
      as "compile.py", which will be automatically ignored by the svn,
      avoiding mistakes.

    - Command to run the compilation flow:

        ipmc/trunk $ make

        or

        ipmc/trunk $ make compile

    - The step above will create a new "config.xml" file in your
      "ipmc/trunk" directory, independently from where you are
      executing the config generator script.

    - NB: The CERN IPMC code is not assuming different IP addresses
          associated to the ATCA slot addresses. You always want to
          use the "default" field.

If the compilation works fine, new files will be available in the
current directory. The only required one is "hpm1all.img".

Load IPMC firmware
------------------

Assuming your blade is inside of an ATCA shelf and that you have the
IP address of the shelf manager, you can use the ipmitool to reprogram
the IPMC firmware.

    $ make upgrade

- Please remember to answer "yes" to the subsequent question raised by
  the ipmitool "Services may be affected during upgrade. Do you wish
  to continue? (y/n):". The IPMI channel will be closed without
  notification if you fail to do that in a reasonable time, which will
  lead you to a long wait until the ipmitool times out.


After having the IPMC successfully programmed, the new firmware needs to be activated:

    $ make activate

Once the new firmware is activated, the IPMC will reboot and the new
network configuration should be used from that point on.

Command line interface
----------------------

In our IPMC firmware, a command line interface is implemented over
TCP/IP on port 2345. This command line interface is used to set and
retrieve value of GPIO signals.

For now, netcat utility is used to connect to it (please make sure you
are using the correct IPMC IP address):

    $ nc <IPMC IP address> 2345

    - When connected for the first time, nothing is returned from
      IPMC, pressing <enter> (empty line) should return a prompt
      marked as ":: ".

    - Case is ignored.

    - Delimiter is space. Extra spaces are ignored.

    - Once connected, any command not recognized is answered with an
      echo of the message sent.

    - There are some documentation directly embedded in the command
      line interface. Try:

        :: help

        :: ?

        :: <command name> help

        :: <command name> ?

    - Some signals should not be easily changed and they are
      protected. To enable this changes:

        :: expert_mode on

        - any other argument will disable this mode (including no argument)

        - after switching this mode on, every command will be followed
          by an notification that it remains enabled. This is an
          effort to remind people that they can cause bad things if
          they are not careful.

        - this command is also described by the help

    - To leave the command line interface, the netcat program should
      be interrupted by pressing C-c (combination of the keys Control
      and C simultaneously).
 

Helper scripts
--------------

There is a "scripts" folder where some helpers are kept.

- debug_listerner.py : opens and prints messages coming from the Debug
  USB-serial port. It excludes some uninteresting (for the moment)
  messages that polute the output.

- ipmc_config_gen.py : Python 3 script to generate configuration file for the
  IPMCs

- cmd.py : this is a Python 3 script to send one command and receive
  one answer from the IPMC. This script is useful if you want to
  repeat commands using the command line prompt from your operating
  system. Usage:

    ipmc/trunk $ python scripts/cmd.py <IPMC IP ADDR> <IPMC commands>

  See some examples below:

    ipmc/trunk $ python scripts/cmd.py help

    ipmc/trunk $ python scripts/cmd.py get_gpio en_12v

    ipmc/trunk $ python scripts/cmd.py expert mode on


GPIO spec
---------

| Name              | IPMC Pin | IPMC IO Name               | Dir | default | notes                                                            |
|-------------------+----------+----------------------------+-----+---------+------------------------------------------------------------------|
| IPMC_ZYNQ_EN      |      198 | USER_IO_3                  | O   |       0 | Start regulator powerup sequence                                 |
| EN_ONE_JTAG_CHAIN |       78 | USER_IO_4                  | O   |       0 | Puts everything on one JTAG chain                                |
| UART_ADDR0        |       79 | USER_IO_5                  | O   |       0 | Selects which device the IPMC UART / FP UART goes to             |
| UART_ADDR1        |      200 | USER_IO_6                  | O   |       0 | Selects which device the IPMC UART / FP UART goes to             |
| ZYNQ_BOOT_MODE0   |      201 | USER_IO_7                  | O   |       1 | Selects which boot source for the ZYNQ (set before IPMC_ZYNQ_EN) |
| ZYNQ_BOOT_MODE1   |       81 | USER_IO_8                  | O   |       1 | Selects which boot source for the ZYNQ                           |
| SENSE_RST         |       82 | USER_IO_9                  | O   |       0 | Reset sense i2c bus mux                                          |
| Mezz2_EN          |      203 | USER_IO_10                 | O   |       0 | NOT USED (jumperable)  In case we don't have a zynq              |
| Mezz1_EN          |      204 | USER_IO_11                 | O   |       0 | NOT USED (jumperable)                                            |
| M24512_WE_N       |       84 | USER_IO_12                 | O   |       1 | Write enable for mgmt i2c bus eeprom                             |
| ETH_SW_PWR_GOOD   |       85 | USER_IO_13                 | I   |       X | Ethernet power good                                              |
| ETH_SW_RESET_N    |       87 | USER_IO_16                 | O   |       1 | reset ethernet switch                                            |
|-------------------+----------+----------------------------+-----+---------+------------------------------------------------------------------|
| EN_12V            |      225 | CFG_PAYLOAD_DCDC_EN_SIGNAL | O   |       X | Enables 12V power. Controlled by IPMC                            |
| FP_LATCH          |      224 | CFG_HANDLE_SWITCH_SIGNAL   | I   |       X | Front Panel handle                                               |
|-------------------+----------+----------------------------+-----+---------+------------------------------------------------------------------|


Questions or comments
---------------------

- What is the maximum length of the data and reply strings in the TCP/IP data handler

- What is the size of the stack?

- What are the values accepted in the second parameter of the signal_set_pin? (SIGNAL_HIGHZ, ...)

    - Those are defines, not enum.




