{
  "type": "function",
  "function": {
    "name": "browser_element_fill",
    "description": "Fill a specific browser input or editable element directly with text. Use this only for a standalone text-entry operation targeting one known element. PREFER browser_actions when text entry is part of a sequence, such as click then type, type then press Enter, keyboard shortcuts, or any other multi-step interaction. Supports arbitrary Unicode text and emoji.",
    "parameters": {
      "type": "object",
      "properties": {
        "id": {
          "type": "integer",
          "description": "Browser element ID from the most recent browser_observe result. Must identify the input or editable element to fill. Never invent an ID and never use a UUID."
        },
        "text": {
          "type": "string",
          "description": "Text to enter into the element. Supports arbitrary Unicode text, including non-ASCII characters and emoji."
        }
      },
      "required": [
        "id",
        "text"
      ],
      "additionalProperties": false
    }
  }
}