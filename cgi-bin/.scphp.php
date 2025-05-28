#!/usr/bin/env php
<?php
// Get the script name from environment variables
$script_name = getenv("SCRIPT_NAME");
if (!$script_name) {
    $script_name = "unknown";
}

$query = getenv('QUERY_STRING');
if (!$query){
    $query = "nothing";
}

// This string contains the body
$body = "<html><center><h1>Request received. Script $script_name executed successfully</h1><h2>Server queried for $query</h2></center></html>";
$content_length = strlen($body);
// Output HTTP headers
echo "HTTP/1.1 200 OK\r\n";
echo "Content-Length: ${content_length}\r\n";
echo "Content-Type: text/html\r\n\r\n";
echo $body;
?>