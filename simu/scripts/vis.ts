
import { Message, StateStruct, StateType, GuiToMain, MainToGui } from './common.js';
import { ProtoDecoder, ProtoPacket, ProtoType } from './proto.js';
import { State, stateStruct } from './struct.js';
import * as xterm from 'xterm';
import * as xtermAddonFit from 'xterm-addon-fit';

declare type Terminal = xterm.Terminal;
declare const Terminal: typeof xterm.Terminal;
declare const FitAddon: { FitAddon: typeof xtermAddonFit.FitAddon; };

let worker: Worker;
let guiWindow: null | GuiToMain = null;
let textEncoder = new TextEncoder();

function formatTime(element, value, strip?: boolean) {
    let date = new Date(1000 * value);
    let str = Math.floor(value / 60 / 60 / 24) + ' ' + date.toISOString().split(/[TZ]/, 2)[1];
    if (strip) {
        str = str.replace(/(^[0 :]+)|(0+$)/g, '').replace(/\.$/g, '');
    }
    element.innerHTML = str;
}
function formatTemp1(element, value) { element.innerHTML = (value || 0).toFixed(1); }
function formatTemp2(element, value) { element.innerHTML = (value || 0).toFixed(2); }
function formatPercent(element, value) { element.innerHTML = (100 * value).toFixed(0) + '%'; }
function formatPlusMinus(element, value) { element.style.fill = Math.round(1000 * value) == 0 ? 'black' : value < 0 ? 'red' : '#06F' }
function formatOnOff(element, value) { element.style.fill = Math.round(1000 * value) == 0 ? '#CCC' : '#4F0' }

let state: State;

let stateFormat = {
    Tele: e => formatTemp1(e, state.Tele),
    Tzadele: e => formatTemp1(e, state.Tzadele),
    running: e => formatOnOff(e, state.running),
    notrunning: e => formatOnOff(e, !state.running),
    Speed: e => { state.speed >= 1 ? formatTime(e, state.speed, true) : (e.innerHTML = '1/' + Math.round(1 / state.speed)) },

    Tpiec: e => formatTemp1(e, state.Tpiec),
    Tpowr: e => formatTemp1(e, state.Tpowr),
    Tzadpiec: e => formatTemp1(e, state.Tzadele), // TODO: piec

    Twejspc: e => formatTemp1(e, state.Twejspc),
    Tspz: e => formatTemp1(e, state.Tspz),
    Twejspz: e => formatTemp1(e, state.Twejspz),
    Tspc: e => formatTemp1(e, state.Tspc),

    Twyj1: e => formatTemp1(e, state.Twyj1),
    Tpodl1: e => formatTemp1(e, state.Tpodl1),
    Twyj2: e => formatTemp1(e, state.Twyj2),
    Tpodl2: e => formatTemp1(e, state.Tpodl2),

    Twyj3: e => formatTemp1(e, state.Twyj3),
    Tzas: e => formatTemp1(e, state.Tzas),

    Tdom: e => formatTemp2(e, state.Tdom),
    Tzewn: e => formatTemp2(e, state.Tzewn),
    Tzaddom: e => formatTemp1(e, state.Tzaddom),
    Twyl1: e => formatTemp2(e, state.Twyl1),
    Twyl2: e => formatTemp2(e, state.Twyl2),

    Z0: e => formatPercent(e, state.Z0),
    Z1: e => formatPercent(e, state.Z1),
    Z2: e => formatPercent(e, state.Z2),
    Z0k: e => formatPlusMinus(e, state.Z0dir),
    Z1k: e => formatPlusMinus(e, state.Z1dir),
    Z2k: e => formatPlusMinus(e, state.Z2dir),
    P0: e => formatOnOff(e, state.P0),
    P1: e => formatOnOff(e, state.P1),
    P2: e => formatOnOff(e, state.P2),
    P3: e => formatOnOff(e, state.P3),
    P4: e => formatOnOff(e, state.P4),

    R0: e => formatOnOff(e, state.R0),
    R1: e => formatOnOff(e, state.R1),
    R2: e => formatOnOff(e, state.R2),
    R3: e => formatOnOff(e, state.R3),
    R4: e => formatOnOff(e, state.R4),
    R5: e => formatOnOff(e, state.R5),
    R6: e => formatOnOff(e, state.R6),
    R7: e => formatOnOff(e, state.R7),
    R8: e => formatOnOff(e, state.R8),
    R9: e => formatOnOff(e, state.R9),
    R10: e => formatOnOff(e, state.R10),
    R11: e => formatOnOff(e, state.R11),
    R12: e => formatOnOff(e, state.R12),
    IN0: e => formatOnOff(e, state.IN0),
    IN1: e => formatOnOff(e, state.IN1),
    IN2: e => formatOnOff(e, state.IN2),

    Czas: e => formatTime(e, state.Time),
}


