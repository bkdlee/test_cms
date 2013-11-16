<?php
    require_once '../src/function/general.php';
    if ( !is_Logged() ){
    	redirect("login.php");
    }


    $task = isset( $_GET['task'] ) ? $_GET['task'] : "";

    switch($task){
    	case "remove_user":
    		break;
    	case "edit_user":
    		
    		break;
    	default:
    		redirect("index.php");
    }


?>