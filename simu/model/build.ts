
import * as process from 'node:process';
import * as fs from 'node:fs';
import * as path from 'node:path';
import * as child_process from 'node:child_process';


const DEBUG = false;

const TARGET = '../web/simu.wasm';

const WASI_SDK = process.platform == 'win32'
    ? 'C:\\work\\triasm\\devtools\\wasi-sdk'
    : '/home/doki/my/wasi-sdk-19.0'
    ;

const DEFINES = [
    DEBUG ? 'DEBUG' : 'NDEBUG'
];

const FLAGS = DEBUG
    ? '-O3 -g -mexec-model=reactor'.split(' ')
    : '-O0 -g -gdwarf-5 -gpubnames'.split(' ')
    ;

const CXX_FLAGS = [];

const LINK_FLAGS = ['-mexec-model=reactor'];

const BUILD_DELAY = 700;

const CLANG = path.join(WASI_SDK, 'bin', process.platform == 'win32' ? 'clang.exe' : 'clang');
const SYSROOT = path.join(WASI_SDK, 'share', 'wasi-sysroot');

const DIR = path.dirname(process.argv[1]);
const FILES_FILE = path.resolve(DIR, 'build-files.txt');

const TARGET_ABS = path.resolve(DIR, TARGET);

function clangExec(args: string[]) {
    let status = 0;

    try {
        child_process.execFileSync(CLANG, args, {
            cwd: DIR,
            stdio: 'inherit'
        });
    } catch (e: any) {
        if ('status' in e && 'signal' in e) {
            status = e.status || 999;
        } else {
            throw e;
        }
    }

    return status == 0;
}

function getPaths(file: string) {
    let input = path.resolve(DIR, file);
    let target = path.resolve(DIR, '../build', path.basename(file, path.extname(file))) + '.o';
    let dep = path.resolve(DIR, '../build', path.basename(file, path.extname(file))) + '.d';
    return [input, target, dep];
}

function getDeps(file: string): null | string[] {
    let [input, target, dep] = getPaths(file);
    if (!fs.existsSync(dep)) {
        return null;
    }
    let isDep = false;
    let files = fs.readFileSync(dep, 'utf8')
        .replace(/\\\r?\n/g, ' ')
        .split(/(?<!\\)(?: |\\\r?\n)+|(?=:(?<!\\)(?: |\\\r?\n)+)/)
        .map(x => x.trim().replace(/\\ /g, ' '))
        .filter(x => { let r = isDep; isDep = isDep || x == ':'; return r; });
    files.push(FILES_FILE);
    return files;
}

function compile(file: string) {

    let [input, target, dep] = getPaths(file);

    console.log(`====== Compiling ${input}...`);

    let args: string[] = [];
    args.push('--sysroot', SYSROOT);
    args.push('-c');
    args.push(...FLAGS);
    args.push(...CXX_FLAGS);
    args.push(...DEFINES.map(x => `-D${x}`));
    args.push('-MMD');
    args.push(input);
    args.push('-o', target);

    let ok = clangExec(args);
    return ok
}

function getFileList(): string[] {
    return fs.readFileSync(FILES_FILE, 'utf8')
        .trim()
        .split(/\s*\r?\n\s*/)
        .filter(x => x.length);
}

function link() {

    console.log(`====== Linking ${TARGET_ABS}...`);

    let args: string[] = [];
    args.push('--sysroot', SYSROOT);
    args.push(...FLAGS);
    args.push(...LINK_FLAGS);
    args.push(...getFileList().map(f => getPaths(f)[1]));
    args.push('-o', TARGET_ABS);

    let ok = clangExec(args);
    return ok
}

function getTime(file: string): number | undefined {
    let stat = fs.statSync(file, { throwIfNoEntry: false });
    if (!stat) {
        return undefined;
    }
    return stat.mtimeMs;
}

function isDirty(target: string, deps: null | string[]): boolean {
    let targetTime = getTime(target);
    if (!targetTime || !deps) {
        return true;
    }
    for (let dep of deps) {
        let time = getTime(dep);
        if (!time || time >= targetTime) {
            return true;
        }
    }
    return false;
}

function isDirtyObj(file: string): boolean {
    let [input, target, _] = getPaths(file);
    return isDirty(target, getDeps(file));
}

async function wait(time: number) {
    await new Promise<void>(resolve => setTimeout(resolve, time));
}

let requestedBuildId = 1;
let lastBuildId = 0;

async function build() {
    console.log('='.repeat(78));
    console.log(' '.repeat(34) + ' BUILDING ' + ' '.repeat(34));
    console.log('='.repeat(78));
    lastBuildId = requestedBuildId;
    let linkDeps: string[] = [];
    for (let file of getFileList()) {
        if (isDirtyObj(file)) {
            if (!compile(file)) return false;
            linkDeps.push('__force_linking__');
            await wait(1);
            if (lastBuildId != requestedBuildId) {
                console.log('Interrupted. New build requested.');
                return true;
            }
        }
        let [_1, target, _2] = getPaths(file);
        linkDeps.push(target);
    }
    if (isDirty(TARGET_ABS, linkDeps)) {
        if (!link()) return false;
    }
    return true;
}

let watchers: { [key: string]: fs.FSWatcher } = {};
let fileEventResolve: ((value: void | PromiseLike<void>) => void) | null = null;

function fileEvent(eventType: string, filename: string | null) {
    requestedBuildId++;
    let tmp: typeof fileEventResolve = null;
    if (fileEventResolve) {
        [fileEventResolve, tmp] = [tmp, fileEventResolve];
        tmp();
    }
    console.log(eventType, filename);
}

async function watch() {
    do {
        if (!await build()) {
            console.error('Build failed!');
        }
        if (requestedBuildId == lastBuildId) {
            let newFiles = new Set<string>();
            for (let file of getFileList()) {
                getDeps(file)?.forEach(dep => newFiles.add(dep));
            }
            let oldFiles = new Set<string>(Object.keys(watchers));
            for (let addFile of [...newFiles].filter(x => !oldFiles.has(x))) {
                watchers[addFile] = fs.watch(addFile, fileEvent);
            }
            for (let delFile of [...oldFiles].filter(x => !newFiles.has(x))) {
                watchers[delFile].close();
                delete watchers[delFile];
            }
            console.log(`Watching ${Object.keys(watchers).length} files...`);
            await new Promise<void>(resolve => { fileEventResolve = resolve });
        }
        await wait(BUILD_DELAY);
    } while (true);
}

if (process.argv[2] === 'watch') {
    watch();
} else {
    build();
}
