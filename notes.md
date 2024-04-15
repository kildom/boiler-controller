
Ideas for GUI:
* node.js or deno HTTP server: serving static pages and do proxy between UART and WebSockets (packet level multiplexing, so multiple clients can run at once)
* Remote control:
  * https://zrok.io/ - tunneling with static domain name


Hardware:

* SBC

  * Orange Pi Zero: http://www.orangepi.org/html/hardWare/computerAndMicrocontrollers/service-and-support/Orange-Pi-Zero-3.html
  * or Banana Pi Zero: https://wiki.banana-pi.org/Banana_Pi_BPI-M2_ZERO
    * https://www.conrad.pl/pl/p/banana-pi-m2-zero-banana-pi-bpi-zero-512-mb-4-x-1-2-ghz-1646892.html
    * U.FL/IPX/IPEX 1 to RP-SMA: https://allegro.pl/oferta/adapter-antenowy-pigtail-u-fl-ipx-ipex-rp-sma-15cm-13535644466
  * 2 x https://www.tme.eu/pl/details/ds1023-05-1x8b816/listwy-i-gniazda-kolkowe/connfly/ds1023-05-1-8b8-a16-0-b6-8/
    * Header 1: Comm: 5V, 5V, GND, Rx, Tx
    * Header 2: Debug: GND, Rx, Tx - maybe over cable: https://www.tme.eu/pl/details/zl263-3sg/listwy-i-gniazda-kolkowe/connfly/ds1024-1-3r0/
    * Header 3: N/C - just holers
  * SD card
  * Node.js:
    * https://unofficial-builds.nodejs.org/download/release/v18.20.2/
    * https://github.com/sdesalas/node-pi-zero
 
* STM32 Nucleo
  * https://os.mbed.com/platforms/ST-Nucleo-L552ZE-Q/
  * diag cable: https://www.tme.eu/pl/details/cu0120/kable-i-adaptery-usb/logilink/
  * header 2x8: https://www.tme.eu/pl/details/zl262-16dg/listwy-i-gniazda-kolkowe/connfly/ds1023-2-8s21/
  * header 2x20: https://www.tme.eu/pl/details/zl262-40dg/listwy-i-gniazda-kolkowe/connfly/ds1023-2-20s21/
  * temp ref. resistor: https://www.tme.eu/pl/details/mp0.6w-2k21/rezystory-tht/royal-ohm/mf006bb2211a10/
  * temp prot. resistor: any >=4K, >= 400V / diode forward current
  * temp prot. diodes: Reverse current < 1uA
  * tmep cap: any >=100nF
  * the same filtering for digital input (active low) like for analog, but maybe with bigger pullup res.
  * Current matsure:
    * https://www.tme.eu/pl/details/ppas100/przekladniki-pradowe/talema/as100/
    * https://www.tme.eu/pl/details/ppas105/przekladniki-pradowe/talema/as105/
  * https://www.tme.eu/pl/details/ds1052-202b2ma2030/kable-wstazkowe-ze-zlaczami-idc/connfly/ds1052-202b2ma203001/

* Relay array
  * https://botland.com.pl/przekazniki-przekazniki-arduino/6940-modul-przekaznikow-16-kanalow-z-optoizolacja-styki-10a250vac-cewka-5v-5904422359911.html

* Case
  * https://kamami.pl/obudowy/1179821-z26-ps-obudowa-plastikowa.html
  * https://allegro.pl/oferta/obudowa-kradex-z95j-12156497710
  * https://allegro.pl/oferta/z-90j-obudowa-z-tworzywa-225-x-175-x-80-mm-jasna-11970558491
  * https://3d-innowacje.pl/sklep/zamow-druk-3d/
* Current
  * https://www.tme.eu/pl/details/ax-0500/przekladniki-pradowe/talema/
 
Old ideas for GUI:
* https://botland.com.pl/moduly-nanopi/14635-nanopi-neo-v14-allwinner-h3-quad-core-12ghz-512mb-ram-bez-zlaczy-5904422377656.html
* https://allegro.pl/oferta/mikrokomputer-orange-pi-zero-512mb-13680197462
* OR: https://www.conrad.pl/pl/p/banana-pi-m2-zero-banana-pi-bpi-zero-512-mb-4-x-1-2-ghz-1646892.html
* with node.js or deno HTTP server: serving static pages and do proxy between UART and WebSockets (packet level multiplexing, so multiple clients can run at once)
* Local control:
  * https://allegro.pl/oferta/wyswietlacz-dotykowy-tft-lcd-2-4-ili9341-12736668886
  * `startx -- `which Xvfb` :2 -fbdir /tmp/fb -screen 0 320x240x16` or simply call `Xvfb` directly. TODO: add option to remove cursor.
  * `/tmp/fb` will contain a file with shared memory (format `.xwd`) - watch for shared memory changes and send it to display over SPI
  * `DISPLAY=:2 firefox -kiosk -private-window http://localhost/lcd.html`
  * Touch emulation: `DISPLAY=:2 xdotool mousemove 10 10` `DISPLAY=:2 xdotool click 1`
  * Optionally check existing driver: https://github.com/notro/fbtft, https://github.com/juj/fbcp-ili9341
* Or simpler local control: buy used phone and use WiFi and browser.
* Remote control:
  * `http://192.168.10.109/` - login required if remote addr != 127.0.0.1
  * Cloudflare for remote tunneling (if not expensive domain): https://www.youtube.com/watch?v=ZvIdFs3M5ic
