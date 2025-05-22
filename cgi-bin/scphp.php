<?php
    $body = "hello there. I'm trying to figure out who you are. Please give me a moment";
    $content_length = strlen($body);
    sleep(10);
    echo "HTTP/1.1 200 OK\r\n";
    echo "Content-Length: ${content_length}\r\n";
    echo "Content-Type: text/html\r\n\r\n";
    echo $body;
    // $name = getenv("QUERY_STRING")->substr();
?>