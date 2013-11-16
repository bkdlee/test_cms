<div class="container">
	<div class="row">
		<div class="col-md-6">

		<form action="login.php" method="POST" role="form">
			<legend>Login</legend>
			<?php if ( $error_message ):?>
				<div class="alert alert-warning">
				    <button type="button" class="close" data-dismiss="alert" aria-hidden="true">&times;</button>
				    <strong>Error:</strong> <?php echo $error_message; ?>
				</div>
			<?php endif;?>
			<div class="form-group">
				<label for="">Email:</label>
				<input type="text" class="form-control" id="" placeholder="Email" name="email">
			</div>
			<div class="form-group">
				<label for="">Password:</label>
				<input type="password" class="form-control" id="" placeholder="Password" name="password">
			</div>
			<div class="form-group">
				<a href="app.php?task=add_user">Register</a> | <a href="auth/facebook">Use Facebook</a>
			</div>
				<button type="submit" class="btn btn-primary">Login</button>
		</form>
		</div>
	</div>
</div>