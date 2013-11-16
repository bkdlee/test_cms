<?php
    require_once '../src/function/general.php';
    
    if ( !is_Logged() ){
    	redirect("login.php");
    }
include('../src/html/header.php'); 
include('../src/html/menu.php');
include('../src/html/userlist.php');
include('../src/html/footer.php'); ?>