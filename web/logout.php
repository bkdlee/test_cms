<?php
	require_once '../src/function/general.php';
	session_destroy();
	redirect("login.php");
?>