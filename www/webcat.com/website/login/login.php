<?php

// includes
require_once __DIR__ . '/../../backend/f_get_json.php';

///////////////////////////////////////////////////////////////////////////////]
// open database
$db = new SQLite3(__DIR__ . '/../backend/database.db');
// If tables don't exist, create them from init.sql
$db->exec(file_get_contents(__DIR__ . '/../backend/init.sql'));

// Send headers first
header('Content-Type: application/json'); // ???

// get username from get query
$login = $_POST['login'] ?? null; // or $_GET if using GET
if (!$login) {
    http_response_code(400);
    echo json_encode(['error' => 'Missing login']);
    exit;
}

// check username if any in db
$stmt = $db->prepare('SELECT * FROM users WHERE name = :name');
$stmt->bindValue(':name', $login, SQLITE3_TEXT);
$result = $stmt->execute();
$user_row = $result->fetchArray(SQLITE3_ASSOC);

$now = time();
if (!$user_row || $user_row['cookie_expires'] < $now) {
    $user_data = get_json($login);
    if (!$user_data) {
        http_response_code(404);
        echo json_encode(['error' => 'Wrong login']);
        exit;
    }
} else {
   $user_data = json_decode($user_row['data'], true);
}

// set cookie function if successful
$cookie_id = bin2hex(random_bytes(16));  // generate random cookie
$expires = time() + 86400;                // 1 day from now
setcookie('webserv_user', $cookie_id, $expires, '/', '', false, true);

// insert back into bd
if ($user_row) {
    // User exists → update
    $stmt = $db->prepare('
        UPDATE users
        SET cookieid = :cookieid,
            updated_at = :updated,
            cookie_expires = :expires,
            data = :data
        WHERE name = :name
    ');
    $stmt->bindValue(':cookieid', $cookie_id, SQLITE3_TEXT);
    $stmt->bindValue(':updated', $now, SQLITE3_INTEGER);
    $stmt->bindValue(':expires', $expires, SQLITE3_INTEGER);
    $stmt->bindValue(':data', json_encode($user_data), SQLITE3_TEXT);
    $stmt->bindValue(':name', $login, SQLITE3_TEXT);
    $stmt->execute();
} else {
    // New user → insert
    $stmt = $db->prepare('
        INSERT INTO users (name, cookieid, created_at, updated_at, cookie_expires, data)
        VALUES (:name, :cookieid, :created, :updated, :expires, :data)
    ');
    $stmt->bindValue(':name', $login, SQLITE3_TEXT);
    $stmt->bindValue(':cookieid', $cookie_id, SQLITE3_TEXT);
    $stmt->bindValue(':created', $now, SQLITE3_INTEGER);
    $stmt->bindValue(':updated', $now, SQLITE3_INTEGER);
    $stmt->bindValue(':expires', $expires, SQLITE3_INTEGER);
    $stmt->bindValue(':data', json_encode($user_data), SQLITE3_TEXT);
    $stmt->execute();
}

http_response_code(200);
echo json_encode([
    'status' => 'OK'
]);

?>