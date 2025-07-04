#!php
<?php
    $file = fopen('php://stdin', 'r');
    while (!feof($file))
    {
        $buffer = fgets($file);
        echo $buffer;
    }
    fclose($file);
?>