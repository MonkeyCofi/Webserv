import requests

response = requests.get(
    'http://127.0.0.1:1234/cgi-bin/scphp.php?name=<script>alert("Hi pierce")</script>`',
)

print(response.text)