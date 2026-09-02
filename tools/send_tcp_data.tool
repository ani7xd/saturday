{
  "type": "function",
  "function": {
    "name": "send_data_tcp",
    "description": "Send text data to the currently connected TCP server and wait for its response.",
    "parameters": {
      "type": "object",
      "properties": {
        "data": {
          "type": "string",
          "description": "The text data to send to the connected TCP server."
        }
      },
      "required": ["data"],
      "additionalProperties": false
    }
  }
}