<?php
	
	class users{
        function __construct() {
       		db_connect();
        }
		function remove_user(){
			$result = false;
			if ( has_permission() ){
				$sql = "UPDATE users SET active = 'F' WHERE id = ".$user_id;
				if ( db_query($sql) ){
					$result = true;
				}
			}
			return $result;
		}
		function edit_user(){
			
		}

		function insert_user(){

		}
	}

?>