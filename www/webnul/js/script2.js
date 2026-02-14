//////////////////////////////////////////////////////////////////////
function sendHello() {
	let path_to_upload = 'donations2/hellos/'; // path to upload to relative to website root
	let body_to_upload = 'anonymous Hello\n';
	
	fetch(path_to_upload, {  // hardcoded location on your server
		method: 'POST',
		headers: {
			'Content-Type': 'application/x-www-form-urlencoded'
		},
		body: body_to_upload
	})
	.then(response => response.text())
	.then(data => alert('Server response: ' + data))
	.catch(err => console.error(err));
}

//////////////////////////////////////////////////////////////////////
function uploadFile() {
	const path_to_upload = 'donations2/uploads/'; // path to upload to relative to website root

	const file = document.getElementById('fileInput').files[0];
	if (!file) {
		alert('No file selected');
		return;
	}

	const formData = new FormData();
	formData.append('file', file);

	fetch(path_to_upload, {  // relative path to the root of your website
		method: 'POST',
		body: formData
	})
	.then(response => response.text())
	.then(data => alert('Server response: ' + data))
	.catch(err => console.error(err));
}

//////////////////////////////////////////////////////////////////////
function uploadFilePut() {
	const path_to_upload = 'donations2/uploadsPut/post_test'; // path to upload to relative to website root

	const file = document.getElementById('fileInput').files[0];
	if (!file) {
		alert('No file selected');
		return;
	}

	const formData = new FormData();
	formData.append('file', file);

	fetch(path_to_upload, {  // relative path to the root of your website
		method: 'PUT',
		body: formData
	})
	.then(response => response.text())
	.then(data => alert('Server response: ' + data))
	.catch(err => console.error(err));
}

//////////////////////////////////////////////////////////////////////
function uploadFiles() {
	const path_to_upload = 'donations2/uploads/'; // server endpoint
	
	const files = document.getElementById('fileInput').files;
	if (!files.length) {
		alert('No files selected');
		return;
	}

	const formData = new FormData();
	for (let i = 0; i < files.length; i++) {
		// append each file; you can use the same field name or different names
		formData.append('files[]', files[i]);
	}

	fetch(path_to_upload, {
		method: 'POST',
		body: formData
	})
	.then(response => response.text())
	.then(data => alert('Server response: ' + data))
	.catch(err => console.error(err));
}

//////////////////////////////////////////////////////////////////////
function deleteFile() {
	const path_to_delete = 'donations2/hellos/webserv_tmp_1';  // relative to website root

	fetch(path_to_delete, {
		method: 'DELETE'
	})
	.then(response => response.text())
	.then(data => alert('Server response: ' + data))
	.catch(err => console.error(err));
}

//////////////////////////////////////////////////////////////////////
function callCgi() {
	fetch("cgi_test/test.py?hello=world", {
		method: "GET"
	})
	.then(res => res.text())
	.then(text => {
		console.log("CGI response:");
		console.log(text);
		alert(text);
	})
	.catch(err => {
		console.error("CGI error:", err);
		alert("CGI failed");
	});
}

//////////////////////////////////////////////////////////////////////
function testOptions(url) {
	const resultBox = document.getElementById('optionsResult');

	fetch(url, {
			method: 'OPTIONS'
		})
		.then(response => {
			const allow = response.headers.get('Allow');
			const corsMethods = response.headers.get('Access-Control-Allow-Methods');

			resultBox.textContent =
				'URL: ' + url + '\n' +
				'Status: ' + response.status + '\n' +
				'Allow: ' + (allow || 'not provided') + '\n' +
				'Access-Control-Allow-Methods: ' + (corsMethods || 'not provided');
		})
		.catch(err => {
			resultBox.textContent = 'Error: ' + err;
		});
}

//////////////////////////////////////////////////////////////////////
function rewriteDirectoryLinks(url) {
	const container = document.getElementById('directoryContainer');

	fetch(url)
		.then(res => res.text())
		.then(html => {
			// Parse the autoindex HTML
			const parser = new DOMParser();
			const doc = parser.parseFromString(html, 'text/html');

			// Rewrite all <a> tags to call testOptions()
			doc.querySelectorAll('a').forEach(link => {
				const href = link.getAttribute('href');
				link.removeAttribute('href');
				link.style.cursor = 'pointer';
				link.onclick = () => testOptions(href);
			});

			container.innerHTML = '';
			container.appendChild(doc.body);
		})
		.catch(err => {
			container.textContent = 'Error fetching directory: ' + err;
		});
}