<?php
	require_once '../src/function/general.php';
	$error_message = "";
	if ( isset($_POST['email']) && isset($_POST['password']) ){
		if ( check_login_details(postvar('email'), postvar('password') ) ){
			redirect("index.php");
		}else{
			$error_message = "Email address or password is not correct, plese try again.";
		}
	}
	include('../src/html/header.php');
	include('../src/html/login.php');
	include('../src/html/footer.php'); 


?>