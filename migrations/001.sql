CREATE INDEX io_events_idx ON io_events(user_id, consumed, ts);
CREATE INDEX tasks_idx     ON io_events(user_id);
