INSERT INTO system_prompt (id, content) VALUES (1, ?)
ON DUPLICATE KEY UPDATE content = VALUES(content);
