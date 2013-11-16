<?php
	
    class users{
            
        function __construct() {
       		db_connect();
        }
        function __destruct() {
	       db_close();
	    }
        function remove_user($user_id){
            $result = false;
            if ( has_permission() ){
                $sql = "UPDATE users SET active = 'D' WHERE id = ".$user_id;
                db_query($sql) or die(mysql_error());
                $result = true;
            }
            return $result;
        }
        function edit_user(){
            
        }

        function insert_user(){

        }

        function save($post){
            $fields = [];
            if ( isset($post['password']) && isset($post['password2']) ){
                $password 	= $post['password'];
                $password2 	= $post['password2'];
                if ( $password === $password2 ){
                    // password validate
                    preg_match_all('!\d+!', $password, $matches);
                    $num = '';
                    foreach($matches[0] as $item){
                        $num .= $item;
                    }
                    if ( strlen($password) >= 8 && strlen($num) >= 2 ){
                        $fields['password'] = md5($password);
                        if ( isset($post['first_name']) && isset($post['first_name']) && isset($post['email']) ){
                            $fields['first_name'] 	= $post['first_name'];
                            $fields['last_name'] 	= $post['last_name'];
                            $fields['email'] 		= $post['email'];
                            $fields['group_id'] 	= isset($post['groupd_id']) ? $post['groupd_id'] : 2;
                            $fields['active']		= isset($post['active']) ? $post['active'] : "T";
                            $sql = "SELECT count(*) FROM users WHERE email = ".db_string($fields['email']);
                            if ( db_lookup($sql) == 0 ){
                                    if ( isset($post['user_id']) && $post['user_id'] > 0 ){
                                        $fields['id'] = intval( $post['user_id'] );
                                        $fields['updated_date']		= isset($post['updated_date']) ? $post['updated_date'] : date("Y/m/d H:i:s");
                                        db_update("users", $fileds, "id = ".$user_id);
                                    }else{
                                        $fields['created_date']		= isset($post['created_date']) ? $post['created_date'] : date("Y/m/d H:i:s");
                                        db_insert("users", $fields);
                                        if ( !isset($_SESSION['user_id'])){
                                            $_SESSION['user_id'] = mysql_insert_id();
                                            $_SESSION['group_id'] = 2;
                                        }
                                        // send email to new user
                                        $this->send_email($fields);
                                    }
                            }else{
                                die_err("This email address is being used.");
                            }

                        }else{
                            die_err("Empty fields");
                        }
                    }
                }else{
                    die_err("two password not match");
                }
            }elseif( isset($post['link']) && strpos($post['link'], "facebook.com") === true ){
                    // for facebook
                    $sql = "SELECT count(*) FROM users WHERE facebook_id = ".db_string($post['id']);
                    if ( db_lookup($sql) == 0 ){
                        $fields['first_name'] 	= $post['first_name'];
                        $fields['last_name'] 	= $post['last_name'];
                        $fields['facebook_id'] 	= $post['id'];
                        $fields['email'] 		= $post['id']; // I can't get email address from facebook
                        $fields['group_id'] 	= 2;
                        $fields['active']		= "T";
                        $fields['created_date']	=  date("Y/m/d H:i:s");
                        db_insert("users", $fields);
                        if ( !isset($_SESSION['user_id'])){
                            $_SESSION['user_id'] = mysql_insert_id();
                            $_SESSION['group_id'] = 2;
                        }
                        $this->send_email($fields);
                    }else{
                        $fields['first_name'] 	= $post['first_name'];
                        $fields['last_name'] 	= $post['last_name'];
                        db_update("users", $fileds, "facebook_id = ".db_string($post['id']));
                        if ( !isset($_SESSION['user_id'])){
                            $_SESSION['user_id'] = $post['id'];
                            $_SESSION['group_id'] = 2;
                        }

                    }
            }else{
                die_err("password is emtpy");
            }
            redirect("index.php");
        }

        function send_email($fields){
            include '../src/config.php';
                require_once '../vendor/phpmailer/PHPMailerAutoload.php';
                $mail = new PHPMailer();
                $mail->isSMTP();
                $mail->Host = $config->email_host;
                $mail->Port = $config->email_port;
                $mail->SMTPSecure = 'tls';
                $mail->SMTPAuth = true;
                $mail->Username = $config->email_username;
                $mail->Password = $config->email_password;
                $mail->setFrom($config->email_frm_add, $config->email_frm_name);

                $mail->addAddress($fields['email'], $fields['first_name']." ".$fields['last_name']);
                $mail->Subject = 'Thank you for your register';
                $mail->msgHTML("Thank you for your register");
                if (!$mail->send()) {
                    die_err("Mailer Error: " . $mail->ErrorInfo);
                } else {
                    echo "Message sent!";
                }

        }
	}

?>