.SHELLFLAGS = -ec

REVISION := $(shell svn info --show-item revision)
BRANCH := $(shell basename $$(pwd))
REVISION_STR := 'static const char version_str[] = "$(REVISION) $(BRANCH)";'


.ONESHELL:
all:
	@echo $(REVISION_STR) > ipmc-user/user_version_def.h
	echo "Current repository: $(REVISION) $(BRANCH)"
	python ./compile.py

