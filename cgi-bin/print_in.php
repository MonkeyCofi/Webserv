#!php
<?php
    $len = getenv("CONTENT_LENGTH)");
    echo $len;
    echo "Content-type: text/html\r\n";
    echo "\r\n";
    $file = fopen('php://stdin', 'r');
    echo "<html><body>\n";
    echo "<p>\n";
    while ($line = fgets($file))
        echo $line;
    fclose($file);
    echo "</p>\n";
    echo "<h1>Done</h1>";
    echo "</body></html>";
?>