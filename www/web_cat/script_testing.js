//////////////////////////////////////////////////////////////////////
function sendHello() {
	const storedLogin = localStorage.getItem('storedLogin');
	let path_to_upload = 'donations/filehello'; // path to upload to relative to website root
	let body_to_upload = 'anonymous Hello\n';
	if (storedLogin) {
		path_to_upload = 'donations/hello/donations_hello'; // path to upload if logged
		body_to_upload = `${storedLogin} donated a Hello\n`;
	}

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
	const path_to_upload = 'donations/file2'; // path to upload to relative to website root

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

function uploadFiles() {
	const path_to_upload = 'donations/file2'; // server endpoint
	
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
    const path_to_delete = 'donations/file1';  // relative to website root

    fetch(path_to_delete, {
        method: 'DELETE'
    })
    .then(response => response.text())
    .then(data => alert('Server response: ' + data))
    .catch(err => console.error(err));
}

//////////////////////////////////////////////////////////////////////
function callCgi() {
	fetch("script/test.py?hello=world", {
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
