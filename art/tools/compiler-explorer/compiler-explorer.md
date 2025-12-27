# Compiler Explorer

This doc describes how to run a local instance of Compiler Explorer
(https://godbolt.org/) with local compilers built from an Android source tree.

## Prerequisites

-   **Install Node.js**: Compiler Explorer requires Node.js. Before running the
    setup script, make sure you have `node` and `npm` installed and available in
    your `PATH`. We recommend using
    [Node Version Manager (nvm)](https://github.com/nvm-sh/nvm) to install and
    manage Node.js versions.
-   You need a **full** Android source tree to build compilers.

## Instructions

The setup process is automated by the `setup_compiler_explorer.sh` script.

### One-time Setup

1.  Go to your Android source tree and initialize the environment as usual:

    ```bash
    source build/envsetup.sh && lunch aosp_cf_x86_64_phone-trunk_staging-userdebug
    ```

    Note: You may use a different `lunch` target, as long as it can build the
    host binaries required by the script.

2.  Run the setup script:

    ```bash
    art/tools/compiler-explorer/setup_compiler_explorer.sh
    ```

    This will download Compiler Explorer to `out/compiler-explorer`, build it,
    build the necessary Android compilers (including dex2oat and boot images),
    and start the server.

    Once you see `Listening on http://localhost:10240/`, you can open a browser
    with that address to access Compiler Explorer.

### Iterating and Rebuilding

When you make changes to the compilers, you can use flags to rebuild only the
necessary components without re-running the full setup.

-   **Rebuild dex2oat:** If you have modified the dex2oat:

    ```bash
    # This will rebuild and copy a fresh dex2oat.
    art/tools/compiler-explorer/setup_compiler_explorer.sh --rebuild-dex2oat

    # Then, restart the server to apply the changes.
    art/tools/compiler-explorer/setup_compiler_explorer.sh --restart
    ```

    Note that if you built the boot image, then changed and rebuilt dex2oat,
    then you must also rebuild the boot image.

-   **Restarting the Server:** To simply stop and restart the server: ```bash
    art/tools/compiler-explorer/setup_compiler_explorer.sh --restart`

### Advanced Options

-   **Custom Installation Directory:** You can specify a different installation
    directory for Compiler Explorer using the `--dir` flag.

    ```bash
    art/tools/compiler-explorer/setup_compiler_explorer.sh --dir /path/to/your/dir
    ```

-   **Custom Instruction Sets:** By default the boot image instruction sets are
    derived from your `lunch` target. You can generate boot images for different
    instruction sets, to use them with Compiler Explorer.

    ```bash
    art/tools/compiler-explorer/setup_compiler_explorer.sh --instruction-sets "arm64 x86_64 riscv64"
    ```

For more options, run the script with `--help`.