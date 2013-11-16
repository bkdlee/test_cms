<?php
	db_connect();
	$sql = "SELECT * FROM users ORDER BY first_name";
	$query = db_query($sql);
	db_close();
?>

<div class="container">
	<div class="row">
		<?php if ( isset($_SESSION['group_id']) && $_SESSION['group_id'] == 1 ) : ?>
			<a type="button" class="btn btn-primary" href="app.php?task=edit_user">New User</a>
		<?php endif; ?>
		<h1>User List</h1>
		<table class="table table-hover">
			<thead>
				<tr>
					<th>#</th>
					<th>First Name</th>
					<th>Last Name</th>
					<th>Email</th>
					<th>Active</th>
					<th>Facebook ID</th>
					<th>Created Date</th>
					<?php if ( $_SESSION['group_id'] == 1 ) : ?>
					<th>Action</th>
					<?php endif; ?>
				</tr>
			</thead>
			<tbody>
				<?php while( $row = db_fetch_array($query) ) :?>
				<tr>
					<td><?php echo $row['id'];?></td>
					<td><?php echo $row['first_name'];?></td>
					<td><?php echo $row['last_name'];?></td>
					<td><?php echo $row['email'];?></td>
					<td><?php echo $row['active'];?></td>
					<td><?php echo $row['facebook_id'];?></td>
					<td><?php echo $row['created_date'];?></td>
					<?php if ( isset($_SESSION['group_id']) && $_SESSION['group_id'] == 1 ) : ?>
					<td>
						<a href="app.php?task=edit_user&id=<?php echo $row['id']?>" class="glyphicon glyphicon-edit">Edit</a> | 
						<a href="app.php?task=remove_user&id=<?php echo $row['id'];?>" class="glyphicon glyphicon-remove">Remove</a>
					</td>
					<?php endif; ?>
				</tr>
				<?php endwhile; ?>
			</tbody>
		</table>
	</div>	
</div>

