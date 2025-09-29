
# foo_input_signal

[foo_input_signal](https://github.com/stuerp/foo_input_signal/releases) is a [foobar2000](https://www.foobar2000.org/) component that generates synthesized signals.

## Features

- Uses Csound Document (CSD) files to generate a signal.

## Requirements

- Tested on [foobar2000](https://www.foobar2000.org/download) v2.0 or later (64-bit only). ![foobar2000](https://www.foobar2000.org/button-small.png)
- Tested on Microsoft Windows 10 and later.

## Getting started

- Double-click `foo_input_signal.fbk2-component`.

or

- Import `foo_input_signal.fbk2-component` into foobar2000 using the "*File / Preferences / Components / Install...*" menu item.

## Usage

You can find the user guide [here](docs/README.md).

## Developing

### Requirements

To build the code you need:

- [Microsoft Visual Studio 2022 Community Edition](https://visualstudio.microsoft.com/downloads/) or later
- [foobar2000 SDK](https://www.foobar2000.org/SDK) 2025-03-07

To create the deployment package you need:

- [PowerShell 7.2](https://github.com/PowerShell/PowerShell) or later

### Setup

Create the following directory structure:

    bin
    bin.x86
    foo_input_signal
    out
    sdk

- `bin` contains a portable version of foobar2000 64-bit for debugging purposes.
- `bin/x86` contains a portable version of foobar2000 32-bit for debugging purposes.
- `foo_input_signal` contains the [Git](https://github.com/stuerp/foo_input_signal) repository.
- `out` receives a deployable version of the component.
- `sdk` contains the foobar2000 SDK.

### Building

Open `foo_input_signal.sln` with Visual Studio and build the solution.

### Packaging

To create the component build the x64 configuration.

## Change Log

v0.1.0.0-alpha1, 2025-xx-xx

- Initial version

You can read the full history [here](docs/History.md).

## Acknowledgements / Credits

- Peter Pawlowski for the [foobar2000](https://www.foobar2000.org/) audio player. ![foobar2000](https://www.foobar2000.org/button-small.png)
- The [Csound team](https://github.com/orgs/csound/people) for [Csound](https://csound.com/index.html).

## Reference Material

- Csound, https://github.com/orgs/csound/repositories
- Csound FLOSS Manual, https://flossmanual.csound.com/introduction/preface
- Csound Reference Manual, https://csound.com/docs/manual/index.html
- Lessons in Csound, https://csound.com/learn-csound-site/

## Test Material

## Links

- Home page: [https://github.com/stuerp/foo_input_signal](https://github.com/stuerp/foo_input_signal)
- Repository: [https://github.com/stuerp/foo_input_signal.git](https://github.com/stuerp/foo_input_signal.git)
- Issue tracker: [https://github.com/stuerp/foo_input_signal/issues](https://github.com/stuerp/foo_input_signal/issues)
## License

![License: MIT](https://img.shields.io/badge/license-MIT-yellow.svg)
