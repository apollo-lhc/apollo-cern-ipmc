.SHELLFLAGS = -ec

VERSIONING := $(shell git describe --tags)
BRANCH := $(shell git rev-parse --abbrev-ref HEAD)
DATE := $(shell date)
VAR := 'static const unsigned char version_str[] ='
VAL := '   "$(VERSIONING) $(BRANCH) $(DATE)";'

TGT := $(shell ls -t built | head -n 1)

.ONESHELL:
compile: 
	@echo $(VAR) > ipmc-user/user_version_def.h
	@echo $(VAL) >> ipmc-user/user_version_def.h
	echo "Current repository: $(VERSIONING) $(BRANCH) $(DATE)"
	$(RM) *.img

	python3 scripts/ipmc_config_gen.py
	python3 ./compile.py
	python3 scripts/wrapping_up.py

activate:
	ipmitool -H $(SHM_IP) -P "" -t $(IPMB_ADDR) hpm activate

upgrade: 
	ipmitool -H $(SHM_IP) -P "" -t $(IPMB_ADDR) hpm upgrade hpm1all.img force activate

sol:
	ipmitool -C 0 -I lanplus -H $(IPMC_IP) -U soluser -P solpassword sol activate

reset:
	ipmitool -H $(SHM_IP) -P "" -t $(IPMB_ADDR) mc reset cold

cp_pcptracker:
	scp "built/$(TGT)"  "pcuptracker001:built"

cp_lxplus:
	scp "built/$(TGT)" "lxplus.cern.ch:built"

cp_axion:
	scp "built/$(TGT)" "axion.bu.edu:built"

clean:
	@rm -rf hpm*.img >& /dev/null
	@rm -rf config.xml >& /dev/null
	@rm -rf nvm.ihx >& /dev/null
