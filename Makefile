.SHELLFLAGS = -ec

REVISION := $(shell svn info --show-item revision)
BRANCH := $(shell basename $$(pwd))
DATE := $(shell date)
VAR := 'static const unsigned char version_str[] ='
VAL := '   "$(REVISION) $(BRANCH) $(DATE)\\n";'

include env

.ONESHELL:
compile: 
	@echo $(VAR) > ipmc-user/user_version_def.h
	@echo $(VAL) >> ipmc-user/user_version_def.h
	echo "Current repository: $(REVISION) $(BRANCH) $(DATE)"
	python ./compile.py

activate:
	ipmitool -H $(SHM_IP) -P "" -t $(IPMB_ADDR) hpm activate

upgrade: 
	ipmitool -H $(SHM_IP) -P "" -t $(IPMB_ADDR) hpm upgrade hpm1all.img force

sol:
	ipmitool -C 0 -I lanplus -H $(IPMC_IP) -U soluser -P solpassword sol activate

reset:
	ipmitool -H $(SHM_IP) -P "" -t $(IPMB_ADDR) mc reset cold
