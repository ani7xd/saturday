{
  "type": "function",
  "function": {
    "name": "wait",
    "description": "Wait for a specified amount of time before continuing. Use this when the browser or another operation needs time to settle, load, update, or transition. Duration is in milliseconds.",
    "parameters": {
      "type": "object",
      "properties": {
        "duration": {
          "type": "integer",
          "description": "Time to wait in milliseconds. Use short waits such as 100, 250, 500, or 1000 when needed.",
          "minimum": 1,
          "maximum": 15000
        }
      },
      "required": [
        "duration"
      ],
      "additionalProperties": false
    }
  }
}
