#!/usr/bin/env php
<?php
// login_script.php

// --- Headers for CGI ---
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *'); // optional for local testing

// --- Get login from query ---
$login = $_GET['login'] ?? null;
// $login = 'agallon';
if (!$login) {
    http_response_code(400);
    echo json_encode(['error' => 'Missing login']);
    exit;
}

///////////////////////////////////////////////////////////////////////////////]
// --- OAuth token handler ---
function get_token() {
    $token_file = __DIR__ . '/token.json';
    
    // Check cached token
    if (file_exists($token_file)) {
        $data = json_decode(file_get_contents($token_file), true);
        if ($data && time() < $data['expires_at']) {
            return $data['access_token'];
        }
    }

    // Request new token
    $ch = curl_init('https://api.intra.42.fr/oauth/token');
    curl_setopt_array($ch, [
        CURLOPT_POST => true,
        CURLOPT_RETURNTRANSFER => true,
        CURLOPT_HTTPHEADER => [
            'Content-Type: application/x-www-form-urlencoded',
            'User-Agent: webserv-cgi'
        ],
        CURLOPT_POSTFIELDS => http_build_query([
            'grant_type' => 'client_credentials',
            'client_id' => 'u-s4t2ud-c226cd35cd1ac08a4c6668deee1c64d7d67a13a766aee672acafd4a1522d483c',
            'client_secret' => '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'
            // 'client_id' => 'id',
            // 'client_secret' => 'psswrd'
        ]),
    ]);
    $response = curl_exec($ch);
    curl_close($ch);

    if (!$response) {
        http_response_code(500);
        echo json_encode(['error' => 'Token request failed']);
        exit;
    }

    $data = json_decode($response, true);
    if (!isset($data['access_token'])) {
        http_response_code(500);
        echo json_encode(['error' => 'Invalid token response']);
        exit;
    }

    // Save token with expiration
    $data['expires_at'] = time() + $data['expires_in'] - 30;
    file_put_contents($token_file, json_encode($data));

    return $data['access_token'];
}

// --- Fetch 42 profile ---
$token = get_token();
$api_url = "https://api.intra.42.fr/v2/users/" . urlencode($login);

$ch = curl_init($api_url);
curl_setopt_array($ch, [
    CURLOPT_RETURNTRANSFER => true,
    CURLOPT_HTTPHEADER => [
        "Authorization: Bearer $token"
    ]
]);
$response = curl_exec($ch);
$status = curl_getinfo($ch, CURLINFO_HTTP_CODE);
curl_close($ch);

// --- Return result to browser ---
http_response_code($status);
echo $response;
