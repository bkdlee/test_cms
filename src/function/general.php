<?php
    require_once '../src/config.php';
    require_once '../src/function/database.php';
    require_once '../src/function/session.php';


    function check_login_details($email, $password){
        $result = false;
        if ((!isset($_SESSION)) OR (empty($_SESSION))) {          
            session_start();
        }
        
        db_connect();
        $sql = "SELECT * 
                FROM users 
                WHERE active = 'T' AND email = ". db_string($email)
                ;
        $query = db_query($sql);
        if ( db_num_rows($query) > 0 ){
            $row = db_fetch_array($query);
            if ( md5($password) === $row['password']){
                $_SESSION['user_id'] = $row['id'];
                $_SESSION['group_id'] = $row['group_id'];
                $_SESSION['first_name'] = $row['first_name'];
                $_SESSION['last_name'] = $row['last_name'];
                $result = true;
            }
        }
        db_close();
        return $result;
    }

    function is_Logged(){
        $result = false;
        if( isset($_SESSION['user_id']) && $_SESSION['user_id'] > 0 ){
            $result = true;
        }
        return $result;
    }

    function generate_random_password($char_length, $num_length) {
        $chars='BCDEFHJKLMNPQRTWXYZ';
        $numbers='123789';
        return random_string($chars, $char_length) . random_string($numbers, $num_length);
    }

    function random_string($allowed_chars, $length) {
        $rndstring = '';
        for ($a=0;$a<=$length;$a++) {
            $b=rand(0, strlen($allowed_chars) - 1);
            $rndstring .= $allowed_chars[$b];
        }
        return $rndstring;
    }

    function repl_fix_filename($filename) {
        $allowed='abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.';
        for ($i=0;$i<strlen($filename);$i++) {
            $x = $filename[$i];
            if (strpos($allowed, $x) === false) {
                $filename[$i]='_';
            }
        }
        return $filename;
    }
    function die_err($message){
    	die($message);
    }
    function explode_user_agent($user_agent=null) {

        $result = array('browser' => 'unknown', 'os' => 'unknown', 'agent' => 'unknown');
        $user_agent = ((is_null($user_agent)) ? $_SERVER['HTTP_USER_AGENT'] : $user_agent);
        if(ini_get("browscap")) {
           $b = get_browser($user_agent);
           if ($b !== false) {
               $result = array(
                   'browser'   => $b->browser . ' ' . $b->version,
                   'os'        => $b->platform,
                   'agent'     => $user_agent
               );
            }
        }
        else {
           // No BrowsCap can be found. Do a poor man's browser detection
           $intpos = strpos($user_agent, 'Gecko');
           if ($intpos !== false) $result['browser'] = 'Firefox';
           $intpos = strpos($user_agent, 'Chrome');
           if ($intpos !== false) $result['browser'] = 'Google Chrome';
           $intpos = strpos($user_agent, 'MSIE');
           if ($intpos !== false) $result['browser'] = substr($user_agent, $intpos, 8);

           $intpos = strpos($user_agent, 'Mac OS');
           if ($intpos !== false) $result['os'] = 'Mac';
           $intpos = strpos($user_agent, 'Mac iPhone');
           if ($intpos !== false) $result['os'] = 'iPhone';
           $intpos = strpos($user_agent, 'Windows');
           if ($intpos !== false) $result['os'] = 'MS-Windows';

        }
        return $result;
    }
    function seconds_to_time($seconds) {
        $day = 60 * 60 * 24;
        $hour = 60 * 60;
        $minute = 60;

        $days = 0;
        $hours = 0;
        $minutes = 0;

        if (abs($seconds) > $day) {
            $days = floor($seconds / $day);
            $label = ' ago';
            if ($seconds > 0) $label = ' ahead';
            return $days . " days" . $label;
        }

        return "Today";
    }
    function redirect($url) {
        session_write_close();
        header('Location: ' . $url);
        exit();
    }
    function get_good_filename($filename) {
        $filename = basename($filename);
        $find=array("/","?", "*", ",", ".", "~", "!", "@", "#", "$", "%", "%", "^", "&", "(", ")", ";", ":", '"', '"', '[', ']', '|', '+', '=', '-', '`', "\\"); // "
        return str_replace($find, '_', $filename);
    }

    function file_to_string($filename) {
        if ((!file_exists($filename)) or (!is_readable($filename))) {
            die('Error, unable to read file to string [' . basename($filename) . '] - file not found or access denied');
        }
        $h = fopen($filename,'r');
        if (filesize($filename) > 0) {
            $content = fread($h,filesize($filename));
        } else {
            $content = '';
        }
        fclose($h);
        return $content;
    }

    function basename_uri($uri) {
        //$url = "/test/test.php?test=/test1.test#test
        // return: test.php
        $filename= $uri;
        if (strpos($filename, '?') !== false) {
            $split = explode("?", $filename);
            $filename=$split[0];
        }
        if (strpos($filename, '#') !== false) {
            $split = explode("#", $filename);
            $filename=$split[0];
        }
        $filename = basename($filename);
        return $filename;
    }
    function get_url_contents($url){
        $crl = curl_init();
        $timeout = 5;
        curl_setopt ($crl, CURLOPT_URL,$url);
        curl_setopt ($crl, CURLOPT_RETURNTRANSFER, 1);
        curl_setopt ($crl, CURLOPT_CONNECTTIMEOUT, $timeout);
        $ret = curl_exec($crl);
        curl_close($crl);
        return $ret;
	}

	function download_file($url, $output_filename) {
		try {
			set_time_limit(0);
			$fp = fopen ($output_filename, 'w+');//This is the file where we save the information
			$ch = curl_init($url);//Here is the file we are downloading
			curl_setopt($ch, CURLOPT_TIMEOUT, 50);
			curl_setopt($ch, CURLOPT_FILE, $fp);
			curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
	                // fix for SSL url
	                curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
			curl_exec($ch);
			curl_close($ch);
			fclose($fp);
		} catch (Exception $e) {
			die($e);
		}
	}

    function getMethod(){
        $method = strtoupper( $_SERVER['REQUEST_METHOD'] );
        return $method;
    }
    function strip($text) {
        if (get_magic_quotes_gpc()) {
            return stripslashes($text);
        } else {
            return $text;
        }
    }
    function postvar($v, $d='', $die=true) {
        global $_POST;
        if (isset($_POST[$v])) {
            return strip($_POST[$v]);
        } else {
            if ($die==true) {
                die('Error, missing required post variable: ' . $v);
            }
            return $d;
        }
    }
    function getvar($v, $d='', $die=true) {
        global $_GET;
        if (is_array($v)) {
            foreach ($v as $key) {
                $default='__ERROR_NOT_SET__';
                $return=getvar($key, $default, false);
                if ($return != $default) {
                    return $return;
                }
            }
            if ($die==true) {
                echo '<pre>';print_r($_GET);'</pre>';
                die('Error, missing required variable: ' . print_r($v,true));
            } else {
                return $d;
            }
        }

        if (isset($_GET[$v])) {
            return strip($_GET[$v]);
        } else {
            if ($die==true) {
                die('Error, missing required variable: ' . $v);
            }
            return $d;
        }
    }

?>