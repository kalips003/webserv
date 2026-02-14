#!/usr/bin/env php
<?php

///////////////////////////////////////////////////////////////////////////////]
/** get_token()
 *
 * Returns a valid 42 API OAuth token as a string.
 * - Uses cached token from token.json if still valid
 * - Requests a new token from 42 API if no cache or expired
 * - Returns null on any error (caller decides handling)   ---*/
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
        ]),
    ]);
    $response = curl_exec($ch);
    curl_close($ch);

    if (!$response)
        return null;
    
    $data = json_decode($response, true);
    if (!isset($data['access_token']))
        return null;

// Save token with expiration
    $data['expires_at'] = time() + $data['expires_in'] - 30;
    file_put_contents($token_file, json_encode($data));

// Return just the token string
    return $data['access_token'];
}

?>