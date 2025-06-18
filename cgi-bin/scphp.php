<?php
    // sleep(2);
    $body = "<p>Test</p>\n";
    $query_string = getenv("QUERY_STRING");
    $name = strstr($query_string, "=");
    $name = substr($name, ($name[0] == '='));
    $final_print = "<h3>hello, " . $name . "</h3>";
    $content_length = strlen($body) + strlen($final_print);
    echo "HTTP/1.1 200 OK\r\n";
    echo "Content-Length: ${content_length}\r\n";
    echo "Content-Type: text/html\r\n\r\n";
    echo $body;
    echo $final_print;
?>