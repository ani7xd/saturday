{
  "type": "function",
  "function": {
    "name": "fetch_url",
    "description": "Fetch a URL and return its contents. Use this when external information or a webpage needs to be retrieved.",
    "parameters": {
      "type": "object",
      "properties": {
        "url": {
          "type": "string",
          "description": "The URL to fetch."
        }
      },
      "required": ["url"]
    }
  }
}
