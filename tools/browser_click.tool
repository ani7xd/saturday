{
  "type": "function",
  "function": {
    "name": "browser_click",
    "description": "Click a browser element. The target ID must come from the most recent browser_search_observe result.",
    "parameters": {
      "type": "object",
      "properties": {
        "target": {
          "type": "integer",
          "description": "Browser element ID from the most recent browser_search_observe result. Do not invent IDs."
        }
      },
      "required": ["target"],
      "additionalProperties": false
    }
  }
}