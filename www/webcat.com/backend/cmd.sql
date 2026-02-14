

/* -- create table.
+----+----------+--------+---------------------+
| id | cookieid | name   | created_at          |  ← columns (vertical headers)
+----+----------+--------+---------------------+
| 1  | abc123   | Alice  | 2026-02-14 13:10   |
| 2  | xyz456   | Bob    | 2026-02-14 13:15   |
| 3  | 456def   | Charlie| 2026-02-14 13:20   |
| 4  | 789ghi   | Dave   | 2026-02-14 13:25   |
+----+----------+--------+---------------------+*/

CREATE TABLE users (
id INTEGER PRIMARY KEY,
cookieid TEXT NOT NULL UNIQUE,
name TEXT NOT NULL UNIQUE,
created_at DATETIME DEFAULT CURRENT_TIMESTAMP);

-- insert new row
INSERT INTO users (cookieid, name)
VALUES ('abc123', 'agallon');

-- checks
SELECT * FROM users;

-- insert new column
ALTER TABLE users ADD COLUMN kitten_killed INTEGER DEFAULT 0;
ALTER TABLE users ADD COLUMN token TEXT;

-- update column for everyone (UNIQUE)
-- string concatenation: string1 || string2 = string1string2
UPDATE users SET token = 'token_' || id;

-- ALTER FUNCTIONALITIES
ALTER TABLE old_name RENAME TO new_name; -- RENAME TABLE
ALTER TABLE users RENAME COLUMN old_col TO new_col; -- RENAME COLUMN
