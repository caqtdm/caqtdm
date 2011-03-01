SUBDIRS = AccStat Analog Analyze_NEW Disco epicsLib Geiss HFdisp IntlockNew IQreset Malus Oraprogs pc104New pidControl pidStrom pioGateway procUCN Scan scl Survey SzentNew MEDM Tendis Touchp Transprofs Verdis

.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@

Scan: Transprofs


SUBDIRS_CLEAN = $(patsubst %,%.clean,$(SUBDIRS))
.PHONY: $(SUBDIRS_CLEAN)
$(SUBDIRS_CLEAN):
	@make -C $(@:.clean=) clean
.PHONY: clean
clean: $(SUBDIRS_CLEAN)