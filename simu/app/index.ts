import * as modelWasm from "./model-wasm";
import * as model from "./model";
import { fatal, getSimulationMode, SerialInterface, SimulationMode, wait } from "./simu-common";


function aaa() {

}


async function main() {
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

    await model.initialize(aaa, modelIf);

    await wait(1000);
    console.log('Started');
    model.state.running = true;
}


window.onload = () => {
    main();
}
