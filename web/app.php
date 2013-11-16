<?php
    require_once '../src/function/general.php';
    require_once '../src/function/users.php';

    $task 	 = getvar("task");
    $user = new users();
    switch($task){
    	case "remove_user":
    	    if ( !is_Logged() ){
		    	redirect("login.php");
		    }else{
		    	$user_id = getvar("id");
    			$user->remove_user($user_id);
		    }
    		break;
    	case "edit_user":
    		if ( !is_Logged() ){
		    	redirect("login.php");
		    }else{
		    	$user_id = getvar("id");
		    }
			
			break;
		case "save":
			if ( isset($_POST) ){
				$user->save($_POST);
			}
			break;
    	case "add_user":
			include('../src/html/header.php');
			include('../src/html/menu.php');
			include('../src/html/edit.php');
			include('../src/html/footer.php'); 
    		break;
    	default:
    		redirect("index.php");
    }


?>