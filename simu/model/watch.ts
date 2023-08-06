
import * as process from 'node:process';
import * as fs from 'node:fs';
import * as path from 'node:path';
import * as child_process from 'node:child_process';

const DIR = path.dirname(process.argv[1]);

function getDeps(): Set<string> {

    let depFiles = fs.readdirSync(DIR + '/obj', { recursive: true, encoding: 'utf8' })
        .filter(file => file.endsWith('.d'));

    let watchFiles = new Set<string>();

    for (let depFile of depFiles) {
        let isDep = false;
        fs.readFileSync(DIR + '/obj/' + depFile, 'utf8')
            .replace(/\\\r?\n/g, ' ')
            .split(/(?<!\\)(?: |\\\r?\n)+|(?=:(?<!\\)(?: |\\\r?\n)+)/)
            .map(x => x.trim().replace(/\\ /g, ' '))
            .filter(x => { let r = isDep; isDep = isDep || x == ':'; return r; })
            .forEach(file => watchFiles.add(DIR + '/' + file));
    }

    watchFiles.add(DIR + '/Makefile');

    return watchFiles;
}

function build() {
    let status = 0;

    try {
        child_process.execFileSync('make', [], { cwd: DIR, stdio: 'inherit' });
    } catch (e: any) {
        if ('status' in e && 'signal' in e) {
            status = e.status || 999;
        } else {
            throw e;
        }
    }

    if (status != 0) {
        console.error(`Compilation failed with status ${status}.`);
    }
}

let filesQueued = new Set<string>();
let filesChangeTime = Date.now();

let fileEventResolve: ((value: void | PromiseLike<void>) => void) | null = null;

function fileEvent(filename: string) {
    filesChangeTime = Date.now();
    filesQueued.add(filename);
    let tmp: typeof fileEventResolve = null;
    if (fileEventResolve) {
        [fileEventResolve, tmp] = [tmp, fileEventResolve];
        tmp();
    }
}

function clearScreen() {
    try {
        child_process.execFileSync('clear', [], { stdio: 'inherit' });
    } catch (e) { }
    try {
        child_process.execFileSync('cmd.exe /c cls', [], { stdio: 'inherit' });
    } catch (e) { }
}

async function main() {
    let watchers = new Map<string, fs.FSWatcher>();
    while (true) {
        if (filesQueued.size == 0 && watchers.size > 0) {
            await new Promise<void>(resolve => { fileEventResolve = resolve });
        }
        while (Date.now() - filesChangeTime < 500) {
            await new Promise<void>(r => setTimeout(r, 100));
        }
        //clearScreen();
        console.log('-------------------------------------------------------------------------------');
        if (filesQueued.size > 0) {
            console.log(`Files changed:\n    ${[...filesQueued].join('\n    ')}`);
            console.log('-------------------------------------------------------------------------------');
        }
        filesQueued.clear();
        build();
        let newDeps = getDeps();
        let oldDeps = new Set(watchers.keys());
        for (let dep of [...newDeps].filter(x => !oldDeps.has(x))) {
            watchers.set(dep, fs.watch(dep, () => fileEvent(dep)));
            console.log(`Start watching ${dep}`);
        }
        for (let dep of [...oldDeps].filter(x => !newDeps.has(x))) {
            watchers.get(dep)!.close();
            watchers.delete(dep);
            console.log(`Stop watching ${dep}`);
        }
    }
}

main();

