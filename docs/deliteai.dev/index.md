# DeliteAI documentation

## Introduction

DeliteAI is a powerful on-device AI platform for building agentic workflows that enables developers
to deliver secure, privacy-aware, and high-performance AI native experiences and applications
across multiple platforms and devices.

## Key Features

### Developer Productivity

- Unified and simplified APIs for seamless AI agents integration in Android/iOS applications
- Python interface for orchestrating complex AI agentic workflows via tool calling, memory and LLMs on-device
- Streamlined development process for both new and existing applications

### Portability & Small Form Factor

- Cross-platform compatibility across operating systems
- Support for various compute platforms and runtimes
- Abstracted development layer for the fragmented device landscape
- Optimized for resource-constrained environments with efficient CPU/memory usage

### Security & Privacy

- Privacy-first approach with on-device processing
- Hardware-accelerated model execution

### Extensibility

- Easy integration of custom Python operators
- Flexible runtime support (ONNX or ExecuTorch)

## Getting Started

To get started, you can:

- Follow the steps in [coreruntime](https://github.com/NimbleEdge/deliteAI/blob/main/coreruntime/README.md) to build and test the core C++ SDK with Python bindings
- Build and try out the [Android](https://github.com/NimbleEdge/deliteAI/blob/main/sdks/android/README.md) and [iOS](https://github.com/NimbleEdge/deliteAI/blob/main/sdks/ios/README.md) SDK and sample app
- Explore the available agents in the [agents](https://github.com/NimbleEdge/deliteAI/blob/main/agents) directory.

Visit the [assistant](https://github.com/NimbleEdge/assistant) repo to see it all in action.

## Documentation

[Coming Soon]

## Community engagement

We welcome any feedback or suggestions - please join our [Discord](https://discord.gg/y8WkMncstk) to engage with the community.

## Contributing

We welcome contributions from the community! Whether you're interested in:

- Adding new Python operators
- Enhancing runtime support
- Improving documentation
- Reporting bugs or suggesting features

Please read our [Contributing Guidelines](https://github.com/NimbleEdge/deliteAI/blob/main/CONTRIBUTING.md) to get started.

## License

This project is licensed under the Apache License 2.0 - see the [LICENSE](https://github.com/NimbleEdge/deliteAI/blob/main/LICENSE) file for details.

```{toctree}
:maxdepth: 1
:hidden:

delitepy/docs/gen/docs/index.md
```

```{toctree}
:maxdepth: 1
:caption: SDK Integrations
:hidden:

DL_SDKS_DIR/ios/README.md
DL_SDKS_DIR/android/README.md
⬢ API reference <DL_SDKS_DIR/android/nimblenet_ktx/build/dokka/gfm/index.md>
```

```{toctree}
:maxdepth: 1
:caption: Developer Docs
:hidden:

coreruntime <https://github.com/NimbleEdge/deliteAI/blob/main/coreruntime/README.md>
iOS <DL_SDKS_DIR/ios/docs/DEVELOPMENT.md>
Android (TBD) <DL_SDKS_DIR/android/README.md>
```

```{toctree}
:maxdepth: 1
:caption: Agent Marketplace
:hidden:

DL_AGENTS_DIR/README.md
DL_AGENTS_DIR/notifications_summarizer/README.md
⬢ Android Agent <DL_AGENTS_DIR/notifications_summarizer/android/README.md>
```

```{toctree}
:maxdepth: 1
:caption: Indices
:hidden:

genindex
modindex
```

```{toctree}
:maxdepth: 1
:caption: Project Links
:hidden:

GitHub <https://github.com/NimbleEdge/deliteAI>
Discord <https://discord.gg/y8WkMncstk>
```
