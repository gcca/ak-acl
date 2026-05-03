-- migrate:up

CREATE TABLE auth_user (
  username TEXT PRIMARY KEY,
  password TEXT NOT NULL,
  created_at TEXT NOT NULL DEFAULT (CURRENT_TIMESTAMP)
);

CREATE TABLE auth_session (
  key TEXT PRIMARY KEY,
  username TEXT NOT NULL REFERENCES auth_user (username),
  expires_at TEXT NOT NULL,
  created_at TEXT NOT NULL DEFAULT (CURRENT_TIMESTAMP)
);

CREATE TABLE auth_application (
  client_id TEXT PRIMARY KEY,
  client_secret TEXT NOT NULL,
  grant_type TEXT NOT NULL,
  scope TEXT NOT NULL,
  expires_in INTEGER NOT NULL,
  created_at TEXT NOT NULL DEFAULT (CURRENT_TIMESTAMP)
);

CREATE TABLE auth_token (
  access_token TEXT PRIMARY KEY,
  expires_at TEXT NOT NULL,
  scope TEXT NOT NULL,
  revoked INTEGER NOT NULL DEFAULT 0 CHECK (revoked IN (0, 1)),
  created_at TEXT NOT NULL DEFAULT (CURRENT_TIMESTAMP)
);

CREATE TABLE inventory_tenant (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL,
  created_at TEXT NOT NULL DEFAULT (CURRENT_TIMESTAMP)
);

CREATE TABLE inventory_employee (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL,
  plate TEXT NOT NULL,
  document_type TEXT NOT NULL CHECK (document_type IN ('dni', 'ruc', 'ca', 'pass')),
  document_number TEXT NOT NULL,
  created_at TEXT NOT NULL DEFAULT (CURRENT_TIMESTAMP)
);

-- migrate:down

DROP TABLE IF EXISTS inventory_employee;
DROP TABLE IF EXISTS inventory_tenant;

DROP TABLE IF EXISTS auth_application;
DROP TABLE IF EXISTS auth_token;
DROP TABLE IF EXISTS auth_session;
DROP TABLE IF EXISTS auth_user;
