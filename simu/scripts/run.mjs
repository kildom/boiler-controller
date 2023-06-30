
import { ChildProcess, spawn } from 'node:child_process';
import { dirname, resolve } from 'node:path';
import * as readline from 'node:readline';
import open from 'open';

const DIR = resolve(dirname(process.argv[1]), '..');

const npx = process.platform === 'win32' ? 'npx.cmd' : 'npx';

const tsc = spawn(npx, 'tsc --watch --project tsconfig.json'.split(' '), {
    cwd: DIR,
    stdio: 'inherit',
});

const http = spawn(npx, 'http-server . -a 127.0.0.1 -p 9592'.split(' '), {
    cwd: resolve(DIR, 'web'),
    stdio: 'inherit',
});

const cpp = spawn(npx, 'ts-node build.ts watch'.split(' '), {
    cwd: resolve(DIR, 'model'),
    stdio: 'inherit',
});

function tryKill(proc, sig) {
    try {
        proc.kill(sig);
    } catch (err) {
    }
}

if (process.platform === "win32") {
    var rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout
    });
    rl.on("SIGINT", function () {
        process.emit("SIGINT");
    });
}

process.on('SIGINT', async function () {
    console.log("Caught interrupt signal");
    tryKill(tsc, 'SIGINT');
    tryKill(http, 'SIGINT');
    tryKill(cpp, 'SIGINT');
    await new Promise(resolve => setTimeout(resolve, 300));
    tryKill(tsc, 'SIGKILL');
    tryKill(http, 'SIGKILL');
    tryKill(cpp, 'SIGKILL');
    await new Promise(resolve => setTimeout(resolve, 300));
    tryKill(tsc);
    tryKill(http);
    tryKill(cpp);
    await new Promise(resolve => setTimeout(resolve, 300));
    tsc.unref();
    http.unref();
    cpp.unref();
    await new Promise(resolve => setTimeout(resolve, 1000));
    console.log(`tsc killed: ${tsc.killed || tsc.exitCode !== null ? 'yes' : 'no'}`);
    console.log(`http killed: ${tsc.killed || http.exitCode !== null ? 'yes' : 'no'}`);
    console.log(`cpp killed: ${tsc.killed || cpp.exitCode !== null ? 'yes' : 'no'}`);
    await new Promise(resolve => setTimeout(resolve, 1000));
    process.exit();
});

setTimeout(async () =>{
    open('http://127.0.0.1:9592/vis.html');
}, 2000);
