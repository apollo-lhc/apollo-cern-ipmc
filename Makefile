.SHELLFLAGS = -ec

REVISION := $(shell svn info --show-item revision)
BRANCH := $(shell basename $$(pwd))
DATE := $(shell date)
REVISION_STR := 'static const unsigned char version_str[] = "$(REVISION) $(BRANCH) $(DATE)";'

.ONESHELL:
all:
	@echo $(REVISION_STR) > ipmc-user/user_version_def.h
	echo "Current repository: $(REVISION) $(BRANCH) $(DATE)"
	python ./compile.py

