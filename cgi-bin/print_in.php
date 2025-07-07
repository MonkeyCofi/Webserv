#!php
<?php
    $file = fopen('php://stdin', 'r');
    echo $file;
    fclose($file);
?>