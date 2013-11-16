<?php
    require_once '../src/function/general.php';
    require_once '../src/function/rest.php';
    
    $rest = &new rest();
    $rest->connect();

    header('Content-type: application/json');
    echo $rest->exec();
?>