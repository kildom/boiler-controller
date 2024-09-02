import fs from 'fs';
import path from 'path';
import esbuild, { BuildOptions } from 'esbuild';
import copyStaticFiles from 'esbuild-copy-static-files';

const ROOT = path.resolve(path.dirname(process.argv[1]), '..');

build({
    entryPoints: [
        path.join(ROOT, 'simu/app/worker-model.ts'),
        path.join(ROOT, 'simu/app/worker-server.ts'),
        path.join(ROOT, 'simu/app/index.ts'),
    ],
    bundle: true,
    sourcemap: true,
    minify: false,
    format: 'iife',
    outdir: path.join(ROOT, 'dist_simu'),
    metafile: true,
    loader: {
        '.ttf': 'file',
        '.svg': 'file',
        '.woff': 'file',
        '.woff2': 'file',
        '.eot': 'file',
    },
    plugins: [
        copyStaticFiles({
            src: path.join(ROOT, 'simu/app/static'),
            dest: path.join(ROOT, 'dist_simu'),
            dereference: true,
            errorOnExist: false,
            recursive: true,
        }),
    ],
}, true, path.join(ROOT, 'dist_simu/web-meta.json'));


async function build(opts: BuildOptions, startServer: boolean, metaFileName: string) {
    let mode = (process.argv[2] || '').substring(0, 1).toLowerCase() as 's' | 'w' | '';
    let ctx = await esbuild.context(opts);
    if (startServer && mode === 's') {
        let result = await ctx.serve({
            host: '127.0.0.1',
            port: 8080,
            servedir: path.join(ROOT, 'dist_simu'),
        });
        console.log('Server running on:');
        console.log(`    http://${result.host}:${result.port}/`);
    } else if (mode !== '') {
        await ctx.watch();
    } else {
        let result = await ctx.rebuild();
        if (result.errors.length > 0) {
            console.error(result.errors);
        }
        if (result.warnings.length > 0) {
            console.error(result.warnings);
        }
        if (!result.errors.length && !result.warnings.length) {
            console.log('Build done.');
        }
        ctx.dispose();
        if (!mode && metaFileName) {
            fs.mkdirSync(path.dirname(metaFileName), { recursive: true });
            fs.writeFileSync(metaFileName, JSON.stringify(result.metafile, null, 4));
        }
    }
}