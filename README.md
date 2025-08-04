<!-- DL_DOCS::WEBSITE_BLOCK_START
# DeliteAI documentation

## Introduction
DL_DOCS::WEBSITE_BLOCK_END -->
<!-- DL_DOCS::GITHUB_BLOCK_START -->
<!-- markdownlint-disable first-line-h1 no-inline-html -->
<div align="center">
  <h1 align="center">DeliteAI</h1>
  <img src="docs/static/images/delite-ai-blue-logo.png" alt="DeliteAI Logo" width="200">
  <h2 align="center">On-Device AI platform for Software 3.0 on Mobile</h2>
</div>

<div align="center">
  <a href="https://discord.gg/y8WkMncstk">
    <img src="https://img.shields.io/badge/Discord-Join%20Us-purple?logo=discord&logoColor=white&style=for-the-badge"
         alt="Join us on Discord">
  </a>
  <a href="https://github.com/NimbleEdge/assistant">
    <img src="https://img.shields.io/badge/Explore-AI Assistant-blue?style=for-the-badge"
         alt="Try out fully on-device AI Assistant">
  </a>
  <a href="https://deliteai.dev/">
    <img src="https://img.shields.io/badge/Documentation-000?logo=googledocs&logoColor=FFE599&style=for-the-badge"
         alt="Check out DeliteAI documentation">
  </a>
  <hr>
</div>
<!-- markdownlint-restore -->
<!-- DL_DOCS::GITHUB_BLOCK_END -->

DeliteAI is a powerful on-device AI platform for building agentic workflows that enables developers
to deliver secure, privacy-aware, and high-performance AI native experiences and applications
on mobile devices.

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
- Flexible runtime support ([ONNX](https://onnx.ai/) or [ExecuTorch](https://docs.pytorch.org/executorch-overview))

## Getting Started

To get started, you can:

- Follow the steps in [coreruntime](coreruntime/README.md) to build and test the core C++ SDK along
  with its Python bindings
- Build and try out the [Android](sdks/android/README.md) and [iOS](sdks/ios/README.md) SDK and
  sample app
<!-- DL_DOCS::GITHUB_BLOCK_START -->
- Explore the available agents in the [agents](agents) directory.
<!-- DL_DOCS::GITHUB_BLOCK_END -->
<!-- DL_DOCS::WEBSITE_BLOCK_START
- Explore the available agents in the
  [agents](https://github.com/NimbleEdge/deliteAI/tree/main/agents) directory.
DL_DOCS::WEBSITE_BLOCK_END -->

Visit the [assistant](https://github.com/NimbleEdge/assistant) repo to see it all in action.

## Documentation

Explore our documentation at [deliteai.dev](https://deliteai.dev/).

## Community engagement

We welcome any feedback or suggestions - please join our
[Discord](https://discord.gg/y8WkMncstk) to engage with the community.

## Contributing

We welcome contributions from the community! Whether you're interested in:

- Adding new Python operators
- Enhancing runtime support
- Improving documentation
- Reporting bugs or suggesting features

Please read our [Contributing Guidelines](CONTRIBUTING.md) to get started.

## License

This project is licensed under the Apache License 2.0 - see the [LICENSE](LICENSE) file for details.

<!-- DL_DOCS::WEBSITE_BLOCK_START
```{toctree}
:maxdepth: 1
:hidden:

coreruntime/delitepy/build/docs/gen/docs/index.md
```

```{toctree}
:maxdepth: 1
:caption: SDK Integrations
:hidden:

sdks/ios/README.md
sdks/android/README.md
⬢ API reference <sdks/android/nimblenet_ktx/build/dokka/gfm/index.md>
```

```{toctree}
:maxdepth: 1
:caption: Developer Docs
:hidden:

coreruntime/README.md
sdks/ios/docs/DEVELOPMENT.md
sdks/android/README.md
```

```{toctree}
:maxdepth: 1
:caption: Agent Marketplace
:hidden:

agents/README.md
agents/notifications_summarizer/README.md
⬢ Android Agent <agents/notifications_summarizer/android/README.md>
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
DL_DOCS::WEBSITE_BLOCK_END -->