let stateUpdaters: (() => void)[] = [];

function updateState() {
    for (let updater of stateUpdaters) {
        updater();
    }

    for (let [groupName, group] of Object.entries(stateStruct)) {
        for (let field of group) {
            let element = document.querySelector('#state-' + field.name) as HTMLInputElement;
            if (field.type == 'double') {
                let oldVal = element.getAttribute('data-original-value') || element.value;
                let newVal = (state[field.name] as number).toString();
                if (oldVal != newVal) {
                    element.value = newVal;
                }
            } else {
                let oldVal = element.checked;
                let newVal = state[field.name] as boolean;
                if (oldVal != newVal) {
                    element.checked = newVal;
                }
            }
        }
    }
}

function start() {
    let vis = document.querySelector('#vis') as Element;

    let stateKeys = Object.keys(stateFormat);

    for (let tspan of vis.querySelectorAll('tspan')) {
        let htmlKey = tspan.innerHTML.trim().toLowerCase();
        let stateKey = stateKeys.find(x => x.toLocaleLowerCase() == htmlKey);
        if (stateKey) {
            const format = stateFormat[stateKey];
            stateUpdaters.push(() => format(tspan));
        }
    }

    for (let element of vis.querySelectorAll('[id^=data-]')) {
        let htmlKey = (element.getAttribute('id') as string).substring(5).toLocaleLowerCase();
        let stateKey = stateKeys.find(x => x.toLocaleLowerCase() == htmlKey);
        if (stateKey) {
            const format = stateFormat[stateKey];
            stateUpdaters.push(() => format(element));
        }
    }
}

async function updateSVG() {
    let response = await fetch('vis.svg');
    let text = await response.text();
    (document.querySelector('#svg-container') as Element).innerHTML = text;
    start();
    updateState();
}

let x = false;

function toggle(sel) {
    let e = document.querySelector(sel);
    e.style.fill = x ? '#F00' : '#00F';
    x = !x;
}

