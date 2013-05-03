SUBDIRS := firmware firmware_loader

.PHONY: all $(SUBDIRS)

all:	$(SUBDIRS)

$(SUBDIRS):
	$(MAKE) --directory=$@

upload configure monitor: all
	$(MAKE) --directory=firmware $@

clean distclean:
	for d in $(SUBDIRS); do $(MAKE) --directory=$$d $@; done

