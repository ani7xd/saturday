# Saturday

**Saturday** is a local AI assistant built in C++.

The goal is to build a capable, low-level AI agent that can interact with the local system, the web, databases, and a browser while keeping the core implementation lightweight and performant.

## Features

- 🤖 Local LLM inference
- 🧠 Conversation/context management
- 💬 Streaming model responses
- 🛠️ Tool/function calling
- 🌐 Web search through SearXNG
- 🌍 URL fetching
- 🌐 Browser automation through Marionette
- 🗄️ MariaDB conversation storage
- 📁 File-system tools
- ⚡ Linux `epoll`-based event handling
- 🔄 Asynchronous response streaming using `eventfd`
- 🖼️ Image/multimodal support

## Architecture

```text
                    ┌───────────────┐
                    │   Saturday    │
                    │    Agent      │
                    └───────┬───────┘
                            │
             ┌──────────────┼──────────────┐
             │              │              │
             ▼              ▼              ▼
          LLM            Tools         Context
             │              │              │
             │       ┌──────┼──────┐       │
             │       │      │      │       │
             ▼       ▼      ▼      ▼       ▼
         Streaming  Files  Web   Browser  MariaDB
                      │      │      │
                      │   SearXNG  │
                      │           │
                      │       Marionette
```

## Requirements

Currently intended for Linux.

You'll need:

- C++ compiler with modern C++ support
- CMake
- Ninja
- MariaDB
- SearXNG
- Ollama / local LLM runtime
- Waterfox/Firefox for browser automation

## Building

Clone the repository:

```bash
git clone https://github.com/ani7xd/Saturday.git
cd Saturday
```

Configure:

```bash
cmake -S . -B build -G Ninja
```

Build:

```bash
cmake --build build
```

Run:

```bash
./build/saturday
```

## Configuration

Secrets and machine-specific configuration should **not** be committed.

Keep credentials in `.env` and make sure it is ignored by Git.

Example:

```env
GOOGLE_API_KEY=your_key_here
```

## Project Status

🚧 **Early development**

Saturday is actively being developed and its architecture and APIs may change significantly.

## License

License information will be added later.
