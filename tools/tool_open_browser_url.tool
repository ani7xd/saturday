{
  "type": "function",
  "function": {
    "name": "open_browser",
    "description": "Open a URL in a new tab in the local Waterfox browser.",
    "parameters": {
      "type": "object",
      "properties": {
        "url": {
          "type": "string",
          "description": "The URL to open in the browser."
        }
      },
      "required": ["url"]
    }
  }
}
