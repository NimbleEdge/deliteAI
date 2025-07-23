# DeliteAI Agents

DeliteAI Agents are pre-built, on-device AI packages that provide high-level APIs for common agentic
use cases, simplifying the integration of delightful AI experiences into your applications.

## Index

- [Overview](#overview)
- [Core Features](#core-features)
- [The Agents Marketplace](#the-agents-marketplace)
- [Available Agents](#available-agents)
- [The Power of Agent Combinations](#the-power-of-agent-combinations)
- [Examples](#examples)

## Overview

We're solving a significant challenge in the mobile applications AI ecosystem: the lack of
discoverability and reusability of AI components. Imagine finding an AI app with an outstanding
feature, like high-quality text-to-speech, but needing to rebuild it from scratch because you can't
isolate that one component.

DeliteAI Agents solve this by encapsulating complex AI functionalities into ready-to-use packages
that run entirely on-device, ensuring privacy and performance.

## Core Features

All agents are built on the DeliteAI SDK and provide a consistent set of powerful features:

- **On-Device AI Processing**: Ensures user privacy and delivers low-latency performance without
  relying on the cloud.
- **High-Level APIs**: Simplifies integration, allowing you to add complex AI features with just a
  few lines of code.
- **Background Processing**: Capable of running tasks efficiently in the background, even when the
  app isn't active.
- **Persistent Storage**: Built-in support for storing and managing results locally.
- **Cross-Platform**: Designed for seamless integration across both Android/Kotlin and iOS/Swift.

## The Agents Marketplace

The DeliteAI Agents Marketplace is the central hub where developers can discover, integrate, and
combine a growing library of agents.

### How the Marketplace Empowers Developers

- **Simple Integration**: Add a dependency to your project to instantly integrate powerful AI
  capabilities.
- **No Heavy Lifting**: Pre-built agents handle complex model management, configurations, and
  platform-specific optimizations.
- **Discoverable & Reusable**: Easily find and reuse agents across different projects and
  applications.

### Upcoming Marketplace Agents

We are continuously expanding the marketplace with new foundational and productivity-focused agents:

- **Core Functionality**: TTS, ASR, on-device LLM execution, and general-purpose summarization.
- **Productivity**: Intelligent agents for managing emails, calendars, and other common tasks.

## Available Agents

### Notifications Summarizer

Automatically processes and summarizes device notifications at scheduled times using on-device
language models. Even when the device is locked, the agent can activate, retrieve notifications, and
prepare a concise summary.

- [Overview](notifications_summarizer/README.md)
- [Android Implementation](notifications_summarizer/android/README.md)

## The Power of Agent Combinations

The true potential of the marketplace emerges when you combine multiple agents to create
sophisticated workflows that would otherwise be difficult to build.

**Morning Briefing System**:

```text
Notification Summarizer + TTS Agent + ASR Agent
→ Summarizes overnight notifications → Reads summary aloud → Responds to voice commands
```

**Smart Email Assistant**:

```text
Email Reader + LLM Execution + TTS Agent
→ Processes emails → Generates responses → Reads important messages aloud
```

**Intelligent Content Pipeline**:

```text
ASR Agent + LLM Execution + Summarizer + TTS Agent
→ Voice input → AI processing → Content summarization → Audio output
```

## Examples

- [Android Example App](examples/android/README.md) - A complete demo showcasing the integration of
  agents in a real-world application.
