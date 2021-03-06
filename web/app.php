<?php
    require_once '../src/function/general.php';
    require_once '../src/function/users.php';
    require_once('../src/model/model.php');

    $task 	 = getvar("task");
    $user = new users();
    switch($task){  		
    	case "remove_user":
    	    if ( !is_Logged() ){
		    	redirect("login.php");
		    }else{
		    	$user_id = getvar("id");
    			if ( $user->remove_user($user_id) ){
    				echo "T";
    			}
		    }
    		break;
    	case "edit_user":
    		if ( !is_Logged() ){
		    	redirect("login.php");
		    }else{
		    	$user_id = getvar("id");
		    	include('../src/html/header.php');
				include('../src/html/menu.php');
				

				$model = &new model();
				$row = $model->get_user_details($user_id);
				include('../src/html/edit.php');

				include('../src/html/footer.php'); 
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

			$model = &new model();
			$row = $model->get_user_details();
			include('../src/html/edit.php');
			include('../src/html/footer.php'); 
    		break;
    	default:
    		redirect("index.php");
    }


?>