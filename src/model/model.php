<?php
	class model{
		function __construct() {
       		db_connect();
        }
        function __destruct() {
	       db_close();
	    }

	    function get_user_details($user_id=""){
    		if ( isset( $user_id) && $user_id > 0 && has_permission() ){
				$sql = "SELECT first_name, last_name, email, facebook_id FROM users WHERE id = ".$user_id;
				$query = db_query($sql);
				$row = db_fetch_array( $query );
	        }else{
	            $row = array(
	                "first_name"    => "",
	                "last_name"     => "",
	                "email"         => "",
	            );
	        }
	        return $row;
	    }

		function get_user_list(){
			$rows = [];
			$sql = "SELECT * FROM users WHERE active != 'D' ORDER BY first_name";
			$query = db_query($sql);
			while( $row = db_fetch_array($query) ){
				$rows[] = $row;
			}
			return $rows;
		}

	}
?>