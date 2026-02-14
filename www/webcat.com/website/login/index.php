<?php
$cookie_name = "session_id";
$html = file_get_contents('index.html');  // default template

$js_injection = '';

if (isset($_COOKIE[$cookie_name])) {
    $session_id = $_COOKIE[$cookie_name];

    // lookup in your server-side cookie map
    // example:
    $user = $server_cookies_map[$session_id] ?? null;

    if ($user) {
        $storedLogin = json_encode($user['login']);  // safely quote
        $logUser = json_encode($user);               // the whole object

        // discussion array can be rebuilt dynamically
        $discussion = json_encode([
            "Welcome {$user['firstName']} {$user['lastName']}.",
            "We heard quite a lot about the piscine of {$user['month']} {$user['year']}...\nIt's surprising to see you here",
            // etc...
        ]);

        $js_injection = "<script>
            var storedLogin = $storedLogin;
            var logUser = $logUser;
            var discussion = $discussion;
            var index_wiskas = 0;
        </script>\n";
    }
}

// Inject JS just before your existing <script> tag
$html = preg_replace('/<script>/', $js_injection . '<script>', $html, 1);

header('Content-Type: text/html');
echo $html;
