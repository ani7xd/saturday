{
  "type": "function",
  "function": {
    "name": "read_file",
    "description": "Read the contents of a local file. Use this when the user asks about a file, source code, logs, configuration, documents, or when additional file context is needed.",
    "parameters": {
      "type": "object",
      "properties": {
        "path": {
          "type": "string",
          "description": "Absolute or relative path to the file."
        }
      },
      "required": ["path"]
    }
  }
}