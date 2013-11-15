<?php
    require_once('../src/config.php');
    require_once '../src/function/general.php';
    require_once('../src/function/database.php');
    
    db_connect();
        
    $sql = "SELECT * FROM users";
    $query = db_query($sql);
    while($row = db_fetch_array($query)){
        print_r($row);
    }

?>