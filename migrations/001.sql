CREATE INDEX events_idx ON events(user_id, consumed, ts);
CREATE INDEX tasks_idx  ON events(user_id);
