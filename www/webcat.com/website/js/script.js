//////////////////////////////////////////////////////////////////////
// CHANGE 404
function changePage() {
    let images = [530, 599, 204, 400, 401, 402, 403, 404, 410];
    var randomErrorNum = images[Math.floor(Math.random() * images.length)];
    window.location.href = `404.html?img=${randomErrorNum}`;
   }

//////////////////////////////////////////////////////////////////////
// STAFF IMG CHANGING
let images = [
    "100.jpg", "101.jpg", "102.jpg", "103.jpg",
    "200.jpg", "201.jpg", "202.jpg", "203.jpg",
    "204.jpg", "205.jpg", "206.jpg", "207.jpg",
    "208.jpg", "226.jpg", "300.jpg", "301.jpg",
    "302.jpg", "303.jpg", "304.jpg", "305.jpg",
    "307.jpg", "308.jpg", "400.jpg", "401.jpg",
    "402.jpg", "403.jpg", "404.jpg", "405.jpg",
    "406.jpg", "407.jpg", "408.jpg", "409.jpg",
    "410.jpg", "411.jpg", "412.jpg", "413.jpg",
    "414.jpg", "415.jpg", "416.jpg", "417.jpg",
    "418.jpg", "420.jpg", "421.jpg", "422.jpg",
    "423.jpg", "424.jpg", "425.jpg", "426.jpg",
    "428.jpg", "429.jpg", "431.jpg", "444.jpg",
    "450.jpg", "451.jpg", "497.jpg", "498.jpg",
    "499.jpg", "500.jpg", "501.jpg", "502.jpg",
    "503.jpg", "504.jpg", "506.jpg", "507.jpg",
    "508.jpg", "509.jpg", "510.jpg", "511.jpg",
    "521.jpg", "522.jpg", "523.jpg", "525.jpg",
    "530.jpg", "599.jpg"
];
var currentIndex = 0;

function changeImage() {
    currentIndex = Math.floor(Math.random() * images.length);
    var randomImageElement = document.getElementById("randomImage");
    randomImageElement.src = 'img/error/' + images[currentIndex];
}
function prevImage() {
    currentIndex = (currentIndex - 1 + images.length) % images.length;
    var currentImage = document.getElementById("randomImage");
    currentImage.src = 'img/error/' + images[currentIndex]
}
function nextImage() {
    currentIndex = (currentIndex + 1) % images.length;
    var currentImage = document.getElementById("randomImage");
    currentImage.src = 'img/error/' + images[currentIndex]
}
document.addEventListener('keyup', function(event) {
    // Check if the pressed key is the left arrow key
    if (event.key === 'ArrowLeft') {
        prevImage();}
    if (event.key === 'ArrowRight') {
        nextImage();}
    if (event.code === 'Space') {
        event.preventDefault();
        changeImage();}
});

//////////////////////////////////////////////////////////////////////
// LEGAL NOTICE POPUP
function updateTime() {
    numberOfCatKilled++; secondPassed++;
    localStorage.setItem('kittenKilled', numberOfCatKilled);
    document.getElementById('timeCounter').textContent = numberOfCatKilled;
    if (secondPassed === 15)
    {show_popup();}
    if (numberOfCatKilled % 2 === 0)
    {
        document.documentElement.style.setProperty('--color1', 'rgb(190, 63, 40)');
        document.documentElement.style.setProperty('--color2', 'rgb(211, 187, 53)');}
    else
    {
        document.documentElement.style.setProperty('--color1', 'rgb(211, 187, 53)');
        document.documentElement.style.setProperty('--color2', 'rgb(190, 63, 40)');}
}

function show_popup() {
    var murderButton = document.getElementById('murderButton');
    murderButton.style.display = 'block';
}

//////////////////////////////////////////////////////////////////////
// INDEX MAIN LOGIN
///////////////////////////////////////////////////////////////////////////////]
async function newStoreValue() {
    const loginInput = document.getElementById('loginInput');
    const login = loginInput.value.trim();
    loginInput.value = '';

    if (!login) return;


    try {
    // Send login to your PHP backend
        const response = await fetch('http://localhost:9999/webcat.com/login/login.php', {
            method: 'POST',
            headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
            body: new URLSearchParams({ login })
        });

    // Check response
        if (!response.ok) {
            const err = await response.json();
            chaberu(`Login failed: ${err.error ?? 'Unknown error'}`, 'popup-chaberu');
            loginInput.placeholder = 'Enter your 42 login';
            return;
        }

    // On success, backend already set the cookie
        const data = await response.json();
        if (data.status === 'OK') {
            localStorage.setItem('storedLogin', login);
            loginInput.placeholder = `Welcome ${login}!`;

            // Optional: reload or fetch user data now that cookie exists
            window.location.reload(); // cookie will be sent automatically
        }
    } catch (err) {
        console.error('Login error:', err);
        chaberu('Network or server error. Try again.', 'popup-chaberu');
    }
}

///////////////////////////////////////////////////////////////////////////////]
function talkWiskas() {
    if (index_wiskas >= discussion.length) return ;
    document.getElementById('popup-chaberu').textContent = '';
    chaberu(discussion[index_wiskas], 'popup-chaberu');
    if (index_wiskas === discussion.length - 1) {
        document.getElementById('header1').textContent = `Time to end this, I have other cats to see.\nEnjoy this place.`;
        document.getElementById('backgroundpicture').src = logUser.photo;
    }
}

///////////////////////////////////////////////////////////////////////////////]
function chaberu(str, id, index = 0) {
    if (chaberuka) return ;
    const chaberuu = () => {
        if (index < str.length) {
            chaberuka = true;
            document.getElementById(id).textContent += str[index];
            index++;
            setTimeout(() => {
                chaberuu(str, id, index);
            }, 20);
        }
        else {
            setTimeout(() => {
                document.getElementById(id).textContent = '';
                chaberuka = false;
                index_wiskas++;
            }, 2000);
        }
    }
    chaberuu();
}
