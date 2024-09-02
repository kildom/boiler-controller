import * as modelWasm from "./model-wasm";
import * as model from "./model";
import { fatal, getSimulationMode, SerialInterface, SimulationMode } from "./simu-common";


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
        modelIf = modelWasm.modelInterface;
        commIf = modelWasm.commInterface;
        modelWasm.initWasmModel();
    } else {
        commIf = undefined;
        fatal('Not implemented yet');
    }

    await model.initialize(aaa, modelIf);
}


window.onload = () => {
    main();
}
