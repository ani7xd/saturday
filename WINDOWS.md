# Saturday on Windows

The existing model, memory, database, HTTP, tool manager, TCP client, and audio classes remain. Windows uses Winsock for TCP and WinMM for audio; Linux keeps its socket and ALSA backends. Both use a standard C++ condition variable and queue for streamed output.

## Requirements

- Windows 10/11, 64 bit.
- CMake 3.24+ and a C++20 compiler. This machine uses MSYS2 UCRT64 GCC 14.2 and mingw32-make on PATH. An MSVC build is supported by the configuration but has not been tested here.
- MySQL Server 8.0 with C headers and libmysql. Workbench is optional and manages the same server; the program does not connect to Workbench itself.
- Ollama running locally, with gemma3:12b and qwen3.5:9b installed.
- Internet access for the first build to download version-pinned curl, simdjson, yyjson, and Lexbor sources. CMake uses Windows Schannel for HTTPS.

## Configure

Copy .env.example to .env only if .env does not already exist. Fill DATABASE_USERNAME, DATABASE_PASSWORD, and optionally TAVILY_API_KEY locally. Never commit .env. DATABASE_PATH is the database name, not a file path. SYSTEM_PROMPT_PATH=system_prompt.txt uses the included prompt. Existing process environment variables take precedence over .env. Quoted values may contain spaces and equals signs; values are literal, with no shell interpolation. Restart Saturday after changing configuration.

Default routing uses Gemma for conversation and Qwen for prompts with common coding keywords or explicit tool requests, such as "web search", "search the internet", or "look up". This is a simple keyword rule, not a second AI classifier. /code forces Qwen, /chat forces Gemma, and /auto restores automatic routing. Modes persist until changed. In /chat mode, recognized tool requests are rejected with instructions to switch modes rather than being sent to a model without tools. Use /code when an unusual phrasing is not recognized automatically.

Gemma 3 does not advertise native tools or thinking in the installed Ollama model. Its requests omit those options. Qwen supports both. The OLLAMA_*_TOOLS and OLLAMA_*_THINKING settings must match the selected models. Conversation history remains shared; previous tool messages are rendered as text when sent to Gemma.

## Build and initialize

Run PowerShell from the project folder:

    .\scripts\build-windows.ps1
    .\scripts\run-windows.ps1 --init-db
    .\scripts\run-windows.ps1 --check-db
    .\scripts\run-windows.ps1

--init-db creates the configured database and missing tables, without dropping existing data. Its account needs CREATE permission. For a restricted account, an administrator can run sql/schema.sql in Workbench and grant access to that database instead. The Workbench script uses the default database name saturday; change that name if you use another one.

--check-db checks authentication and database selection. Normal startup also prepares the queries and loads system_prompt.txt into the system_prompt table. Changing the prompt file updates row 1 on the next startup. If an existing database has a different schema, review it before attempting a migration; --init-db does not alter existing tables.

Use /end on its own line to submit multiline text. Use img>>> C:/path/image.png to attach an image. Use exit or quit, or close standard input, to stop cleanly. An image-only message still requires accompanying text.

## Implementation and limits

CMake now builds the complete active application using C++20 and MySQL-compatible binding types. The old Makefile delegates to CMake. The unused direct llama.cpp backend is optional with -DSATURDAY_LLAMA=ON and requires compatible llama headers/libraries; the normal Ollama application does not need them.

The Windows audio classes compile into the application, but the existing chat flow still does not enable microphone input, speech recognition, or text-to-speech. Windows recording accepts default or a numeric wave input device ID. Physical microphone/speaker testing requires explicit use of these APIs.

TCP connection establishment and send/receive use bounded waits on Windows. send_data_tcp reports bytes sent; it does not receive a response. Browser opening uses the default browser on Windows and xdg-open on Linux. write_file appends; edit_file replaces exact text. Filesystem and TCP tools retain the original application's access privileges.

History is still one shared chat table and is not automatically summarized or partitioned. Images are stored as local paths. Dependencies are pinned to reproducible versions, not asserted to be the latest security releases. Update and validate them before distributing this application broadly.

## Verification on this machine

Native Windows GCC build and both CTest suites pass. Coverage includes queue shutdown/draining, environment parsing, Winsock loopback send/receive, split Ollama stream parsing and errors, Base64 vectors, Unicode file editing/appending, repeated result cleanup, and loading all ten tool schemas. MySQL authentication and schema initialization succeeded. Real Gemma chat and a Qwen time-tool round trip completed successfully. Physical audio and the optional direct llama.cpp backend were not exercised.
