<?php

// index.php

// --- Open database ---
$db = new SQLite3(__DIR__ . '/../backend/database.db');

// --- Check if user is logged in via cookie ---
$cookie = $_COOKIE['webserv_user'] ?? null;
$user_data = null;

if ($cookie) {
    $stmt = $db->prepare('SELECT * FROM users WHERE cookieid = :cookie AND cookie_expires > :now');
    $stmt->bindValue(':cookie', $cookie, SQLITE3_TEXT);
    $stmt->bindValue(':now', time(), SQLITE3_INTEGER);
    $result = $stmt->execute();
    $user_row = $result->fetchArray(SQLITE3_ASSOC);

    if ($user_row) {
        $user_data = json_decode($user_row['data'], true);
    }
}

// --- Default values if not logged in ---
$header1_text = 'Welcome to CAT !';
$discussion = [
        "Well hi there...",
        "Please refrain from touching\n the yellow button",
        "We are going to take actions\n if you continue..",
        "Actions taken,\n you are only\n making it worse.."
    ]; // empty array
if ($user_data) {
    $firstName = $user_data['firstName'];
    $lastName  = $user_data['lastName'];
    $month     = $user_data['month'];
    $year      = $user_data['year'];
    $projects  = $user_data['projects'];
    $perfect   = $user_data['perfect'];
    $bh        = $user_data['bh'];

    $header1_text = "Let's talk a bit, {$firstName}!";

    $discussion = [
        "Welcome {$firstName} {$lastName}.",
        "We heard quite a lot about the piscine of {$month} {$year}...\nIt's surprising to see you here",
        "How is your {$projects[array_rand($projects)]} coming along?",
        "Perfect score for {$perfect[array_rand($perfect)]}, impressive.. Should you really spend so much time in front of a screen?",
        "Your BH is in {$bh} days... A cat wouldn't take that much time.",
        "Shouldn't you be working on your {$projects[array_rand($projects)]}?",
        "Quite an ugly human...\n but then again, you aren't a cat"
    ];
}
$discussion = json_encode($discussion);

// --- Serve HTML ---
?>
<!DOCTYPE html>
<html>
	<head>
		<link rel="stylesheet" type="text/css" href="css/style2.css">
		<script src="js/script.js"></script>
		<title>CAT</title>
	</head>
	<body>
		<!--🐾🐈🐱-->
		<div class="header_index">
			<button class="button1" style="margin-left: 50px;" onclick="window.location.href='donate.html'">Donate</button>
			<div>
				<h1 id="header1" class="header1" onmouseover="this.style.backgroundColor='#363636'" onmouseout="this.style.backgroundColor='black'" onclick="talkWiskas()">Welcome to CAT !</h1>
			</div>
			<span style="margin-right: 30px;">
				<div>
					<input type="text" id="loginInput" placeholder="enter your 42 login">
				</div>
				<div style="display: flex; justify-content: space-between;">
					<button class="button1" onclick="newStoreValue();">login</button>
					<button class="button1" onclick="window.location.href = `https://profile.intra.42.fr/users/${storedLogin}`;">go to ></button>
				</div>
			</span>
		</div>

		<div class="popup-chaberu" id="popup-chaberu" style="color: rgb(218, 145, 12);"></div>

		<img id="backgroundpicture" style="margin: auto; display: block;" src="img/wiskas-the-third.jpg">

		<section style="display: flex;
				justify-content: center;
				width: 1000px;
				margin: 0 auto;">
			<p>I, am wiskas-the-third,
				We are the  cat company, we  dont need to present our self for you already know
				who we are, we created the internet, and we are still managing it now<br>
				We at CAT are the admin, creator, and workers of the internet
				Everytime a  human goes to sleep, a cat start its shift, 1 billion pair of whiskers that are always here for you
				Why? because we are philantropists, dont question it. Our goals are beyond your understanding
				the internet was created by us, for us, and you should be glad we allow you to use it.
			</p>
		</section>

		<section style="display: flex;">
			<button style="margin-left: 50px;" class="button1" onclick="window.location.href = 'biblio.html';">
				Latest News</button><br>
			<button style="margin-left: 50px;" class="button1" onclick="window.location.href = 'staff.html';">
				meet the staff</button><br>
		</section>

		<footer>
			<br><br><br>
			<div class="footer_div" style="margin-top: 100px;">
				<img class="ico_footer" src="img/facebook_logo.png">
				<img class="ico_footer" src="img/insta_logo.png">
				<img class="ico_footer" src="img/twitter_logo.png">
			</div>
			<div class="footer_div" style="margin-bottom: 50px;">
				<a href="https://www.facebook.com/">MIAOUBOOK</a>
				<a href="https://www.instagram.com/">INSTAMIA</a>
				<a href="https://twitter.com/">BLUE-SNACK</a>
			</div>
			<a href="mentions_legales.html">- LEGAL NOTICES -<br>(boring stuff, really, dont go look into this, i mean we are obligated to include it, but it will bore you, like, really)
			<br>Dont do it! every seconds you spend in this next page, a kitten dies. so dont</a>
		</footer>

        <script>
            var storedLogin;
            var logUser = <?php echo $user_data ? json_encode($user_data) : 'null'; ?>;
            var discussion = <?php echo $discussion_json; ?>;
            var index_wiskas = 0;
            let chaberuka = false;

            document.getElementById("loginInput").addEventListener("keydown", function(event) {
                if (event.keyCode === 13) { storeValue() }
            });
        </script>
	</body>
</html>
