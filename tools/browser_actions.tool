{
  "type": "function",
  "function": {
    "name": "browser_actions",
    "description": "Perform a sequential sequence of browser interactions using mouse, keyboard, and wheel input. PREFER THIS TOOL whenever a task requires multiple browser interactions in order, such as click then type, focus then type, type then press Enter, keyboard shortcuts, or mouse + keyboard combinations. Actions execute strictly in the order provided. Use the most recent browser_observe result for element IDs. Never invent element IDs. Do not use browser_fill or browser_click separately when the requested operation can be expressed as one browser_actions sequence. Use the keyboard 'text' action to enter arbitrary text, including Unicode and emoji.",
    "parameters": {
      "type": "object",
      "properties": {
        "actions": {
          "type": "array",
          "description": "Ordered browser actions. Each action is completed before the next action begins.",
          "items": {
            "type": "object",
            "properties": {
              "source": {
                "type": "string",
                "enum": [
                  "keyboard",
                  "mouse",
                  "wheel"
                ],
                "description": "Input source. keyboard performs keyboard actions, mouse performs pointer actions, and wheel performs scrolling."
              },

              "action": {
                "type": "string",
                "enum": [
                  "press",
                  "text",
                  "down",
                  "up",
                  "move",
                  "click",
                  "scroll"
                ],
                "description": "Action type. keyboard: press, text, down, up. mouse: move, click, down, up. wheel: scroll. Use 'text' for entering a complete string instead of generating one press action per character."
              },

              "target": {
                "type": "integer",
                "description": "Browser element ID returned by the most recent browser_observe result. Never use a UUID and never invent an ID. Use this for mouse actions that target an element and for keyboard text that should be entered into a specific element."
              },

              "key": {
                "type": "string",
                "description": "Keyboard key for press, down, or up. Normal keys can be single characters such as A, a, 1, or ?. Special keys include ENTER, TAB, BACKSPACE, ESC, CTRL, SHIFT, ALT, META, LEFT, RIGHT, UP, and DOWN."
              },

              "text": {
                "type": "string",
                "description": "Complete text to enter for a keyboard text action. Supports normal Unicode text, including non-ASCII characters and emoji. Use this instead of generating individual key presses for a string."
              },

              "button": {
                "type": "integer",
                "enum": [
                  0,
                  1,
                  2
                ],
                "description": "Mouse button: 0 = left, 1 = middle, 2 = right."
              },

              "x": {
                "type": "integer",
                "description": "Horizontal pointer offset in pixels for a mouse move."
              },

              "y": {
                "type": "integer",
                "description": "Vertical pointer offset in pixels for a mouse move."
              },

              "delta_x": {
                "type": "integer",
                "description": "Horizontal wheel scroll amount."
              },

              "delta_y": {
                "type": "integer",
                "description": "Vertical wheel scroll amount. Positive values scroll down and negative values scroll up."
              },

              "duration": {
                "type": "integer",
                "description": "Action duration in milliseconds, when applicable."
              }
            },
            "required": [
              "source",
              "action"
            ],
            "additionalProperties": false
          }
        }
      },
      "required": [
        "actions"
      ],
      "additionalProperties": false
    }
  }
}