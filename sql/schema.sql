-- Run in MySQL Workbench. Creates missing objects without deleting data.
CREATE DATABASE IF NOT EXISTS `saturday` CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE `saturday`;
CREATE TABLE IF NOT EXISTS chat (
  id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
  role VARCHAR(16) NOT NULL,
  message_type VARCHAR(32) NOT NULL,
  message LONGTEXT NOT NULL,
  images JSON NOT NULL
);
CREATE TABLE IF NOT EXISTS system_prompt (
  id BIGINT UNSIGNED NOT NULL PRIMARY KEY,
  content LONGTEXT NOT NULL
);
