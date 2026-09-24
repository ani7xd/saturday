{
  "type": "function",
  "function": {
    "name": "browser_element_click",
    "description": "Click one browser element from the most recent browser_observe result. Use this for a standalone single-element click. PREFER browser_actions when the click is part of a sequence with other interactions, such as click then type, click then press a key, or multiple mouse and keyboard operations.",
    "parameters": {
      "type": "object",
      "properties": {
        "id": {
          "type": "integer",
          "description": "Browser element ID from the most recent browser_observe result. Never invent an ID and never use a UUID. The element must be visible, interactable, and clickable."
        }
      },
      "required": [
        "id"
      ],
      "additionalProperties": false
    }
  }
}