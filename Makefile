.SHELLFLAGS = -ec

REVISION := $(shell svn info --show-item revision)
BRANCH := $(shell basename $$(pwd))
DATE := $(shell date)
VAR := 'static const unsigned char version_str[] ='
VAL := '   "$(REVISION) $(BRANCH) $(DATE)\\n";'

.ONESHELL:
all:
	@echo $(VAR) > ipmc-user/user_version_def.h
	@echo $(VAL) >> ipmc-user/user_version_def.h
	echo "Current repository: $(REVISION) $(BRANCH) $(DATE)"
	python ./compile.py

