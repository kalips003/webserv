import fetch from 'node-fetch';
import express, { response } from 'express';
import cors from 'cors';

const app = express();
const PORT = 3000//process.env.PORT || 3000;

app.use(express.json());
app.use(cors());

let token;
async function set_token()
{
	fetch("https://api.intra.42.fr/oauth/token", {
		method: "POST",
		body: "grant_type=client_credentials&client_id=u-s4t2ud-c226cd35cd1ac08a4c6668deee1c64d7d67a13a766aee672acafd4a1522d483c&client_secret=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",
		headers: {
			"User-Agent": "agallon",
			'Content-Type': 'application/x-www-form-urlencoded',}
	})
	.then(response => {
		return response.json();
	})
	.then(data => {
		token = data;
		setTimeout(set_token, token.expires_in);
	})
	.catch(error => {
        console.error('Error fetching token:', error);
    });
}
set_token();

app.get('/proxy/profile/:login', async (req, res) => {
    const { login } = req.params;
    const profileURL = `https://api.intra.42.fr/v2/users/${login}`;
    try {
        const response = await fetch(profileURL, {
			headers: {
				"Authorization": `Bearer ${token.access_token}`}});
		console.log(`response.status =  ${response.status}`);
		if (response.status !== 200) {
			throw new Error('User not found');
		}
        const data =  await response.json();
	    res.status(200).json(data);
    } catch (error) {
        console.error('Error fetching profile:', error);
        res.status(500).json({ error: 'Failed to fetch profile' });
    }
});

app.listen(PORT, () => {
    console.log(`Proxy server running on port ${PORT}`);
});

