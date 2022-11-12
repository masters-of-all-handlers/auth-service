CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

DROP SCHEMA IF EXISTS admins CASCADE;

CREATE TABLE IF NOT EXISTS uservice_dynconf.users(
    id TEXT PRIMARY KEY DEFAULT uuid_generate_v4(),
    login TEXT UNIQUE NOT NULL,
    password TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS uservice_dynconf.auth_sessions (
    id TEXT PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id TEXT NOT NULL,
    foreign key(user_id) REFERENCES uservice_dynconf.users(id)
);

