{
  "type": "function",
  "function": {
    "name": "write_file",
    "description": "Write text to a local file. Creates the file if it does not exist. Creates parent directories if necessary. Use this when the user asks to create, write, save, append, or overwrite a file.",
    "parameters": {
      "type": "object",
      "properties": {
        "path": {
          "type": "string",
          "description": "Absolute or relative path of the file."
        },
        "content": {
          "type": "string",
          "description": "The text to write to the file."
        } 
      },
      "required": ["path", "content"]
    }
  }
}
