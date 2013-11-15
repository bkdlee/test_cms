<?php

	class form{
		public $fields = array();
		public $action = "";
		public $method = "";

		function make(){
			$html = "";
			return $html;
		}

		function addField($item){
			$html = "";
			$type = $item['type'];
			switch($type){
				default:
					$html = '<input type="'.$type.'" value="'.$item['value'].'" name="'.$item['name'].'" class="'.$item['required'].'" />';
					break;
			}

			return $html;
		}

		function isValid(){
			$result = false;

			return $result;

		}



	}
?>