function escapeHtml(unsafe: string) {
    return unsafe
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;")
        .replace(/'/g, "&#039;");
}


function createStateTable() {
    let out = '<table class="state">';
    for (let [groupName, group] of Object.entries(stateStruct)) {
        out += `<tr><td colspan="3">${escapeHtml(groupName)}</td></tr>`;
        for (let field of group) {
            let readonly = field.method != 'param' && field.method != 'in' && field.method != 'out'; // TODO: delete OUT
            if (field.type == 'double') {
                out += `<tr><td><input type="text" value="" id="state-${field.name}" ${readonly ? 'readonly' : ''}></td>`;
            } else {
                out += `<tr><td><input type="checkbox" id="state-${field.name}" ${readonly ? 'onclick="return false;"' : ''}></td>`;
            }
            out += `<td>${escapeHtml(field.name)}</td><td>${escapeHtml(field.comment)}</td></tr>`;
        }
    }
    out += '</table>';
    (document.querySelector('#state-container') as Element).innerHTML = out;

    for (let [groupName, group] of Object.entries(stateStruct)) {
        for (let field of group) {
            if (field.type == 'double') {
                textControl('#state-' + field.name, (v: string) => {
                    msg({ type: 'set-state', name: field.name, value: parseFloat(v) });
                }, (v: string) => numberInRange(v, -Infinity, Infinity));
            } else {
                checkControl('#state-' + field.name, (v: boolean) => {
                    msg({ type: 'set-state', name: field.name, value: v });
                });
            }
            msg({ type: 'get-state' });
        }
    }
}

let pd = new ProtoDecoder();
pd.onPacket = (packet: ProtoPacket) => {
    switch (packet.type) {
        case ProtoType.INVALID:
            console.error('Invalid packet');
            break;
        case ProtoType.LOG:
            switch (packet.level) {
                case 0:
                    console.debug(packet.text);
                    break;
                case 1:
                    console.info('%c' + packet.text, 'color: #55F');
                    break;
                default:
                    console.error(packet.text);
                    break;
            }
            if (packet.droppedPackets.filter(x => x).length > 0) {
                console.error(`Dropped packets: ${packet.droppedPackets.join(', ')}`);
            }
    }
}

function workerMessage(e: MessageEvent<Message>) {
    let data = e.data;
    switch (data.type) {
        case 'ready':
            console.log('Worker ready');
            state = data.state;
            createStateTable();
            updateState();
            updateSVG();
            break;
        case 'state':
            state = data.state;
            updateState();
            break;
        case 'comm':
            pd.push(data.buffer);
            break;
        case 'diag':
            if (guiWindow) {
                guiWindow.term.write(data.buffer);
            }
            break;
    }
}

function msg(data: Message) {
    worker.postMessage(data);
}

function textControl(selector: string, acceptFunc?: (v: string) => void, validateFunc?: (v: string) => boolean) {
    let element = document.querySelector(selector) as HTMLInputElement;
    element.onkeydown = (event: KeyboardEvent) => {
        let originalValue = element.getAttribute('data-original-value');
        if (!originalValue) {
            originalValue = element.value;
            element.setAttribute('data-original-value', originalValue);
        }
        if (event.key == 'Escape') {
            element.value = originalValue;
            element.removeAttribute('data-original-value');
        }
        if (validateFunc && !validateFunc(element.value)) {
            element.style.backgroundColor = '#F97';
        } else {
            if (originalValue !== element.value) {
                element.style.backgroundColor = '#FF0';
            } else {
                element.style.backgroundColor = '';
            }
            if (event.key == 'Enter') {
                if (acceptFunc) {
                    acceptFunc(element.value);
                }
                element.removeAttribute('data-original-value');
            }
        }
    };
    element.onkeyup = element.onkeydown;
}

function selectControl(selector: string, acceptFunc?: (v: string) => void) {
    let element = document.querySelector(selector) as HTMLSelectElement;
    element.onchange = (event: Event) => acceptFunc?.(element.value);
}

function checkControl(selector: string, acceptFunc?: (v: boolean) => void) {
    let element = document.querySelector(selector) as HTMLInputElement;
    element.onchange = (event: Event) => acceptFunc?.(element.checked);
}

function buttonControl(selector: string, acceptFunc?: () => void) {
    let element = document.querySelector(selector) as HTMLInputElement;
    element.onclick = (event: Event) => acceptFunc?.();
}

function numberInRange(value: string, start: number, end: number) {
    if (parseFloat(value).toString() != value.trim()) return false;
    if (parseFloat(value) < start || parseFloat(value) > end) return false;
    return true;
}

function diagTermInput(value: string) {
    msg({ type: 'diag', buffer: textEncoder.encode(value) });
}

async function main() {
    worker = new Worker('worker.js', { type: 'module' });
    worker.onmessage = workerMessage;
    worker.onerror = (...e) => console.log(...e);
}

window.onload = () => { window.open('gui.html', 'simuGUI', 'popup'); };

(window as any).guiReady = (data: GuiToMain) => {
    if (guiWindow) {
        document.location.reload();
        return null;
    }
    guiWindow = data;
    main();
    let guiSend: MainToGui = {
        onDiagTermInput(value) {
            diagTermInput(value)
        },
    };
    return guiSend;
}

let windowInterface = {
    updateSpeed(value: number) {
        const speeds = [
            1 / 60, 1 / 30, 1 / 10, 1 / 5, 1 / 2,
            1, 2, 5, 10, 30, 60,
            2 * 60, 5 * 60, 10 * 60, 30 * 60, 60 * 60,
            2 * 60 * 60, 6 * 60 * 60, 12 * 60 * 60, 24 * 60 * 60,
            2 * 24 * 60 * 60, 4 * 24 * 60 * 60, 7 * 24 * 60 * 60,
        ];
        let curr = state.speed;
        let index = speeds.filter(x => x < curr).length;
        if (value > 0) {
            msg({ type: 'set-state', name: 'speed', value: speeds[index < speeds.length - 1 ? index + 1 : index] });
        } else {
            msg({ type: 'set-state', name: 'speed', value: speeds[index > 0 ? index - 1 : index] });
        }
        msg({ type: 'get-state' });
    },
    play() {
        msg({ type: 'start' });
    },
    pause() {
        msg({ type: 'stop' });
        msg({ type: 'get-state' });
    },
    update(name: string, increase: number) {
        msg({ type: 'inc-state', name, value: increase });
        msg({ type: 'get-state' });
    }
}

for (let [name, value] of Object.entries(windowInterface)) {
    window[name] = value;
}


/*
TODO: parametry wejściowe:
 * elektryczny / pellet - fizyczny przełącznik
 * lato / zima - fizyczny przełącznik
 * temp. podł. 1 (min 15, max: max podł - 3)
 * temp. podł. 2
 * temp. min powrotu (min 0, max 70)
 * temp. max podłogówki (min 20, max 50)
 * temp. max pieca (min 50, max 90)
 * temp. w domu (później) (min 10, max 30)
 * Czas i temp. uruchomienia kotła na petel
 * Czy grać wodę elektrycznym
 * Chłodzenie podłogi chłoną wodą wchodzącą do CWU (w przyszłości)

Parametry symulacyjne:
 * Awaria zasilania (na określony czas)
 * Upływ temeratury z CWU
 * Skok w dół temperatury CWU
 * Szybkość nagrzewania się domu
 * Szybkość nagrzewania się podłogi
 * Szybkość nagrzewania się CWU
 * Awarie pompek
 * Awarie czujników
 * Awarie zaworów
 * Awarie kotłów
*/