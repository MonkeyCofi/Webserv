<!DOCTYPE html>
<html>
<head>
	<title>Upload File</title>
	<style>
		body {
			font-family: Arial, sans-serif;
			text-align: center;
			margin-top: 50px;
		}
	</style>
</head>
<body>

<h1>Your uploaded file</h1>

<?php
    // $content_type = "content-type: text/html";
    // $request_method = $_SERVER['REQUEST_METHOD'];
    // $body = "<html><body>" . $request_method . "</body></html>";

    // echo $content_type . "\r\n\r\n";
    // echo $body;

    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_FILES['file'])) {
        $upload_dir = __DIR__ . '/uploads/';
        $filename = basename($_FILES['file']['name']);
        $target_path = $upload_dir . $filename;

        // Create upload directory if it doesn't exist
        if (!is_dir($upload_dir)) {
            mkdir($upload_dir, 0777, true);
        }

        if (move_uploaded_file($_FILES['file']['tmp_name'], $target_path)) {
            echo "<p>File uploaded successfully.</p>";
            $file_url = "uploads/" . urlencode($filename);

            // Check if it's an image and display it
            $mime_type = mime_content_type($target_path);
            if (str_starts_with($mime_type, 'image/')) {
                echo "<img src=\"$file_url\" alt=\"Uploaded Image\" style=\"max-width: 80%; margin-top: 20px;\" />";
            } else {
                echo "<p>Uploaded file: <a href=\"$file_url\">$filename</a></p>";
            }
        } else {
            echo "<p>Failed to upload file.</p>";
        }
    }
?>