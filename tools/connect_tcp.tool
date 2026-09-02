{
  "type": "function",
  "function": {
    "name": "connect_to_tcp",
    "description": "Connect to a TCP server at the specified IP address and port.",
    "parameters": {
      "type": "object",
      "properties": {
        "ip": {
          "type": "string",
          "description": "IPv4 address or hostname of the TCP server."
        },
        "port": {
          "type": "integer",
          "description": "TCP port number to connect to.",
          "minimum": 1,
          "maximum": 65535
        }
      },
      "required": ["ip", "port"],
      "additionalProperties": false
    }
  }
}