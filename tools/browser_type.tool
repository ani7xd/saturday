{
  "type": "function",
  "function": {
    "name": "browser_type",
    "description": "Click a browser input element and type text into it. The target ID must come from the most recent browser_search_observe result.",
    "parameters": {
      "type": "object",
      "properties": {
        "target": {
          "type": "integer",
          "description": "Browser element ID from the most recent browser_search_observe result."
        },
        "text": {
          "type": "string",
          "description": "Text to type into the target element."
        }
      },
      "required": ["target", "text"],
      "additionalProperties": false
    }
  }
}