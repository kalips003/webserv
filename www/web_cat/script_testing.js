//////////////////////////////////////////////////////////////////////
function sendHello() {
	const path_to_upload = 'donations/file1'; // path to upload to relative to website root

	fetch(path_to_upload, {  // hardcoded location on your server
		method: 'POST',
		headers: {
			'Content-Type': 'application/x-www-form-urlencoded'
		},
		body: 'message=hello'
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
