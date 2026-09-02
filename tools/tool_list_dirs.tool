{
  "type": "function",
  "function": {
    "name": "list_dir",
    "description": "List the entries directly inside a local directory. Returns the entry name and filesystem type for each entry. Hidden files and directories are included.",
    "parameters": {
      "type": "object",
      "properties": {
        "path": {
          "type": "string",
          "description": "Path to the local directory whose contents should be listed."
        }
      },
      "required": [
        "path"
      ]
    }
  }
}