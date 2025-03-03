CREATE INDEX events_idx ON events(user_id, consumed, ts);
CREATE INDEX tasks_idx  ON events(user_id);
CREATE INDEX calls_idx  ON events(user_id, ts);