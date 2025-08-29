#!/usr/bin/php
<?php
	echo "HTTP/1.1 200 OK\r\n";
	echo "Content-type:text/html\r\n";
	echo "\r\n";
	echo "<html><body>\r\n";
	echo "<h2>Server Time: " . date('Y-m-d H:i:s') . "</h2>\r\n";
	echo "<form method=\"GET\" action=\"./time.php\">";
	echo "<button type=\"submit\">Check time again</button>";
	echo "</form>";
	echo "</body></html>\r\n";
	exit(0);
?>
