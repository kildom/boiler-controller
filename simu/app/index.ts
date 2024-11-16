import * as xterm from 'xterm';
import * as xtermAddonFit from 'xterm-addon-fit';
import * as modelWasm from "./model-wasm";
import * as model from "./model";
import "xterm/css/xterm.css";
import { fatal, getSimulationMode, SerialInterface, SimulationMode, wait } from "./simu-common";

let s: any = undefined;
let visSate = 0;
let updaters: (() => void)[] = [];

const speedsChunk = [1, 1.5, 2, 3.5, 5, 7, 10, 15, 20, 30, 45];
const speeds = [
    ...[...speedsChunk].reverse().map(x=>1/x),
    ...speedsChunk,
    ...speedsChunk.map(x=> x * 60),
    ...speedsChunk.map(x=> x * 60 * 60),
    ...speedsChunk.map(x=> x * 60 * 60 * 60),
    ...speedsChunk.map(x=> x * 60 * 60 * 60 * 60),
];

console.log(speeds);

(window as any).vis = {
    play() {
        model.state.running = !model.state.running;
        updateVisualization();
    },
    speed(dir: number) {
        let cur = model.state.speed;
        let s = cur;
        if (dir > 0) {
            for (s of speeds) if (s > cur) break;
        } else {
            for (s of [...speeds].reverse()) if (s < cur) break;
        }
        model.state.speed = s;
        updateVisualization();
    },
    update(name: string, delta: number) {
        model.state[name] += delta;
        updateVisualization();
    }
};

function commonElementFillFilter(value: any, element: SVGElement): any {
    let num = typeof value === 'number' ? value : value ? 1 : 0;
    element.style.fill = num > 0 ? '#0F0' : num < 0 ? '#F00' : '#DDD';
    return undefined;
}

function commonElementHideFilter(value: any, element: SVGElement): any {
    element.style.display = value ? '' : 'none';
    return undefined;
}

function commonElementTextFilter(value: any, element: SVGElement, units: string[]): any {
    let suffix = ' ' + units.join(' ');
    if (typeof value === 'boolean') {
        element.textContent = (value ? 'TAK' : 'NIE') + suffix;
    } else if (typeof value === 'number') {
        let n = Math.abs(value);
        let digits = 0;
        while (n > 0.000000000001 && Math.round(n) < 100) {
            digits++;
            n *= 10;
        }
        element.textContent = value.toFixed(digits) + suffix;
    } else {
        element.textContent = value.toString() + suffix;
    }
    return undefined;
}

const filterFunctions: { [key: string]: (value: any, element: SVGElement, units: string[]) => any } = {
    h2s: value => value * 60 * 60,
    pr: value => value * 100,
    time: value => {
        let ms = Math.round(value * 1000);
        const days = Math.floor(ms / 86400000);
        const hours = Math.floor((ms % 86400000) / 3600000);
        const minutes = Math.floor((ms % 3600000) / 60000);
        const seconds = Math.floor((ms % 60000) / 1000);
        const milliseconds = ms % 1000;
        const hh = String(hours).padStart(2, '0');
        const mm = String(minutes).padStart(2, '0');
        const ss = String(seconds).padStart(2, '0');
        const mmm = String(milliseconds).padStart(3, '0');
        return `${days}d ${hh}:${mm}:${ss}.${mmm}`;
    },

    hide: commonElementHideFilter,
    g: commonElementHideFilter,

    rect: commonElementFillFilter,
    path: commonElementFillFilter,
    circle: commonElementFillFilter,

    tspanText: commonElementTextFilter,
    textText: commonElementTextFilter,
};

function updateElement(element: Element, name: string, units: string[], filters: string[]) {
    let value = model.state[name];
    for (let filterName of filters) {
        if (!filterFunctions[filterName]) {
            console.warn('Filter ' + filterName);
            return;
        }
        value = filterFunctions[filterName](value, element as SVGElement, units);
        if (value === undefined) return;
    }
}

function activeElement(element: Element, source: string, lastFilter: string) {
    let [pattern, ...units] = source.trim().split(/\s+/);
    let [name, ...filters] = pattern.trim().split(/[:|]/);
    if (name in model.state) {
        filters.push(lastFilter);
        updaters.push(() => {
            updateElement(element, name, units, filters);
        });
    }
}

async function updateVisualization() {
    if (visSate === 0) {
        visSate = 1;
        let res = await fetch("./vis.svg");
        let svg = await res.text();
        let vis = document.querySelector('#vis')!;
        vis.innerHTML = svg;
        for (let element of vis.querySelectorAll('*')) {
            activeElement(element, element.id || '', element.tagName);
            if (element.tagName === 'tspan' || (element.tagName === 'text' && !element.firstElementChild)) {
                activeElement(element, element.textContent || '', element.tagName + 'Text');
            }
        }
        visSate = 2;
    } else if (visSate != 2) {
        return;
    }
    for (let updater of updaters) {
        updater();
    }
}


async function main() {
    initTerm();

    let mode = getSimulationMode();

    if (mode === undefined) {
        fatal('<>No simulation mode selected. Choose <a href="#sw">SOFTWARE</a> or <a href="#hw">HARDWARE</a>.');
    }

    let modelIf: SerialInterface;
    let commIf: SerialInterface | undefined;

    if (mode === SimulationMode.SOFTWARE) {
        console.log('Fully software simulation mode.');
        modelIf = modelWasm.modelInterface;
        commIf = modelWasm.commInterface;
        await modelWasm.initWasmModel();
    } else {
        console.log('HW simulation mode.');
        commIf = undefined;
        fatal('Not implemented yet');
    }

    await model.initialize(updateVisualization, modelIf);

    await wait(1000);
    console.log('Started');
    model.state.running = true;
    await wait(5000);
    //model.state.speed = 500000;
}


window.onload = () => {
    main();
}

let term: xterm.Terminal;
let fitAddon: xtermAddonFit.FitAddon;
let encoder = new TextEncoder();

function diagTermInput(data: string) {
    let bin = encoder.encode(data);
    model.diagInterface?.send?.(bin);
}

function initTerm() {
    let container = document.querySelector<HTMLElement>('#terminal') as HTMLElement;
    term = new xterm.Terminal();
    fitAddon = new xtermAddonFit.FitAddon();
    term.loadAddon(fitAddon);
    term.onData(diagTermInput);
    term.open(container);
    fitAddon.fit();
    window.addEventListener('resize', () => {
        fitAddon.fit();
    });
    model.diagInterface.onReceive = (data: Uint8Array) => {
        term.write(data);
    }
}
