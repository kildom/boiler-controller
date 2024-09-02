import * as xterm from 'xterm';
import * as xtermAddonFit from 'xterm-addon-fit';
import { GuiToMain, MainToGui } from './common.js';


declare type Terminal = xterm.Terminal;
declare const Terminal: typeof xterm.Terminal;
declare const FitAddon: { FitAddon: typeof xtermAddonFit.FitAddon; };

let term: Terminal;
let fitAddon: xtermAddonFit.FitAddon;
let mainWindow: MainToGui | null;

function diagTermInput(value: string) {
    if (mainWindow) {
        mainWindow.onDiagTermInput(value);
    }
}

function initTerm() {
    let container = document.querySelector<HTMLElement>('#terminal') as HTMLElement;
    term = new Terminal();
    fitAddon = new FitAddon.FitAddon();
    term.loadAddon(fitAddon);
    term.onData(diagTermInput);
    term.open(container);
    fitAddon.fit();
    window.addEventListener('resize', () => {
        fitAddon.fit();
    });
}


async function main() {
    initTerm();
    let send: GuiToMain = {
        term,
    }
    mainWindow = window.opener.guiReady(send);
    if (!mainWindow) {
        document.body.innerHTML = '';
    }
}

window.onload = () => { main(); };
