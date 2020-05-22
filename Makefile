.SHELLFLAGS = -ec

VERSIONING := $(shell git describe --tags)
BRANCH := $(shell git rev-parse --abbrev-ref HEAD)
DATE := $(shell date)
VAR := 'static const unsigned char version_str[] ='
VAL := '   "$(VERSIONING) $(BRANCH) $(DATE)\\n";'

.ONESHELL:
compile: 
	@echo $(VAR) > ipmc-user/user_version_def.h
	@echo $(VAL) >> ipmc-user/user_version_def.h
	echo "Current repository: $(VERSIONING) $(BRANCH) $(DATE)"
	$(RM) *.img

	python scripts/ipmc_config_gen.py
	python ./compile.py
	python scripts/wrapping_up.py

activate:
	ipmitool -H $(SHM_IP) -P "" -t $(IPMB_ADDR) hpm activate

upgrade: 
	ipmitool -H $(SHM_IP) -P "" -t $(IPMB_ADDR) hpm upgrade hpm1all.img force activate

sol:
	ipmitool -C 0 -I lanplus -H $(IPMC_IP) -U soluser -P solpassword sol activate

reset:
	ipmitool -H $(SHM_IP) -P "" -t $(IPMB_ADDR) mc reset cold

cp_pcptracker:
	scp hpm1all.img  pcuptracker001:

cp_lxplus:
	scp hpm1all.img  lxplus.cern.ch:

cp_axion:
	scp hpm1all.img  axion.bu.edu:

clean:
	@rm -rf hpm*.img >& /dev/null
	@rm -rf config.xml >& /dev/null
	@rm -rf nvm.ihx >& /dev/null
