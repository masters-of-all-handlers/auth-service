CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

DROP SCHEMA IF EXISTS bookmarker CASCADE;

CREATE SCHEMA IF NOT EXISTS bookmarker;

CREATE TABLE IF NOT EXISTS bookmarker.users (
    id TEXT PRIMARY KEY DEFAULT uuid_generate_v4(),
    email TEXT UNIQUE NOT NULL,
    password TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS bookmarker.auth_sessions (
    id TEXT PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id TEXT NOT NULL,
    foreign key(user_id) REFERENCES bookmarker.users(id)
);

CREATE TABLE IF NOT EXISTS bookmarker.bookmarks (
    id TEXT PRIMARY KEY DEFAULT uuid_generate_v4(),
    owner_id TEXT,
    url TEXT UNIQUE NOT NULL,
    title TEXT NOT NULL,
    tag TEXT,
    created_ts timestamp not null default current_timestamp,
    foreign key(owner_id) REFERENCES bookmarker.users(id)
);
