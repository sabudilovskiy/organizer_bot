CREATE INDEX io_events_idx   ON io_events  (user_id, consumed, ts);
CREATE INDEX time_events_idx ON time_events(user_id, meta_type, consumed, next_occurence);
