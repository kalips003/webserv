/* -- create table.
+----+----------+--------+---------------------+
| id | cookieid | name   | created_at          | 
+----+----------+--------+---------------------+
| 1  | abc123   | Alice  | 2026-02-14 13:10   |
| 2  | xyz456   | Bob    | 2026-02-14 13:15   |
| 3  | 456def   | Charlie| 2026-02-14 13:20   |
| 4  | 789ghi   | Dave   | 2026-02-14 13:25   |
+----+----------+--------+---------------------+
+ Num, kitten killed
+ bool, num donations
+ json, data
+ cookie expiration?
*/

CREATE TABLE IF NOT EXIST users (

    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    cookieid TEXT,
    created_at INTEGER,
    updated_at INTEGER,
    cookie_expires INTEGER,
    num_kittens INTEGER DEFAULT 0,
    num_donations INTEGER DEFAULT 0,
    data TEXT

);