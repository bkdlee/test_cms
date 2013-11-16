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
							}
						}else{
							die_err("This email address is being used.");
						}

					}else{
						 die_err("Empty fields");
					}
 				}else{
 					die_err("two password not match");
 				}
			}else{
				die_err("password is emtpy");
			}

			//$this->send_email($fields);
			redirect("index.php");
		}

		function send_email($fields){
			require_once '../vendor/phpmailer/PHPMailerAutoload.php';
			$mail = new PHPMailer();
			$mail->isSMTP();
			$mail->Host = 'smtp.gmail.com';
			$mail->Port = 587;
			$mail->SMTPSecure = 'tls';
			$mail->SMTPAuth = true;
			$mail->Username = "username@gmail.com";
			$mail->Password = "yourpassword";
			$mail->setFrom('bkd.lee@gmail.com', 'Jay Lee');

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