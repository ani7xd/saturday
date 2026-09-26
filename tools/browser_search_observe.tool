{
  "type": "function",
  "function": {
    "name": "browser_search_observe",
    "description": "Observe the current browser page and return the elements most relevant to the given search query. Use this when you need to find a specific element or piece of page content before interacting with it. Browser element IDs returned by this tool can be used with browser_click, browser_fill, and browser_actions.",
    "parameters": {
      "type": "object",
      "properties": {
        "query": {
          "type": "string",
          "description": "What you are looking for on the current browser page. Describe the target or task naturally, for example: 'find the search box', 'find the login button', or 'find the Mushoku Tensei link'."
        }
      },
      "required": [
        "query"
      ],
      "additionalProperties": false
    }
  }
}
