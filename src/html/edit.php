<div class="container">
	<div class="row">
		<form action="app.php?task=save" method="POST" role="form" id="edit_user">
			<legend>User</legend>
			<div class="col-xs-8 col-sm-8 col-md-8 col-lg-8">
				<div class="form-group">
					<label for="">First Name</label>
					<input type="text" class="form-control required" id="first_name" placeholder="First Name" name="first_name" value="<?php echo $row['first_name'];?>">
					<label for="">Last Name</label>
					<input type="text" class="form-control required" id="last_name" placeholder="Last Name" name="last_name" value="<?php echo $row['last_name'];?>">
					<label for="">Email</label>
					<input type="text" class="form-control required email" id="email" placeholder="Email" name="email" value="<?php echo $row['email'];?>">
					<label for="">Password</label>
					<input type="password" class="form-control required" id="password" placeholder="password" name="password">
					<label for="">Confirm Password</label>
					<input type="password" class="form-control required" id="password2" placeholder="Confirm Password" name="password2">
				</div>
				<div class="form-group">
					<select name="group_id" id="input" class="form-control required">
						<option value="">-- Select Group --</option>
						<option value="1">Admin</option>
						<option value="2">user</option>
					</select>
				</div>
				<div class="checkbox">
					<label>
						<input type="checkbox" value="T" class="required" name="active">
						Active
					</label>
				</div>
				<?php if ( isset($user_id) && $user_id > 0) :?>
				<div class="form-group">
					<label for="">Facebook ID</label>
					<input type="text" disabled class="form-control" id="facebook_id" placeholder="" name="facebook_id" value="<?php echo $row['facebook_id'];?>">
				</div>
				<?php endif; ?>
				<input type="hidden" name="created_date" value="<?php echo date("Y/m/d H:i:s"); ?>" />
				<input type="hidden" name="updated_date" value="<?php echo date("Y/m/d H:i:s"); ?>" />
				<input type="hidden" name="user_id" value="<?php echo ( !empty($user_id) ? $user_id : 0 ) ?>" />

				<button type="submit" class="btn btn-primary">Submit</button>
			</div>
			
		
			
		</form>
	</div>

</div>