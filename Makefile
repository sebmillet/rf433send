ALL:

clean:
	make -C examples/01_send/ $@
	make -C examples/02_send_otio/ $@
	make -C examples/03_send_short/ $@
	make -C examples/04_rcswitch_send/ $@
	make -C examples/05_send_20220829083700/ $@

mrproper:
	make -C examples/01_send/ $@
	make -C examples/02_send_otio/ $@
	make -C examples/03_send_short/ $@
	make -C examples/04_rcswitch_send/ $@
	make -C examples/05_send_20220829083700/ $@

