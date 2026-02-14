#!/usr/bin/env php
<?php

require_once __DIR__ . '/f_get_token.php';
///////////////////////////////////////////////////////////////////////////////]
/** fetch_42_user
 *
 * Fetches a 42 user profile JSON by login.
 * Returns associative array on success, null on failure.   ---*/
function get_json(string $login) {

    $token = get_token();
    if (!$token) return null; // token error

    $api_url = "https://api.intra.42.fr/v2/users/" . urlencode($login);
    $ch = curl_init($url);
    curl_setopt_array($ch, [
        CURLOPT_RETURNTRANSFER => true,
        CURLOPT_HTTPHEADER => [
            "Authorization: Bearer $token",
            "User-Agent: webserv-cgi"
        ]
    ]);

    $response = curl_exec($ch);
    $status = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    curl_close($ch);

    if (!$response || $status !== 200) return null; // request failed

    $data = json_decode($response, true);
    if (!is_array($data)) return null; // invalid JSON

    return $data;
}

?>