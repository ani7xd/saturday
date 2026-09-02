{
  "type": "function",
  "function": {
    "name": "edit_file",
    "description": "Edit an existing local file by replacing exact text. The replacement begins at the specified zero-based occurrence offset. By default, only one matching occurrence is replaced. Set occurrence_count to 0 to replace every matching occurrence from the specified offset onward. After a successful edit, do not call this tool again unless another edit is required.",
    "parameters": {
      "type": "object",
      "properties": {
        "file": {
          "type": "string",
          "description": "Path to the existing local file to edit."
        },
        "old_text": {
          "type": "string",
          "description": "Exact text to find and replace. Use enough surrounding context to uniquely identify the intended text when necessary."
        },
        "new_text": {
          "type": "string",
          "description": "Text that replaces each matched occurrence."
        },
        "occurrence_count": {
          "type": "integer",
          "minimum": 0,
          "default": 1,
          "description": "Number of matching occurrences to replace. Defaults to 1. Set to 0 to replace all matching occurrences from occurrence_offset onward."
        },
        "occurrence_offset": {
          "type": "integer",
          "minimum": 0,
          "default": 0,
          "description": "Zero-based occurrence offset at which replacement begins. 0 means the first matching occurrence, 1 means the second, 2 means the third, and so on."
        }
      },
      "required": [
        "file",
        "old_text",
        "new_text"
      ]
    }
  }
}