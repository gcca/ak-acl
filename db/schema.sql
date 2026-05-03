CREATE TABLE IF NOT EXISTS "schema_migrations" (version varchar(128) primary key);
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
-- Dbmate schema migrations
INSERT INTO "schema_migrations" (version) VALUES
  ('20260503020141');
