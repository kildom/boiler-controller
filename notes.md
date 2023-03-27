
Ideas for GUI:
* https://botland.com.pl/moduly-nanopi/14635-nanopi-neo-v14-allwinner-h3-quad-core-12ghz-512mb-ram-bez-zlaczy-5904422377656.html
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
