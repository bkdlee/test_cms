<?php
    //session_name('icsid');
    //session_start();
    $path = dirname($_SERVER['PHP_SELF']) . '/';
    session_set_cookie_params($expire=0, $path);

    if ((!isset($_SESSION)) OR (empty($_SESSION))) {
    	
        session_name('sid');
        
        // allow session_id passed via GET
        if (isset($_GET[session_name()])) {
            session_id($_GET[session_name()]);
        }
	    // allow session_id passed via POST prefer GET
        if (!isset($_GET[session_name()]) && isset($_POST[session_name()])) {
            session_id($_POST[session_name()]);
        }
    	
        session_start();
    }
    
    $agent = (isset($_SERVER['HTTP_USER_AGENT']) ? $_SERVER['HTTP_USER_AGENT'] : 'NotSpecified');
    $session_key = ((isset($_SERVER['REMOTE_ADDR'])) ? $_SERVER['REMOTE_ADDR'] : 'LOCAL') . '/' . $agent;
    if (!isset($_SESSION['session_key'])) {
        $_SESSION['session_key'] = $session_key;
    }

    if ($_SESSION['session_key'] != $session_key) {
        if ($_SERVER['HTTP_USER_AGENT'] == 'Shockwave Flash') {
            // ok
        } else {
            $_SESSION = array();
            if (isset($_COOKIE[session_name()])) {
                setcookie(session_name(), '', time()-42000, '/');
            }
            session_destroy();
            $new_session_id = md5(rand(10000000,9999999) . microtime(true) . print_r($_SERVER, 1) . rand(1000000,9999999));
            session_id($new_session_id);
            session_start();
            $_SESSION['session_key'] = $session_key;
            session_write_close();
            header('Location: session_start.php');
            exit();
        }
    }
    if ( (isset($start_new_session) && $start_new_session === true)) {
    	$_SESSION = array();
        if (isset($_COOKIE[session_name()])) {
            setcookie(session_name(), '', time()-42000, '/');
        }
        session_destroy();

        $new_session_id = md5(rand(10000000,9999999) . microtime(true) . print_r($_SERVER, 1) . rand(1000000,9999999));

        session_id($new_session_id);
        session_start();

        $application = new application_class();
        $_SESSION['x_application'] = $application;
        $_SESSION['x_server']      = $_SERVER['SERVER_NAME'];
        if (isset($_REQUEST['error'])) {
            $_SESSION['session_check_message'] = $_REQUEST['error'];
        }
    }
