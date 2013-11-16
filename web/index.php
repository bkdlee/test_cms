<?php
    require_once '../src/function/general.php';
    
    if ( !is_Logged() ){
    	redirect("login.php");
    }
	include('../src/html/header.php'); 
	include('../src/html/menu.php');
	include('../src/model/model.php');

	$model = &new model();
	$rows = $model->get_user_list();
	include('../src/html/userlist.php');

	include('../src/html/footer.php'); 


?>