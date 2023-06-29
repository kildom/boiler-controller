
const input = {
    deps: [
        './build/**/*.dep'
    ],
    buildAtStartup: true,
    run: [
        {
            command: 'make',
            args: ['-j4'],
            options: {
                shell: true
            }
        }
    ],
};
