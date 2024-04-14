
import { ModelChannel } from './graphCommon';
import { Model } from './model';
import { ModelWasm } from './modelWasm';

let modelChannel: ModelChannel;
let model: Model;

async function main() {
    modelChannel = new ModelWasm();
    await modelChannel.init();
    model = new Model(modelChannel);
}


window.onload = () => {
    main();
}
