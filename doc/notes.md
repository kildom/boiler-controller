
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
* 230V Inputs (low power):
  * https://www.falstad.com/circuit/circuitjs.html?ctz=CQAgjCAMB0l3BWcA2aAOMB2ALGXyEw1sESRkIkFIQEBTAWjDACgA3EbbNEAJgWSduIAMyRsUcLRpiacqNAQsA5iEzFRG9ROwjekyCwDG5XhP6DS5gZLCMeYAJzpscZI7SRMIsJGQ9DEzQ9UXEQYP0RDTlYeEhHBMSk5McFdwRHZExINF4REmwCESgWACdaLlEEfWQcqv0aXngWAAcQR0qxCQ6JKN6DFgB3cJC+9s7oofHesIjQiUNy3xsu8DAm+clXQ2HfOtXlwVWdtY2D9ZkRQRPmM409njHDDi4eJp5XvhsaCH8IXwMCiUABNyJhItU1GcwvpgXQAGYAQwArgAbAAuUyyEJq+0hJ3BMjC2k2J2QZi+gmxlJKw3JvS0GieLAAzkJHpCrPVAUjUSy6FMSWM5szhkLZqMwoY2npGvszpDinIprK+HVPu9aaIFfpPiJ8SxQdSLFCieYQHCkWjMUaKatqfr9LCESiMYbTDMJMabM6rW66XaNPTSex2Zs9fi+BVKICYEp8iAAF50AB2dFKxX+0EweEdGEcvAw8F4jEwkl4sTiVfgTPALET4Dgj3uTc2+mTaYzUx6mweId2rbGPeZrjLt0ugnWwhEVzrqnWYCO-nAvEXnBCyqWq6Os4XghNNGocBYACUV2uTXu+BT5K5RNAlQZFFMrzPJ2ZHrPDABZcB6T+TiImA7vu4DZiwACSK7Tru-6iF+QK0C+FyiMu46aAsUwanUXKaicuE4ZUB4sAA9rQKC2IWkDFD4IjQI4wH6ouq6FLwqQwPAECNHwXGkaIFE-FRSrZgIng5mx-AkBQ1yxNQ1QIPJHTeNgjhrhAkSiHxDjXCunhcZW+nwJgCSEF4yBASIjjUMgoH6I8LBAA
  * From LED lamp: https://www.falstad.com/circuit/circuitjs.html?ctz=CQAgjCAMB0l3BWcBOaAOAbAdgCwCYsBmHMBMHZNQkBPGmyGgUwFowwAoANxDwQ179BAwpBxRwDEKMayo0BBwDmILGnGF1qrTkJ1ZHAMara0sSbqbxs2PEjIHjp8+QTIHAE4hkODeaqW5ozEHADu4MQapmCR0lru4WCQQlbgcGhx1tzCvJAZfAIFEhAYaBBJbvKKACYg2JamWHjB5nTVTABmAIYArgA2AC5hdU3Sphh5Y-rDTS3iWKaiWeET+UL1OQl1eBpaC5bxHADOOYSL5-oS3X1HTDNaqQGZUDOL-npmWQAO4BO5GWA-mdLG5hoDGM01oVJltwVNTqZ3LUNkVZp9eCB2t1+kNkTt0RtgRisb1Bhw8X5xCihG1OqShit8akMEygtkiksEZc6AgSPQ5DBFOEfJS0pNObD0s8Rc93DhIFgIr5nuRRqlOCpyPKQCQBFrGDg0ALPOBfMEtPr0YwEPAOAAlU3azmWgiXA3BaDUAUKMFmlU4NWHACy4AQiwtE3N4joYGgWA4AElTYHxKRw9YqjQwbFdUrxLppuEipC6pMS+5ATHkiJCHrq89adjBiw+kxahBvZBOF4no8PhLpJG4MMKCJTPxgoiOJXfoCVRg5wXiXScS225JO5wZ0kaWgAfXCLXl02Bmv25UYF3pyy0jTtTvCnvj-Szxu3LAtzf2OZ1UkNEfGxfVtz03a8Yz-aQj2-aMn0A1dgLfGwr23CC8CfaDeG1ODmwQjt32Qr9SEsKCiIbTEVxw9c8KQz9wNMNCAVIw9CnIk9X2o+QCLougdkYdh6Ng1igKoi8PzA8BmghdDJMwiEhPgkTQO3GTmIkuBeEEkkFJA-DOESetePAVDtVhCDf3TF5ElIwywBkwzYRUqCdgyVSHMNSC9Wc-5LIk9ySySMsYTBKV-KlCUTVHeFIqKYIh3cYwJxyRKiQhT07HS9K8FYRUDHCIl8vooQtmilJCoELZVm8yqYuGZLxnFKdwkSooSvKjhCCwRgZT7A5U1BcJ9meNRdiyAB7UNfmKNDIC9Wwu04uA8M4cav3KnV7FcS94CwFlkEIHwEEIWykEvY7JDoG9CA4caYzW+UHFycBoBYnjpA4IA

Rapsberry PI configuration:
 * Serial port `/dev/serial0` - only data (not console). Probably must be set by `sudo raspi-config` (interface options)
 * BT not needed: `dtoverlay=disable-bt` in `boot/config.txt`
 * Controller should be able to reset Rapsberry PI if no communication for some time.
   * It will first set GPIO and some service will reboot the system `sudo reboot`.
   * If still no response, use hard reset.
 * Partitions:
   * boot
   * Normal OS
   * Recovery OS 
 * Two OSes - Selection using switch
   * GPIO detection at early OS startup stage
   * `/boot/cmdline.txt` can select partition
   * RegEx replace: `(root=PARTUUID=[0-9a-fA-F]{8}-[0-9]*)[0-9]` => `$1N`
     * where `N` is partition number starting from 1
     * Numbers can be listed by: `sudo fdisk -x /dev/mmcblk0`
   * if `/boot/cmdline.txt` was changed `sudo reboot` should be executed.
 * Normal OS
   * Controller server + zrok tunnel
   * Mantainace options in controller server:
     * Reboot
     * Change WiFi SSID and passwort and reboot
   * SSH + ttyd over HTTP
 * Recovery OS
   * Creates an access point with constant IP address
   * Enables SSH + ttyd over HTTP
   * Simple password for WiFi and SSH
   * Simple HTTP interface (maybe Python) with:
     * OS selection + reboot
     * WiFi SSID and password (for Normal OS) + reboot
     * Normal OS partition image upload ??? maybe
     * Start Controller server (from Normal OS partition)
   * no zrok tunnel
   * controller will be also connected to recovery switch and it will not reset the Recovery OS
 * Cross zrok for ARMv6:
   * Install gcc: `sudo apt-get install gcc-arm-linux-gnueabi` (but make sure libc <= GLIBC_2.31)
   * Install go: `sudo snap install go --classic`, `sudo snap install goreleaser --classic`
   * Build UI from `BUILD.md`
   * build bin: `goreleaser release --clean --skip-publish --skip-validate --config .goreleaser-linux-armv6.yml`

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
