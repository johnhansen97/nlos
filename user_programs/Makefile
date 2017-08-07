#
# NLOS User Program Makefile
# John Hansen
#
# This Makefile will invoke make install/clean for all user programs

PROGRAMS = calculator

.PHONY: install clean

install:
	for program in $(PROGRAMS); do \
	$(MAKE) -C $$program $@; \
	done

clean:
	for program in $(PROGRAMS); do \
	$(MAKE) -C $$program $@; \
	done
