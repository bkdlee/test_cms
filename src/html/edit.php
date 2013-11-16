<?php
	if ( isset( $user_id) && has_permission() ){
		$sql = "SELECT * FROM users WHERE id = ".$user_id;
		$query = db_query($sql);
		$row = db_fetch_array( $query );
	}
	

?>

<div class="container">
	<div class="row">
		<form action="app.php?task=save" method="POST" role="form">
			<legend>User</legend>
			<div class="col-xs-8 col-sm-8 col-md-8 col-lg-8">
				<div class="form-group">
					<label for="">First Name</label>
					<input type="text" class="form-control required" id="" placeholder="First Name" name="first_name" value="<?php echo $row['first_name'];?>">
					<label for="">Last Name</label>
					<input type="text" class="form-control required" id="" placeholder="Last Name" name="last_name" value="<?php echo $row['last_name'];?>">
					<label for="">Email</label>
					<input type="text" class="form-control required" id="" placeholder="Email" name="email" value="<?php echo $row['email'];?>">
					<label for="">Password</label>
					<input type="password" class="form-control required" id="" placeholder="password" name="password">
					<label for="">Confirm Password</label>
					<input type="password" class="form-control required" id="" placeholder="Confirm Password" name="password2">
				</div>
				<div class="form-group">
					<select name="" id="input" class="form-control required">
						<option value="">-- Select Group --</option>

						<option value="1">Admin</option>
						<option value="2">user</option>
					</select>
				</div>
				<div class="checkbox">
					<label>
						<input type="checkbox" value="T" class="required">
						Active
					</label>
				</div>
				<input type="hidden" name="created_date" value="<?php echo date("Y/m/d H:i:s"); ?>" />
				<input type="hidden" name="updated_date" value="<?php echo date("Y/m/d H:i:s"); ?>" />
				<input type="hidden" name="user_id" value="<?php echo ( !empty($user_id) ? $user_id : 0 ) ?>" />

				<button type="submit" class="btn btn-primary">Submit</button>
			</div>
			
		
			
		</form>
	</div>

</div>