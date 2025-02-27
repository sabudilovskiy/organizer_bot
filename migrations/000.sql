CREATE TABLE users (
    user_id             INTEGER PRIMARY KEY        NOT NULL,
    chat_id             INTEGER                    NOT NULL,
    message_id          INTEGER                            ,
    additional_messages INTEGER                    NOT NULL,
    gmt_offset_m        INTEGER                    NOT NULL
);

CREATE TABLE tasks (
    id          INTEGER PRIMARY KEY AUTOINCREMENT  NOT NULL,
    user_id     INTEGER                            NOT NULL,
    title       TEXT                               NOT NULL,
    description TEXT                               NOT NULL,
    status      INTEGER                            NOT NULL,
    created_at  TEXT                               NOT NULL
);

CREATE TABLE events (
    event_id INTEGER PRIMARY KEY AUTOINCREMENT    NOT NULL,
    user_id  INTEGER                              NOT NULL,
    ts       TEXT                                 NOT NULL,
    meta     TEXT                                 NOT NULL,
    consumed INTEGER                              NOT NULL
);
